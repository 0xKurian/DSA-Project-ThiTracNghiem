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

static const int CONFIGS_PER_PAGE = 8;

struct ConfigRow {
    Class* classroom;
    ExamConfig* config;
};

static bool configMatches(GuiState& g, const ConfigRow& row, const char* query) {
    if (query == nullptr || query[0] == '\0') return true;
    if (GuiMatchText(row.classroom->id, query)) return true;
    if (GuiMatchText(row.config->subjectId, query)) return true;
    Subject* sub = findSubject(g.root, row.config->subjectId);
    return sub != nullptr && GuiMatchText(sub->name, query);
}

static int collectConfigRows(GuiState& g, Arr<ConfigRow>& out) {
    for (int i = 0; i < g.classCount; ++i) {
        if (g.classes[i] == nullptr) continue;
        for (ExamConfig* cfg = g.classes[i]->configList; cfg != nullptr; cfg = cfg->next) {
            ConfigRow row { g.classes[i], cfg };
            if (configMatches(g, row, g.searchConfig)) out.push_back(row);
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
    Arr<ConfigRow> rows;
    int total = collectConfigRows(g, rows);

    size_t searchLenBefore = strlen(g.searchConfig);
    GuiTextBoxInput({ pane.x + 15, pane.y + 8, pane.width - 30, 34 }, g.searchConfig, sizeof(g.searchConfig),
                    g.activeSearchConfig, "Tim lop / ma mon / ten mon...");
    if (strlen(g.searchConfig) != searchLenBefore) {
        g.configSelected = -1;
        g.configPage = 0;
        g.configPreviewPage = 0;
    }
    DrawTxt(TextFormat("DANH SACH CAU HINH (%d)", total), pane.x + 15, pane.y + 50, 15, COLOR_TEXT_DARK, true);

    int totalPages = (total <= 0) ? 1 : (total + CONFIGS_PER_PAGE - 1) / CONFIGS_PER_PAGE;
    if (g.configPage >= totalPages) g.configPage = totalPages - 1;
    if (g.configPage < 0) g.configPage = 0;
    if (g.configSelected >= total) g.configSelected = -1;

    int listTop = (int)pane.y + 75;
    if (total == 0) {
        DrawTxt("Chua co cau hinh nao.", pane.x + 15, (float)listTop + 15, 15, COLOR_TEXT_MUTED);
        DrawTxt("Bam '+ Tao Cau Hinh' de tao cau hinh lop - mon.", pane.x + 15, (float)listTop + 42, 15, COLOR_TEXT_MUTED);
        return;
    }

    int start = g.configPage * CONFIGS_PER_PAGE;
    int end = std::min(start + CONFIGS_PER_PAGE, total);
    for (int i = start; i < end; ++i) {
        ConfigRow& rowRef = rows[i];
        int rowY = listTop + (i - start) * 52;
        Rectangle row = { pane.x + 10, (float)rowY, pane.width - 20, 48 };
        bool selected = (g.configSelected == i);
        DrawRectangleRounded(row, 0.1f, 4, selected ? MakeColor(219, 234, 254) : COLOR_ROW_ALT);
        DrawRectangleRoundedLines(row, 0.1f, 4, selected ? COLOR_PRIMARY : COLOR_BORDER);

        Subject* sub = findSubject(g.root, rowRef.config->subjectId);
        bool frozen = classHasTakenSubject(g.classes, g.classCount, rowRef.classroom->id, rowRef.config->subjectId);
        DrawTxt(TextFormat("%s - %s", rowRef.classroom->id, sub != nullptr ? sub->name : rowRef.config->subjectId),
                row.x + 12, row.y + 8, 15, COLOR_TEXT_DARK, true);
        DrawTxt(TextFormat("%s | %d cau / %d phut", rowRef.config->subjectId, rowRef.config->questionCount, rowRef.config->minutes),
                row.x + 12, row.y + 27, 13, COLOR_TEXT_MUTED);

        Rectangle xoaRec = { row.x + row.width - 62, row.y + 10, 50, 28 };
        Vector2 mouse = GetMousePosition();
        bool rowPressed = !GuiModalAbsorbsInput() && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)
                          && CheckCollisionPointRec(mouse, row);

        if (frozen) {
            DrawTxt("Khoa", row.x + row.width - 95, row.y + 17, 13, COLOR_WARNING, true);
        } else if (GuiBtnOutline(xoaRec, "Xoa", COLOR_DANGER, COLOR_DANGER, 13)) {
            openDeleteConfigModal(g, rowRef.classroom, rowRef.config);
        }

        if (rowPressed && (frozen || !CheckCollisionPointRec(mouse, xoaRec))) {
            g.configSelected = i;
            g.configPreviewPage = 0;
        }
    }

    int pageBarY = (int)(pane.y + pane.height - 40);
    DrawTxt(TextFormat("Trang %d / %d", g.configPage + 1, totalPages), pane.x + 15, (float)pageBarY + 6, 14, COLOR_TEXT_MUTED);
    if (GuiBtnOutline({ pane.x + pane.width - 230, (float)pageBarY, 100, 28 }, "< Truoc", COLOR_PRIMARY, COLOR_PRIMARY, 13, false, g.configPage == 0)) {
        if (g.configPage > 0) g.configPage--;
    }
    if (GuiBtnOutline({ pane.x + pane.width - 120, (float)pageBarY, 100, 28 }, "Sau >", COLOR_PRIMARY, COLOR_PRIMARY, 13, false, g.configPage >= totalPages - 1)) {
        if (g.configPage < totalPages - 1) g.configPage++;
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
    DrawTxt(TextFormat("Lop: %s - %s | Mon: %s - %s", cls->id, cls->name,
                       cfg->subjectId, sub != nullptr ? sub->name : "?"),
            pane.x + 20, pane.y + 50, 14, COLOR_TEXT_MUTED);
    DrawTxt(TextFormat("%d cau / %d phut | Ngan hang: %d cau | Tao luc: %s", cfg->questionCount, cfg->minutes,
                       sub != nullptr ? countQuestions(sub->questionList) : 0, cfg->createdAt),
            pane.x + 20, pane.y + 70, 14, COLOR_SUCCESS, true);

    if (frozen) {
        DrawTxt("Da co sinh vien thi mon nay - cau hinh bi khoa.", pane.x + 20, pane.y + 92, 13, COLOR_WARNING, true);
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

    int totalPages = (cfg->questionCount + QUESTIONS_PER_PAGE - 1) / QUESTIONS_PER_PAGE;
    if (totalPages < 1) totalPages = 1;
    if (g.configPreviewPage >= totalPages) g.configPreviewPage = totalPages - 1;
    if (g.configPreviewPage < 0) g.configPreviewPage = 0;

    int start = g.configPreviewPage * QUESTIONS_PER_PAGE;
    int end = std::min(start + QUESTIONS_PER_PAGE, cfg->questionCount);
    int top = (int)pane.y + 120;
    for (int k = start; k < end; ++k) {
        Rectangle card = { pane.x + 15, (float)(top + (k - start) * 150), pane.width - 30, 140 };
        DrawCardPanel(card, COLOR_ROW_ALT);
        DrawTxt(TextFormat("Cau %d:", k + 1), card.x + 12, card.y + 10, 15, COLOR_PRIMARY, true);

        Question* q = findQuestion(sub->questionList, cfg->questionIds[k]);
        if (q == nullptr) {
            DrawTxt("Cau hoi nay da bi xoa khoi ngan hang.", card.x + 75, card.y + 10, 14, COLOR_DANGER, true);
            continue;
        }

        DrawTxtWrapped(q->content, card.x + 70, card.y + 8, card.width - 85, 13, COLOR_TEXT_DARK);
        bool isA = (q->correctAnswer == 'A');
        bool isB = (q->correctAnswer == 'B');
        bool isC = (q->correctAnswer == 'C');
        bool isD = (q->correctAnswer == 'D');
        DrawTxt(TextFormat("A. %s", q->answerA), card.x + 20, card.y + 52, 13, isA ? COLOR_SUCCESS : COLOR_TEXT_MUTED, isA);
        DrawTxt(TextFormat("B. %s", q->answerB), card.x + 20, card.y + 68, 13, isB ? COLOR_SUCCESS : COLOR_TEXT_MUTED, isB);
        DrawTxt(TextFormat("C. %s", q->answerC), card.x + 20, card.y + 84, 13, isC ? COLOR_SUCCESS : COLOR_TEXT_MUTED, isC);
        DrawTxt(TextFormat("D. %s", q->answerD), card.x + 20, card.y + 100, 13, isD ? COLOR_SUCCESS : COLOR_TEXT_MUTED, isD);
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

    drawConfigList(g, leftPane);
    drawConfigPreview(g, rightPane);
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
