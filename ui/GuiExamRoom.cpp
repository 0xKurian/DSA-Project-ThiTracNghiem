// phong thi: lam bai + dong ho tu dong nop; modal xac nhan nop cung o day
#include "GuiCommon.h"
#include "GuiState.h"

#include "../business/BusinessErrors.h"
#include "../business/ExamService.h"    // saveExamSession, submitExamWorkflow, removeExamSession
#include "../business/FileStorage.h"    // saveAllData

#include <cstdio>


void updateExamSession(GuiState& g, float dt) {
    if (g.currentScreen != SCREEN_STUDENT_EXAM) return;

    g.examTimerClock += dt;
    if (g.examTimerClock < 1.0f) return;
    g.examTimerClock -= 1.0f;

    if (g.examRemainingSeconds > 0) {
        g.examRemainingSeconds--;
        // luu moi giay, dong app mo thi van thi lai duoc
        saveExamSession(g.loggedStudent->id, g.activeExamSubId.c_str(), g.examRemainingSeconds, g.examHead);
    } else {
        // Hết giờ: tự động nộp bài
        g.currentModal = MODAL_NONE;
        submitExamWorkflow(g.loggedStudent, g.activeExamSubId.c_str(), g.examHead, g.lastScore, g.lastCorrect, g.lastTotal);
        removeExamSession(g.loggedStudent->id);
        saveAllData(g.root, g.classes, g.classCount, "data");
        g.examHead = nullptr;
        g.examList.clear();
        g.currentScreen = SCREEN_EXAM_RESULT;
        g.showToast("Het gio lam bai! He thong da tu dong nop bai thi.", 4.0f, false);
    }
}

void drawExamRoom(GuiState& g) {
    // Thanh cong cu: mon thi + dong ho + nut nop bai
    DrawRectangle(0, 0, GUI_WIDTH, 70, COLOR_SIDEBAR);
    DrawTxt(TextFormat("BAI THI MON: %s", g.activeExamSubId.c_str()), 30, 22, 22, WHITE, true);

    int mins = g.examRemainingSeconds / 60;
    int secs = g.examRemainingSeconds % 60;
    Rectangle timerRec = { (float)(GUI_WIDTH / 2 - 120), 12, 240, 46 };
    DrawRectangleRounded(timerRec, 0.2f, 4, (g.examRemainingSeconds < 60) ? COLOR_DANGER : COLOR_PRIMARY);
    DrawTxt(TextFormat("THOI GIAN: %02d:%02d", mins, secs), timerRec.x + 30, timerRec.y + 12, 20, WHITE, true);

    if (GuiBtn({ (float)(GUI_WIDTH - 160), 15, 130, 40 }, "NOP BAI", COLOR_WARNING, COLOR_PRIMARY_HOVER, WHITE, 17, true)) {
        g.currentModal = MODAL_CONFIRM_SUBMIT;
    }

    // Khung noi dung cau hoi hien thi
    Rectangle qPane = { 30, 95, 840, (float)(GUI_HEIGHT - 125) };
    DrawCardPanel(qPane);

    ExamDetail* curQ = g.examList[g.currentExamQuestionIdx];
    DrawTxt(TextFormat("CAU HOI %d / %d", g.currentExamQuestionIdx + 1, (int)g.examList.size()), qPane.x + 30, qPane.y + 25, 20, COLOR_PRIMARY, true);
    DrawLine((int)qPane.x + 30, (int)qPane.y + 55, (int)qPane.x + (int)qPane.width - 30, (int)qPane.y + 55, COLOR_BORDER);

    DrawTxtWrapped(curQ->content, qPane.x + 30, qPane.y + 70, qPane.width - 60, 18, COLOR_TEXT_DARK, true);

    struct AnsChoice { char letter; const char* text; };
    AnsChoice choices[4] = {
        { 'A', curQ->answerA },
        { 'B', curQ->answerB },
        { 'C', curQ->answerC },
        { 'D', curQ->answerD }
    };

    for (int i = 0; i < 4; ++i) {
        Rectangle ansRec = { qPane.x + 30, (float)(qPane.y + 140 + i * 85), qPane.width - 60, 70 };
        bool isSelected = (curQ->selectedAnswer == choices[i].letter);

        Color btnBg = isSelected ? MakeColor(219, 234, 254) : COLOR_CARD;
        Color borderC = isSelected ? COLOR_PRIMARY : COLOR_BORDER;
        Color textC = isSelected ? COLOR_PRIMARY : COLOR_TEXT_DARK;

        DrawRectangleRounded(ansRec, 0.12f, 4, btnBg);
        DrawRectangleRoundedLines(ansRec, 0.12f, 4, borderC);

        DrawTxt(TextFormat("[%c]", choices[i].letter), ansRec.x + 20, ansRec.y + 22, 20, textC, true);
        DrawTxtWrapped(choices[i].text, ansRec.x + 75, ansRec.y + 22, ansRec.width - 95, 17, textC, isSelected);

        Vector2 mouse = GetMousePosition();
        if (g.currentModal == MODAL_NONE && CheckCollisionPointRec(mouse, ansRec) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            curQ->selectedAnswer = choices[i].letter;
            saveExamSession(g.loggedStudent->id, g.activeExamSubId.c_str(), g.examRemainingSeconds, g.examHead);
        }
    }

    // Di chuyen giua cac cau
    if (g.currentExamQuestionIdx > 0) {
        if (GuiBtnOutline({ qPane.x + 30, qPane.y + qPane.height - 60, 150, 42 }, "< Cau truoc", COLOR_PRIMARY, COLOR_PRIMARY, 15, true)) {
            g.currentExamQuestionIdx--;
        }
    }
    if (g.currentExamQuestionIdx < (int)g.examList.size() - 1) {
        if (GuiBtn({ qPane.x + qPane.width - 180, qPane.y + qPane.height - 60, 150, 42 }, "Cau tiep >", COLOR_PRIMARY, COLOR_PRIMARY_HOVER, WHITE, 15, true)) {
            g.currentExamQuestionIdx++;
        }
    }

    // Luoi danh sach cau hoi ben phai
    Rectangle gridPane = { 890, 95, (float)(GUI_WIDTH - 920), (float)(GUI_HEIGHT - 125) };
    DrawCardPanel(gridPane);

    DrawTxt("DANH SACH CAU HOI", gridPane.x + 25, gridPane.y + 22, 17, COLOR_TEXT_DARK, true);
    DrawTxt("Xanh: Da lam | Trang: Chua lam", gridPane.x + 25, gridPane.y + 48, 13, COLOR_TEXT_MUTED);

    int btnSize = (g.examList.size() > 36) ? 36 : 42;
    int btnGap = (g.examList.size() > 36) ? 8 : 12;
    int cols = 6;
    for (size_t i = 0; i < g.examList.size(); ++i) {
        int col = (int)i % cols;
        int row = (int)i / cols;
        Rectangle numRec = { (float)(gridPane.x + 25 + col * (btnSize + btnGap)), (float)(gridPane.y + 80 + row * (btnSize + btnGap)), (float)btnSize, (float)btnSize };

        bool isAnswered = (g.examList[i]->selectedAnswer != '\0');
        bool isCurrent = (g.currentExamQuestionIdx == (int)i);

        Color numBg = isAnswered ? COLOR_SUCCESS : MakeColor(241, 245, 249);
        Color numText = isAnswered ? WHITE : COLOR_TEXT_DARK;

        DrawRectangleRounded(numRec, 0.2f, 4, numBg);
        if (isCurrent) {
            DrawRectangleRoundedLines(numRec, 0.2f, 4, COLOR_PRIMARY);
        }

        const char* numStr = TextFormat("%d", (int)i + 1);
        DrawTxt(numStr, numRec.x + (btnSize - MeasureTxt(numStr, 15, true)) / 2.0f, numRec.y + (btnSize - 15) / 2.0f, 15, numText, true);

        Vector2 mouse = GetMousePosition();
        if (g.currentModal == MODAL_NONE && CheckCollisionPointRec(mouse, numRec) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            g.currentExamQuestionIdx = (int)i;
        }
    }
}

