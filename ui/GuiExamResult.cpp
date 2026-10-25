#include "GuiCommon.h"
#include "GuiState.h"

#include "../business/BusinessQueries.h"

#include <cstdio>

void drawExamResult(GuiState& g) {
    DrawRectangle(0, 0, GUI_WIDTH, GUI_HEIGHT, COLOR_BG);

    Rectangle card = { (float)(GUI_WIDTH / 2 - 250), (float)(GUI_HEIGHT / 2 - 210), 500, 420 };
    DrawCardPanel(card);

    DrawTxt("HOAN THANH BAI THI!", card.x + (500 - MeasureTxt("HOAN THANH BAI THI!", 24, true)) / 2.0f, card.y + 35, 24, COLOR_SUCCESS, true);
    DrawTxt(TextFormat("Mon thi: %s", g.activeExamSubId.c_str()), card.x + 40, card.y + 90, 17, COLOR_TEXT_DARK);

    DrawTxt("DIEM SO DAT DUOC:", card.x + (500 - MeasureTxt("DIEM SO DAT DUOC:", 17, true)) / 2.0f, card.y + 130, 17, COLOR_TEXT_MUTED, true);
    const char* scoreTxt = TextFormat("%.2f", g.lastScore);
    DrawTxt(scoreTxt, card.x + (500 - MeasureTxt(scoreTxt, 48, true)) / 2.0f, card.y + 165, 48, COLOR_PRIMARY, true);

    DrawTxt(TextFormat("Ket qua: %d / %d cau dung", g.lastCorrect, g.lastTotal), card.x + (500 - MeasureTxt(TextFormat("Ket qua: %d / %d cau dung", g.lastCorrect, g.lastTotal), 17)) / 2.0f, card.y + 240, 17, COLOR_TEXT_DARK);

    bool passed = isPassingScore(g.lastScore);
    const char* badge = passed ? "[ DAT ]" : "[ CHUA DAT ]";
    Color badgeC = passed ? COLOR_SUCCESS : COLOR_DANGER;
    DrawTxt(badge, card.x + (500 - MeasureTxt(badge, 17, true)) / 2.0f, card.y + 275, 17, badgeC, true);

    if (GuiBtn({ card.x + 40, card.y + 330, 420, 48 }, "VE MAN HINH CHINH", COLOR_PRIMARY, COLOR_PRIMARY_HOVER, WHITE, 17, true)) {
        g.currentScreen = SCREEN_STUDENT_DASHBOARD;
        g.studentTab = 1;
        g.studentReviewScoreIdx = -1;
    }
}
