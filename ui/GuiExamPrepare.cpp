#include "GuiCommon.h"
#include "GuiState.h"

#include "../business/BusinessErrors.h"
#include "../business/ExamService.h"
#include "../core/ScoreList.h"

#include <algorithm>
#include <cstdlib>
#include <cstdio>

void drawExamPrepareTab(GuiState& g) {
    Rectangle contentRec = { 260, 85, (float)(GUI_WIDTH - 280), (float)(GUI_HEIGHT - 105) };

    DrawTxt("CHUAN BI THI TRAC NGHIEM", contentRec.x + 30, contentRec.y + 30, 22, COLOR_TEXT_DARK, true);

    int formStartY = (int)contentRec.y + 80;

    char pendSub[32] = {};
    int pendSec = 0;
    if (checkExamSession(g.loggedStudent->id, pendSub, pendSec)) {
        Rectangle warnCard = { contentRec.x + 30, (float)formStartY, contentRec.width - 60, 95 };
        DrawCardPanel(warnCard, MakeColor(254, 243, 199), COLOR_WARNING);
        DrawTxt(TextFormat("[CANH BAO] Ban dang co phien thi do dang mon: %s (con %d phut %d giay)", pendSub, pendSec / 60, pendSec % 60), warnCard.x + 20, warnCard.y + 18, 16, COLOR_WARNING, true);

        if (GuiBtn({ warnCard.x + 20, warnCard.y + 50, 230, 34 }, ">> TIEP TUC PHIEN THI <<", COLOR_PRIMARY, COLOR_PRIMARY_HOVER, WHITE, 14, true)) {
            cancelExam(g.examHead);
            if (loadExamSession(g.loggedStudent->id, pendSub, pendSec, g.examHead, g.root)) {
                g.examList.clear();
                ExamDetail* cur = g.examHead;
                while (cur != nullptr) { g.examList.push_back(cur); cur = cur->next; }
                g.activeExamSubId = pendSub;
                g.examRemainingSeconds = pendSec;
                g.currentExamQuestionIdx = 0;
                g.currentScreen = SCREEN_STUDENT_EXAM;
            } else {
                g.showToast("Khong the khoi phuc phien thi cu.", 3.0f, true);
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
        int totalQ = 0;
        Question* qCur = chosenSub->questionList;
        while (qCur != nullptr) { totalQ++; qCur = qCur->next; }

        int cfgQ = chosenSub->examQuestionCount;
        int cfgM = chosenSub->examMinutes;

        Score* takenScore = findScore(g.loggedStudent, chosenSub->id);
        bool cfgUsable = (cfgQ > 0 && cfgM > 0 && cfgM <= 180 && cfgQ <= totalQ && takenScore == nullptr);

        DrawTxt(TextFormat("-> Mon: %s | Ngan hang hien co: %d cau hoi", chosenSub->name, totalQ), contentRec.x + 30, (float)formStartY + 85, 15, COLOR_TEXT_MUTED);

        if (cfgQ <= 0 || cfgM <= 0) {
            DrawTxt("Mon nay chua duoc giang vien cau hinh de thi.", contentRec.x + 30, (float)formStartY + 125, 16, COLOR_WARNING, true);
        } else if (takenScore != nullptr) {
            DrawTxt(TextFormat("Ban da thi mon nay (diem %.2f). Khong the thi lai.", takenScore->score), contentRec.x + 30, (float)formStartY + 125, 16, COLOR_WARNING, true);
        } else if (!cfgUsable) {
            DrawTxt(TextFormat("Cau hinh thi (%d cau) khong hop le voi ngan hang (%d cau).", cfgQ, totalQ), contentRec.x + 30, (float)formStartY + 125, 16, COLOR_WARNING, true);
        } else {
            DrawTxt(TextFormat("So cau thi: %d | Thoi gian: %d phut", cfgQ, cfgM), contentRec.x + 30, (float)formStartY + 125, 17, COLOR_TEXT_DARK, true);
        }

        if (GuiBtn({ contentRec.x + 30, (float)formStartY + 180, 320, 52 }, ">> BAT DAU THI NGAY <<", COLOR_SUCCESS, COLOR_SUCCESS_HOVER, WHITE, 18, true, !cfgUsable)) {
            BusinessResult prepRes = prepareExamWorkflow(g.root, g.loggedStudent, chosenSub->id, cfgQ, cfgM, g.examHead);
            if (prepRes == SUCCESS) {
                g.examList.clear();
                ExamDetail* cur = g.examHead;
                while (cur != nullptr) { g.examList.push_back(cur); cur = cur->next; }
                g.activeExamSubId = chosenSub->id;
                g.examRemainingSeconds = cfgM * 60;
                g.currentExamQuestionIdx = 0;
                g.currentScreen = SCREEN_STUDENT_EXAM;
            } else if (prepRes == ERR_CONSTRAINT_VIOLATION) {
                g.showToast("Ban da thi mon nay roi, khong the thi lai.", 3.5f, true);
            } else {
                g.showToast(getErrorMessage(prepRes), 3.5f, true);
            }
        }

        if (g.notifyTimer > 0 && g.notifyIsError) {
            DrawTxt(g.notifyMessage, contentRec.x + 30, (float)formStartY + 245, 15, COLOR_DANGER, true);
        }
    }

    if (!g.subjects.empty()) {
        std::vector<SuggestItem> items;
        GuiMakeSubjectItems(g.subjects, true, items);
        int picked = GuiSearchSuggest(prepSearchRec, g.searchPrepare, sizeof(g.searchPrepare),
                                      g.activeSearchPrepare, items, g.studentSubChoice,
                                      "Tim theo ma hoac ten mon...");
        if (picked >= 0) g.studentSubChoice = picked;
    }
}