void drawExamSubmitConfirmDialog(GuiState& g) {
    // The modal dat o trung tam man hinh
    Rectangle modalRec = { (float)(GUI_WIDTH / 2 - 250), (float)(GUI_HEIGHT / 2 - 180), 500, 360 };
    DrawCardPanel(modalRec);
    DrawTxt("XAC NHAN NOP BAI THI", modalRec.x + 30, modalRec.y + 30, 20, COLOR_WARNING, true);

    int answeredCount = 0;
    for (size_t i = 0; i < g.examList.size(); ++i) {
        if (g.examList[i]->selectedAnswer != '\0') answeredCount++;
    }

    DrawTxt(TextFormat("Ban da tra loi: %d / %d cau hoi.", answeredCount, (int)g.examList.size()), modalRec.x + 30, modalRec.y + 80, 17, COLOR_TEXT_DARK);
    DrawTxt("Ban co chac chan muon nop bai ngay bay gio?", modalRec.x + 30, modalRec.y + 115, 15, COLOR_TEXT_MUTED);

    if (GuiBtn({ modalRec.x + 30, modalRec.y + 210, 200, 44 }, "Dong Y Nop Bai", COLOR_SUCCESS, COLOR_SUCCESS_HOVER, WHITE, 16, true)) {
        g.currentModal = MODAL_NONE;
        submitExamWorkflow(g.loggedStudent, g.activeExamSubId.c_str(), g.examHead, g.lastScore, g.lastCorrect, g.lastTotal);
        removeExamSession(g.loggedStudent->id);
        saveAllData(g.root, g.classes, g.classCount, "data");
        g.examHead = nullptr;
        g.examList.clear();
        g.currentScreen = SCREEN_EXAM_RESULT;
        g.showToast("Da nop bai thi thanh cong!", 3.5f, false);
    }
    if (GuiBtnOutline({ modalRec.x + 270, modalRec.y + 210, 200, 44 }, "Quay Lai Lam Tiep", COLOR_PRIMARY, COLOR_PRIMARY, 16, true)) {
        g.currentModal = MODAL_NONE;
    }
}
