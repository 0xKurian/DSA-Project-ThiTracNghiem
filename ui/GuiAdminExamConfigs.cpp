#include "GuiCommon.h"
#include "GuiState.h"

#include "../business/BusinessErrors.h"
#include "../business/ExamConfigService.h"

#include "../core/ClassArray.h"
#include "../core/ExamConfigList.h"
#include "../core/QuestionList.h"
#include "../core/SubjectBST.h"

#include <algorithm>
#include <cstdio>
#include <cstring>

static const int PREVIEW_PER_PAGE = 2;

struct ConfigRow {
    Class* classroom;
    ExamConfig* config;
};

static int collectConfigRows(GuiState& g, Arr<ConfigRow>& out) {
    size_t clsLen = strlen(g.configFilterClass);
    for (int i = 0; i < g.classCount; ++i) {
        if (g.classes[i] == nullptr) continue;
        if (clsLen > 0 && strncmp(g.classes[i]->id, g.configFilterClass, clsLen) != 0) continue;
        for (ExamConfig* cfg = g.classes[i]->configList; cfg != nullptr; cfg = cfg->next) {
            if (g.configFilterSubject[0] != '\0'
                && strncmp(cfg->subjectId, g.configFilterSubject, strlen(g.configFilterSubject)) != 0) continue;
            ConfigRow row { g.classes[i], cfg };
            out.push_back(row);
        }
    }
    return (int)out.size();
}

static void openEditModal(GuiState& g, const Class* classroom, const ExamConfig* cfg) {
    g.currentModal = MODAL_CLASS_EXAM_CONFIG;
    const char* cls = (classroom != nullptr) ? classroom->id : "";
    snprintf(g.configClassId, sizeof(g.configClassId), "%s", cls);
    snprintf(g.configEditClass, sizeof(g.configEditClass), "%s", cls);
    snprintf(g.configEditSubject, sizeof(g.configEditSubject), "%s", cfg != nullptr ? cfg->subjectId : "");
    snprintf(g.formExamQ, sizeof(g.formExamQ), "%d", cfg != nullptr ? cfg->questionCount : 0);
    snprintf(g.formExamMinutes, sizeof(g.formExamMinutes), "%d", cfg != nullptr ? cfg->minutes : 0);
    g.configSubjectId[0] = '\0';
    if (cfg == nullptr) {
        g.formExamQ[0] = '\0';
        g.formExamMinutes[0] = '\0';
    }
    g.activeConfigClassSearch = false;
    g.activeConfigSubjectSearch = false;
    g.activeConfigFilterClass = false;
    g.activeConfigFilterSubject = false;
    g.modalError[0] = '\0';
    g.activeFormBox = (cfg == nullptr) ? 1 : 3;
}

static void openDeleteConfigModal(GuiState& g, const Class* classroom, const ExamConfig* cfg) {
    g.currentModal = MODAL_DELETE_CLASS_EXAM_CONFIG;
    snprintf(g.configEditClass, sizeof(g.configEditClass), "%s", classroom->id);
    snprintf(g.configEditSubject, sizeof(g.configEditSubject), "%s", cfg->subjectId);
    g.modalError[0] = '\0';
}

