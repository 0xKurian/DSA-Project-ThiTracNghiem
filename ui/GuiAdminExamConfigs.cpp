#include "GuiCommon.h"
#include "GuiState.h"

#include "../core/ClassArray.h"
#include "../core/QuestionList.h"
#include "../core/StringUtils.h"
#include "../core/SubjectBST.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

static const int CONFIGS_PER_PAGE = 8;

static void normalizeCode(char* s) {
    int w = 0;
    for (int i = 0; s[i] != '\0'; ++i) {
        unsigned char c = (unsigned char)s[i];
        if (c == ' ' || c == '\t') continue;
        s[w++] = (char)toupper(c);
    }
    s[w] = '\0';
}

int GuiFindExamConfigIndex(const GuiState& g, const char* classId, const char* subjectId) {
    if (classId == nullptr || subjectId == nullptr) return -1;
    for (size_t i = 0; i < g.examConfigs.size(); ++i) {
        if (strcmp(g.examConfigs[i].classId, classId) == 0 &&
            strcmp(g.examConfigs[i].subjectId, subjectId) == 0) {
            return (int)i;
        }
    }
    return -1;
}

static bool drawDeck(Subject* sub, int n, int ids[]) {
    Arr<Question*> pool;
    collectQuestions(sub->questionList, pool);
    if (n <= 0 || (int)pool.size() < n) return false;

    static bool seeded = false;
    if (!seeded) {
        srand((unsigned)time(nullptr));
        seeded = true;
    }

    for (int i = (int)pool.size() - 1; i > 0; --i) {
        int j = rand() % (i + 1);
        Question* tmp = pool[i];
        pool[i] = pool[j];
        pool[j] = tmp;
    }
    for (int i = 0; i < n; ++i) ids[i] = pool[i]->id;
    return true;
}

void GuiFreeExamConfigs(GuiState& g) {
    for (size_t i = 0; i < g.examConfigs.size(); ++i) {
        delete[] g.examConfigs[i].questionIds;
    }
    g.examConfigs.clear();
    g.configSelected = -1;
}

void GuiRemoveExamConfigs(GuiState& g, const char* classId, const char* subjectId) {
    bool removed = false;
    size_t i = 0;
    while (i < g.examConfigs.size()) {
        bool match = (classId != nullptr && strcmp(g.examConfigs[i].classId, classId) == 0)
                  || (subjectId != nullptr && strcmp(g.examConfigs[i].subjectId, subjectId) == 0);
        if (match) {
            int* ids = g.examConfigs[i].questionIds;
            g.examConfigs.erase_at(i);
            delete[] ids;
            removed = true;
        } else {
            ++i;
        }
    }
    if (removed) {
        g.configSelected = -1;
        g.configEditIndex = -1;
    }
}

