// man hinh dang nhap
#include "GuiCommon.h"
#include "GuiState.h"

#include "../business/AuthService.h"
#include "../business/BusinessErrors.h"

#include <cstdio>

// tai khoan demo: GV/GV, SV001/pass123 (pass123 la mat khau trong data/classes.txt); loi hien ngay trong the dang nhap

void drawLoginScreen(GuiState& g) {
    DrawRectangle(0, 0, GUI_WIDTH, 180, COLOR_SIDEBAR);
    const char* title = "HE THONG QUAN LY VA THI TRAC NGHIEM C++";
    DrawTxt(title, GUI_WIDTH / 2.0f - MeasureTxt(title, 26, true) / 2.0f, 45, 26, WHITE, true);
    const char* subtitle = "Do an Cau truc Du lieu & Giai thuat (BST, Mang con tro, Danh sach lien ket)";
    DrawTxt(subtitle, GUI_WIDTH / 2.0f - MeasureTxt(subtitle, 16) / 2.0f, 95, 16, COLOR_TEXT_MUTED);

    Rectangle loginCard = { (float)(GUI_WIDTH / 2 - 250), 220, 500, 470 };
    DrawCardPanel(loginCard);

    DrawTxt("DANG NHAP HE THONG", loginCard.x + 30, loginCard.y + 30, 22, COLOR_TEXT_DARK, true);
    DrawLine((int)loginCard.x + 30, (int)loginCard.y + 65, (int)loginCard.x + 470, (int)loginCard.y + 65, COLOR_BORDER);

    DrawTxt("Ten dang nhap (GV hoac MASV):", loginCard.x + 30, loginCard.y + 85, 15, COLOR_TEXT_DARK);
    Rectangle userRec = { loginCard.x + 30, loginCard.y + 110, 440, 42 };
    GuiTextBoxInput(userRec, g.loginUser, sizeof(g.loginUser), g.activeUserBox, "Nhap GV hoac MASV (vd: SV001)");

    DrawTxt("Mat khau:", loginCard.x + 30, loginCard.y + 165, 15, COLOR_TEXT_DARK);
    Rectangle passRec = { loginCard.x + 30, loginCard.y + 190, 440, 42 };
    GuiTextBoxInput(passRec, g.loginPass, sizeof(g.loginPass), g.activePassBox, "Nhap mat khau", true);

    if (IsKeyPressed(KEY_TAB)) {
        if (g.activeUserBox) { g.activeUserBox = false; g.activePassBox = true; }
        else if (g.activePassBox) { g.activePassBox = false; g.activeUserBox = true; }
    }

    DrawTxt("Chon nhanh tai khoan de trai nghiem:", loginCard.x + 30, loginCard.y + 250, 14, COLOR_TEXT_MUTED);
    if (GuiBtnOutline({ loginCard.x + 30, loginCard.y + 275, 195, 36 }, "Giang vien (GV / GV)", COLOR_PRIMARY, COLOR_PRIMARY, 14, true)) {
        snprintf(g.loginUser, sizeof(g.loginUser), "GV");
        snprintf(g.loginPass, sizeof(g.loginPass), "GV");
    }
    // mat khau SV001 trong data/classes.txt la pass123; nut to hon de chu khong tran khung
    if (GuiBtnOutline({ loginCard.x + 235, loginCard.y + 275, 235, 36 }, "Sinh vien (SV001 / pass123)", COLOR_SUCCESS, COLOR_SUCCESS, 14, true)) {
        snprintf(g.loginUser, sizeof(g.loginUser), "SV001");
        snprintf(g.loginPass, sizeof(g.loginPass), "pass123");
    }

    bool doLogin = GuiBtn({ loginCard.x + 30, loginCard.y + 345, 440, 48 }, "DANG NHAP NGAY", COLOR_PRIMARY, COLOR_PRIMARY_HOVER, WHITE, 18, true);
    if (IsKeyPressed(KEY_ENTER) && (g.activeUserBox || g.activePassBox)) {
        doLogin = true;
    }

    if (doLogin) {
        BusinessResult res = loginWorkflow(g.classes, g.classCount, g.loginUser, g.loginPass, g.currentRole, g.loggedStudent);
        if (res == SUCCESS) {
            if (g.currentRole == LOGIN_TEACHER) {
                g.currentScreen = SCREEN_TEACHER_DASHBOARD;
                g.teacherTab = 0;
                g.subjectPage = 0;
            } else if (g.currentRole == LOGIN_STUDENT && g.loggedStudent != nullptr) {
                g.currentScreen = SCREEN_STUDENT_DASHBOARD;
                g.studentTab = 0;
            }
            g.showToast("Dang nhap thanh cong!", 2.5f, false);
        } else {
            g.showToast(getErrorMessage(res), 3.5f, true);
        }
    }

    if (g.notifyTimer > 0 && g.notifyIsError) {
        DrawTxt(g.notifyMessage, loginCard.x + 30, loginCard.y + 412, 15, COLOR_DANGER, true);
    }
}
