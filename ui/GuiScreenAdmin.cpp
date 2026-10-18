// dashboard gv: topbar + sidebar chon tab; noi dung tung tab nam o file rieng
#include "GuiCommon.h"
#include "GuiState.h"

#include "../business/FileStorage.h"

#include <cstdio>

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
    }

    DrawRectangle(0, 65, 240, GUI_HEIGHT - 65, COLOR_SIDEBAR);
    const char* tabs[] = {
        "Mon hoc (BST)",
        "Ngan hang cau hoi",
        "Danh muc lop hoc",
        "Sinh vien theo lop",
        "Bang diem mon hoc",
        "Chi tiet bai thi"
    };

    for (int i = 0; i < 6; ++i) {
        Rectangle tabRec = { 10, (float)(85 + i * 55), 220, 45 };
        bool active = (g.teacherTab == i);
        Color tabBg = active ? COLOR_SIDEBAR_ACTIVE : COLOR_SIDEBAR;
        if (GuiBtn(tabRec, tabs[i], tabBg, COLOR_SIDEBAR_HOVER, WHITE, 15, active)) {
            g.teacherTab = i;
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
    case 3:
        drawAdminTabStudents(g);
        break;
    case 4:
        drawAdminTabScores(g);
        break;
    default:
        drawAdminTabExamReview(g);
        break;
    }
}