static void drawConfigList(GuiState& g, Rectangle pane) {
    if (g.configFilterClass[0] != '\0' && findClass(g.classes, g.classCount, g.configFilterClass) == nullptr)
        g.configFilterClass[0] = '\0';
    if (g.configFilterSubject[0] != '\0' && findSubject(g.root, g.configFilterSubject) == nullptr)
        g.configFilterSubject[0] = '\0';

    Arr<ConfigRow> rows;
    int total = collectConfigRows(g, rows);
    DrawTxt(TextFormat("DANH SACH CAU HINH (%d)", total), pane.x + 15, pane.y + 50, 15, COLOR_TEXT_DARK, true);

    int totalPages = (total <= 0) ? 1 : (total + SCORES_PER_PAGE - 1) / SCORES_PER_PAGE;
    if (g.configPage >= totalPages) g.configPage = totalPages - 1;
    if (g.configPage < 0) g.configPage = 0;
    if (g.configSelected >= total) g.configSelected = -1;

    int listTop = (int)pane.y + 75;
    if (total == 0) {
        DrawTxt("Chua co cau hinh nao.", pane.x + 15, (float)listTop + 15, 15, COLOR_TEXT_MUTED);
        DrawTxt("Bam '+ Tao Cau Hinh' de tao cau hinh lop - mon.", pane.x + 15, (float)listTop + 42, 15, COLOR_TEXT_MUTED);
    }

    int start = g.configPage * SCORES_PER_PAGE;
    int end = std::min(start + SCORES_PER_PAGE, total);
    for (int i = start; i < end; ++i) {
        ConfigRow& rowRef = rows[i];
        int rowY = listTop + (i - start) * 52;
        Rectangle row = { pane.x + 10, (float)rowY, pane.width - 20, 48 };
        bool selected = (g.configSelected == i);
        bool frozen = classHasTakenSubject(g.classes, g.classCount, rowRef.classroom->id, rowRef.config->subjectId);
        DrawRectangleRounded(row, 0.1f, 4, selected ? MakeColor(219, 234, 254) : (frozen ? MakeColor(254, 226, 226) : COLOR_ROW_ALT));
        DrawRectangleRoundedLines(row, 0.1f, 4, selected ? COLOR_PRIMARY : (frozen ? COLOR_WARNING : COLOR_BORDER));

        Subject* sub = findSubject(g.root, rowRef.config->subjectId);
        DrawTxt(TextFormat("%s - %s", rowRef.classroom->id, sub != nullptr ? sub->name : rowRef.config->subjectId),
                row.x + 12, row.y + 8, 15, COLOR_TEXT_DARK, true);
        DrawTxt(TextFormat("%s | %d cau / %d phut", rowRef.config->subjectId, rowRef.config->questionCount, rowRef.config->minutes),
                row.x + 12, row.y + 27, 13, COLOR_TEXT_MUTED);

        Rectangle xoaRec = { row.x + row.width - 62, row.y + 10, 50, 28 };
        Vector2 mouse = GetMousePosition();
        bool rowPressed = !GuiModalAbsorbsInput() && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)
                          && CheckCollisionPointRec(mouse, row);

        if (!frozen && GuiBtnOutline(xoaRec, "Xoa", COLOR_DANGER, COLOR_DANGER, 13)) {
            openDeleteConfigModal(g, rowRef.classroom, rowRef.config);
        }

        if (rowPressed && (frozen || !CheckCollisionPointRec(mouse, xoaRec))) {
            g.configSelected = i;
            g.configPreviewPage = 0;
        }
    }

    int pageBarY = (int)(pane.y + pane.height - 40);
    if (total > 0) {
        DrawTxt(TextFormat("Trang %d / %d", g.configPage + 1, totalPages), pane.x + 15, (float)pageBarY + 6, 14, COLOR_TEXT_MUTED);
        if (GuiBtnOutline({ pane.x + pane.width - 230, (float)pageBarY, 100, 28 }, "< Truoc", COLOR_PRIMARY, COLOR_PRIMARY, 13, false, g.configPage == 0)) {
            if (g.configPage > 0) g.configPage--;
        }
        if (GuiBtnOutline({ pane.x + pane.width - 120, (float)pageBarY, 100, 28 }, "Sau >", COLOR_PRIMARY, COLOR_PRIMARY, 13, false, g.configPage >= totalPages - 1)) {
            if (g.configPage < totalPages - 1) g.configPage++;
        }
    }

    Arr<SuggestItem> classItems;
    GuiMakeClassItems(g.classes, g.classCount, classItems);
    int pickedClass = GuiSearchSuggest({ pane.x + 15, pane.y + 8, 190, 34 }, g.configFilterClass, sizeof(g.configFilterClass),
                                       g.activeConfigFilterClass, classItems, "Loc theo lop...");
    if (pickedClass >= 0) {
        g.configPage = 0;
        g.configSelected = -1;
        g.configPreviewPage = 0;
    }

    Arr<SuggestItem> subjItems;
    GuiMakeSubjectItems(g.subjects, true, subjItems);
    int pickedSub = GuiSearchSuggest({ pane.x + 215, pane.y + 8, 190, 34 }, g.configFilterSubject, sizeof(g.configFilterSubject),
                                     g.activeConfigFilterSubject, subjItems, "Loc theo mon...");
    if (pickedSub >= 0) {
        g.configPage = 0;
        g.configSelected = -1;
        g.configPreviewPage = 0;
    }

    if (g.configFilterClass[0] != '\0'
        && GuiBtnOutline({ pane.x + 182, pane.y + 11, 20, 28 }, "X", COLOR_BORDER, COLOR_TEXT_MUTED, 12)) {
        g.configFilterClass[0] = '\0';
        g.activeConfigFilterClass = false;
        g.configPage = 0;
        g.configSelected = -1;
    }
    if (g.configFilterSubject[0] != '\0'
        && GuiBtnOutline({ pane.x + 380, pane.y + 11, 20, 28 }, "X", COLOR_BORDER, COLOR_TEXT_MUTED, 12)) {
        g.configFilterSubject[0] = '\0';
        g.activeConfigFilterSubject = false;
        g.configPage = 0;
        g.configSelected = -1;
    }
}

