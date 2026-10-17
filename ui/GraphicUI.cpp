#include "GraphicUI.h"

#include "../business/AuthService.h"
#include "../business/BusinessErrors.h"
#include "../business/ClassService.h"
#include "../business/ExamService.h"
#include "../business/FileStorage.h"
#include "../business/QuestionService.h"
#include "../business/ReportService.h"
#include "../business/StudentService.h"
#include "../business/SubjectService.h"

#include "../core/ClassArray.h"
#include "../core/Constants.h"
#include "../core/QuestionList.h"
#include "../core/ScoreList.h"
#include "../core/StudentList.h"
#include "../core/SubjectBST.h"
#include "../core/StringUtils.h"

#include "raylib.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

// ==================== COLOR HELPERS ====================
static inline Color MakeColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255) {
    Color c;
    c.r = r; c.g = g; c.b = b; c.a = a;
    return c;
}

static const Color COLOR_BG = { 245, 247, 250, 255 };          // Light gray-blue
static const Color COLOR_CARD = { 255, 255, 255, 255 };        // White
static const Color COLOR_SIDEBAR = { 30, 41, 59, 255 };        // Slate 800
static const Color COLOR_SIDEBAR_HOVER = { 51, 65, 85, 255 };  // Slate 700
static const Color COLOR_SIDEBAR_ACTIVE = { 37, 99, 235, 255 };// Blue 600
static const Color COLOR_PRIMARY = { 37, 99, 235, 255 };       // Blue 600
static const Color COLOR_PRIMARY_HOVER = { 29, 78, 216, 255 }; // Blue 700
static const Color COLOR_SUCCESS = { 22, 163, 74, 255 };       // Green 600
static const Color COLOR_SUCCESS_HOVER = { 21, 128, 61, 255 }; // Green 700
static const Color COLOR_DANGER = { 220, 38, 38, 255 };        // Red 600
static const Color COLOR_DANGER_HOVER = { 185, 28, 28, 255 };  // Red 700
static const Color COLOR_WARNING = { 234, 88, 12, 255 };       // Orange 600
static const Color COLOR_TEXT_DARK = { 15, 23, 42, 255 };      // Slate 900
static const Color COLOR_TEXT_MUTED = { 100, 116, 139, 255 };  // Slate 500
static const Color COLOR_BORDER = { 226, 232, 240, 255 };      // Slate 200

// ==================== GLOBAL FONTS & ANTI-ALIASED TYPOGRAPHY ====================
static Font gFontRegular;
static Font gFontBold;

static inline void DrawTxt(const char* text, float posX, float posY, float fontSize, Color color = COLOR_TEXT_DARK, bool bold = false) {
    if (text == nullptr || text[0] == '\0') return;
    Font f = bold ? gFontBold : gFontRegular;
    DrawTextEx(f, text, Vector2{ posX, posY }, fontSize, 1.0f, color);
}

static inline float MeasureTxt(const char* text, float fontSize, bool bold = false) {
    if (text == nullptr || text[0] == '\0') return 0.0f;
    Font f = bold ? gFontBold : gFontRegular;
    Vector2 sz = MeasureTextEx(f, text, fontSize, 1.0f);
    return sz.x;
}

// ==================== ENUMS & UI STATE ====================
enum ScreenState {
    SCREEN_LOGIN,
    SCREEN_TEACHER_DASHBOARD,
    SCREEN_STUDENT_DASHBOARD,
    SCREEN_STUDENT_EXAM,
    SCREEN_EXAM_RESULT
};

enum ModalType {
    MODAL_NONE = 0,
    MODAL_ADD_SUBJECT,
    MODAL_EDIT_SUBJECT,
    MODAL_DELETE_SUBJECT,
    MODAL_ADD_QUESTION,
    MODAL_EDIT_QUESTION,
    MODAL_DELETE_QUESTION,
    MODAL_ADD_CLASS,
    MODAL_DELETE_CLASS,
    MODAL_ADD_STUDENT,
    MODAL_EDIT_STUDENT,
    MODAL_DELETE_STUDENT,
    MODAL_CONFIRM_SUBMIT
};

// ==================== HELPER WIDGETS ====================

