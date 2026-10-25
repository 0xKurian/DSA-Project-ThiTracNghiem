#include "GuiCommon.h"
#include "GuiState.h"

#include "../business/BusinessErrors.h"
#include "../business/ExamConfigService.h"
#include "../business/ExamService.h"
#include "../core/ExamConfigList.h"

#include <cstdio>

void drawExamPrepareTab(GuiState& g) {
    Rectangle contentRec = { 260, 85, (float)(GUI_WIDTH - 280), (float)(GUI_HEIGHT - 105) };

    DrawTxt("CHUAN BI THI TRAC NGHIEM", contentRec.x + 30, contentRec.y + 30, 22, COLOR_TEXT_DARK, true);

    int formStartY = (int)contentRec.y + 80;

    char pendSub[SUBJECT_ID_LEN] = {};
    int pendSec = 0;
    if (checkExamSession(g.loggedStudent->id, pendSub, pendSec)) {
        Rectangle warnCard = { contentRec.x + 30, (float)formStartY, contentRec.width - 60, 95 };
        DrawCardPanel(warnCard, MakeColor(254, 243, 199), COLOR_WARNING);
        DrawTxt(TextFormat("[CANH BAO] Ban dang co phien thi do dang mon: %s (con %d phut %d giay)", pendSub, pendSec / 60, pendSec % 60), warnCard.x + 20, warnCard.y + 18, 16, COLOR_WARNING, true);

        if (GuiBtn({ warnCard.x + 20, warnCard.y + 50, 230, 34 }, ">> TIEP TUC PHIEN THI <<", COLOR_PRIMARY, COLOR_PRIMARY_HOVER, WHITE, 14, true)) {
            g.examHead = nullptr;
            BusinessResult res = resumeExamWorkflow(g.root, g.classes, g.classCount, g.loggedStudent,
                                                    g.examHead, pendSub, pendSec);
            if (res == SUCCESS) {
                g.examList.clear();
                ExamDetail* cur = g.examHead;
                while (cur != nullptr) { g.examList.push_back(cur); cur = cur->next; }
                g.activeExamSubId = pendSub;
                g.examTimer.resume(pendSec);
                g.examTracker.lastSavedSecond = -1;
                g.currentExamQuestionIdx = 0;
                g.currentScreen = SCREEN_STUDENT_EXAM;
            } else {
                g.showToast(TextFormat("Khong the khoi phuc phien thi: %s", getErrorMessage(res)), 3.5f, true);
            }
        }
        if (GuiBtnOutline({ warnCard.x + 270, warnCard.y + 50, 200, 34 }, "Huy phien thi do dang", COLOR_DANGER, COLOR_DANGER, 14, true)) {
            g.currentModal = MODAL_CANCEL_EXAM;
        }

        formStartY += 120;
    }

    DrawTxt("1. Chon mon hoc:", contentRec.x + 30, (float)formStartY, 17, COLOR_TEXT_DARK, true);
    Rectangle prepSearchRec = { contentRec.x + 30, (float)formStartY + 30, 420, 40 };
    if (g.subjects.empty()) {
        DrawTxt("He thong chua co mon hoc nao de thi.", contentRec.x + 30, (float)formStartY + 40, 16, COLOR_TEXT_MUTED);
    }

    if (!g.subjects.empty() && g.studentSubChoice < (int)g.subjects.size()) {
        Subject* chosenSub = g.subjects[g.studentSubChoice];
        Class* myClass = findStudentClass(g.classes, g.classCount, g.loggedStudent->id);
        ExamConfig* cfg = (myClass != nullptr) ? findExamConfig(myClass, chosenSub->id) : nullptr;
        const char* reason = nullptr;
        BusinessResult canRes = canStudentTakeSubject(g.root, g.classes, g.classCount,
                                                      g.loggedStudent, chosenSub->id, &reason);
        bool canTake = (canRes == SUCCESS);

        if (!canTake) {
            const char* msg = (reason != nullptr) ? reason : getErrorMessage(canRes);
            DrawTxt(TextFormat("Khong the thi: %s.", msg), contentRec.x + 30, (float)formStartY + 125, 16, COLOR_WARNING, true);
        } else if (cfg != nullptr) {
            DrawTxt(TextFormat("Lop: %s | So cau thi: %d | Thoi gian: %d phut", myClass->id, cfg->questionCount, cfg->minutes),
                    contentRec.x + 30, (float)formStartY + 125, 17, COLOR_TEXT_DARK, true);
        }

        if (GuiBtn({ contentRec.x + 30, (float)formStartY + 180, 320, 52 }, ">> BAT DAU THI NGAY <<", COLOR_SUCCESS, COLOR_SUCCESS_HOVER, WHITE, 18, true, !canTake)) {
            g.examHead = nullptr;
            BusinessResult res = prepareExamFromConfig(g.root, g.classes, g.classCount,
                                                       g.loggedStudent, chosenSub->id, g.examHead);
            if (res == SUCCESS && cfg != nullptr) {
                g.examList.clear();
                ExamDetail* cur = g.examHead;
                while (cur != nullptr) { g.examList.push_back(cur); cur = cur->next; }
                g.activeExamSubId = chosenSub->id;
                g.examTimer.start(cfg->minutes * 60);
                g.examTracker.lastSavedSecond = -1;
                g.currentExamQuestionIdx = 0;
                g.currentScreen = SCREEN_STUDENT_EXAM;
            } else {
                if (g.examHead != nullptr) {
                    cancelExam(g.examHead);
                    g.examHead = nullptr;
                }
                g.showToast(TextFormat("Khong the bat dau thi: %s", getErrorMessage(res)), 4.0f, true);
            }
        }

        if (g.notifyTimer > 0 && g.notifyIsError) {
            DrawTxt(g.notifyMessage, contentRec.x + 30, (float)formStartY + 245, 15, COLOR_DANGER, true);
        }
    }

    if (!g.subjects.empty()) {
        Arr<SuggestItem> items;
        GuiMakeSubjectItems(g.subjects, true, items);
        int picked = GuiSearchSuggest(prepSearchRec, g.searchPrepare, sizeof(g.searchPrepare),
                                      g.activeSearchPrepare, items,
                                      "Tim theo ma hoac ten mon...");
        if (picked >= 0) g.studentSubChoice = picked;
    }
}