static void drawConfigPreview(GuiState& g, Rectangle pane) {
    Arr<ConfigRow> rows;
    int total = collectConfigRows(g, rows);
    if (g.configSelected < 0 || g.configSelected >= total) {
        DrawTxt("Chon mot cau hinh ben trai de xem de da boc.", pane.x + 20, pane.y + 20, 16, COLOR_TEXT_MUTED);
        return;
    }

    Class* cls = rows[g.configSelected].classroom;
    ExamConfig* cfg = rows[g.configSelected].config;
    Subject* sub = findSubject(g.root, cfg->subjectId);
    bool frozen = classHasTakenSubject(g.classes, g.classCount, cls->id, cfg->subjectId);

    DrawTxt("DE THI DA BOC", pane.x + 20, pane.y + 12, 17, COLOR_TEXT_DARK, true);
    DrawTxtClipped(TextFormat("Lop: %s - %s", cls->id, cls->name),
                   pane.x + 20, pane.y + 46, 14, COLOR_TEXT_MUTED, false, pane.width - 60);
    DrawTxtClipped(TextFormat("Mon: %s - %s", cfg->subjectId, sub != nullptr ? sub->name : "?"),
                   pane.x + 20, pane.y + 62, 14, COLOR_TEXT_MUTED, false, pane.width - 60);
    char rightStat[48];
    snprintf(rightStat, sizeof(rightStat), "Tao luc: %s", cfg->createdAt);
    float rightW = MeasureTxt(rightStat, 13, true);
    float leftX = pane.x + 20;
    float rightX = pane.x + pane.width - 20 - rightW;
    DrawTxtClipped(TextFormat("%d cau / %d phut | Ngan hang: %d cau", cfg->questionCount, cfg->minutes,
                              sub != nullptr ? countQuestions(sub->questionList) : 0),
                   leftX, pane.y + 78, 13, COLOR_SUCCESS, true, rightX - leftX - 12);
    DrawTxtClipped(rightStat, rightX, pane.y + 78, 13, COLOR_SUCCESS, true, rightW + 1);

    if (frozen) {
        DrawTxtClipped("Da co sinh vien thi mon nay - cau hinh bi khoa.", pane.x + 20, pane.y + 94, 13, COLOR_WARNING, true, pane.width - 60);
    } else {
        if (GuiBtnOutline({ pane.x + pane.width - 210, pane.y + 10, 90, 30 }, "Sua", COLOR_PRIMARY, COLOR_PRIMARY, 14)) {
            openEditModal(g, cls, cfg);
        }
        if (GuiBtnOutline({ pane.x + pane.width - 110, pane.y + 10, 90, 30 }, "Xoa", COLOR_DANGER, COLOR_DANGER, 14)) {
            openDeleteConfigModal(g, cls, cfg);
        }
    }

    if (sub == nullptr) {
        DrawTxt("Mon hoc da bi xoa khoi he thong.", pane.x + 20, pane.y + 115, 15, COLOR_DANGER, true);
        return;
    }

    int totalPages = (cfg->questionCount + PREVIEW_PER_PAGE - 1) / PREVIEW_PER_PAGE;
    if (totalPages < 1) totalPages = 1;
    if (g.configPreviewPage >= totalPages) g.configPreviewPage = totalPages - 1;
    if (g.configPreviewPage < 0) g.configPreviewPage = 0;

    int start = g.configPreviewPage * PREVIEW_PER_PAGE;
    int end = std::min(start + PREVIEW_PER_PAGE, cfg->questionCount);
    int top = (int)pane.y + 120;
    for (int k = start; k < end; ++k) {
        Rectangle card = { pane.x + 15, (float)(top + (k - start) * 150), pane.width - 30, 140 };
        DrawCardPanel(card, COLOR_ROW_ALT);
        char label[32];
        snprintf(label, sizeof(label), "Cau %d:", k + 1);
        DrawTxt(label, card.x + 12, card.y + 10, 15, COLOR_PRIMARY, true);

        Question* q = findQuestion(sub->questionList, cfg->questionIds[k]);
        if (q == nullptr) {
            DrawTxt("Cau hoi nay da bi xoa khoi ngan hang.", card.x + 75, card.y + 10, 14, COLOR_DANGER, true);
            continue;
        }

        float qx = std::max(card.x + 70.0f, card.x + 12 + MeasureTxt(label, 15, true) + 8);
        DrawTxtWrapped(q->content, qx, card.y + 8, card.x + card.width - 15 - qx, 13, COLOR_TEXT_DARK);
        bool isA = (q->correctAnswer == 'A');
        bool isB = (q->correctAnswer == 'B');
        bool isC = (q->correctAnswer == 'C');
        bool isD = (q->correctAnswer == 'D');
        DrawTxtClipped(TextFormat("A. %s", q->answerA), card.x + 20, card.y + 52, 13, isA ? COLOR_SUCCESS : COLOR_TEXT_MUTED, isA, card.width - 44);
        DrawTxtClipped(TextFormat("B. %s", q->answerB), card.x + 20, card.y + 68, 13, isB ? COLOR_SUCCESS : COLOR_TEXT_MUTED, isB, card.width - 44);
        DrawTxtClipped(TextFormat("C. %s", q->answerC), card.x + 20, card.y + 84, 13, isC ? COLOR_SUCCESS : COLOR_TEXT_MUTED, isC, card.width - 44);
        DrawTxtClipped(TextFormat("D. %s", q->answerD), card.x + 20, card.y + 100, 13, isD ? COLOR_SUCCESS : COLOR_TEXT_MUTED, isD, card.width - 44);
        DrawTxt(TextFormat("Dap an dung: %c", q->correctAnswer), card.x + 20, card.y + 118, 13, COLOR_SUCCESS, true);
    }

    int pageBarY = (int)(pane.y + pane.height - 40);
    DrawTxt(TextFormat("Trang %d / %d (Tong: %d cau)", g.configPreviewPage + 1, totalPages, cfg->questionCount),
            pane.x + 20, (float)pageBarY + 6, 14, COLOR_TEXT_MUTED);
    if (GuiBtnOutline({ pane.x + pane.width - 230, (float)pageBarY, 100, 28 }, "< Truoc", COLOR_PRIMARY, COLOR_PRIMARY, 13, false, g.configPreviewPage == 0)) {
        if (g.configPreviewPage > 0) g.configPreviewPage--;
    }
    if (GuiBtnOutline({ pane.x + pane.width - 120, (float)pageBarY, 100, 28 }, "Sau >", COLOR_PRIMARY, COLOR_PRIMARY, 13, false, g.configPreviewPage >= totalPages - 1)) {
        if (g.configPreviewPage < totalPages - 1) g.configPreviewPage++;
    }
}