static void drawConfigList(GuiState& g, Rectangle pane) {
    DrawTxt(TextFormat("DANH SACH CAU HINH (%d)", (int)g.examConfigs.size()), pane.x + 15, pane.y + 12, 16, COLOR_TEXT_DARK, true);

    int totalPages = g.examConfigs.empty() ? 1 : ((int)g.examConfigs.size() + CONFIGS_PER_PAGE - 1) / CONFIGS_PER_PAGE;
    if (g.configPage >= totalPages) g.configPage = totalPages - 1;
    if (g.configPage < 0) g.configPage = 0;

    int listTop = (int)pane.y + 45;
    if (g.examConfigs.empty()) {
        DrawTxt("Chua co cau hinh nao.", pane.x + 15, (float)listTop + 15, 15, COLOR_TEXT_MUTED);
        DrawTxt("Bam '+ Tao Cau Hinh' de tao cau hinh lop - mon.", pane.x + 15, (float)listTop + 42, 15, COLOR_TEXT_MUTED);
        return;
    }

    int start = g.configPage * CONFIGS_PER_PAGE;
    int end = std::min(start + CONFIGS_PER_PAGE, (int)g.examConfigs.size());
    for (int i = start; i < end; ++i) {
        int rowY = listTop + (i - start) * 52;
        Rectangle row = { pane.x + 10, (float)rowY, pane.width - 20, 48 };
        bool selected = (g.configSelected == i);
        DrawRectangleRounded(row, 0.1f, 4, selected ? MakeColor(219, 234, 254) : COLOR_ROW_ALT);
        DrawRectangleRoundedLines(row, 0.1f, 4, selected ? COLOR_PRIMARY : COLOR_BORDER);

        Subject* sub = findSubject(g.root, g.examConfigs[i].subjectId);
        DrawTxt(TextFormat("%s - %s", g.examConfigs[i].classId, sub != nullptr ? sub->name : g.examConfigs[i].subjectId),
                row.x + 12, row.y + 8, 15, COLOR_TEXT_DARK, true);
        DrawTxt(TextFormat("%s | %d cau / %d phut", g.examConfigs[i].subjectId, g.examConfigs[i].questionCount, g.examConfigs[i].minutes),
                row.x + 12, row.y + 27, 13, COLOR_TEXT_MUTED);

        Rectangle xoaRec = { row.x + row.width - 62, row.y + 10, 50, 28 };
        if (GuiBtnOutline(xoaRec, "Xoa", COLOR_DANGER, COLOR_DANGER, 13)) {
            g.currentModal = MODAL_DELETE_CLASS_EXAM_CONFIG;
            g.configEditIndex = i;
            g.modalError[0] = '\0';
        }

        Vector2 mouse = GetMousePosition();
        if (!GuiModalAbsorbsInput() && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)
            && CheckCollisionPointRec(mouse, row) && !CheckCollisionPointRec(mouse, xoaRec)) {
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
    if (g.configSelected < 0 || g.configSelected >= (int)g.examConfigs.size()) {
        DrawTxt("Chon mot cau hinh ben trai de xem de da boc.", pane.x + 20, pane.y + 20, 16, COLOR_TEXT_MUTED);
        return;
    }

    UiExamConfig& cfg = g.examConfigs[g.configSelected];
    Subject* sub = findSubject(g.root, cfg.subjectId);
    Class* cls = findClass(g.classes, g.classCount, cfg.classId);

    DrawTxt("DE THI DA BOC", pane.x + 20, pane.y + 12, 17, COLOR_TEXT_DARK, true);
    DrawTxt(TextFormat("Lop: %s - %s | Mon: %s - %s", cfg.classId, cls != nullptr ? cls->name : "?",
                       cfg.subjectId, sub != nullptr ? sub->name : "?"),
            pane.x + 20, pane.y + 50, 14, COLOR_TEXT_MUTED);
    DrawTxt(TextFormat("%d cau / %d phut | Ngan hang: %d cau", cfg.questionCount, cfg.minutes,
                       sub != nullptr ? countQuestions(sub->questionList) : 0),
            pane.x + 20, pane.y + 70, 14, COLOR_SUCCESS, true);

    if (GuiBtnOutline({ pane.x + pane.width - 210, pane.y + 10, 90, 30 }, "Sua", COLOR_PRIMARY, COLOR_PRIMARY, 14)) {
        g.currentModal = MODAL_CLASS_EXAM_CONFIG;
        g.configEditIndex = g.configSelected;
        snprintf(g.configClassId, sizeof(g.configClassId), "%s", cfg.classId);
        snprintf(g.configSubjectId, sizeof(g.configSubjectId), "%s", cfg.subjectId);
        snprintf(g.formExamQ, sizeof(g.formExamQ), "%d", cfg.questionCount);
        snprintf(g.formExamMinutes, sizeof(g.formExamMinutes), "%d", cfg.minutes);
        g.modalError[0] = '\0';
        g.activeFormBox = 3;
    }
    if (GuiBtnOutline({ pane.x + pane.width - 110, pane.y + 10, 90, 30 }, "Xoa", COLOR_DANGER, COLOR_DANGER, 14)) {
        g.currentModal = MODAL_DELETE_CLASS_EXAM_CONFIG;
        g.configEditIndex = g.configSelected;
        g.modalError[0] = '\0';
    }

    if (sub == nullptr) {
        DrawTxt("Mon hoc da bi xoa khoi he thong.", pane.x + 20, pane.y + 95, 15, COLOR_DANGER, true);
        return;
    }

    int totalPages = (cfg.questionCount + QUESTIONS_PER_PAGE - 1) / QUESTIONS_PER_PAGE;
    if (totalPages < 1) totalPages = 1;
    if (g.configPreviewPage >= totalPages) g.configPreviewPage = totalPages - 1;
    if (g.configPreviewPage < 0) g.configPreviewPage = 0;

    int start = g.configPreviewPage * QUESTIONS_PER_PAGE;
    int end = std::min(start + QUESTIONS_PER_PAGE, cfg.questionCount);
    int top = (int)pane.y + 100;
    for (int k = start; k < end; ++k) {
        Rectangle card = { pane.x + 15, (float)(top + (k - start) * 150), pane.width - 30, 140 };
        DrawCardPanel(card, COLOR_ROW_ALT);
        DrawTxt(TextFormat("Cau %d:", k + 1), card.x + 12, card.y + 10, 15, COLOR_PRIMARY, true);

        Question* q = findQuestion(sub->questionList, cfg.questionIds[k]);
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
    DrawTxt(TextFormat("Trang %d / %d (Tong: %d cau)", g.configPreviewPage + 1, totalPages, cfg.questionCount),
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
        g.currentModal = MODAL_CLASS_EXAM_CONFIG;
        g.configEditIndex = -1;
        g.configClassId[0] = '\0';
        g.configSubjectId[0] = '\0';
        g.formExamQ[0] = '\0';
        g.formExamMinutes[0] = '\0';
        g.modalError[0] = '\0';
        g.activeFormBox = 1;
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
    bool adding = (g.configEditIndex < 0 || g.configEditIndex >= (int)g.examConfigs.size());
    DrawTxt(adding ? "TAO CAU HINH THI (LOP - MON)" : "SUA CAU HINH THI (LOP - MON)",
            rec.x + 30, rec.y + 22, 19, COLOR_TEXT_DARK, true);

    DrawTxt("Ma lop:", rec.x + 30, rec.y + 60, 14, COLOR_TEXT_DARK);
    bool actClass = (g.activeFormBox == 1);
    GuiTextBoxInput({ rec.x + 30, rec.y + 82, 230, 36 }, g.configClassId, CLASS_ID_LEN, actClass, "VD: D23CQCN01");
    if (actClass) g.activeFormBox = 1;

    DrawTxt("Ma mon:", rec.x + 290, rec.y + 60, 14, COLOR_TEXT_DARK);
    bool actSub = (g.activeFormBox == 2);
    GuiTextBoxInput({ rec.x + 290, rec.y + 82, 230, 36 }, g.configSubjectId, SUBJECT_ID_LEN, actSub, "VD: CSDL");
    if (actSub) g.activeFormBox = 2;

    DrawTxt("So cau hoi thi:", rec.x + 30, rec.y + 140, 14, COLOR_TEXT_DARK);
    bool actQ = (g.activeFormBox == 3);
    GuiTextBoxInput({ rec.x + 30, rec.y + 162, 230, 36 }, g.formExamQ, sizeof(g.formExamQ), actQ, "VD: 10");
    if (actQ) g.activeFormBox = 3;

    DrawTxt("Thoi gian (phut):", rec.x + 290, rec.y + 140, 14, COLOR_TEXT_DARK);
    bool actM = (g.activeFormBox == 4);
    GuiTextBoxInput({ rec.x + 290, rec.y + 162, 230, 36 }, g.formExamMinutes, sizeof(g.formExamMinutes), actM, "VD: 15");
    if (actM) g.activeFormBox = 4;

    DrawTxt("De duoc boc mot lan; moi sinh vien trong lop thi cung bo cau nay.",
            rec.x + 30, rec.y + 218, 14, COLOR_TEXT_MUTED);

    if (g.modalError[0] != '\0') {
        DrawTxt(g.modalError, rec.x + 30, rec.y + 250, 14, COLOR_DANGER, true);
    }

    bool submit = GuiBtn({ rec.x + 30, rec.y + 330, 200, 42 }, "Luu Cau Hinh", COLOR_SUCCESS, COLOR_SUCCESS_HOVER, WHITE, 16, true);
    if (IsKeyPressed(KEY_ENTER)) submit = true;
    if (submit) {
        normalizeCode(g.configClassId);
        normalizeCode(g.configSubjectId);
        Class* cls = findClass(g.classes, g.classCount, g.configClassId);
        Subject* sub = findSubject(g.root, g.configSubjectId);
        int q = atoi(g.formExamQ);
        int m = atoi(g.formExamMinutes);
        int bank = (sub != nullptr) ? countQuestions(sub->questionList) : 0;
        int dup = GuiFindExamConfigIndex(g, g.configClassId, g.configSubjectId);
        bool keepDeck = !adding && dup == g.configEditIndex
                     && g.examConfigs[g.configEditIndex].questionCount == q;

        if (cls == nullptr) {
            snprintf(g.modalError, sizeof(g.modalError), "Ma lop khong ton tai.");
        } else if (sub == nullptr) {
            snprintf(g.modalError, sizeof(g.modalError), "Ma mon khong ton tai.");
        } else if (bank <= 0) {
            snprintf(g.modalError, sizeof(g.modalError), "Mon nay chua co cau hoi nao.");
        } else if (q <= 0 || q > bank) {
            snprintf(g.modalError, sizeof(g.modalError), "So cau phai tu 1 den %d.", bank);
        } else if (m <= 0 || m > 180) {
            snprintf(g.modalError, sizeof(g.modalError), "Thoi gian phai tu 1 den 180 phut.");
        } else if (dup >= 0 && dup != g.configEditIndex) {
            snprintf(g.modalError, sizeof(g.modalError), "Lop nay da co cau hinh cho mon nay.");
        } else if (keepDeck) {
            UiExamConfig& cfg = g.examConfigs[g.configEditIndex];
            cfg.minutes = m;
            g.currentModal = MODAL_NONE;
            g.showToast("Da cap nhat cau hinh thi!", 3.0f, false);
        } else {
            int* ids = new int[q];
            if (!drawDeck(sub, q, ids)) {
                delete[] ids;
                snprintf(g.modalError, sizeof(g.modalError), "Khong boc duoc %d cau tu ngan hang.", q);
            } else if (adding) {
                UiExamConfig cfg {};
                stringCopy(cfg.classId, g.configClassId, sizeof(cfg.classId));
                stringCopy(cfg.subjectId, g.configSubjectId, sizeof(cfg.subjectId));
                cfg.questionCount = q;
                cfg.minutes = m;
                cfg.questionIds = ids;
                g.examConfigs.push_back(cfg);
                g.configSelected = (int)g.examConfigs.size() - 1;
                g.configPage = g.configSelected / CONFIGS_PER_PAGE;
                g.configPreviewPage = 0;
                g.currentModal = MODAL_NONE;
                g.showToast("Da tao va boc de thi cho lop!", 3.0f, false);
            } else {
                UiExamConfig& cfg = g.examConfigs[g.configEditIndex];
                delete[] cfg.questionIds;
                stringCopy(cfg.classId, g.configClassId, sizeof(cfg.classId));
                stringCopy(cfg.subjectId, g.configSubjectId, sizeof(cfg.subjectId));
                cfg.questionCount = q;
                cfg.minutes = m;
                cfg.questionIds = ids;
                g.configSelected = g.configEditIndex;
                g.configPreviewPage = 0;
                g.currentModal = MODAL_NONE;
                g.showToast("Da cap nhat va boc lai de thi!", 3.0f, false);
            }
        }
    }
    if (GuiBtnOutline({ rec.x + 270, rec.y + 330, 200, 42 }, "Huy bo", COLOR_PRIMARY, COLOR_PRIMARY, 16, true)) {
        g.currentModal = MODAL_NONE;
    }
}

void drawDeleteClassExamConfigModal(GuiState& g) {
    Rectangle rec = { (float)(GUI_WIDTH / 2 - 250), (float)(GUI_HEIGHT / 2 - 170), 500, 340 };
    DrawCardPanel(rec);
    DrawTxt("XOA CAU HINH THI", rec.x + 30, rec.y + 28, 20, COLOR_DANGER, true);

    int idx = g.configEditIndex;
    if (idx < 0 || idx >= (int)g.examConfigs.size()) {
        g.currentModal = MODAL_NONE;
        return;
    }

    UiExamConfig& cfg = g.examConfigs[idx];
    DrawTxt(TextFormat("Xoa cau hinh lop %s - mon %s?", cfg.classId, cfg.subjectId), rec.x + 30, rec.y + 80, 16, COLOR_TEXT_DARK);
    DrawTxt("Sinh vien chua thi se khong thi duoc mon nay cho toi khi", rec.x + 30, rec.y + 115, 14, COLOR_TEXT_MUTED);
    DrawTxt("giang vien tao lai cau hinh.", rec.x + 30, rec.y + 137, 14, COLOR_TEXT_MUTED);

    if (GuiBtn({ rec.x + 30, rec.y + 210, 200, 42 }, "Xac Nhan Xoa", COLOR_DANGER, COLOR_DANGER_HOVER, WHITE, 16, true)) {
        int* ids = cfg.questionIds;
        char deletedClass[CLASS_ID_LEN] = {};
        char deletedSub[SUBJECT_ID_LEN] = {};
        snprintf(deletedClass, sizeof(deletedClass), "%s", cfg.classId);
        snprintf(deletedSub, sizeof(deletedSub), "%s", cfg.subjectId);
        g.examConfigs.erase_at(idx);
        delete[] ids;
        g.configSelected = -1;
        g.currentModal = MODAL_NONE;
        g.showToast(TextFormat("Da xoa cau hinh lop %s - mon %s.", deletedClass, deletedSub), 3.0f, false);
    }
    if (GuiBtnOutline({ rec.x + 270, rec.y + 210, 200, 42 }, "Huy bo", COLOR_PRIMARY, COLOR_PRIMARY, 16, true)) {
        g.currentModal = MODAL_NONE;
    }
}