static bool GuiBtn(Rectangle rec, const char* text, Color bg = COLOR_PRIMARY, Color hoverBg = COLOR_PRIMARY_HOVER, Color textColor = WHITE, float fontSize = 17.0f, bool bold = false) {
    Vector2 mouse = GetMousePosition();
    bool hovered = CheckCollisionPointRec(mouse, rec);
    Color currentBg = hovered ? hoverBg : bg;

    DrawRectangleRounded(rec, 0.15f, 4, currentBg);
    DrawRectangleRoundedLines(rec, 0.15f, 4, currentBg);

    float textW = MeasureTxt(text, fontSize, bold);
    float posX = rec.x + (rec.width - textW) / 2.0f;
    float posY = rec.y + (rec.height - fontSize) / 2.0f;
    DrawTxt(text, posX, posY, fontSize, textColor, bold);

    return hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

static bool GuiBtnOutline(Rectangle rec, const char* text, Color borderColor = COLOR_PRIMARY, Color textColor = COLOR_PRIMARY, float fontSize = 16.0f, bool bold = false) {
    Vector2 mouse = GetMousePosition();
    bool hovered = CheckCollisionPointRec(mouse, rec);
    Color currentBg = hovered ? MakeColor(borderColor.r, borderColor.g, borderColor.b, 30) : COLOR_CARD;

    DrawRectangleRounded(rec, 0.15f, 4, currentBg);
    DrawRectangleRoundedLines(rec, 0.15f, 4, borderColor);

    float textW = MeasureTxt(text, fontSize, bold);
    float posX = rec.x + (rec.width - textW) / 2.0f;
    float posY = rec.y + (rec.height - fontSize) / 2.0f;
    DrawTxt(text, posX, posY, fontSize, textColor, bold);

    return hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

static void GuiTextBoxInput(Rectangle rec, char textBuffer[], int maxLen, bool& isActive, const char* placeholder = "", bool isPassword = false) {
    Vector2 mouse = GetMousePosition();
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        isActive = CheckCollisionPointRec(mouse, rec);
    }

    if (isActive) {
        int key = GetCharPressed();
        while (key > 0) {
            int len = (int)strlen(textBuffer);
            if ((key >= 32) && (key <= 125) && (len < maxLen - 1)) {
                textBuffer[len] = (char)key;
                textBuffer[len + 1] = '\0';
            }
            key = GetCharPressed();
        }

        if (IsKeyPressed(KEY_BACKSPACE)) {
            int len = (int)strlen(textBuffer);
            if (len > 0) {
                textBuffer[len - 1] = '\0';
            }
        }
    }

    DrawRectangleRounded(rec, 0.15f, 4, COLOR_CARD);
    DrawRectangleRoundedLines(rec, 0.15f, 4, isActive ? COLOR_PRIMARY : COLOR_BORDER);

    float fontSize = 17.0f;
    float posY = rec.y + (rec.height - fontSize) / 2.0f;
    float posX = rec.x + 14.0f;

    if (strlen(textBuffer) == 0 && !isActive) {
        DrawTxt(placeholder, posX, posY, fontSize, COLOR_TEXT_MUTED);
    } else {
        if (isPassword) {
            std::string masked(strlen(textBuffer), '*');
            DrawTxt(masked.c_str(), posX, posY, fontSize, COLOR_TEXT_DARK);
        } else {
            DrawTxt(textBuffer, posX, posY, fontSize, COLOR_TEXT_DARK);
        }

        if (isActive && ((GetTime() - (int)GetTime()) < 0.5)) {
            float textW = isPassword ? MeasureTxt(std::string(strlen(textBuffer), '*').c_str(), fontSize)
                                     : MeasureTxt(textBuffer, fontSize);
            DrawTxt("|", posX + textW, posY, fontSize, COLOR_PRIMARY, true);
        }
    }
}

static void DrawCardPanel(Rectangle rec, Color bg = COLOR_CARD, Color border = COLOR_BORDER) {
    DrawRectangleRounded(rec, 0.08f, 6, bg);
    DrawRectangleRoundedLines(rec, 0.08f, 6, border);
}

// ==================== TREE & LIST EXTRACTION HELPERS ====================

static void collectSubjects(Subject* root, std::vector<Subject*>& list) {
    if (root == nullptr) return;
    collectSubjects(root->left, list);
    list.push_back(root);
    collectSubjects(root->right, list);
}

static void collectQuestions(Question* head, std::vector<Question*>& list) {
    Question* cur = head;
    while (cur != nullptr) {
        list.push_back(cur);
        cur = cur->next;
    }
}

static void collectStudents(Student* head, std::vector<Student*>& list) {
    Student* cur = head;
    while (cur != nullptr) {
        list.push_back(cur);
        cur = cur->next;
    }
}

// ==================== MAIN GRAPHIC APPLICATION ====================

void runGraphicApp() {
    const int screenWidth = 1280;
    const int screenHeight = 768;

    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "HE THONG THI TRAC NGHIEM - C++ DATA STRUCTURES & ALGORITHMS");
    SetTargetFPS(60);

    // Nạp Font chữ mượt mà chống vỡ hình (Bilinear Filtered TrueType Fonts)
    if (FileExists("assets/fonts/regular.ttf")) {
        gFontRegular = LoadFontEx("assets/fonts/regular.ttf", 48, 0, 0);
        SetTextureFilter(gFontRegular.texture, TEXTURE_FILTER_BILINEAR);
    } else {
        gFontRegular = GetFontDefault();
    }

    if (FileExists("assets/fonts/bold.ttf")) {
        gFontBold = LoadFontEx("assets/fonts/bold.ttf", 52, 0, 0);
        SetTextureFilter(gFontBold.texture, TEXTURE_FILTER_BILINEAR);
    } else {
        gFontBold = GetFontDefault();
    }

    // Dữ liệu ứng dụng
    Subject* root = nullptr;
    Class* classes[MAX_CLASS] = {};
    int classCount = 0;

    loadAllData(root, classes, classCount, "data");

    // Trạng thái chung
    ScreenState currentScreen = SCREEN_LOGIN;
    LoginRole currentRole = LOGIN_FAILED;
    Student* loggedStudent = nullptr;

    char loginUser[32] = "GV";
    char loginPass[32] = "GV";
    bool activeUserBox = false;
    bool activePassBox = false;
    char notifyMessage[128] = "";
    float notifyTimer = 0.0f;

    // Teacher navigation
    int teacherTab = 0; // 0: Mon hoc, 1: Cau hoi, 2: Lop hoc, 3: Sinh vien, 4: Bang diem, 5: Chi tiet
    int selectedSubjectIdx = 0;
    int selectedClassIdx = 0;
    int selectedStudentIdx = 0;

    // Student navigation
    int studentTab = 0; // 0: Vao thi, 1: Bang diem, 2: Xem lai bai thi
    int studentSubChoice = 0;
    char numQuestionsInput[16] = "4";
    char examMinutesInput[16] = "10";
    bool activeNumQ = false;
    bool activeMinutes = false;

    // Active exam state
    ExamDetail* examHead = nullptr;
    std::vector<ExamDetail*> examList;
    int currentExamQuestionIdx = 0;
    std::string activeExamSubId = "";
    int examRemainingSeconds = 0;
    float examTimerClock = 0.0f;

    // Exam result state
    float lastScore = 0.0f;
    int lastCorrect = 0;
    int lastTotal = 0;

    // Modal forms buffer
    ModalType currentModal = MODAL_NONE;
    char formId[32] = "";
    char formName[128] = "";
    char formHo[64] = "";
    char formTen[32] = "";
    char formGender[16] = "Nam";
    char formPass[PASSWORD_LEN] = "123";
    char formContent[QUESTION_CONTENT_LEN] = "";
    char formA[ANSWER_CONTENT_LEN] = "";
    char formB[ANSWER_CONTENT_LEN] = "";
    char formC[ANSWER_CONTENT_LEN] = "";
    char formD[ANSWER_CONTENT_LEN] = "";
    char formCorrect[8] = "A";
    int formEditQId = 0;
    int activeFormBox = 0;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        if (notifyTimer > 0) notifyTimer -= dt;

        std::vector<Subject*> allSubjects;
        collectSubjects(root, allSubjects);

        // Exam timer tick
        if (currentScreen == SCREEN_STUDENT_EXAM) {
            examTimerClock += dt;
            if (examTimerClock >= 1.0f) {
                examTimerClock -= 1.0f;
                if (examRemainingSeconds > 0) {
                    examRemainingSeconds--;
                    saveExamSession(loggedStudent->id, activeExamSubId.c_str(), examRemainingSeconds, examHead);
                } else {
                    submitExamWorkflow(loggedStudent, activeExamSubId.c_str(), examHead, lastScore, lastCorrect, lastTotal);
                    removeExamSession(loggedStudent->id);
                    examHead = nullptr;
                    examList.clear();
                    currentScreen = SCREEN_EXAM_RESULT;
                }
            }
        }

        // ==================== DRAWING ====================
        BeginDrawing();
        ClearBackground(COLOR_BG);

        // ----------------------------------------------------
        // 1. LOGIN SCREEN
        // ----------------------------------------------------
        if (currentScreen == SCREEN_LOGIN) {
            DrawRectangle(0, 0, screenWidth, 180, COLOR_SIDEBAR);
            DrawTxt("HE THONG QUAN LY VA THI TRAC NGHIEM C++", screenWidth / 2.0f - MeasureTxt("HE THONG QUAN LY VA THI TRAC NGHIEM C++", 26, true) / 2.0f, 45, 26, WHITE, true);
            DrawTxt("Data Structures & Algorithms Project (BST, Pointer Array, Linked Lists)", screenWidth / 2.0f - MeasureTxt("Data Structures & Algorithms Project (BST, Pointer Array, Linked Lists)", 17) / 2.0f, 95, 17, COLOR_TEXT_MUTED);

            Rectangle loginCard = { (float)(screenWidth / 2 - 250), 220, 500, 460 };
            DrawCardPanel(loginCard);

            DrawTxt("DANG NHAP HE THONG", loginCard.x + 30, loginCard.y + 30, 22, COLOR_TEXT_DARK, true);
            DrawLine((int)loginCard.x + 30, (int)loginCard.y + 65, (int)loginCard.x + 470, (int)loginCard.y + 65, COLOR_BORDER);

            DrawTxt("Ten dang nhap (GV hoac MASV):", loginCard.x + 30, loginCard.y + 90, 16, COLOR_TEXT_DARK);
            Rectangle userRec = { loginCard.x + 30, loginCard.y + 115, 440, 42 };
            GuiTextBoxInput(userRec, loginUser, sizeof(loginUser), activeUserBox, "Nhap GV hoac MASV (vd: SV001)");

            DrawTxt("Mat khau:", loginCard.x + 30, loginCard.y + 175, 16, COLOR_TEXT_DARK);
            Rectangle passRec = { loginCard.x + 30, loginCard.y + 200, 440, 42 };
            GuiTextBoxInput(passRec, loginPass, sizeof(loginPass), activePassBox, "Nhap mat khau", true);

            DrawTxt("Chon nhanh tai khoan mau de test:", loginCard.x + 30, loginCard.y + 260, 14, COLOR_TEXT_MUTED);
            if (GuiBtnOutline({ loginCard.x + 30, loginCard.y + 285, 210, 36 }, "Giang vien (GV / GV)", COLOR_PRIMARY, COLOR_PRIMARY, 15, true)) {
                snprintf(loginUser, sizeof(loginUser), "GV");
                snprintf(loginPass, sizeof(loginPass), "GV");
            }
            if (GuiBtnOutline({ loginCard.x + 260, loginCard.y + 285, 210, 36 }, "Sinh vien (SV001 / 123)", COLOR_SUCCESS, COLOR_SUCCESS, 15, true)) {
                snprintf(loginUser, sizeof(loginUser), "SV001");
                snprintf(loginPass, sizeof(loginPass), "123");
            }

            if (GuiBtn({ loginCard.x + 30, loginCard.y + 355, 440, 48 }, "DANG NHAP NGAY", COLOR_PRIMARY, COLOR_PRIMARY_HOVER, WHITE, 18, true)) {
                BusinessResult res = loginWorkflow(classes, classCount, loginUser, loginPass, currentRole, loggedStudent);
                if (res == SUCCESS) {
                    if (currentRole == LOGIN_TEACHER) {
                        currentScreen = SCREEN_TEACHER_DASHBOARD;
                        teacherTab = 0;
                    } else if (currentRole == LOGIN_STUDENT && loggedStudent != nullptr) {
                        currentScreen = SCREEN_STUDENT_DASHBOARD;
                        studentTab = 0;
                    }
                } else {
                    snprintf(notifyMessage, sizeof(notifyMessage), "%s", getErrorMessage(res));
                    notifyTimer = 3.0f;
                }
            }

            if (notifyTimer > 0) {
                DrawTxt(notifyMessage, loginCard.x + 30, loginCard.y + 418, 15, COLOR_DANGER);
            }
        }

        // ----------------------------------------------------
        // 2. TEACHER DASHBOARD
        // ----------------------------------------------------
        else if (currentScreen == SCREEN_TEACHER_DASHBOARD) {
            DrawRectangle(0, 0, screenWidth, 65, COLOR_SIDEBAR);
            DrawTxt("BANG DIEU KHIEN GIANG VIEN", 30, 20, 22, WHITE, true);

            if (GuiBtnOutline({ (float)(screenWidth - 260), 15, 110, 35 }, "Luu File", COLOR_SUCCESS, WHITE, 15, true)) {
                saveAllData(root, classes, classCount, "data");
                snprintf(notifyMessage, sizeof(notifyMessage), "Da luu du lieu thanh cong vao thu muc 'data'!");
                notifyTimer = 3.0f;
            }

            if (GuiBtn({ (float)(screenWidth - 130), 15, 100, 35 }, "Dang xuat", COLOR_DANGER, COLOR_DANGER_HOVER, WHITE, 15, true)) {
                currentScreen = SCREEN_LOGIN;
                currentRole = LOGIN_FAILED;
                loggedStudent = nullptr;
            }

            DrawRectangle(0, 65, 240, screenHeight - 65, COLOR_SIDEBAR);
            const char* tabs[] = {
                "Mon hoc (BST)",
                "Cau hoi thi",
                "Lop hoc",
                "Sinh vien",
                "Bang diem lop",
                "Chi tiet bai thi"
            };

            for (int i = 0; i < 6; ++i) {
                Rectangle tabRec = { 10, (float)(85 + i * 55), 220, 45 };
                bool active = (teacherTab == i);
                Color tabBg = active ? COLOR_SIDEBAR_ACTIVE : COLOR_SIDEBAR;
                if (GuiBtn(tabRec, tabs[i], tabBg, COLOR_SIDEBAR_HOVER, WHITE, 16, active)) {
                    teacherTab = i;
                }
            }

            Rectangle contentRec = { 260, 85, (float)(screenWidth - 280), (float)(screenHeight - 105) };
            DrawCardPanel(contentRec);

            // TAB 0: MON HOC
            if (teacherTab == 0) {
                DrawTxt("QUAN LY DANH MUC MON HOC (CAY NHI PHAN TIM KIEM BST)", contentRec.x + 25, contentRec.y + 25, 20, COLOR_TEXT_DARK, true);
                if (GuiBtn({ contentRec.x + contentRec.width - 180, contentRec.y + 20, 155, 38 }, "+ Them Mon Hoc", COLOR_SUCCESS, COLOR_SUCCESS_HOVER, WHITE, 15, true)) {
                    currentModal = MODAL_ADD_SUBJECT;
                    formId[0] = '\0';
                    formName[0] = '\0';
                }

                int tableY = (int)contentRec.y + 80;
                DrawRectangle((int)contentRec.x + 25, tableY, (int)contentRec.width - 50, 40, COLOR_SIDEBAR);
                DrawTxt("STT", contentRec.x + 40, (float)tableY + 11, 16, WHITE, true);
                DrawTxt("MA MON HOC (Key BST)", contentRec.x + 100, (float)tableY + 11, 16, WHITE, true);
                DrawTxt("TEN MON HOC", contentRec.x + 350, (float)tableY + 11, 16, WHITE, true);
                DrawTxt("SO CAU HOI", contentRec.x + 680, (float)tableY + 11, 16, WHITE, true);
                DrawTxt("THAO TAC", contentRec.x + 820, (float)tableY + 11, 16, WHITE, true);

                for (size_t i = 0; i < allSubjects.size() && i < 10; ++i) {
                    int rowY = tableY + 45 + (int)i * 45;
                    Color rowBg = (i % 2 == 0) ? COLOR_CARD : MakeColor(248, 250, 252);
                    DrawRectangle((int)contentRec.x + 25, rowY, (int)contentRec.width - 50, 42, rowBg);
                    DrawRectangleLines((int)contentRec.x + 25, rowY, (int)contentRec.width - 50, 42, COLOR_BORDER);

                    DrawTxt(TextFormat("%d", (int)i + 1), contentRec.x + 45, (float)rowY + 11, 16, COLOR_TEXT_DARK);
                    DrawTxt(allSubjects[i]->id, contentRec.x + 100, (float)rowY + 11, 16, COLOR_PRIMARY, true);
                    DrawTxt(allSubjects[i]->name, contentRec.x + 350, (float)rowY + 11, 16, COLOR_TEXT_DARK);

                    int qCount = countQuestions(allSubjects[i]->questionList);
                    DrawTxt(TextFormat("%d cau", qCount), contentRec.x + 690, (float)rowY + 11, 16, COLOR_TEXT_MUTED);

                    if (GuiBtnOutline({ contentRec.x + 810, (float)rowY + 6, 65, 30 }, "Sua", COLOR_PRIMARY, COLOR_PRIMARY, 14)) {
                        currentModal = MODAL_EDIT_SUBJECT;
                        snprintf(formId, sizeof(formId), "%s", allSubjects[i]->id);
                        snprintf(formName, sizeof(formName), "%s", allSubjects[i]->name);
                    }
                    if (GuiBtnOutline({ contentRec.x + 885, (float)rowY + 6, 65, 30 }, "Xoa", COLOR_DANGER, COLOR_DANGER, 14)) {
                        currentModal = MODAL_DELETE_SUBJECT;
                        snprintf(formId, sizeof(formId), "%s", allSubjects[i]->id);
                        snprintf(formName, sizeof(formName), "%s", allSubjects[i]->name);
                    }
                }
            }

            // TAB 1: CAU HOI THI
            else if (teacherTab == 1) {
                DrawTxt("NGAN HANG CAU HOI THI TRAC NGHIEM", contentRec.x + 25, contentRec.y + 25, 20, COLOR_TEXT_DARK, true);

                DrawTxt("Chon Mon Hoc:", contentRec.x + 25, contentRec.y + 68, 16, COLOR_TEXT_DARK, true);
                for (size_t i = 0; i < allSubjects.size() && i < 5; ++i) {
                    bool sel = (selectedSubjectIdx == (int)i);
                    Rectangle btnRec = { contentRec.x + 150 + i * 140, contentRec.y + 62, 130, 34 };
                    if (GuiBtn(btnRec, allSubjects[i]->id, sel ? COLOR_PRIMARY : COLOR_SIDEBAR, COLOR_PRIMARY_HOVER, WHITE, 14, sel)) {
                        selectedSubjectIdx = (int)i;
                    }
                }

                if (!allSubjects.empty() && selectedSubjectIdx < (int)allSubjects.size()) {
                    Subject* curSub = allSubjects[selectedSubjectIdx];
                    if (GuiBtn({ contentRec.x + contentRec.width - 180, contentRec.y + 62, 155, 34 }, "+ Them Cau Hoi", COLOR_SUCCESS, COLOR_SUCCESS_HOVER, WHITE, 14, true)) {
                        currentModal = MODAL_ADD_QUESTION;
                        formContent[0] = '\0';
                        formA[0] = '\0';
                        formB[0] = '\0';
                        formC[0] = '\0';
                        formD[0] = '\0';
                        formCorrect[0] = 'A';
                    }

                    std::vector<Question*> qList;
                    collectQuestions(curSub->questionList, qList);

                    DrawTxt(TextFormat("Tong so: %d cau hoi", (int)qList.size()), contentRec.x + 25, contentRec.y + 112, 16, COLOR_TEXT_MUTED);

                    int startY = (int)contentRec.y + 140;
                    for (size_t i = 0; i < qList.size() && i < 3; ++i) {
                        Rectangle qCard = { contentRec.x + 25, (float)startY + i * 155, contentRec.width - 50, 145 };
                        DrawCardPanel(qCard, MakeColor(248, 250, 252));

                        DrawTxt(TextFormat("ID %d: %s", qList[i]->id, qList[i]->content), qCard.x + 15, qCard.y + 15, 17, COLOR_TEXT_DARK, true);
                        DrawTxt(TextFormat("A. %s", qList[i]->answerA), qCard.x + 25, qCard.y + 45, 15, (qList[i]->correctAnswer == 'A') ? COLOR_SUCCESS : COLOR_TEXT_MUTED);
                        DrawTxt(TextFormat("B. %s", qList[i]->answerB), qCard.x + 25, qCard.y + 70, 15, (qList[i]->correctAnswer == 'B') ? COLOR_SUCCESS : COLOR_TEXT_MUTED);
                        DrawTxt(TextFormat("C. %s", qList[i]->answerC), qCard.x + 450, qCard.y + 45, 15, (qList[i]->correctAnswer == 'C') ? COLOR_SUCCESS : COLOR_TEXT_MUTED);
                        DrawTxt(TextFormat("D. %s", qList[i]->answerD), qCard.x + 450, qCard.y + 70, 15, (qList[i]->correctAnswer == 'D') ? COLOR_SUCCESS : COLOR_TEXT_MUTED);

                        DrawTxt(TextFormat("Dap an dung: %c", qList[i]->correctAnswer), qCard.x + 25, qCard.y + 105, 16, COLOR_SUCCESS, true);

                        if (GuiBtnOutline({ qCard.x + qCard.width - 150, qCard.y + 100, 65, 30 }, "Sua", COLOR_PRIMARY, COLOR_PRIMARY, 14)) {
                            currentModal = MODAL_EDIT_QUESTION;
                            formEditQId = qList[i]->id;
                            snprintf(formContent, sizeof(formContent), "%s", qList[i]->content);
                            snprintf(formA, sizeof(formA), "%s", qList[i]->answerA);
                            snprintf(formB, sizeof(formB), "%s", qList[i]->answerB);
                            snprintf(formC, sizeof(formC), "%s", qList[i]->answerC);
                            snprintf(formD, sizeof(formD), "%s", qList[i]->answerD);
                            formCorrect[0] = qList[i]->correctAnswer;
                        }
                        if (GuiBtnOutline({ qCard.x + qCard.width - 75, qCard.y + 100, 65, 30 }, "Xoa", COLOR_DANGER, COLOR_DANGER, 14)) {
                            currentModal = MODAL_DELETE_QUESTION;
                            formEditQId = qList[i]->id;
                        }
                    }
                }
            }

            // TAB 2: LOP HOC
            else if (teacherTab == 2) {
                DrawTxt("QUAN LY DANH MUC LOP HOC (MANG CON TRO)", contentRec.x + 25, contentRec.y + 25, 20, COLOR_TEXT_DARK, true);
                if (GuiBtn({ contentRec.x + contentRec.width - 180, contentRec.y + 20, 155, 38 }, "+ Them Lop Hoc", COLOR_SUCCESS, COLOR_SUCCESS_HOVER, WHITE, 15, true)) {
                    currentModal = MODAL_ADD_CLASS;
                    formId[0] = '\0';
                    formName[0] = '\0';
                }

                int tableY = (int)contentRec.y + 80;
                DrawRectangle((int)contentRec.x + 25, tableY, (int)contentRec.width - 50, 40, COLOR_SIDEBAR);
                DrawTxt("STT", contentRec.x + 40, (float)tableY + 11, 16, WHITE, true);
                DrawTxt("MA LOP", contentRec.x + 100, (float)tableY + 11, 16, WHITE, true);
                DrawTxt("TEN LOP", contentRec.x + 350, (float)tableY + 11, 16, WHITE, true);
                DrawTxt("SI SO", contentRec.x + 680, (float)tableY + 11, 16, WHITE, true);
                DrawTxt("THAO TAC", contentRec.x + 820, (float)tableY + 11, 16, WHITE, true);

                for (int i = 0; i < classCount && i < 10; ++i) {
                    if (classes[i] == nullptr) continue;
                    int rowY = tableY + 45 + i * 45;
                    Color rowBg = (i % 2 == 0) ? COLOR_CARD : MakeColor(248, 250, 252);
                    DrawRectangle((int)contentRec.x + 25, rowY, (int)contentRec.width - 50, 42, rowBg);
                    DrawRectangleLines((int)contentRec.x + 25, rowY, (int)contentRec.width - 50, 42, COLOR_BORDER);

                    DrawTxt(TextFormat("%d", i + 1), contentRec.x + 45, (float)rowY + 11, 16, COLOR_TEXT_DARK);
                    DrawTxt(classes[i]->id, contentRec.x + 100, (float)rowY + 11, 16, COLOR_PRIMARY, true);
                    DrawTxt(classes[i]->name, contentRec.x + 350, (float)rowY + 11, 16, COLOR_TEXT_DARK);

                    int count = 0;
                    Student* s = classes[i]->studentList;
                    while (s != nullptr) { count++; s = s->next; }
                    DrawTxt(TextFormat("%d SV", count), contentRec.x + 690, (float)rowY + 11, 16, COLOR_TEXT_MUTED);

                    if (GuiBtnOutline({ contentRec.x + 810, (float)rowY + 6, 75, 30 }, "Xoa", COLOR_DANGER, COLOR_DANGER, 14)) {
                        currentModal = MODAL_DELETE_CLASS;
                        snprintf(formId, sizeof(formId), "%s", classes[i]->id);
                        snprintf(formName, sizeof(formName), "%s", classes[i]->name);
                    }
                }
            }

            // TAB 3: SINH VIEN
            else if (teacherTab == 3) {
                DrawTxt("QUAN LY SINH VIEN THEO LOP (DANH SACH LIEN KET)", contentRec.x + 25, contentRec.y + 25, 20, COLOR_TEXT_DARK, true);

                DrawTxt("Chon Lop:", contentRec.x + 25, contentRec.y + 68, 16, COLOR_TEXT_DARK, true);
                for (int i = 0; i < classCount && i < 5; ++i) {
                    if (classes[i] == nullptr) continue;
                    bool sel = (selectedClassIdx == i);
                    Rectangle btnRec = { contentRec.x + 120 + i * 150, contentRec.y + 62, 140, 34 };
                    if (GuiBtn(btnRec, classes[i]->id, sel ? COLOR_PRIMARY : COLOR_SIDEBAR, COLOR_PRIMARY_HOVER, WHITE, 14, sel)) {
                        selectedClassIdx = i;
                    }
                }

                if (classCount > 0 && selectedClassIdx < classCount && classes[selectedClassIdx] != nullptr) {
                    Class* curClass = classes[selectedClassIdx];
                    if (GuiBtn({ contentRec.x + contentRec.width - 180, contentRec.y + 62, 155, 34 }, "+ Them Sinh Vien", COLOR_SUCCESS, COLOR_SUCCESS_HOVER, WHITE, 14, true)) {
                        currentModal = MODAL_ADD_STUDENT;
                        formId[0] = '\0';
                        formHo[0] = '\0';
                        formTen[0] = '\0';
                        snprintf(formGender, sizeof(formGender), "Nam");
                        snprintf(formPass, sizeof(formPass), "123");
                    }

                    std::vector<Student*> sList;
                    collectStudents(curClass->studentList, sList);

                    int tableY = (int)contentRec.y + 115;
                    DrawRectangle((int)contentRec.x + 25, tableY, (int)contentRec.width - 50, 40, COLOR_SIDEBAR);
                    DrawTxt("STT", contentRec.x + 40, (float)tableY + 11, 16, WHITE, true);
                    DrawTxt("MASV", contentRec.x + 100, (float)tableY + 11, 16, WHITE, true);
                    DrawTxt("HO VA TEN", contentRec.x + 250, (float)tableY + 11, 16, WHITE, true);
                    DrawTxt("PHAI", contentRec.x + 550, (float)tableY + 11, 16, WHITE, true);
                    DrawTxt("MAT KHAU", contentRec.x + 680, (float)tableY + 11, 16, WHITE, true);
                    DrawTxt("THAO TAC", contentRec.x + 820, (float)tableY + 11, 16, WHITE, true);

                    for (size_t i = 0; i < sList.size() && i < 10; ++i) {
                        int rowY = tableY + 45 + (int)i * 45;
                        Color rowBg = (i % 2 == 0) ? COLOR_CARD : MakeColor(248, 250, 252);
                        DrawRectangle((int)contentRec.x + 25, rowY, (int)contentRec.width - 50, 42, rowBg);
                        DrawRectangleLines((int)contentRec.x + 25, rowY, (int)contentRec.width - 50, 42, COLOR_BORDER);

                        DrawTxt(TextFormat("%d", (int)i + 1), contentRec.x + 45, (float)rowY + 11, 16, COLOR_TEXT_DARK);
                        DrawTxt(sList[i]->id, contentRec.x + 100, (float)rowY + 11, 16, COLOR_PRIMARY, true);
                        DrawTxt(TextFormat("%s %s", sList[i]->ho, sList[i]->ten), contentRec.x + 250, (float)rowY + 11, 16, COLOR_TEXT_DARK);
                        DrawTxt(sList[i]->gender, contentRec.x + 550, (float)rowY + 11, 16, COLOR_TEXT_MUTED);
                        DrawTxt("******", contentRec.x + 680, (float)rowY + 11, 16, COLOR_TEXT_MUTED);

                        if (GuiBtnOutline({ contentRec.x + 810, (float)rowY + 6, 65, 30 }, "Sua", COLOR_PRIMARY, COLOR_PRIMARY, 14)) {
                            currentModal = MODAL_EDIT_STUDENT;
                            snprintf(formId, sizeof(formId), "%s", sList[i]->id);
                            snprintf(formHo, sizeof(formHo), "%s", sList[i]->ho);
                            snprintf(formTen, sizeof(formTen), "%s", sList[i]->ten);
                            snprintf(formGender, sizeof(formGender), "%s", sList[i]->gender);
                            snprintf(formPass, sizeof(formPass), "%s", sList[i]->password);
                        }
                        if (GuiBtnOutline({ contentRec.x + 885, (float)rowY + 6, 65, 30 }, "Xoa", COLOR_DANGER, COLOR_DANGER, 14)) {
                            currentModal = MODAL_DELETE_STUDENT;
                            snprintf(formId, sizeof(formId), "%s", sList[i]->id);
                        }
                    }
                }
            }

            // TAB 4: BANG DIEM LOP THEO MON
            else if (teacherTab == 4) {
                DrawTxt("BANG DIEM MON HOC THEO LOP", contentRec.x + 25, contentRec.y + 25, 20, COLOR_TEXT_DARK, true);

                DrawTxt("Lop:", contentRec.x + 25, contentRec.y + 68, 16, COLOR_TEXT_DARK, true);
                for (int i = 0; i < classCount && i < 4; ++i) {
                    if (classes[i] == nullptr) continue;
                    bool sel = (selectedClassIdx == i);
                    if (GuiBtn({ contentRec.x + 75 + i * 130, contentRec.y + 62, 120, 32 }, classes[i]->id, sel ? COLOR_PRIMARY : COLOR_SIDEBAR, COLOR_PRIMARY_HOVER, WHITE, 14, sel)) {
                        selectedClassIdx = i;
                    }
                }

                DrawTxt("Mon:", contentRec.x + 620, contentRec.y + 68, 16, COLOR_TEXT_DARK, true);
                for (size_t i = 0; i < allSubjects.size() && i < 4; ++i) {
                    bool sel = (selectedSubjectIdx == (int)i);
                    if (GuiBtn({ contentRec.x + 670 + i * 110, contentRec.y + 62, 100, 32 }, allSubjects[i]->id, sel ? COLOR_PRIMARY : COLOR_SIDEBAR, COLOR_PRIMARY_HOVER, WHITE, 14, sel)) {
                        selectedSubjectIdx = (int)i;
                    }
                }

                if (classCount > 0 && selectedClassIdx < classCount && classes[selectedClassIdx] != nullptr && !allSubjects.empty() && selectedSubjectIdx < (int)allSubjects.size()) {
                    Class* curC = classes[selectedClassIdx];
                    Subject* curS = allSubjects[selectedSubjectIdx];

                    std::vector<Student*> sList;
                    collectStudents(curC->studentList, sList);

                    int tableY = (int)contentRec.y + 115;
                    DrawRectangle((int)contentRec.x + 25, tableY, (int)contentRec.width - 50, 40, COLOR_SIDEBAR);
                    DrawTxt("STT", contentRec.x + 40, (float)tableY + 11, 16, WHITE, true);
                    DrawTxt("MASV", contentRec.x + 120, (float)tableY + 11, 16, WHITE, true);
                    DrawTxt("HO VA TEN", contentRec.x + 320, (float)tableY + 11, 16, WHITE, true);
                    DrawTxt("DIEM SO", contentRec.x + 750, (float)tableY + 11, 16, WHITE, true);

                    for (size_t i = 0; i < sList.size() && i < 10; ++i) {
                        int rowY = tableY + 45 + (int)i * 45;
                        Color rowBg = (i % 2 == 0) ? COLOR_CARD : MakeColor(248, 250, 252);
                        DrawRectangle((int)contentRec.x + 25, rowY, (int)contentRec.width - 50, 42, rowBg);
                        DrawRectangleLines((int)contentRec.x + 25, rowY, (int)contentRec.width - 50, 42, COLOR_BORDER);

                        DrawTxt(TextFormat("%d", (int)i + 1), contentRec.x + 45, (float)rowY + 11, 16, COLOR_TEXT_DARK);
                        DrawTxt(sList[i]->id, contentRec.x + 120, (float)rowY + 11, 16, COLOR_PRIMARY, true);
                        DrawTxt(TextFormat("%s %s", sList[i]->ho, sList[i]->ten), contentRec.x + 320, (float)rowY + 11, 16, COLOR_TEXT_DARK);

                        float sc = 0.0f;
                        if (hasStudentTakenExam(sList[i], curS->id, sc)) {
                            DrawTxt(TextFormat("%.2f", sc), contentRec.x + 750, (float)rowY + 11, 18, COLOR_SUCCESS, true);
                        } else {
                            DrawTxt("Chua thi", contentRec.x + 750, (float)rowY + 11, 16, COLOR_TEXT_MUTED);
                        }
                    }
                }
            }

            // TAB 5: CHI TIET BAI THI
            else if (teacherTab == 5) {
                DrawTxt("XEM LAI CHI TIET BAI THI CUA SINH VIEN", contentRec.x + 25, contentRec.y + 25, 20, COLOR_TEXT_DARK, true);

                if (classCount > 0 && selectedClassIdx < classCount && classes[selectedClassIdx] != nullptr) {
                    Class* curC = classes[selectedClassIdx];
                    std::vector<Student*> sList;
                    collectStudents(curC->studentList, sList);

                    DrawTxt("Chon SV:", contentRec.x + 25, contentRec.y + 68, 16, COLOR_TEXT_DARK, true);
                    for (size_t i = 0; i < sList.size() && i < 4; ++i) {
                        bool sel = (selectedStudentIdx == (int)i);
                        if (GuiBtn({ contentRec.x + 110 + i * 130, contentRec.y + 62, 120, 32 }, sList[i]->id, sel ? COLOR_PRIMARY : COLOR_SIDEBAR, COLOR_PRIMARY_HOVER, WHITE, 14, sel)) {
                            selectedStudentIdx = (int)i;
                        }
                    }

                    DrawTxt("Mon:", contentRec.x + 650, contentRec.y + 68, 16, COLOR_TEXT_DARK, true);
                    for (size_t i = 0; i < allSubjects.size() && i < 3; ++i) {
                        bool sel = (selectedSubjectIdx == (int)i);
                        if (GuiBtn({ contentRec.x + 700 + i * 100, contentRec.y + 62, 90, 32 }, allSubjects[i]->id, sel ? COLOR_PRIMARY : COLOR_SIDEBAR, COLOR_PRIMARY_HOVER, WHITE, 14, sel)) {
                            selectedSubjectIdx = (int)i;
                        }
                    }

                    if (!sList.empty() && selectedStudentIdx < (int)sList.size() && !allSubjects.empty() && selectedSubjectIdx < (int)allSubjects.size()) {
                        Student* curStd = sList[selectedStudentIdx];
                        Subject* curSub = allSubjects[selectedSubjectIdx];
                        Score* scNode = findScore(curStd, curSub->id);

                        if (scNode == nullptr) {
                            DrawTxt(TextFormat("Sinh vien %s chua lam bai thi mon %s.", curStd->id, curSub->id), contentRec.x + 25, contentRec.y + 130, 18, COLOR_WARNING, true);
                        } else {
                            DrawTxt(TextFormat("Ket qua: %.2f diem", scNode->score), contentRec.x + 25, contentRec.y + 115, 18, COLOR_SUCCESS, true);

                            ExamDetail* d = scNode->detailList;
                            int startY = (int)contentRec.y + 150;
                            int qNo = 1;
                            while (d != nullptr && qNo <= 3) {
                                Rectangle dCard = { contentRec.x + 25, (float)startY + (qNo - 1) * 140, contentRec.width - 50, 130 };
                                DrawCardPanel(dCard, MakeColor(248, 250, 252));

                                DrawTxt(TextFormat("Cau %d: %s", qNo, d->content), dCard.x + 15, dCard.y + 12, 16, COLOR_TEXT_DARK, true);
                                DrawTxt(TextFormat("A. %s", d->answerA), dCard.x + 25, dCard.y + 40, 15, COLOR_TEXT_MUTED);
                                DrawTxt(TextFormat("B. %s", d->answerB), dCard.x + 25, dCard.y + 65, 15, COLOR_TEXT_MUTED);
                                DrawTxt(TextFormat("C. %s", d->answerC), dCard.x + 450, dCard.y + 40, 15, COLOR_TEXT_MUTED);
                                DrawTxt(TextFormat("D. %s", d->answerD), dCard.x + 450, dCard.y + 65, 15, COLOR_TEXT_MUTED);

                                bool isCorrect = (d->selectedAnswer == d->correctAnswer);
                                DrawTxt(TextFormat("Ban chon: %c | Dap an dung: %c", d->selectedAnswer ? d->selectedAnswer : '-', d->correctAnswer), dCard.x + 25, dCard.y + 95, 15, isCorrect ? COLOR_SUCCESS : COLOR_DANGER, true);
                                DrawTxt(isCorrect ? "[ DUNG ]" : "[ SAI ]", dCard.x + 350, dCard.y + 95, 15, isCorrect ? COLOR_SUCCESS : COLOR_DANGER, true);

                                d = d->next;
                                qNo++;
                            }
                        }
                    }
                }
            }
        }

        // ----------------------------------------------------
        // 3. STUDENT DASHBOARD
        // ----------------------------------------------------
        else if (currentScreen == SCREEN_STUDENT_DASHBOARD) {
            DrawRectangle(0, 0, screenWidth, 65, COLOR_SIDEBAR);
            DrawTxt(TextFormat("SINH VIEN: %s - %s %s", loggedStudent->id, loggedStudent->ho, loggedStudent->ten), 30, 20, 22, WHITE, true);

            if (GuiBtn({ (float)(screenWidth - 130), 15, 100, 35 }, "Dang xuat", COLOR_DANGER, COLOR_DANGER_HOVER, WHITE, 15, true)) {
                currentScreen = SCREEN_LOGIN;
                currentRole = LOGIN_FAILED;
                loggedStudent = nullptr;
            }

            DrawRectangle(0, 65, 240, screenHeight - 65, COLOR_SIDEBAR);
            const char* stdTabs[] = {
                "Vao thi trac nghiem",
                "Bang diem ca nhan",
                "Xem lai bai thi"
            };

            for (int i = 0; i < 3; ++i) {
                Rectangle tabRec = { 10, (float)(85 + i * 55), 220, 45 };
                bool active = (studentTab == i);
                Color tabBg = active ? COLOR_SIDEBAR_ACTIVE : COLOR_SIDEBAR;
                if (GuiBtn(tabRec, stdTabs[i], tabBg, COLOR_SIDEBAR_HOVER, WHITE, 16, active)) {
                    studentTab = i;
                }
            }

            Rectangle contentRec = { 260, 85, (float)(screenWidth - 280), (float)(screenHeight - 105) };
            DrawCardPanel(contentRec);

            // TAB 0: VAO THI
            if (studentTab == 0) {
                DrawTxt("CHUAN BI THI TRAC NGHIEM", contentRec.x + 30, contentRec.y + 30, 22, COLOR_TEXT_DARK, true);

                char pendSub[32] = {};
                int pendSec = 0;
                if (checkExamSession(loggedStudent->id, pendSub, pendSec)) {
                    Rectangle warnCard = { contentRec.x + 30, contentRec.y + 70, contentRec.width - 60, 90 };
                    DrawCardPanel(warnCard, MakeColor(254, 243, 199), COLOR_WARNING);
                    DrawTxt(TextFormat("[CANH BAO] Ban dang co phien thi do dang mon: %s (con %d phut %d giay)", pendSub, pendSec / 60, pendSec % 60), warnCard.x + 20, warnCard.y + 18, 17, COLOR_WARNING, true);

                    if (GuiBtn({ warnCard.x + 20, warnCard.y + 48, 230, 34 }, ">> TIEP TUC PHIEN THI <<", COLOR_PRIMARY, COLOR_PRIMARY_HOVER, WHITE, 15, true)) {
                        if (loadExamSession(loggedStudent->id, pendSub, pendSec, examHead, root)) {
                            examList.clear();
                            ExamDetail* cur = examHead;
                            while (cur != nullptr) { examList.push_back(cur); cur = cur->next; }
                            activeExamSubId = pendSub;
                            examRemainingSeconds = pendSec;
                            currentExamQuestionIdx = 0;
                            currentScreen = SCREEN_STUDENT_EXAM;
                        }
                    }
                    if (GuiBtnOutline({ warnCard.x + 270, warnCard.y + 48, 160, 34 }, "Huy phien thi do dang", COLOR_DANGER, COLOR_DANGER, 14, true)) {
                        removeExamSession(loggedStudent->id);
                    }
                }

                int formStartY = (int)contentRec.y + 180;
                DrawTxt("1. Chon mon hoc:", contentRec.x + 30, (float)formStartY, 18, COLOR_TEXT_DARK, true);
                for (size_t i = 0; i < allSubjects.size() && i < 4; ++i) {
                    bool sel = (studentSubChoice == (int)i);
                    Rectangle btnRec = { contentRec.x + 30 + i * 160, (float)formStartY + 30, 150, 42 };
                    if (GuiBtn(btnRec, allSubjects[i]->id, sel ? COLOR_PRIMARY : COLOR_SIDEBAR, COLOR_PRIMARY_HOVER, WHITE, 16, sel)) {
                        studentSubChoice = (int)i;
                    }
                }

                if (!allSubjects.empty() && studentSubChoice < (int)allSubjects.size()) {
                    Subject* chosenSub = allSubjects[studentSubChoice];
                    int totalQ = countQuestions(chosenSub->questionList);
                    DrawTxt(TextFormat("-> Mon: %s | Ngan hang co: %d cau hoi", chosenSub->name, totalQ), contentRec.x + 30, (float)formStartY + 90, 16, COLOR_TEXT_MUTED);

                    DrawTxt("2. So luong cau hoi thi:", contentRec.x + 30, (float)formStartY + 130, 18, COLOR_TEXT_DARK, true);
                    Rectangle numQRec = { contentRec.x + 30, (float)formStartY + 160, 200, 42 };
                    GuiTextBoxInput(numQRec, numQuestionsInput, sizeof(numQuestionsInput), activeNumQ, "Nhap so cau");

                    DrawTxt("3. Thoi gian thi (phut):", contentRec.x + 280, (float)formStartY + 130, 18, COLOR_TEXT_DARK, true);
                    Rectangle minRec = { contentRec.x + 280, (float)formStartY + 160, 200, 42 };
                    GuiTextBoxInput(minRec, examMinutesInput, sizeof(examMinutesInput), activeMinutes, "Nhap so phut");

                    if (GuiBtn({ contentRec.x + 30, (float)formStartY + 240, 320, 52 }, ">> BAT DAU THI NGAY <<", COLOR_SUCCESS, COLOR_SUCCESS_HOVER, WHITE, 20, true)) {
                        int numQ = atoi(numQuestionsInput);
                        int mins = atoi(examMinutesInput);

                        BusinessResult prepRes = prepareExamWorkflow(root, loggedStudent, chosenSub->id, numQ, mins, examHead);
                        if (prepRes == SUCCESS) {
                            examList.clear();
                            ExamDetail* cur = examHead;
                            while (cur != nullptr) { examList.push_back(cur); cur = cur->next; }
                            activeExamSubId = chosenSub->id;
                            examRemainingSeconds = mins * 60;
                            currentExamQuestionIdx = 0;
                            currentScreen = SCREEN_STUDENT_EXAM;
                        } else {
                            snprintf(notifyMessage, sizeof(notifyMessage), "Loi: %s", getErrorMessage(prepRes));
                            notifyTimer = 3.0f;
                        }
                    }

                    if (notifyTimer > 0) {
                        DrawTxt(notifyMessage, contentRec.x + 30, (float)formStartY + 310, 16, COLOR_DANGER, true);
                    }
                }
            }

            // TAB 1: BANG DIEM CA NHAN
            else if (studentTab == 1) {
                DrawTxt("KET QUA THI CAC MON HOC", contentRec.x + 30, contentRec.y + 30, 22, COLOR_TEXT_DARK, true);

                int tableY = (int)contentRec.y + 80;
                DrawRectangle((int)contentRec.x + 30, tableY, (int)contentRec.width - 60, 40, COLOR_SIDEBAR);
                DrawTxt("MA MON HOC", contentRec.x + 50, (float)tableY + 11, 16, WHITE, true);
                DrawTxt("TEN MON HOC", contentRec.x + 250, (float)tableY + 11, 16, WHITE, true);
                DrawTxt("DIEM THI (Thang 10)", contentRec.x + 700, (float)tableY + 11, 16, WHITE, true);

                Score* sc = loggedStudent->scoreList;
                int idx = 0;
                while (sc != nullptr) {
                    int rowY = tableY + 45 + idx * 45;
                    Color rowBg = (idx % 2 == 0) ? COLOR_CARD : MakeColor(248, 250, 252);
                    DrawRectangle((int)contentRec.x + 30, rowY, (int)contentRec.width - 60, 42, rowBg);
                    DrawRectangleLines((int)contentRec.x + 30, rowY, (int)contentRec.width - 60, 42, COLOR_BORDER);

                    DrawTxt(sc->subjectId, contentRec.x + 50, (float)rowY + 11, 16, COLOR_PRIMARY, true);
                    Subject* sNode = findSubject(root, sc->subjectId);
                    DrawTxt(sNode ? sNode->name : "Mon hoc", contentRec.x + 250, (float)rowY + 11, 16, COLOR_TEXT_DARK);
                    DrawTxt(TextFormat("%.2f diem", sc->score), contentRec.x + 700, (float)rowY + 11, 18, COLOR_SUCCESS, true);

                    sc = sc->next;
                    idx++;
                }

                if (idx == 0) {
                    DrawTxt("Ban chua co diem mon nao trong he thong.", contentRec.x + 50, (float)tableY + 60, 18, COLOR_TEXT_MUTED);
                }
            }

            // TAB 2: XEM LAI BAI THI
            else if (studentTab == 2) {
                DrawTxt("XEM LAI BAI THI DA HOAN THANH", contentRec.x + 30, contentRec.y + 30, 22, COLOR_TEXT_DARK, true);

                Score* sc = loggedStudent->scoreList;
                int sIdx = 0;
                DrawTxt("Chon Mon:", contentRec.x + 30, contentRec.y + 70, 16, COLOR_TEXT_DARK, true);
                while (sc != nullptr && sIdx < 4) {
                    bool sel = (studentSubChoice == sIdx);
                    if (GuiBtn({ contentRec.x + 120 + sIdx * 120, contentRec.y + 65, 110, 32 }, sc->subjectId, sel ? COLOR_PRIMARY : COLOR_SIDEBAR, COLOR_PRIMARY_HOVER, WHITE, 14, sel)) {
                        studentSubChoice = sIdx;
                    }
                    sc = sc->next;
                    sIdx++;
                }

                sc = loggedStudent->scoreList;
                int curIdx = 0;
                while (sc != nullptr && curIdx < studentSubChoice) {
                    sc = sc->next;
                    curIdx++;
                }

                if (sc != nullptr) {
                    DrawTxt(TextFormat("Diem so dat duoc: %.2f / 10.0", sc->score), contentRec.x + 30, contentRec.y + 115, 18, COLOR_SUCCESS, true);

                    ExamDetail* d = sc->detailList;
                    int startY = (int)contentRec.y + 150;
                    int qNo = 1;
                    while (d != nullptr && qNo <= 3) {
                        Rectangle dCard = { contentRec.x + 30, (float)startY + (qNo - 1) * 140, contentRec.width - 60, 130 };
                        DrawCardPanel(dCard, MakeColor(248, 250, 252));

                        DrawTxt(TextFormat("Cau %d: %s", qNo, d->content), dCard.x + 15, dCard.y + 12, 16, COLOR_TEXT_DARK, true);
                        DrawTxt(TextFormat("A. %s", d->answerA), dCard.x + 25, dCard.y + 40, 15, COLOR_TEXT_MUTED);
                        DrawTxt(TextFormat("B. %s", d->answerB), dCard.x + 25, dCard.y + 65, 15, COLOR_TEXT_MUTED);
                        DrawTxt(TextFormat("C. %s", d->answerC), dCard.x + 450, dCard.y + 40, 15, COLOR_TEXT_MUTED);
                        DrawTxt(TextFormat("D. %s", d->answerD), dCard.x + 450, dCard.y + 65, 15, COLOR_TEXT_MUTED);

                        bool isCorrect = (d->selectedAnswer == d->correctAnswer);
                        DrawTxt(TextFormat("Ban chon: %c | Dap an dung: %c", d->selectedAnswer ? d->selectedAnswer : '-', d->correctAnswer), dCard.x + 25, dCard.y + 95, 15, isCorrect ? COLOR_SUCCESS : COLOR_DANGER, true);
                        DrawTxt(isCorrect ? "[ CHINH XAC ]" : "[ SAI ]", dCard.x + 350, dCard.y + 95, 15, isCorrect ? COLOR_SUCCESS : COLOR_DANGER, true);

                        d = d->next;
                        qNo++;
                    }
                }
            }
        }

        // ----------------------------------------------------
        // 4. SCREEN STUDENT EXAM (MAN HINH LAM BAI THI)
        // ----------------------------------------------------
        else if (currentScreen == SCREEN_STUDENT_EXAM && !examList.empty()) {
            DrawRectangle(0, 0, screenWidth, 70, COLOR_SIDEBAR);
            DrawTxt(TextFormat("BAI THI MON: %s", activeExamSubId.c_str()), 30, 22, 22, WHITE, true);

            int mins = examRemainingSeconds / 60;
            int secs = examRemainingSeconds % 60;
            Rectangle timerRec = { (float)(screenWidth / 2 - 120), 12, 240, 46 };
            DrawRectangleRounded(timerRec, 0.2f, 4, (examRemainingSeconds < 60) ? COLOR_DANGER : COLOR_PRIMARY);
            DrawTxt(TextFormat("THOI GIAN: %02d:%02d", mins, secs), timerRec.x + 30, timerRec.y + 12, 20, WHITE, true);

            if (GuiBtn({ (float)(screenWidth - 160), 15, 130, 40 }, "NOP BAI", COLOR_WARNING, COLOR_PRIMARY_HOVER, WHITE, 18, true)) {
                currentModal = MODAL_CONFIRM_SUBMIT;
            }

            Rectangle qPane = { 30, 95, 840, (float)(screenHeight - 125) };
            DrawCardPanel(qPane);

            ExamDetail* curQ = examList[currentExamQuestionIdx];
            DrawTxt(TextFormat("CAU HOI %d / %d", currentExamQuestionIdx + 1, (int)examList.size()), qPane.x + 30, qPane.y + 25, 20, COLOR_PRIMARY, true);
            DrawLine((int)qPane.x + 30, (int)qPane.y + 55, (int)qPane.x + (int)qPane.width - 30, (int)qPane.y + 55, COLOR_BORDER);

            DrawTxt(curQ->content, qPane.x + 30, qPane.y + 75, 20, COLOR_TEXT_DARK, true);

            struct AnsChoice { char letter; const char* text; };
            AnsChoice choices[4] = {
                { 'A', curQ->answerA },
                { 'B', curQ->answerB },
                { 'C', curQ->answerC },
                { 'D', curQ->answerD }
            };

            for (int i = 0; i < 4; ++i) {
                Rectangle ansRec = { qPane.x + 30, (float)(qPane.y + 130 + i * 85), qPane.width - 60, 68 };
                bool isSelected = (curQ->selectedAnswer == choices[i].letter);

                Color btnBg = isSelected ? MakeColor(219, 234, 254) : COLOR_CARD;
                Color borderC = isSelected ? COLOR_PRIMARY : COLOR_BORDER;
                Color textC = isSelected ? COLOR_PRIMARY : COLOR_TEXT_DARK;

                DrawRectangleRounded(ansRec, 0.12f, 4, btnBg);
                DrawRectangleRoundedLines(ansRec, 0.12f, 4, borderC);

                DrawTxt(TextFormat("[%c]", choices[i].letter), ansRec.x + 20, ansRec.y + 22, 22, textC, true);
                DrawTxt(choices[i].text, ansRec.x + 75, ansRec.y + 22, 18, textC, isSelected);

                Vector2 mouse = GetMousePosition();
                if (CheckCollisionPointRec(mouse, ansRec) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    curQ->selectedAnswer = choices[i].letter;
                    saveExamSession(loggedStudent->id, activeExamSubId.c_str(), examRemainingSeconds, examHead);
                }
            }

            if (currentExamQuestionIdx > 0) {
                if (GuiBtnOutline({ qPane.x + 30, qPane.y + qPane.height - 60, 150, 42 }, "< Cau truoc", COLOR_PRIMARY, COLOR_PRIMARY, 16, true)) {
                    currentExamQuestionIdx--;
                }
            }
            if (currentExamQuestionIdx < (int)examList.size() - 1) {
                if (GuiBtn({ qPane.x + qPane.width - 180, qPane.y + qPane.height - 60, 150, 42 }, "Cau tiep >", COLOR_PRIMARY, COLOR_PRIMARY_HOVER, WHITE, 16, true)) {
                    currentExamQuestionIdx++;
                }
            }

            Rectangle gridPane = { 890, 95, (float)(screenWidth - 920), (float)(screenHeight - 125) };
            DrawCardPanel(gridPane);

            DrawTxt("DANH SACH CAU HOI", gridPane.x + 25, gridPane.y + 25, 18, COLOR_TEXT_DARK, true);
            DrawTxt("Xanh: Da lam | Trang: Chua lam", gridPane.x + 25, gridPane.y + 50, 14, COLOR_TEXT_MUTED);

            int btnSize = 42;
            int btnGap = 12;
            int cols = 6;
            for (size_t i = 0; i < examList.size(); ++i) {
                int col = (int)i % cols;
                int row = (int)i / cols;
                Rectangle numRec = { (float)(gridPane.x + 25 + col * (btnSize + btnGap)), (float)(gridPane.y + 85 + row * (btnSize + btnGap)), (float)btnSize, (float)btnSize };

                bool isAnswered = (examList[i]->selectedAnswer != '\0');
                bool isCurrent = (currentExamQuestionIdx == (int)i);

                Color numBg = isAnswered ? COLOR_SUCCESS : MakeColor(241, 245, 249);
                Color numText = isAnswered ? WHITE : COLOR_TEXT_DARK;

                DrawRectangleRounded(numRec, 0.2f, 4, numBg);
                if (isCurrent) {
                    DrawRectangleRoundedLines(numRec, 0.2f, 4, COLOR_PRIMARY);
                }

                const char* numStr = TextFormat("%d", (int)i + 1);
                DrawTxt(numStr, numRec.x + (btnSize - MeasureTxt(numStr, 16, true)) / 2.0f, numRec.y + 12, 16, numText, true);

                Vector2 mouse = GetMousePosition();
                if (CheckCollisionPointRec(mouse, numRec) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    currentExamQuestionIdx = (int)i;
                }
            }
        }

        // ----------------------------------------------------
        // 5. SCREEN EXAM RESULT
        // ----------------------------------------------------
        else if (currentScreen == SCREEN_EXAM_RESULT) {
            DrawRectangle(0, 0, screenWidth, screenHeight, COLOR_BG);

            Rectangle card = { (float)(screenWidth / 2 - 250), (float)(screenHeight / 2 - 200), 500, 400 };
            DrawCardPanel(card);

            DrawTxt("HOAN THANH BAI THI!", card.x + (500 - MeasureTxt("HOAN THANH BAI THI!", 24, true)) / 2.0f, card.y + 40, 24, COLOR_SUCCESS, true);
            DrawTxt(TextFormat("Mon thi: %s", activeExamSubId.c_str()), card.x + 40, card.y + 95, 18, COLOR_TEXT_DARK);

            DrawTxt("DIEM SO DAT DUOC:", card.x + (500 - MeasureTxt("DIEM SO DAT DUOC:", 18, true)) / 2.0f, card.y + 140, 18, COLOR_TEXT_MUTED, true);
            const char* scoreTxt = TextFormat("%.2f", lastScore);
            DrawTxt(scoreTxt, card.x + (500 - MeasureTxt(scoreTxt, 52, true)) / 2.0f, card.y + 175, 52, COLOR_PRIMARY, true);

            DrawTxt(TextFormat("So cau tra loi dung: %d / %d cau", lastCorrect, lastTotal), card.x + (500 - MeasureTxt(TextFormat("So cau tra loi dung: %d / %d cau", lastCorrect, lastTotal), 18)) / 2.0f, card.y + 250, 18, COLOR_TEXT_DARK);

            if (GuiBtn({ card.x + 50, card.y + 310, 400, 48 }, "VE MAN HINH CHINH", COLOR_PRIMARY, COLOR_PRIMARY_HOVER, WHITE, 18, true)) {
                currentScreen = SCREEN_STUDENT_DASHBOARD;
                studentTab = 1;
            }
        }

        // ==================== MODAL DIALOGS ====================
        if (currentModal != MODAL_NONE) {
            DrawRectangle(0, 0, screenWidth, screenHeight, MakeColor(0, 0, 0, 120));
            Rectangle modalRec = { (float)(screenWidth / 2 - 250), (float)(screenHeight / 2 - 180), 500, 360 };

            if (currentModal == MODAL_ADD_SUBJECT || currentModal == MODAL_EDIT_SUBJECT) {
                DrawCardPanel(modalRec);
                DrawTxt((currentModal == MODAL_ADD_SUBJECT) ? "THEM MON HOC MOI" : "HIEU CHINH MON HOC", modalRec.x + 30, modalRec.y + 25, 20, COLOR_TEXT_DARK, true);

                DrawTxt("Ma mon hoc (MAMH <= 15):", modalRec.x + 30, modalRec.y + 65, 15, COLOR_TEXT_DARK);
                bool actId = (activeFormBox == 1);
                GuiTextBoxInput({ modalRec.x + 30, modalRec.y + 90, 440, 38 }, formId, sizeof(formId), actId, "VD: CTDL");
                if (actId) activeFormBox = 1;

                DrawTxt("Ten mon hoc:", modalRec.x + 30, modalRec.y + 145, 15, COLOR_TEXT_DARK);
                bool actName = (activeFormBox == 2);
                GuiTextBoxInput({ modalRec.x + 30, modalRec.y + 170, 440, 38 }, formName, sizeof(formName), actName, "VD: Cau truc du lieu");
                if (actName) activeFormBox = 2;

                if (GuiBtn({ modalRec.x + 30, modalRec.y + 270, 200, 42 }, "Luu Mon Hoc", COLOR_SUCCESS, COLOR_SUCCESS_HOVER, WHITE, 16, true)) {
                    if (currentModal == MODAL_ADD_SUBJECT) {
                        addSubjectWorkflow(root, formId, formName);
                    } else {
                        editSubjectWorkflow(root, classes, classCount, formId, formId, formName);
                    }
                    currentModal = MODAL_NONE;
                }
                if (GuiBtnOutline({ modalRec.x + 270, modalRec.y + 270, 200, 42 }, "Huy bo", COLOR_DANGER, COLOR_DANGER, 16, true)) {
                    currentModal = MODAL_NONE;
                }
            } else if (currentModal == MODAL_DELETE_SUBJECT) {
                DrawCardPanel(modalRec);
                DrawTxt("XAC NHAN XOA MON HOC", modalRec.x + 30, modalRec.y + 30, 20, COLOR_DANGER, true);
                DrawTxt(TextFormat("Ban co chac chan muon xoa mon hoc [%s]?", formId), modalRec.x + 30, modalRec.y + 80, 16, COLOR_TEXT_DARK);
                DrawTxt("Luu y: Chi xoa duoc mon chua co diem thi trong he thong.", modalRec.x + 30, modalRec.y + 115, 14, COLOR_TEXT_MUTED);

                if (GuiBtn({ modalRec.x + 30, modalRec.y + 200, 200, 42 }, "Xac Nhan Xoa", COLOR_DANGER, COLOR_DANGER_HOVER, WHITE, 16, true)) {
                    deleteSubjectWorkflow(root, classes, classCount, formId);
                    currentModal = MODAL_NONE;
                }
                if (GuiBtnOutline({ modalRec.x + 270, modalRec.y + 200, 200, 42 }, "Huy bo", COLOR_PRIMARY, COLOR_PRIMARY, 16, true)) {
                    currentModal = MODAL_NONE;
                }
            } else if (currentModal == MODAL_ADD_CLASS) {
                DrawCardPanel(modalRec);
                DrawTxt("THEM LOP HOC MOI", modalRec.x + 30, modalRec.y + 25, 20, COLOR_TEXT_DARK, true);

                DrawTxt("Ma lop hoc (MALOP <= 19):", modalRec.x + 30, modalRec.y + 65, 15, COLOR_TEXT_DARK);
                bool actId = (activeFormBox == 1);
                GuiTextBoxInput({ modalRec.x + 30, modalRec.y + 90, 440, 38 }, formId, sizeof(formId), actId, "VD: D23CQCN01");
                if (actId) activeFormBox = 1;

                DrawTxt("Ten lop hoc:", modalRec.x + 30, modalRec.y + 145, 15, COLOR_TEXT_DARK);
                bool actName = (activeFormBox == 2);
                GuiTextBoxInput({ modalRec.x + 30, modalRec.y + 170, 440, 38 }, formName, sizeof(formName), actName, "VD: Cong nghe thong tin 1");
                if (actName) activeFormBox = 2;

                if (GuiBtn({ modalRec.x + 30, modalRec.y + 270, 200, 42 }, "Luu Lop Hoc", COLOR_SUCCESS, COLOR_SUCCESS_HOVER, WHITE, 16, true)) {
                    addClassWorkflow(classes, classCount, formId, formName);
                    currentModal = MODAL_NONE;
                }
                if (GuiBtnOutline({ modalRec.x + 270, modalRec.y + 270, 200, 42 }, "Huy bo", COLOR_DANGER, COLOR_DANGER, 16, true)) {
                    currentModal = MODAL_NONE;
                }
            } else if (currentModal == MODAL_DELETE_CLASS) {
                DrawCardPanel(modalRec);
                DrawTxt("XAC NHAN XOA LOP HOC", modalRec.x + 30, modalRec.y + 30, 20, COLOR_DANGER, true);
                DrawTxt(TextFormat("Ban co chac chan muon xoa lop [%s]?", formId), modalRec.x + 30, modalRec.y + 80, 16, COLOR_TEXT_DARK);
                DrawTxt("Luu y: Khong duoc xoa neu lop hoc dang co sinh vien.", modalRec.x + 30, modalRec.y + 115, 14, COLOR_TEXT_MUTED);

                if (GuiBtn({ modalRec.x + 30, modalRec.y + 200, 200, 42 }, "Xac Nhan Xoa", COLOR_DANGER, COLOR_DANGER_HOVER, WHITE, 16, true)) {
                    deleteClassWorkflow(classes, classCount, formId);
                    currentModal = MODAL_NONE;
                }
                if (GuiBtnOutline({ modalRec.x + 270, modalRec.y + 200, 200, 42 }, "Huy bo", COLOR_PRIMARY, COLOR_PRIMARY, 16, true)) {
                    currentModal = MODAL_NONE;
                }
            } else if (currentModal == MODAL_ADD_STUDENT || currentModal == MODAL_EDIT_STUDENT) {
                Rectangle bigModal = { (float)(screenWidth / 2 - 270), (float)(screenHeight / 2 - 230), 540, 460 };
                DrawCardPanel(bigModal);

                DrawTxt((currentModal == MODAL_ADD_STUDENT) ? "THEM SINH VIEN MOI" : "HIEU CHINH SINH VIEN", bigModal.x + 30, bigModal.y + 25, 20, COLOR_TEXT_DARK, true);

                DrawTxt("Ma sinh vien (MASV):", bigModal.x + 30, bigModal.y + 65, 14, COLOR_TEXT_DARK);
                bool actId = (activeFormBox == 1);
                GuiTextBoxInput({ bigModal.x + 30, bigModal.y + 85, 480, 34 }, formId, sizeof(formId), actId, "VD: SV004");
                if (actId) activeFormBox = 1;

                DrawTxt("Ho dem:", bigModal.x + 30, bigModal.y + 130, 14, COLOR_TEXT_DARK);
                bool actHo = (activeFormBox == 2);
                GuiTextBoxInput({ bigModal.x + 30, bigModal.y + 150, 230, 34 }, formHo, sizeof(formHo), actHo, "VD: Nguyen Van");
                if (actHo) activeFormBox = 2;

                DrawTxt("Ten:", bigModal.x + 280, bigModal.y + 130, 14, COLOR_TEXT_DARK);
                bool actTen = (activeFormBox == 3);
                GuiTextBoxInput({ bigModal.x + 280, bigModal.y + 150, 230, 34 }, formTen, sizeof(formTen), actTen, "VD: Dung");
                if (actTen) activeFormBox = 3;

                DrawTxt("Phai (Nam / Nu):", bigModal.x + 30, bigModal.y + 195, 14, COLOR_TEXT_DARK);
                bool actGen = (activeFormBox == 4);
                GuiTextBoxInput({ bigModal.x + 30, bigModal.y + 215, 230, 34 }, formGender, sizeof(formGender), actGen, "Nam");
                if (actGen) activeFormBox = 4;

                DrawTxt("Mat khau:", bigModal.x + 280, bigModal.y + 195, 14, COLOR_TEXT_DARK);
                bool actP = (activeFormBox == 5);
                GuiTextBoxInput({ bigModal.x + 280, bigModal.y + 215, 230, 34 }, formPass, sizeof(formPass), actP, "123");
                if (actP) activeFormBox = 5;

                if (GuiBtn({ bigModal.x + 30, bigModal.y + 360, 230, 42 }, "Luu Sinh Vien", COLOR_SUCCESS, COLOR_SUCCESS_HOVER, WHITE, 16, true)) {
                    if (selectedClassIdx < classCount && classes[selectedClassIdx] != nullptr) {
                        if (currentModal == MODAL_ADD_STUDENT) {
                            addStudentWorkflow(classes, classCount, classes[selectedClassIdx]->id, formId, formHo, formTen, formGender, formPass);
                        } else {
                            editStudentWorkflow(classes, classCount, formId, formHo, formTen, formGender, formPass);
                        }
                    }
                    currentModal = MODAL_NONE;
                }
                if (GuiBtnOutline({ bigModal.x + 280, bigModal.y + 360, 230, 42 }, "Huy bo", COLOR_DANGER, COLOR_DANGER, 16, true)) {
                    currentModal = MODAL_NONE;
                }
            } else if (currentModal == MODAL_DELETE_STUDENT) {
                DrawCardPanel(modalRec);
                DrawTxt("XAC NHAN XOA SINH VIEN", modalRec.x + 30, modalRec.y + 30, 20, COLOR_DANGER, true);
                DrawTxt(TextFormat("Ban co chac chan muon xoa sinh vien [%s]?", formId), modalRec.x + 30, modalRec.y + 80, 16, COLOR_TEXT_DARK);

                if (GuiBtn({ modalRec.x + 30, modalRec.y + 200, 200, 42 }, "Xac Nhan Xoa", COLOR_DANGER, COLOR_DANGER_HOVER, WHITE, 16, true)) {
                    if (selectedClassIdx < classCount && classes[selectedClassIdx] != nullptr) {
                        deleteStudentWorkflow(classes, classCount, classes[selectedClassIdx]->id, formId);
                    }
                    currentModal = MODAL_NONE;
                }
                if (GuiBtnOutline({ modalRec.x + 270, modalRec.y + 200, 200, 42 }, "Huy bo", COLOR_PRIMARY, COLOR_PRIMARY, 16, true)) {
                    currentModal = MODAL_NONE;
                }
            } else if (currentModal == MODAL_ADD_QUESTION || currentModal == MODAL_EDIT_QUESTION) {
                Rectangle qModal = { (float)(screenWidth / 2 - 320), (float)(screenHeight / 2 - 270), 640, 540 };
                DrawCardPanel(qModal);

                DrawTxt((currentModal == MODAL_ADD_QUESTION) ? "THEM CAU HOI THI MOI" : "HIEU CHINH CAU HOI", qModal.x + 30, qModal.y + 20, 20, COLOR_TEXT_DARK, true);

                DrawTxt("Noi dung cau hoi:", qModal.x + 30, qModal.y + 55, 14, COLOR_TEXT_DARK);
                bool actC = (activeFormBox == 1);
                GuiTextBoxInput({ qModal.x + 30, qModal.y + 75, 580, 36 }, formContent, sizeof(formContent), actC, "VD: 1 byte bang bao nhieu bit?");
                if (actC) activeFormBox = 1;

                DrawTxt("Dap an A:", qModal.x + 30, qModal.y + 120, 14, COLOR_TEXT_DARK);
                bool actA = (activeFormBox == 2);
                GuiTextBoxInput({ qModal.x + 30, qModal.y + 140, 580, 34 }, formA, sizeof(formA), actA, "A");
                if (actA) activeFormBox = 2;

                DrawTxt("Dap an B:", qModal.x + 30, qModal.y + 185, 14, COLOR_TEXT_DARK);
                bool actB = (activeFormBox == 3);
                GuiTextBoxInput({ qModal.x + 30, qModal.y + 205, 580, 34 }, formB, sizeof(formB), actB, "B");
                if (actB) activeFormBox = 3;

                DrawTxt("Dap an C:", qModal.x + 30, qModal.y + 250, 14, COLOR_TEXT_DARK);
                bool actCc = (activeFormBox == 4);
                GuiTextBoxInput({ qModal.x + 30, qModal.y + 270, 580, 34 }, formC, sizeof(formC), actCc, "C");
                if (actCc) activeFormBox = 4;

                DrawTxt("Dap an D:", qModal.x + 30, qModal.y + 315, 14, COLOR_TEXT_DARK);
                bool actD = (activeFormBox == 5);
                GuiTextBoxInput({ qModal.x + 30, qModal.y + 335, 580, 34 }, formD, sizeof(formD), actD, "D");
                if (actD) activeFormBox = 5;

                DrawTxt("Dap an dung (A / B / C / D):", qModal.x + 30, qModal.y + 380, 14, COLOR_TEXT_DARK);
                for (int c = 0; c < 4; ++c) {
                    char l = (char)('A' + c);
                    bool isCur = (formCorrect[0] == l);
                    if (GuiBtn({ qModal.x + 230 + c * 60, qModal.y + 375, 45, 30 }, TextFormat("%c", l), isCur ? COLOR_SUCCESS : COLOR_SIDEBAR, COLOR_SUCCESS_HOVER, WHITE, 14, isCur)) {
                        formCorrect[0] = l;
                    }
                }

                if (GuiBtn({ qModal.x + 30, qModal.y + 460, 270, 42 }, "Luu Cau Hoi", COLOR_SUCCESS, COLOR_SUCCESS_HOVER, WHITE, 16, true)) {
                    if (selectedSubjectIdx < (int)allSubjects.size()) {
                        Subject* sNode = allSubjects[selectedSubjectIdx];
                        if (currentModal == MODAL_ADD_QUESTION) {
                            int newId = 0;
                            addQuestionWorkflow(root, sNode->id, formContent, formA, formB, formC, formD, formCorrect[0], newId);
                        } else {
                            editQuestionWorkflow(root, sNode->id, formEditQId, formContent, formA, formB, formC, formD, formCorrect[0]);
                        }
                    }
                    currentModal = MODAL_NONE;
                }
                if (GuiBtnOutline({ qModal.x + 340, qModal.y + 460, 270, 42 }, "Huy bo", COLOR_DANGER, COLOR_DANGER, 16, true)) {
                    currentModal = MODAL_NONE;
                }
            } else if (currentModal == MODAL_CONFIRM_SUBMIT) {
                DrawCardPanel(modalRec);
                DrawTxt("XAC NHAN NOP BAI THI", modalRec.x + 30, modalRec.y + 30, 20, COLOR_WARNING, true);

                int answeredCount = 0;
                for (size_t i = 0; i < examList.size(); ++i) {
                    if (examList[i]->selectedAnswer != '\0') answeredCount++;
                }

                DrawTxt(TextFormat("Ban da tra loi: %d / %d cau hoi.", answeredCount, (int)examList.size()), modalRec.x + 30, modalRec.y + 80, 18, COLOR_TEXT_DARK);
                DrawTxt("Ban co chac chan muon nop bai ngay bay gio?", modalRec.x + 30, modalRec.y + 115, 16, COLOR_TEXT_MUTED);

                if (GuiBtn({ modalRec.x + 30, modalRec.y + 200, 200, 44 }, "Dong Y Nop Bai", COLOR_SUCCESS, COLOR_SUCCESS_HOVER, WHITE, 16, true)) {
                    currentModal = MODAL_NONE;
                    submitExamWorkflow(loggedStudent, activeExamSubId.c_str(), examHead, lastScore, lastCorrect, lastTotal);
                    removeExamSession(loggedStudent->id);
                    examHead = nullptr;
                    examList.clear();
                    currentScreen = SCREEN_EXAM_RESULT;
                }
                if (GuiBtnOutline({ modalRec.x + 270, modalRec.y + 200, 200, 44 }, "Quay Lai Lam Tiep", COLOR_PRIMARY, COLOR_PRIMARY, 16, true)) {
                    currentModal = MODAL_NONE;
                }
            }
        }

        EndDrawing();
    }

    // Cleanup khi đóng cửa sổ
    saveAllData(root, classes, classCount, "data");

    while (classCount > 0) {
        deleteClass(classes, classCount, classes[0]->id);
    }
    while (root != nullptr) {
        root = deleteSubject(root, root->id);
    }

    if (gFontRegular.texture.id != GetFontDefault().texture.id) UnloadFont(gFontRegular);
    if (gFontBold.texture.id != GetFontDefault().texture.id) UnloadFont(gFontBold);

    CloseWindow();
}
