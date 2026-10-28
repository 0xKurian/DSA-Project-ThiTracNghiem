#include "GuiCommon.h"
#include "GuiState.h"

#include "../business/FileStorage.h"

void drawAdminDashboard(GuiState& g) {
    DrawRectangle(0, 0, GUI_WIDTH, 65, COLOR_SIDEBAR);
    DrawTxt("BANG DIEU KHIEN GIANG VIEN", 30, 20, 22, WHITE, true);

    if (GuiBtnOutline({ (float)(GUI_WIDTH - 260), 15, 110, 35 }, "Luu File", COLOR_SUCCESS, COLOR_SUCCESS, 15, true)) {
        saveAllData(g.root, g.classes, g.classCount, "data");
        g.showToast("Da luu du lieu thanh cong vao thu muc 'data'!", 3.0f, false);
    }

    if (GuiBtn({ (float)(GUI_WIDTH - 130), 15, 100, 35 }, "Dang xuat", COLOR_DANGER, COLOR_DANGER_HOVER, WHITE, 15, true)) {
        g.currentScreen = SCREEN_LOGIN;
        g.currentRole = LOGIN_FAILED;
        g.loggedStudent = nullptr;
        g.loginUser[0] = '\0';
        g.loginPass[0] = '\0';
    }

    DrawRectangle(0, 65, 240, GUI_HEIGHT - 65, COLOR_SIDEBAR);
    const char* tabs[] = {
        "Mon hoc (BST)",
        "Lop hoc & Sinh vien",
        "Bang diem & Bai thi",
        "Cau hinh thi (Lop-Mon)"
    };
    const int tabIds[] = { 0, 2, 4, 5 };

    for (int i = 0; i < 4; ++i) {
        Rectangle tabRec = { 10, (float)(85 + i * 55), 220, 45 };
        bool active = (g.teacherTab == tabIds[i]) || (tabIds[i] == 0 && g.teacherTab == 1);
        Color tabBg = active ? COLOR_SIDEBAR_ACTIVE : COLOR_SIDEBAR;
        if (GuiBtn(tabRec, tabs[i], tabBg, COLOR_SIDEBAR_HOVER, WHITE, 15, active)) {
            g.teacherTab = tabIds[i];
        }
    }

    Rectangle contentRec = { 260, 85, (float)(GUI_WIDTH - 280), (float)(GUI_HEIGHT - 105) };
    DrawCardPanel(contentRec);

    switch (g.teacherTab) {
    case 0:
        drawAdminTabSubjects(g);
        break;
    case 1:
        drawAdminTabQuestions(g);
        break;
    case 2:
        drawAdminTabClasses(g);
        break;
    case 5:
        drawAdminTabExamConfigs(g);
        break;
    default:
        drawAdminTabScores(g);
        break;
    }
}
