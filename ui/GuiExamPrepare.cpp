// tab vao thi: neu ton tai session thi chua nop -> hoi tiep tuc / huy,
// khong thi chon mon + so cau + so phut roi vao phong thi
#include "GuiCommon.h"
#include "GuiState.h"

#include "../business/BusinessErrors.h"
#include "../business/ExamService.h"   // prepareExamWorkflow, checkExamSession, loadExamSession, removeExamSession

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
        if (GuiBtnOutline({ warnCard.x + 270, warnCard.y + 50, 160, 34 }, "Huy phien thi do dang", COLOR_DANGER, COLOR_DANGER, 14, true)) {
            removeExamSession(g.loggedStudent->id);
            g.showToast("Da huy phien thi cu.", 2.5f, false);
        }

        formStartY += 120;
    }

    DrawTxt("1. Chon mon hoc:", contentRec.x + 30, (float)formStartY, 17, COLOR_TEXT_DARK, true);
    if (g.subjects.empty()) {
        DrawTxt("He thong chua co mon hoc nao de thi.", contentRec.x + 30, (float)formStartY + 35, 16, COLOR_TEXT_MUTED);
    } else {
        int maxVis = 4;
        if (g.studentSubNavOffset > 0) {
            if (GuiBtnOutline({ contentRec.x + 30, (float)formStartY + 30, 35, 36 }, "<", COLOR_PRIMARY, COLOR_PRIMARY, 14, true)) {
                g.studentSubNavOffset--;
            }
        }

        for (int s = 0; s < maxVis; ++s) {
            int idx = g.studentSubNavOffset + s;
            if (idx >= (int)g.subjects.size()) break;
            bool sel = (g.studentSubChoice == idx);
            Rectangle btnRec = { contentRec.x + 75 + s * 155, (float)formStartY + 30, 145, 36 };
            if (GuiBtn(btnRec, g.subjects[idx]->id, sel ? COLOR_PRIMARY : COLOR_SIDEBAR, COLOR_PRIMARY_HOVER, WHITE, 15, sel)) {
                g.studentSubChoice = idx;
            }
        }

        if (g.studentSubNavOffset + maxVis < (int)g.subjects.size()) {
            if (GuiBtnOutline({ contentRec.x + 75 + maxVis * 155, (float)formStartY + 30, 35, 36 }, ">", COLOR_PRIMARY, COLOR_PRIMARY, 14, true)) {
                g.studentSubNavOffset++;
            }
        }
    }

    if (!g.subjects.empty() && g.studentSubChoice < (int)g.subjects.size()) {
        Subject* chosenSub = g.subjects[g.studentSubChoice];
        int totalQ = 0;
        Question* qCur = chosenSub->questionList;
        while (qCur != nullptr) { totalQ++; qCur = qCur->next; }
        DrawTxt(TextFormat("-> Mon: %s | Ngan hang hien co: %d cau hoi", chosenSub->name, totalQ), contentRec.x + 30, (float)formStartY + 85, 15, COLOR_TEXT_MUTED);

        DrawTxt("2. So luong cau hoi thi:", contentRec.x + 30, (float)formStartY + 125, 17, COLOR_TEXT_DARK, true);
        Rectangle numQRec = { contentRec.x + 30, (float)formStartY + 155, 200, 42 };
        GuiTextBoxInput(numQRec, g.numQuestionsInput, sizeof(g.numQuestionsInput), g.activeNumQ, "Nhap so cau");

        DrawTxt("3. Thoi gian thi (phut):", contentRec.x + 280, (float)formStartY + 125, 17, COLOR_TEXT_DARK, true);
        Rectangle minRec = { contentRec.x + 280, (float)formStartY + 155, 200, 42 };
        GuiTextBoxInput(minRec, g.examMinutesInput, sizeof(g.examMinutesInput), g.activeMinutes, "Nhap so phut");

        if (IsKeyPressed(KEY_TAB)) {
            if (g.activeNumQ) { g.activeNumQ = false; g.activeMinutes = true; }
            else if (g.activeMinutes) { g.activeMinutes = false; g.activeNumQ = true; }
        }

        if (GuiBtn({ contentRec.x + 30, (float)formStartY + 230, 320, 52 }, ">> BAT DAU THI NGAY <<", COLOR_SUCCESS, COLOR_SUCCESS_HOVER, WHITE, 18, true)) {
            int numQ = atoi(g.numQuestionsInput);
            int mins = atoi(g.examMinutesInput);

            BusinessResult prepRes = prepareExamWorkflow(g.root, g.loggedStudent, chosenSub->id, numQ, mins, g.examHead);
            if (prepRes == SUCCESS) {
                g.examList.clear();
                ExamDetail* cur = g.examHead;
                while (cur != nullptr) { g.examList.push_back(cur); cur = cur->next; }
                g.activeExamSubId = chosenSub->id;
                g.examRemainingSeconds = mins * 60;
                g.currentExamQuestionIdx = 0;
                g.currentScreen = SCREEN_STUDENT_EXAM;
            } else {
                g.showToast(getErrorMessage(prepRes), 3.5f, true);
            }
        }

        if (g.notifyTimer > 0 && g.notifyIsError) {
            DrawTxt(g.notifyMessage, contentRec.x + 30, (float)formStartY + 300, 15, COLOR_DANGER, true);
        }
    }
}