void drawAdminTabExamConfigs(GuiState& g) {
    Rectangle contentRec = { 260, 85, (float)(GUI_WIDTH - 280), (float)(GUI_HEIGHT - 105) };
    DrawTxt("CAU HINH THI THEO LOP & MON", contentRec.x + 25, contentRec.y + 20, 20, COLOR_TEXT_DARK, true);
    if (GuiBtn({ contentRec.x + contentRec.width - 180, contentRec.y + 18, 155, 38 }, "+ Tao Cau Hinh", COLOR_SUCCESS, COLOR_SUCCESS_HOVER, WHITE, 15, true)) {
        openEditModal(g, nullptr, nullptr);
    }

    Rectangle leftPane = { contentRec.x + 20, contentRec.y + 70, 440, contentRec.height - 90 };
    Rectangle rightPane = { contentRec.x + 475, contentRec.y + 70, contentRec.width - 495, contentRec.height - 90 };
    DrawCardPanel(leftPane);
    DrawCardPanel(rightPane);

    drawConfigPreview(g, rightPane);
    drawConfigList(g, leftPane);
}

void drawClassExamConfigModal(GuiState& g) {
    Rectangle rec = { (float)(GUI_WIDTH / 2 - 280), (float)(GUI_HEIGHT / 2 - 240), 560, 480 };
    DrawCardPanel(rec);
    bool editing = (g.configEditSubject[0] != '\0');
    DrawTxt(editing ? "SUA CAU HINH THI (LOP - MON)" : "TAO CAU HINH THI (LOP - MON)",
            rec.x + 30, rec.y + 22, 19, COLOR_TEXT_DARK, true);

    DrawTxt("Ma lop:", rec.x + 30, rec.y + 60, 14, COLOR_TEXT_DARK);
    if (editing) {
        DrawTxt(g.configEditClass, rec.x + 30, rec.y + 84, 16, COLOR_TEXT_DARK, true);
    }

    DrawTxt("Ma mon:", rec.x + 290, rec.y + 60, 14, COLOR_TEXT_DARK);
    if (editing) {
        DrawTxt(g.configEditSubject, rec.x + 290, rec.y + 84, 16, COLOR_TEXT_DARK, true);
    }

    DrawTxt("So cau hoi thi:", rec.x + 30, rec.y + 140, 14, COLOR_TEXT_DARK);
    bool actQ = (g.activeFormBox == 3);
    GuiTextBoxInput({ rec.x + 30, rec.y + 162, 230, 36 }, g.formExamQ, sizeof(g.formExamQ), actQ, "VD: 10");
    GuiLeaveOnlyDigits(g.formExamQ, sizeof(g.formExamQ));
    if (actQ) g.activeFormBox = 3;

    DrawTxt("Thoi gian (phut):", rec.x + 290, rec.y + 140, 14, COLOR_TEXT_DARK);
    bool actM = (g.activeFormBox == 4);
    GuiTextBoxInput({ rec.x + 290, rec.y + 162, 230, 36 }, g.formExamMinutes, sizeof(g.formExamMinutes), actM, "VD: 15");
    GuiLeaveOnlyDigits(g.formExamMinutes, sizeof(g.formExamMinutes));
    if (actM) g.activeFormBox = 4;

    DrawTxt("De duoc boc mot lan; moi sinh vien trong lop thi cung bo cau nay.",
            rec.x + 30, rec.y + 218, 14, COLOR_TEXT_MUTED);
    if (editing) {
        DrawTxt("Giu nguyen so cau de giu bo de; doi so cau se boc de moi.", rec.x + 30, rec.y + 240, 14, COLOR_TEXT_MUTED);
    }

    if (g.modalError[0] != '\0') {
        DrawTxt(g.modalError, rec.x + 30, rec.y + 268, 14, COLOR_DANGER, true);
    }

    bool submit = GuiBtn({ rec.x + 30, rec.y + 330, 200, 42 }, "Luu Cau Hinh", COLOR_SUCCESS, COLOR_SUCCESS_HOVER, WHITE, 16, true);
    if (IsKeyPressed(KEY_ENTER)) submit = true;
    if (submit) {
        const char* classId = editing ? g.configEditClass : g.configClassId;
        const char* subjectId = editing ? g.configEditSubject : g.configSubjectId;
        BusinessResult res = setClassExamConfigWorkflow(
            g.root, g.classes, g.classCount, classId, subjectId,
            atoi(g.formExamQ), atoi(g.formExamMinutes));
        if (res == SUCCESS) {
            g.currentModal = MODAL_NONE;
            g.showToast(editing ? "Da cap nhat cau hinh thi!" : "Da tao va boc de thi cho lop!", 3.0f, false);
        } else if (res == ERR_INVALID_FORMAT) {
            Subject* sub = findSubject(g.root, subjectId);
            int bank = (sub != nullptr) ? countQuestions(sub->questionList) : 0;
            snprintf(g.modalError, sizeof(g.modalError),
                     "So cau phai tu 1 den %d, thoi gian tu 1 den 180 phut.", bank);
        } else if (res == ERR_NOT_FOUND) {
            snprintf(g.modalError, sizeof(g.modalError), "Ma lop hoac ma mon khong ton tai.");
        } else if (res == ERR_CONSTRAINT_VIOLATION) {
            snprintf(g.modalError, sizeof(g.modalError), "Lop da co sinh vien thi mon nay, khong sua duoc.");
        } else {
            snprintf(g.modalError, sizeof(g.modalError), "%s", getErrorMessage(res));
        }
    }
    if (GuiBtnOutline({ rec.x + 270, rec.y + 330, 200, 42 }, "Huy bo", COLOR_PRIMARY, COLOR_PRIMARY, 16, true)) {
        g.currentModal = MODAL_NONE;
    }

    // Goi y lop/mon ve cuoi de panel dropdown nam tren cac o nhap phia duoi.
    if (!editing) {
        Arr<SuggestItem> classItems;
        GuiMakeClassItems(g.classes, g.classCount, classItems);
        GuiSearchSuggest({ rec.x + 30, rec.y + 82, 230, 36 }, g.configClassId, sizeof(g.configClassId),
                         g.activeConfigClassSearch, classItems, "Tim ma lop...");

        Arr<SuggestItem> subjectItems;
        GuiMakeSubjectItems(g.subjects, true, subjectItems);
        GuiSearchSuggest({ rec.x + 290, rec.y + 82, 230, 36 }, g.configSubjectId, sizeof(g.configSubjectId),
                         g.activeConfigSubjectSearch, subjectItems, "Tim ma mon...");

        if (g.activeConfigClassSearch) g.activeFormBox = 1;
        else if (g.activeConfigSubjectSearch) g.activeFormBox = 2;
    }
}

