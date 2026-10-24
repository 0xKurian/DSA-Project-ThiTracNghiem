#include "GraphicUI.h"

#include "GuiCommon.h"
#include "GuiState.h"

#include "../business/FileStorage.h"

#include "../core/ClassArray.h"
#include "../core/SubjectBST.h"

#include <cstdio>

void runGraphicApp() {
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(GUI_WIDTH, GUI_HEIGHT, "HE THONG QUAN LY VA THI TRAC NGHIEM - C++ DSA PROJECT");
    SetExitKey(KEY_NULL);
    SetTargetFPS(60);

    loadVietnameseFonts();

    GuiState g {};

    loadAllData(g.root, g.classes, g.classCount, "data");

    g.currentScreen = SCREEN_LOGIN;
    g.currentRole = LOGIN_FAILED;
    g.loggedStudent = nullptr;

    snprintf(g.formGender, sizeof(g.formGender), "Nam");
    snprintf(g.formPass, sizeof(g.formPass), "123");
    snprintf(g.formCorrect, sizeof(g.formCorrect), "A");

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        GuiBeginFrame();
        if (g.notifyTimer > 0) g.notifyTimer -= dt;

        g.subjects.clear();
        collectSubjects(g.root, g.subjects);

        GuiSetModalState(g.currentModal != MODAL_NONE);

        updateExamSession(g);

        BeginDrawing();
        ClearBackground(COLOR_BG);

        if (g.currentScreen == SCREEN_LOGIN) {
            drawLoginScreen(g);
        } else if (g.currentScreen == SCREEN_TEACHER_DASHBOARD) {
            drawAdminDashboard(g);
        } else if (g.currentScreen == SCREEN_STUDENT_DASHBOARD) {
            drawStudentHome(g);
        } else if (g.currentScreen == SCREEN_STUDENT_EXAM && !g.examList.empty()) {
            drawExamRoom(g);
        } else if (g.currentScreen == SCREEN_EXAM_RESULT) {
            drawExamResult(g);
        }

        if (g.currentModal != MODAL_NONE) {
            renderModalDialogs(g);
        }

        if (g.notifyTimer > 0) {
            DrawToastNotification(g.notifyMessage, g.notifyTimer, g.notifyIsError);
        }

        EndDrawing();
    }

    saveAllData(g.root, g.classes, g.classCount, "data");

    while (g.classCount > 0) {
        deleteClass(g.classes, g.classCount, g.classes[0]->id);
    }
    while (g.root != nullptr) {
        g.root = deleteSubject(g.root, g.root->id);
    }

    GuiFreeExamConfigs(g);

    unloadVietnameseFonts();
    CloseWindow();
}