void drawDeleteClassExamConfigModal(GuiState& g) {
    Rectangle rec = { (float)(GUI_WIDTH / 2 - 250), (float)(GUI_HEIGHT / 2 - 170), 500, 340 };
    DrawCardPanel(rec);
    DrawTxt("XOA CAU HINH THI", rec.x + 30, rec.y + 28, 20, COLOR_DANGER, true);

    if (g.configEditSubject[0] == '\0') {
        g.currentModal = MODAL_NONE;
        return;
    }

    DrawTxt(TextFormat("Xoa cau hinh lop %s - mon %s?", g.configEditClass, g.configEditSubject), rec.x + 30, rec.y + 80, 16, COLOR_TEXT_DARK);
    DrawTxt("Sinh vien chua thi se khong thi duoc mon nay cho toi khi", rec.x + 30, rec.y + 115, 14, COLOR_TEXT_MUTED);
    DrawTxt("giang vien tao lai cau hinh.", rec.x + 30, rec.y + 137, 14, COLOR_TEXT_MUTED);

    if (g.modalError[0] != '\0') {
        DrawTxt(g.modalError, rec.x + 30, rec.y + 168, 14, COLOR_DANGER, true);
    }

    if (GuiBtn({ rec.x + 30, rec.y + 210, 200, 42 }, "Xac Nhan Xoa", COLOR_DANGER, COLOR_DANGER_HOVER, WHITE, 16, true)) {
        BusinessResult res = deleteClassExamConfigWorkflow(g.classes, g.classCount, g.configEditClass, g.configEditSubject);
        if (res == SUCCESS) {
            g.configSelected = -1;
            g.currentModal = MODAL_NONE;
            g.showToast(TextFormat("Da xoa cau hinh lop %s - mon %s.", g.configEditClass, g.configEditSubject), 3.0f, false);
        } else if (res == ERR_CONSTRAINT_VIOLATION) {
            snprintf(g.modalError, sizeof(g.modalError), "Lop da co sinh vien thi mon nay, khong xoa duoc.");
        } else {
            snprintf(g.modalError, sizeof(g.modalError), "%s", getErrorMessage(res));
        }
    }
    if (GuiBtnOutline({ rec.x + 270, rec.y + 210, 200, 42 }, "Huy bo", COLOR_PRIMARY, COLOR_PRIMARY, 16, true)) {
        g.currentModal = MODAL_NONE;
    }
}
