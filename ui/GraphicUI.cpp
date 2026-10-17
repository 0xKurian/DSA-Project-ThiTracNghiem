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
#include <algorithm>

// ==================== COLOR HELPERS & PALETTE ====================
static inline Color MakeColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255) {
    Color c;
    c.r = r; c.g = g; c.b = b; c.a = a;
    return c;
}

static const Color COLOR_BG             = { 245, 247, 250, 255 }; // Light gray-blue background
static const Color COLOR_CARD           = { 255, 255, 255, 255 }; // White card
static const Color COLOR_SIDEBAR        = { 30, 41, 59, 255 };    // Slate 800
static const Color COLOR_SIDEBAR_HOVER  = { 51, 65, 85, 255 };    // Slate 700
static const Color COLOR_SIDEBAR_ACTIVE = { 37, 99, 235, 255 };   // Blue 600
static const Color COLOR_PRIMARY        = { 37, 99, 235, 255 };   // Blue 600
static const Color COLOR_PRIMARY_HOVER  = { 29, 78, 216, 255 };   // Blue 700
static const Color COLOR_SUCCESS        = { 22, 163, 74, 255 };   // Green 600
static const Color COLOR_SUCCESS_HOVER  = { 21, 128, 61, 255 };   // Green 700
static const Color COLOR_DANGER         = { 220, 38, 38, 255 };   // Red 600
static const Color COLOR_DANGER_HOVER   = { 185, 28, 28, 255 };   // Red 700
static const Color COLOR_WARNING        = { 234, 88, 12, 255 };   // Orange 600
static const Color COLOR_TEXT_DARK      = { 15, 23, 42, 255 };    // Slate 900
static const Color COLOR_TEXT_MUTED     = { 100, 116, 139, 255 }; // Slate 500
static const Color COLOR_BORDER         = { 226, 232, 240, 255 }; // Slate 200

// ==================== GLOBAL FONTS & ANTI-ALIASED TYPOGRAPHY ====================
static Font gFontRegular;
static Font gFontBold;

// Trạng thái modal toàn cục nhằm ngăn chặn việc click xuyên thấu (click-through)
static bool gModalOpen = false;
static bool gInsideModal = false;

static std::vector<int> getVietnameseCodepoints() {
    std::vector<int> cps;
    // ASCII cơ bản: 32..126
    for (int i = 32; i <= 126; ++i) cps.push_back(i);
    // Latin-1 Supplement: 160..255 (à, á, â, ã, è, é, ê, ì, í, ò, ó, ô, õ, ù, ú, ý, ...)
    for (int i = 160; i <= 255; ++i) cps.push_back(i);
    // Latin Extended-A: 0x0100..0x017F (Đ = 0x0110, đ = 0x0111, ă, Ă, ...)
    for (int i = 0x0100; i <= 0x017F; ++i) cps.push_back(i);
    // Latin Extended-B: 0x01A0..0x01B0 (ơ, ư, Ơ, Ư)
    for (int i = 0x01A0; i <= 0x01B0; ++i) cps.push_back(i);
    // Latin Extended Additional: 0x1EA0..0x1EF9 (Ký tự tiếng Việt có dấu tổ hợp sẵn)
    for (int i = 0x1EA0; i <= 0x1EF9; ++i) cps.push_back(i);
    // Dấu câu thông dụng: 0x2010..0x2026
    for (int i = 0x2010; i <= 0x2026; ++i) cps.push_back(i);
    return cps;
}

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

// Hàm vẽ văn bản tự động xuống dòng khi vượt quá chiều rộng (Word Wrapping)
static float DrawTxtWrapped(const char* text, float posX, float posY, float maxWidth, float fontSize, Color color = COLOR_TEXT_DARK, bool bold = false, float lineSpacing = 4.0f) {
    if (text == nullptr || text[0] == '\0') return 0.0f;
    Font f = bold ? gFontBold : gFontRegular;

    float currentY = posY;
    std::string textStr(text);
    std::string currentLine = "";

    size_t i = 0;
    while (i < textStr.length()) {
        size_t nextSpace = textStr.find(' ', i);
        std::string word;
        if (nextSpace == std::string::npos) {
            word = textStr.substr(i);
            i = textStr.length();
        } else {
            word = textStr.substr(i, nextSpace - i);
            i = nextSpace + 1;
        }

        std::string testLine = currentLine.empty() ? word : (currentLine + " " + word);
        Vector2 sz = MeasureTextEx(f, testLine.c_str(), fontSize, 1.0f);
        if (sz.x > maxWidth && !currentLine.empty()) {
            DrawTextEx(f, currentLine.c_str(), Vector2{ posX, currentY }, fontSize, 1.0f, color);
            currentY += fontSize + lineSpacing;
            currentLine = word;
        } else {
            currentLine = testLine;
        }
    }
    if (!currentLine.empty()) {
        DrawTextEx(f, currentLine.c_str(), Vector2{ posX, currentY }, fontSize, 1.0f, color);
        currentY += fontSize + lineSpacing;
    }
    return currentY - posY;
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

static bool GuiBtn(Rectangle rec, const char* text, Color bg = COLOR_PRIMARY, Color hoverBg = COLOR_PRIMARY_HOVER, Color textColor = WHITE, float fontSize = 16.0f, bool bold = false, bool disabled = false) {
    if (disabled || (gModalOpen && !gInsideModal)) {
        DrawRectangleRounded(rec, 0.15f, 4, MakeColor(bg.r, bg.g, bg.b, 160));
        DrawRectangleRoundedLines(rec, 0.15f, 4, MakeColor(bg.r, bg.g, bg.b, 160));
        float textW = MeasureTxt(text, fontSize, bold);
        float posX = rec.x + (rec.width - textW) / 2.0f;
        float posY = rec.y + (rec.height - fontSize) / 2.0f;
        DrawTxt(text, posX, posY, fontSize, MakeColor(textColor.r, textColor.g, textColor.b, 160), bold);
        return false;
    }

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

static bool GuiBtnOutline(Rectangle rec, const char* text, Color borderColor = COLOR_PRIMARY, Color textColor = COLOR_PRIMARY, float fontSize = 15.0f, bool bold = false, bool disabled = false) {
    if (disabled || (gModalOpen && !gInsideModal)) {
        DrawRectangleRounded(rec, 0.15f, 4, COLOR_CARD);
        DrawRectangleRoundedLines(rec, 0.15f, 4, COLOR_BORDER);
        float textW = MeasureTxt(text, fontSize, bold);
        float posX = rec.x + (rec.width - textW) / 2.0f;
        float posY = rec.y + (rec.height - fontSize) / 2.0f;
        DrawTxt(text, posX, posY, fontSize, COLOR_TEXT_MUTED, bold);
        return false;
    }

    Vector2 mouse = GetMousePosition();
    bool hovered = CheckCollisionPointRec(mouse, rec);
    Color currentBg = hovered ? MakeColor(borderColor.r, borderColor.g, borderColor.b, 35) : COLOR_CARD;

    DrawRectangleRounded(rec, 0.15f, 4, currentBg);
    DrawRectangleRoundedLines(rec, 0.15f, 4, borderColor);

    float textW = MeasureTxt(text, fontSize, bold);
    float posX = rec.x + (rec.width - textW) / 2.0f;
    float posY = rec.y + (rec.height - fontSize) / 2.0f;
    DrawTxt(text, posX, posY, fontSize, textColor, bold);

    return hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

static void GuiTextBoxInput(Rectangle rec, char textBuffer[], int maxLen, bool& isActive, const char* placeholder = "", bool isPassword = false) {
    if (gModalOpen && !gInsideModal) {
        isActive = false;
    } else {
        Vector2 mouse = GetMousePosition();
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            isActive = CheckCollisionPointRec(mouse, rec);
        }
    }

    if (isActive) {
        int key = GetCharPressed();
        while (key > 0) {
            int len = (int)strlen(textBuffer);
            if (key >= 32 && len < maxLen - 1) {
                if (key > 127) {
                    int utf8Size = 0;
                    const char* utf8Bytes = CodepointToUTF8(key, &utf8Size);
                    if (utf8Bytes && (len + utf8Size < maxLen)) {
                        memcpy(textBuffer + len, utf8Bytes, utf8Size);
                        textBuffer[len + utf8Size] = '\0';
                    }
                } else if (key <= 126) {
                    textBuffer[len] = (char)key;
                    textBuffer[len + 1] = '\0';
                }
            }
            key = GetCharPressed();
        }

        if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) {
            int len = (int)strlen(textBuffer);
            if (len > 0) {
                while (len > 0) {
                    len--;
                    if ((textBuffer[len] & 0xC0) != 0x80) {
                        textBuffer[len] = '\0';
                        break;
                    }
                }
            }
        }
    }

    DrawRectangleRounded(rec, 0.15f, 4, COLOR_CARD);
    DrawRectangleRoundedLines(rec, 0.15f, 4, isActive ? COLOR_PRIMARY : COLOR_BORDER);

    float fontSize = 16.0f;
    float posY = rec.y + (rec.height - fontSize) / 2.0f;
    float posX = rec.x + 12.0f;
    float textMaxW = rec.width - 24.0f;

    BeginScissorMode((int)rec.x + 4, (int)rec.y + 2, (int)rec.width - 8, (int)rec.height - 4);

    if (strlen(textBuffer) == 0 && !isActive) {
        DrawTxt(placeholder, posX, posY, fontSize, COLOR_TEXT_MUTED);
    } else {
        std::string displayText;
        if (isPassword) {
            displayText = std::string(strlen(textBuffer), '*');
        } else {
            displayText = textBuffer;
        }

        float textW = MeasureTxt(displayText.c_str(), fontSize);
        float renderX = posX;
        if (textW > textMaxW) {
            renderX = posX - (textW - textMaxW);
        }

        DrawTxt(displayText.c_str(), renderX, posY, fontSize, COLOR_TEXT_DARK);

        if (isActive && ((GetTime() - (int)GetTime()) < 0.5)) {
            DrawTxt("|", renderX + textW + 1.0f, posY, fontSize, COLOR_PRIMARY, true);
        }
    }

    EndScissorMode();
}

static void DrawCardPanel(Rectangle rec, Color bg = COLOR_CARD, Color border = COLOR_BORDER) {
    DrawRectangleRounded(rec, 0.08f, 6, bg);
    DrawRectangleRoundedLines(rec, 0.08f, 6, border);
}

static void DrawToastNotification(const char* message, float timer, bool isError = false) {
    if (message == nullptr || message[0] == '\0' || timer <= 0.0f) return;

    float alpha = 1.0f;
    if (timer < 0.5f) {
        alpha = timer / 0.5f;
    }

    float fontSize = 15.0f;
    const char* prefix = isError ? "[!] " : "[v] ";
    std::string fullMsg = std::string(prefix) + message;
    float textW = MeasureTxt(fullMsg.c_str(), fontSize, true);
    float cardW = textW + 48.0f;
    if (cardW < 320.0f) cardW = 320.0f;
    float cardH = 44.0f;
    float cardX = (1280.0f - cardW) / 2.0f;
    float cardY = 16.0f;

    Color bg = isError ? COLOR_DANGER : COLOR_SUCCESS;
    bg.a = (unsigned char)(240 * alpha);
    Color border = isError ? COLOR_DANGER_HOVER : COLOR_SUCCESS_HOVER;
    border.a = (unsigned char)(255 * alpha);
    Color textC = WHITE;
    textC.a = (unsigned char)(255 * alpha);

    Rectangle rec = { cardX, cardY, cardW, cardH };
    DrawRectangleRounded(rec, 0.3f, 6, bg);
    DrawRectangleRoundedLines(rec, 0.3f, 6, border);

    float fullW = MeasureTxt(fullMsg.c_str(), fontSize, true);
    DrawTxt(fullMsg.c_str(), cardX + (cardW - fullW) / 2.0f, cardY + (cardH - fontSize) / 2.0f, fontSize, textC, true);
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
    InitWindow(screenWidth, screenHeight, "HE THONG QUAN LY VA THI TRAC NGHIEM - C++ DSA PROJECT");
    SetTargetFPS(60);

    // Tải bộ Font TrueType tiếng Việt mượt mà chống vỡ hình
    std::vector<int> cps = getVietnameseCodepoints();
    if (FileExists("assets/fonts/regular.ttf")) {
        gFontRegular = LoadFontEx("assets/fonts/regular.ttf", 36, cps.data(), (int)cps.size());
        SetTextureFilter(gFontRegular.texture, TEXTURE_FILTER_BILINEAR);
    } else {
        gFontRegular = GetFontDefault();
    }

    if (FileExists("assets/fonts/bold.ttf")) {
        gFontBold = LoadFontEx("assets/fonts/bold.ttf", 40, cps.data(), (int)cps.size());
        SetTextureFilter(gFontBold.texture, TEXTURE_FILTER_BILINEAR);
    } else {
        gFontBold = GetFontDefault();
    }

    // Dữ liệu ứng dụng
    Subject* root = nullptr;
    Class* classes[MAX_CLASS] = {};
    int classCount = 0;

    loadAllData(root, classes, classCount, "data");

    // Trạng thái ứng dụng
    ScreenState currentScreen = SCREEN_LOGIN;
    LoginRole currentRole = LOGIN_FAILED;
    Student* loggedStudent = nullptr;

    char loginUser[32] = "GV";
    char loginPass[32] = "GV";
    bool activeUserBox = false;
    bool activePassBox = false;
    char notifyMessage[128] = "";
    float notifyTimer = 0.0f;
    bool notifyIsError = false;

    // Phân trang & Điều hướng Giáo viên
    int teacherTab = 0; // 0: Môn học, 1: Câu hỏi, 2: Lớp học, 3: Sinh viên, 4: Bảng điểm, 5: Chi tiết bài thi
    int subjectPage = 0;
    const int SUBJECTS_PER_PAGE = 8;
    int classPage = 0;
    const int CLASSES_PER_PAGE = 8;
    int studentPage = 0;
    const int STUDENTS_PER_PAGE = 8;
    int questionPage = 0;
    const int QUESTIONS_PER_PAGE = 3;
    int scorePage = 0;
    const int SCORES_PER_PAGE = 8;
    int reviewQPage = 0;
    const int REVIEW_PER_PAGE = 3;

    int selectedSubjectIdx = 0;
    int selectedClassIdx = 0;
    int selectedStudentIdx = 0;
    int teacherSubNavOffset = 0;
    int teacherClassNavOffset = 0;

    // Phân trang & Điều hướng Sinh viên
    int studentTab = 0; // 0: Vào thi, 1: Bảng điểm, 2: Xem lại bài thi
    int studentSubChoice = 0;
    int studentSubNavOffset = 0;
    char numQuestionsInput[16] = "4";
    char examMinutesInput[16] = "10";
    bool activeNumQ = false;
    bool activeMinutes = false;

    // Trạng thái phòng thi
    ExamDetail* examHead = nullptr;
    std::vector<ExamDetail*> examList;
    int currentExamQuestionIdx = 0;
    std::string activeExamSubId = "";
    int examRemainingSeconds = 0;
    float examTimerClock = 0.0f;

    // Trạng thái kết quả thi
    float lastScore = 0.0f;
    int lastCorrect = 0;
    int lastTotal = 0;

    // Quản lý Modal Dialogs
    ModalType currentModal = MODAL_NONE;
    char modalError[128] = "";
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
    int activeFormBox = 1;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        if (notifyTimer > 0) notifyTimer -= dt;

        std::vector<Subject*> allSubjects;
        collectSubjects(root, allSubjects);

        // Thiết lập cờ chặn sự kiện khi có modal
        gModalOpen = (currentModal != MODAL_NONE);
        gInsideModal = false;

        // Đếm ngược thời gian thi trong phòng thi
        if (currentScreen == SCREEN_STUDENT_EXAM) {
            examTimerClock += dt;
            if (examTimerClock >= 1.0f) {
                examTimerClock -= 1.0f;
                if (examRemainingSeconds > 0) {
                    examRemainingSeconds--;
                    saveExamSession(loggedStudent->id, activeExamSubId.c_str(), examRemainingSeconds, examHead);
                } else {
                    currentModal = MODAL_NONE;
                    submitExamWorkflow(loggedStudent, activeExamSubId.c_str(), examHead, lastScore, lastCorrect, lastTotal);
                    removeExamSession(loggedStudent->id);
                    saveAllData(root, classes, classCount, "data");
                    examHead = nullptr;
                    examList.clear();
                    currentScreen = SCREEN_EXAM_RESULT;
                    snprintf(notifyMessage, sizeof(notifyMessage), "Het gio lam bai! He thong da tu dong nop bai thi.");
                    notifyTimer = 4.0f;
                    notifyIsError = false;
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
            DrawTxt("Do an Cau truc Du lieu & Giai thuat (BST, Mang con tro, Danh sach lien ket)", screenWidth / 2.0f - MeasureTxt("Do an Cau truc Du lieu & Giai thuat (BST, Mang con tro, Danh sach lien ket)", 16) / 2.0f, 95, 16, COLOR_TEXT_MUTED);

            Rectangle loginCard = { (float)(screenWidth / 2 - 250), 220, 500, 470 };
            DrawCardPanel(loginCard);

            DrawTxt("DANG NHAP HE THONG", loginCard.x + 30, loginCard.y + 30, 22, COLOR_TEXT_DARK, true);
            DrawLine((int)loginCard.x + 30, (int)loginCard.y + 65, (int)loginCard.x + 470, (int)loginCard.y + 65, COLOR_BORDER);

            DrawTxt("Ten dang nhap (GV hoac MASV):", loginCard.x + 30, loginCard.y + 85, 15, COLOR_TEXT_DARK);
            Rectangle userRec = { loginCard.x + 30, loginCard.y + 110, 440, 42 };
            GuiTextBoxInput(userRec, loginUser, sizeof(loginUser), activeUserBox, "Nhap GV hoac MASV (vd: SV001)");

            DrawTxt("Mat khau:", loginCard.x + 30, loginCard.y + 165, 15, COLOR_TEXT_DARK);
            Rectangle passRec = { loginCard.x + 30, loginCard.y + 190, 440, 42 };
            GuiTextBoxInput(passRec, loginPass, sizeof(loginPass), activePassBox, "Nhap mat khau", true);

            // Phím Tab chuyển đổi giữa Username và Password
            if (IsKeyPressed(KEY_TAB)) {
                if (activeUserBox) { activeUserBox = false; activePassBox = true; }
                else if (activePassBox) { activePassBox = false; activeUserBox = true; }
            }

            DrawTxt("Chon nhanh tai khoan de trai nghiem:", loginCard.x + 30, loginCard.y + 250, 14, COLOR_TEXT_MUTED);
            if (GuiBtnOutline({ loginCard.x + 30, loginCard.y + 275, 210, 36 }, "Giang vien (GV / GV)", COLOR_PRIMARY, COLOR_PRIMARY, 14, true)) {
                snprintf(loginUser, sizeof(loginUser), "GV");
                snprintf(loginPass, sizeof(loginPass), "GV");
            }
            if (GuiBtnOutline({ loginCard.x + 260, loginCard.y + 275, 210, 36 }, "Sinh vien (SV001 / 123)", COLOR_SUCCESS, COLOR_SUCCESS, 14, true)) {
                snprintf(loginUser, sizeof(loginUser), "SV001");
                snprintf(loginPass, sizeof(loginPass), "123");
            }

            bool doLogin = GuiBtn({ loginCard.x + 30, loginCard.y + 345, 440, 48 }, "DANG NHAP NGAY", COLOR_PRIMARY, COLOR_PRIMARY_HOVER, WHITE, 18, true);
            if (IsKeyPressed(KEY_ENTER) && (activeUserBox || activePassBox)) {
                doLogin = true;
            }

            if (doLogin) {
                BusinessResult res = loginWorkflow(classes, classCount, loginUser, loginPass, currentRole, loggedStudent);
                if (res == SUCCESS) {
                    if (currentRole == LOGIN_TEACHER) {
                        currentScreen = SCREEN_TEACHER_DASHBOARD;
                        teacherTab = 0;
                        subjectPage = 0;
                    } else if (currentRole == LOGIN_STUDENT && loggedStudent != nullptr) {
                        currentScreen = SCREEN_STUDENT_DASHBOARD;
                        studentTab = 0;
                    }
                    snprintf(notifyMessage, sizeof(notifyMessage), "Dang nhap thanh cong!");
                    notifyTimer = 2.5f;
                    notifyIsError = false;
                } else {
                    snprintf(notifyMessage, sizeof(notifyMessage), "%s", getErrorMessage(res));
                    notifyTimer = 3.5f;
                    notifyIsError = true;
                }
            }

            if (notifyTimer > 0 && notifyIsError) {
                DrawTxt(notifyMessage, loginCard.x + 30, loginCard.y + 412, 15, COLOR_DANGER, true);
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
                notifyIsError = false;
            }

            if (GuiBtn({ (float)(screenWidth - 130), 15, 100, 35 }, "Dang xuat", COLOR_DANGER, COLOR_DANGER_HOVER, WHITE, 15, true)) {
                currentScreen = SCREEN_LOGIN;
                currentRole = LOGIN_FAILED;
                loggedStudent = nullptr;
            }

            DrawRectangle(0, 65, 240, screenHeight - 65, COLOR_SIDEBAR);
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
                bool active = (teacherTab == i);
                Color tabBg = active ? COLOR_SIDEBAR_ACTIVE : COLOR_SIDEBAR;
                if (GuiBtn(tabRec, tabs[i], tabBg, COLOR_SIDEBAR_HOVER, WHITE, 15, active)) {
                    teacherTab = i;
                }
            }

            Rectangle contentRec = { 260, 85, (float)(screenWidth - 280), (float)(screenHeight - 105) };
            DrawCardPanel(contentRec);

            // ==================== TAB 0: MON HOC (BST) ====================
            if (teacherTab == 0) {
                DrawTxt("QUAN LY DANH MUC MON HOC (CAY NHI PHAN TIM KIEM BST)", contentRec.x + 25, contentRec.y + 25, 20, COLOR_TEXT_DARK, true);
                if (GuiBtn({ contentRec.x + contentRec.width - 180, contentRec.y + 20, 155, 38 }, "+ Them Mon Hoc", COLOR_SUCCESS, COLOR_SUCCESS_HOVER, WHITE, 15, true)) {
                    currentModal = MODAL_ADD_SUBJECT;
                    formId[0] = '\0';
                    formName[0] = '\0';
                    modalError[0] = '\0';
                    activeFormBox = 1;
                }

                int totalSubPages = allSubjects.empty() ? 1 : ((int)allSubjects.size() + SUBJECTS_PER_PAGE - 1) / SUBJECTS_PER_PAGE;
                if (subjectPage >= totalSubPages) subjectPage = totalSubPages - 1;
                if (subjectPage < 0) subjectPage = 0;

                int tableY = (int)contentRec.y + 75;
                DrawRectangle((int)contentRec.x + 25, tableY, (int)contentRec.width - 50, 40, COLOR_SIDEBAR);
                DrawTxt("STT", contentRec.x + 40, (float)tableY + 11, 15, WHITE, true);
                DrawTxt("MA MON HOC (Key BST)", contentRec.x + 100, (float)tableY + 11, 15, WHITE, true);
                DrawTxt("TEN MON HOC", contentRec.x + 350, (float)tableY + 11, 15, WHITE, true);
                DrawTxt("SO CAU HOI", contentRec.x + 680, (float)tableY + 11, 15, WHITE, true);
                DrawTxt("THAO TAC", contentRec.x + 820, (float)tableY + 11, 15, WHITE, true);

                if (allSubjects.empty()) {
                    DrawTxt("Chua co mon hoc nao. Hay bam '+ Them Mon Hoc' de them mon dau tien.", contentRec.x + 40, (float)tableY + 60, 16, COLOR_TEXT_MUTED);
                } else {
                    int startIdx = subjectPage * SUBJECTS_PER_PAGE;
                    int endIdx = std::min(startIdx + SUBJECTS_PER_PAGE, (int)allSubjects.size());

                    for (int i = startIdx; i < endIdx; ++i) {
                        int rowOffset = i - startIdx;
                        int rowY = tableY + 45 + rowOffset * 45;
                        Color rowBg = (rowOffset % 2 == 0) ? COLOR_CARD : MakeColor(248, 250, 252);
                        DrawRectangle((int)contentRec.x + 25, rowY, (int)contentRec.width - 50, 42, rowBg);
                        DrawRectangleLines((int)contentRec.x + 25, rowY, (int)contentRec.width - 50, 42, COLOR_BORDER);

                        DrawTxt(TextFormat("%d", i + 1), contentRec.x + 45, (float)rowY + 11, 15, COLOR_TEXT_DARK);
                        DrawTxt(allSubjects[i]->id, contentRec.x + 100, (float)rowY + 11, 15, COLOR_PRIMARY, true);
                        DrawTxt(allSubjects[i]->name, contentRec.x + 350, (float)rowY + 11, 15, COLOR_TEXT_DARK);

                        int qCount = countQuestions(allSubjects[i]->questionList);
                        DrawTxt(TextFormat("%d cau", qCount), contentRec.x + 690, (float)rowY + 11, 15, COLOR_TEXT_MUTED);

                        if (GuiBtnOutline({ contentRec.x + 810, (float)rowY + 6, 65, 30 }, "Sua", COLOR_PRIMARY, COLOR_PRIMARY, 14)) {
                            currentModal = MODAL_EDIT_SUBJECT;
                            snprintf(formId, sizeof(formId), "%s", allSubjects[i]->id);
                            snprintf(formName, sizeof(formName), "%s", allSubjects[i]->name);
                            modalError[0] = '\0';
                            activeFormBox = 2;
                        }
                        if (GuiBtnOutline({ contentRec.x + 885, (float)rowY + 6, 65, 30 }, "Xoa", COLOR_DANGER, COLOR_DANGER, 14)) {
                            currentModal = MODAL_DELETE_SUBJECT;
                            snprintf(formId, sizeof(formId), "%s", allSubjects[i]->id);
                            snprintf(formName, sizeof(formName), "%s", allSubjects[i]->name);
                            modalError[0] = '\0';
                        }
                    }

                    // Thanh phân trang (Pagination bar)
                    int pageBarY = (int)(contentRec.y + contentRec.height - 50);
                    DrawTxt(TextFormat("Trang %d / %d  (Tong: %d mon)", subjectPage + 1, totalSubPages, (int)allSubjects.size()), contentRec.x + 40, (float)pageBarY + 8, 15, COLOR_TEXT_MUTED, true);

                    if (GuiBtnOutline({ contentRec.x + contentRec.width - 240, (float)pageBarY, 105, 32 }, "< Trang truoc", COLOR_PRIMARY, COLOR_PRIMARY, 13, false, subjectPage == 0)) {
                        if (subjectPage > 0) subjectPage--;
                    }
                    if (GuiBtnOutline({ contentRec.x + contentRec.width - 125, (float)pageBarY, 105, 32 }, "Trang sau >", COLOR_PRIMARY, COLOR_PRIMARY, 13, false, subjectPage >= totalSubPages - 1)) {
                        if (subjectPage < totalSubPages - 1) subjectPage++;
                    }
                }
            }

            // ==================== TAB 1: CAU HOI THI ====================
            else if (teacherTab == 1) {
                DrawTxt("NGAN HANG CAU HOI THI TRAC NGHIEM", contentRec.x + 25, contentRec.y + 25, 20, COLOR_TEXT_DARK, true);

                DrawTxt("Chon Mon Hoc:", contentRec.x + 25, contentRec.y + 68, 15, COLOR_TEXT_DARK, true);
                if (allSubjects.empty()) {
                    DrawTxt("Chua co mon hoc nao de tao cau hoi.", contentRec.x + 160, contentRec.y + 68, 15, COLOR_TEXT_MUTED);
                } else {
                    int maxVisibleSub = 4;
                    if (teacherSubNavOffset > 0) {
                        if (GuiBtnOutline({ contentRec.x + 150, contentRec.y + 62, 35, 32 }, "<", COLOR_PRIMARY, COLOR_PRIMARY, 14, true)) {
                            teacherSubNavOffset--;
                        }
                    }

                    for (int s = 0; s < maxVisibleSub; ++s) {
                        int idx = teacherSubNavOffset + s;
                        if (idx >= (int)allSubjects.size()) break;
                        bool sel = (selectedSubjectIdx == idx);
                        Rectangle btnRec = { contentRec.x + 195 + s * 125, contentRec.y + 62, 115, 32 };
                        if (GuiBtn(btnRec, allSubjects[idx]->id, sel ? COLOR_PRIMARY : COLOR_SIDEBAR, COLOR_PRIMARY_HOVER, WHITE, 14, sel)) {
                            selectedSubjectIdx = idx;
                            questionPage = 0;
                        }
                    }

                    if (teacherSubNavOffset + maxVisibleSub < (int)allSubjects.size()) {
                        if (GuiBtnOutline({ contentRec.x + 195 + maxVisibleSub * 125, contentRec.y + 62, 35, 32 }, ">", COLOR_PRIMARY, COLOR_PRIMARY, 14, true)) {
                            teacherSubNavOffset++;
                        }
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
                        modalError[0] = '\0';
                        activeFormBox = 1;
                    }

                    std::vector<Question*> qList;
                    collectQuestions(curSub->questionList, qList);

                    int totalQPages = qList.empty() ? 1 : ((int)qList.size() + QUESTIONS_PER_PAGE - 1) / QUESTIONS_PER_PAGE;
                    if (questionPage >= totalQPages) questionPage = totalQPages - 1;
                    if (questionPage < 0) questionPage = 0;

                    DrawTxt(TextFormat("Mon: %s (%s) - Tong so: %d cau hoi", curSub->id, curSub->name, (int)qList.size()), contentRec.x + 25, contentRec.y + 110, 15, COLOR_TEXT_MUTED, true);

                    if (qList.empty()) {
                        DrawTxt("Mon hoc nay chua co cau hoi thi nao. Hay bam '+ Them Cau Hoi' de bo sung.", contentRec.x + 30, contentRec.y + 150, 16, COLOR_TEXT_MUTED);
                    } else {
                        int startQ = questionPage * QUESTIONS_PER_PAGE;
                        int endQ = std::min(startQ + QUESTIONS_PER_PAGE, (int)qList.size());
                        int startY = (int)contentRec.y + 135;

                        for (int i = startQ; i < endQ; ++i) {
                            int offset = i - startQ;
                            Rectangle qCard = { contentRec.x + 25, (float)startY + offset * 155, contentRec.width - 50, 145 };
                            DrawCardPanel(qCard, MakeColor(248, 250, 252));

                            DrawTxt(TextFormat("ID %d:", qList[i]->id), qCard.x + 15, qCard.y + 12, 16, COLOR_PRIMARY, true);
                            DrawTxtWrapped(qList[i]->content, qCard.x + 75, qCard.y + 12, qCard.width - 160, 16, COLOR_TEXT_DARK, true);

                            DrawTxt(TextFormat("A. %s", qList[i]->answerA), qCard.x + 25, qCard.y + 45, 14, (qList[i]->correctAnswer == 'A') ? COLOR_SUCCESS : COLOR_TEXT_MUTED);
                            DrawTxt(TextFormat("B. %s", qList[i]->answerB), qCard.x + 25, qCard.y + 70, 14, (qList[i]->correctAnswer == 'B') ? COLOR_SUCCESS : COLOR_TEXT_MUTED);
                            DrawTxt(TextFormat("C. %s", qList[i]->answerC), qCard.x + 450, qCard.y + 45, 14, (qList[i]->correctAnswer == 'C') ? COLOR_SUCCESS : COLOR_TEXT_MUTED);
                            DrawTxt(TextFormat("D. %s", qList[i]->answerD), qCard.x + 450, qCard.y + 70, 14, (qList[i]->correctAnswer == 'D') ? COLOR_SUCCESS : COLOR_TEXT_MUTED);

                            DrawTxt(TextFormat("Dap an dung: %c", qList[i]->correctAnswer), qCard.x + 25, qCard.y + 105, 15, COLOR_SUCCESS, true);

                            if (GuiBtnOutline({ qCard.x + qCard.width - 150, qCard.y + 100, 65, 30 }, "Sua", COLOR_PRIMARY, COLOR_PRIMARY, 14)) {
                                currentModal = MODAL_EDIT_QUESTION;
                                formEditQId = qList[i]->id;
                                snprintf(formContent, sizeof(formContent), "%s", qList[i]->content);
                                snprintf(formA, sizeof(formA), "%s", qList[i]->answerA);
                                snprintf(formB, sizeof(formB), "%s", qList[i]->answerB);
                                snprintf(formC, sizeof(formC), "%s", qList[i]->answerC);
                                snprintf(formD, sizeof(formD), "%s", qList[i]->answerD);
                                formCorrect[0] = qList[i]->correctAnswer;
                                modalError[0] = '\0';
                                activeFormBox = 1;
                            }
                            if (GuiBtnOutline({ qCard.x + qCard.width - 75, qCard.y + 100, 65, 30 }, "Xoa", COLOR_DANGER, COLOR_DANGER, 14)) {
                                currentModal = MODAL_DELETE_QUESTION;
                                formEditQId = qList[i]->id;
                                modalError[0] = '\0';
                            }
                        }

                        // Pagination bar cho câu hỏi
                        int pageBarY = (int)(contentRec.y + contentRec.height - 45);
                        DrawTxt(TextFormat("Trang %d / %d (Tong: %d cau hoi)", questionPage + 1, totalQPages, (int)qList.size()), contentRec.x + 40, (float)pageBarY + 6, 15, COLOR_TEXT_MUTED, true);

                        if (GuiBtnOutline({ contentRec.x + contentRec.width - 240, (float)pageBarY, 105, 30 }, "< Trang truoc", COLOR_PRIMARY, COLOR_PRIMARY, 13, false, questionPage == 0)) {
                            if (questionPage > 0) questionPage--;
                        }
                        if (GuiBtnOutline({ contentRec.x + contentRec.width - 125, (float)pageBarY, 105, 30 }, "Trang sau >", COLOR_PRIMARY, COLOR_PRIMARY, 13, false, questionPage >= totalQPages - 1)) {
                            if (questionPage < totalQPages - 1) questionPage++;
                        }
                    }
                }
            }

            // ==================== TAB 2: LOP HOC ====================
            else if (teacherTab == 2) {
                DrawTxt("QUAN LY DANH MUC LOP HOC (MANG CON TRO)", contentRec.x + 25, contentRec.y + 25, 20, COLOR_TEXT_DARK, true);
                if (GuiBtn({ contentRec.x + contentRec.width - 180, contentRec.y + 20, 155, 38 }, "+ Them Lop Hoc", COLOR_SUCCESS, COLOR_SUCCESS_HOVER, WHITE, 15, true)) {
                    currentModal = MODAL_ADD_CLASS;
                    formId[0] = '\0';
                    formName[0] = '\0';
                    modalError[0] = '\0';
                    activeFormBox = 1;
                }

                int totalClassPages = (classCount == 0) ? 1 : (classCount + CLASSES_PER_PAGE - 1) / CLASSES_PER_PAGE;
                if (classPage >= totalClassPages) classPage = totalClassPages - 1;
                if (classPage < 0) classPage = 0;

                int tableY = (int)contentRec.y + 75;
                DrawRectangle((int)contentRec.x + 25, tableY, (int)contentRec.width - 50, 40, COLOR_SIDEBAR);
                DrawTxt("STT", contentRec.x + 40, (float)tableY + 11, 15, WHITE, true);
                DrawTxt("MA LOP", contentRec.x + 100, (float)tableY + 11, 15, WHITE, true);
                DrawTxt("TEN LOP HOC", contentRec.x + 350, (float)tableY + 11, 15, WHITE, true);
                DrawTxt("SI SO", contentRec.x + 680, (float)tableY + 11, 15, WHITE, true);
                DrawTxt("THAO TAC", contentRec.x + 820, (float)tableY + 11, 15, WHITE, true);

                if (classCount == 0) {
                    DrawTxt("Chua co lop hoc nao trong he thong. Hay bam '+ Them Lop Hoc' de them moi.", contentRec.x + 40, (float)tableY + 60, 16, COLOR_TEXT_MUTED);
                } else {
                    int startIdx = classPage * CLASSES_PER_PAGE;
                    int endIdx = std::min(startIdx + CLASSES_PER_PAGE, classCount);

                    for (int i = startIdx; i < endIdx; ++i) {
                        if (classes[i] == nullptr) continue;
                        int rowOffset = i - startIdx;
                        int rowY = tableY + 45 + rowOffset * 45;
                        Color rowBg = (rowOffset % 2 == 0) ? COLOR_CARD : MakeColor(248, 250, 252);
                        DrawRectangle((int)contentRec.x + 25, rowY, (int)contentRec.width - 50, 42, rowBg);
                        DrawRectangleLines((int)contentRec.x + 25, rowY, (int)contentRec.width - 50, 42, COLOR_BORDER);

                        DrawTxt(TextFormat("%d", i + 1), contentRec.x + 45, (float)rowY + 11, 15, COLOR_TEXT_DARK);
                        DrawTxt(classes[i]->id, contentRec.x + 100, (float)rowY + 11, 15, COLOR_PRIMARY, true);
                        DrawTxt(classes[i]->name, contentRec.x + 350, (float)rowY + 11, 15, COLOR_TEXT_DARK);

                        int count = 0;
                        Student* s = classes[i]->studentList;
                        while (s != nullptr) { count++; s = s->next; }
                        DrawTxt(TextFormat("%d SV", count), contentRec.x + 690, (float)rowY + 11, 15, COLOR_TEXT_MUTED);

                        if (GuiBtnOutline({ contentRec.x + 810, (float)rowY + 6, 75, 30 }, "Xoa", COLOR_DANGER, COLOR_DANGER, 14)) {
                            currentModal = MODAL_DELETE_CLASS;
                            snprintf(formId, sizeof(formId), "%s", classes[i]->id);
                            snprintf(formName, sizeof(formName), "%s", classes[i]->name);
                            modalError[0] = '\0';
                        }
                    }

                    int pageBarY = (int)(contentRec.y + contentRec.height - 50);
                    DrawTxt(TextFormat("Trang %d / %d (Tong: %d lop hoc)", classPage + 1, totalClassPages, classCount), contentRec.x + 40, (float)pageBarY + 8, 15, COLOR_TEXT_MUTED, true);

                    if (GuiBtnOutline({ contentRec.x + contentRec.width - 240, (float)pageBarY, 105, 32 }, "< Trang truoc", COLOR_PRIMARY, COLOR_PRIMARY, 13, false, classPage == 0)) {
                        if (classPage > 0) classPage--;
                    }
                    if (GuiBtnOutline({ contentRec.x + contentRec.width - 125, (float)pageBarY, 105, 32 }, "Trang sau >", COLOR_PRIMARY, COLOR_PRIMARY, 13, false, classPage >= totalClassPages - 1)) {
                        if (classPage < totalClassPages - 1) classPage++;
                    }
                }
            }

            // ==================== TAB 3: SINH VIEN ====================
            else if (teacherTab == 3) {
                DrawTxt("QUAN LY SINH VIEN THEO LOP (DANH SACH LIEN KET)", contentRec.x + 25, contentRec.y + 25, 20, COLOR_TEXT_DARK, true);

                DrawTxt("Chon Lop:", contentRec.x + 25, contentRec.y + 68, 15, COLOR_TEXT_DARK, true);
                if (classCount == 0) {
                    DrawTxt("Chua co lop hoc nao de quan ly sinh vien.", contentRec.x + 120, contentRec.y + 68, 15, COLOR_TEXT_MUTED);
                } else {
                    int maxVis = 4;
                    if (teacherClassNavOffset > 0) {
                        if (GuiBtnOutline({ contentRec.x + 110, contentRec.y + 62, 35, 32 }, "<", COLOR_PRIMARY, COLOR_PRIMARY, 14, true)) {
                            teacherClassNavOffset--;
                        }
                    }

                    for (int c = 0; c < maxVis; ++c) {
                        int idx = teacherClassNavOffset + c;
                        if (idx >= classCount || classes[idx] == nullptr) break;
                        bool sel = (selectedClassIdx == idx);
                        Rectangle btnRec = { contentRec.x + 155 + c * 135, contentRec.y + 62, 125, 32 };
                        if (GuiBtn(btnRec, classes[idx]->id, sel ? COLOR_PRIMARY : COLOR_SIDEBAR, COLOR_PRIMARY_HOVER, WHITE, 14, sel)) {
                            selectedClassIdx = idx;
                            studentPage = 0;
                        }
                    }

                    if (teacherClassNavOffset + maxVis < classCount) {
                        if (GuiBtnOutline({ contentRec.x + 155 + maxVis * 135, contentRec.y + 62, 35, 32 }, ">", COLOR_PRIMARY, COLOR_PRIMARY, 14, true)) {
                            teacherClassNavOffset++;
                        }
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
                        modalError[0] = '\0';
                        activeFormBox = 1;
                    }

                    std::vector<Student*> sList;
                    collectStudents(curClass->studentList, sList);

                    int totalStdPages = sList.empty() ? 1 : ((int)sList.size() + STUDENTS_PER_PAGE - 1) / STUDENTS_PER_PAGE;
                    if (studentPage >= totalStdPages) studentPage = totalStdPages - 1;
                    if (studentPage < 0) studentPage = 0;

                    int tableY = (int)contentRec.y + 115;
                    DrawRectangle((int)contentRec.x + 25, tableY, (int)contentRec.width - 50, 40, COLOR_SIDEBAR);
                    DrawTxt("STT", contentRec.x + 40, (float)tableY + 11, 15, WHITE, true);
                    DrawTxt("MASV", contentRec.x + 100, (float)tableY + 11, 15, WHITE, true);
                    DrawTxt("HO VA TEN", contentRec.x + 250, (float)tableY + 11, 15, WHITE, true);
                    DrawTxt("PHAI", contentRec.x + 550, (float)tableY + 11, 15, WHITE, true);
                    DrawTxt("MAT KHAU", contentRec.x + 680, (float)tableY + 11, 15, WHITE, true);
                    DrawTxt("THAO TAC", contentRec.x + 820, (float)tableY + 11, 15, WHITE, true);

                    if (sList.empty()) {
                        DrawTxt("Lop nay hien chua co sinh vien nao. Hay bam '+ Them Sinh Vien' de them.", contentRec.x + 40, (float)tableY + 60, 16, COLOR_TEXT_MUTED);
                    } else {
                        int startIdx = studentPage * STUDENTS_PER_PAGE;
                        int endIdx = std::min(startIdx + STUDENTS_PER_PAGE, (int)sList.size());

                        for (int i = startIdx; i < endIdx; ++i) {
                            int rowOffset = i - startIdx;
                            int rowY = tableY + 45 + rowOffset * 45;
                            Color rowBg = (rowOffset % 2 == 0) ? COLOR_CARD : MakeColor(248, 250, 252);
                            DrawRectangle((int)contentRec.x + 25, rowY, (int)contentRec.width - 50, 42, rowBg);
                            DrawRectangleLines((int)contentRec.x + 25, rowY, (int)contentRec.width - 50, 42, COLOR_BORDER);

                            DrawTxt(TextFormat("%d", i + 1), contentRec.x + 45, (float)rowY + 11, 15, COLOR_TEXT_DARK);
                            DrawTxt(sList[i]->id, contentRec.x + 100, (float)rowY + 11, 15, COLOR_PRIMARY, true);
                            DrawTxt(TextFormat("%s %s", sList[i]->ho, sList[i]->ten), contentRec.x + 250, (float)rowY + 11, 15, COLOR_TEXT_DARK);
                            DrawTxt(sList[i]->gender, contentRec.x + 550, (float)rowY + 11, 15, COLOR_TEXT_MUTED);
                            DrawTxt("******", contentRec.x + 680, (float)rowY + 11, 15, COLOR_TEXT_MUTED);

                            if (GuiBtnOutline({ contentRec.x + 810, (float)rowY + 6, 65, 30 }, "Sua", COLOR_PRIMARY, COLOR_PRIMARY, 14)) {
                                currentModal = MODAL_EDIT_STUDENT;
                                snprintf(formId, sizeof(formId), "%s", sList[i]->id);
                                snprintf(formHo, sizeof(formHo), "%s", sList[i]->ho);
                                snprintf(formTen, sizeof(formTen), "%s", sList[i]->ten);
                                snprintf(formGender, sizeof(formGender), "%s", sList[i]->gender);
                                snprintf(formPass, sizeof(formPass), "%s", sList[i]->password);
                                modalError[0] = '\0';
                                activeFormBox = 2; // Không sửa ID
                            }
                            if (GuiBtnOutline({ contentRec.x + 885, (float)rowY + 6, 65, 30 }, "Xoa", COLOR_DANGER, COLOR_DANGER, 14)) {
                                currentModal = MODAL_DELETE_STUDENT;
                                snprintf(formId, sizeof(formId), "%s", sList[i]->id);
                                modalError[0] = '\0';
                            }
                        }

                        int pageBarY = (int)(contentRec.y + contentRec.height - 50);
                        DrawTxt(TextFormat("Trang %d / %d (Tong: %d sinh vien)", studentPage + 1, totalStdPages, (int)sList.size()), contentRec.x + 40, (float)pageBarY + 8, 15, COLOR_TEXT_MUTED, true);

                        if (GuiBtnOutline({ contentRec.x + contentRec.width - 240, (float)pageBarY, 105, 32 }, "< Trang truoc", COLOR_PRIMARY, COLOR_PRIMARY, 13, false, studentPage == 0)) {
                            if (studentPage > 0) studentPage--;
                        }
                        if (GuiBtnOutline({ contentRec.x + contentRec.width - 125, (float)pageBarY, 105, 32 }, "Trang sau >", COLOR_PRIMARY, COLOR_PRIMARY, 13, false, studentPage >= totalStdPages - 1)) {
                            if (studentPage < totalStdPages - 1) studentPage++;
                        }
                    }
                }
            }

            // ==================== TAB 4: BANG DIEM LOP ====================
            else if (teacherTab == 4) {
                DrawTxt("BANG DIEM MON HOC THEO LOP", contentRec.x + 25, contentRec.y + 25, 20, COLOR_TEXT_DARK, true);

                DrawTxt("Lop:", contentRec.x + 25, contentRec.y + 68, 15, COLOR_TEXT_DARK, true);
                for (int i = 0; i < classCount && i < 4; ++i) {
                    if (classes[i] == nullptr) continue;
                    bool sel = (selectedClassIdx == i);
                    if (GuiBtn({ contentRec.x + 75 + i * 130, contentRec.y + 62, 120, 32 }, classes[i]->id, sel ? COLOR_PRIMARY : COLOR_SIDEBAR, COLOR_PRIMARY_HOVER, WHITE, 14, sel)) {
                        selectedClassIdx = i;
                        scorePage = 0;
                    }
                }

                DrawTxt("Mon:", contentRec.x + 620, contentRec.y + 68, 15, COLOR_TEXT_DARK, true);
                for (size_t i = 0; i < allSubjects.size() && i < 4; ++i) {
                    bool sel = (selectedSubjectIdx == (int)i);
                    if (GuiBtn({ contentRec.x + 670 + i * 105, contentRec.y + 62, 95, 32 }, allSubjects[i]->id, sel ? COLOR_PRIMARY : COLOR_SIDEBAR, COLOR_PRIMARY_HOVER, WHITE, 14, sel)) {
                        selectedSubjectIdx = (int)i;
                    }
                }

                if (classCount > 0 && selectedClassIdx < classCount && classes[selectedClassIdx] != nullptr && !allSubjects.empty() && selectedSubjectIdx < (int)allSubjects.size()) {
                    Class* curC = classes[selectedClassIdx];
                    Subject* curS = allSubjects[selectedSubjectIdx];

                    std::vector<Student*> sList;
                    collectStudents(curC->studentList, sList);

                    int totalScorePages = sList.empty() ? 1 : ((int)sList.size() + SCORES_PER_PAGE - 1) / SCORES_PER_PAGE;
                    if (scorePage >= totalScorePages) scorePage = totalScorePages - 1;
                    if (scorePage < 0) scorePage = 0;

                    int tableY = (int)contentRec.y + 115;
                    DrawRectangle((int)contentRec.x + 25, tableY, (int)contentRec.width - 50, 40, COLOR_SIDEBAR);
                    DrawTxt("STT", contentRec.x + 40, (float)tableY + 11, 15, WHITE, true);
                    DrawTxt("MASV", contentRec.x + 120, (float)tableY + 11, 15, WHITE, true);
                    DrawTxt("HO VA TEN", contentRec.x + 320, (float)tableY + 11, 15, WHITE, true);
                    DrawTxt("DIEM THI (Thang 10)", contentRec.x + 720, (float)tableY + 11, 15, WHITE, true);

                    if (sList.empty()) {
                        DrawTxt("Lop nay chua co sinh vien.", contentRec.x + 40, (float)tableY + 60, 16, COLOR_TEXT_MUTED);
                    } else {
                        int startIdx = scorePage * SCORES_PER_PAGE;
                        int endIdx = std::min(startIdx + SCORES_PER_PAGE, (int)sList.size());

                        for (int i = startIdx; i < endIdx; ++i) {
                            int rowOffset = i - startIdx;
                            int rowY = tableY + 45 + rowOffset * 45;
                            Color rowBg = (rowOffset % 2 == 0) ? COLOR_CARD : MakeColor(248, 250, 252);
                            DrawRectangle((int)contentRec.x + 25, rowY, (int)contentRec.width - 50, 42, rowBg);
                            DrawRectangleLines((int)contentRec.x + 25, rowY, (int)contentRec.width - 50, 42, COLOR_BORDER);

                            DrawTxt(TextFormat("%d", i + 1), contentRec.x + 45, (float)rowY + 11, 15, COLOR_TEXT_DARK);
                            DrawTxt(sList[i]->id, contentRec.x + 120, (float)rowY + 11, 15, COLOR_PRIMARY, true);
                            DrawTxt(TextFormat("%s %s", sList[i]->ho, sList[i]->ten), contentRec.x + 320, (float)rowY + 11, 15, COLOR_TEXT_DARK);

                            float sc = 0.0f;
                            if (hasStudentTakenExam(sList[i], curS->id, sc)) {
                                DrawTxt(TextFormat("%.2f diem", sc), contentRec.x + 720, (float)rowY + 11, 16, COLOR_SUCCESS, true);
                            } else {
                                DrawTxt("Chua thi", contentRec.x + 720, (float)rowY + 11, 15, COLOR_TEXT_MUTED);
                            }
                        }

                        int pageBarY = (int)(contentRec.y + contentRec.height - 50);
                        DrawTxt(TextFormat("Trang %d / %d (Tong: %d sinh vien)", scorePage + 1, totalScorePages, (int)sList.size()), contentRec.x + 40, (float)pageBarY + 8, 15, COLOR_TEXT_MUTED, true);

                        if (GuiBtnOutline({ contentRec.x + contentRec.width - 240, (float)pageBarY, 105, 32 }, "< Trang truoc", COLOR_PRIMARY, COLOR_PRIMARY, 13, false, scorePage == 0)) {
                            if (scorePage > 0) scorePage--;
                        }
                        if (GuiBtnOutline({ contentRec.x + contentRec.width - 125, (float)pageBarY, 105, 32 }, "Trang sau >", COLOR_PRIMARY, COLOR_PRIMARY, 13, false, scorePage >= totalScorePages - 1)) {
                            if (scorePage < totalScorePages - 1) scorePage++;
                        }
                    }
                }
            }

            // ==================== TAB 5: CHI TIET BAI THI ====================
            else if (teacherTab == 5) {
                DrawTxt("XEM LAI CHI TIET BAI THI CUA SINH VIEN", contentRec.x + 25, contentRec.y + 25, 20, COLOR_TEXT_DARK, true);

                if (classCount > 0 && selectedClassIdx < classCount && classes[selectedClassIdx] != nullptr) {
                    Class* curC = classes[selectedClassIdx];
                    std::vector<Student*> sList;
                    collectStudents(curC->studentList, sList);

                    DrawTxt("Chon SV:", contentRec.x + 25, contentRec.y + 68, 15, COLOR_TEXT_DARK, true);
                    for (size_t i = 0; i < sList.size() && i < 4; ++i) {
                        bool sel = (selectedStudentIdx == (int)i);
                        if (GuiBtn({ contentRec.x + 105 + i * 125, contentRec.y + 62, 115, 32 }, sList[i]->id, sel ? COLOR_PRIMARY : COLOR_SIDEBAR, COLOR_PRIMARY_HOVER, WHITE, 14, sel)) {
                            selectedStudentIdx = (int)i;
                            reviewQPage = 0;
                        }
                    }

                    DrawTxt("Mon:", contentRec.x + 630, contentRec.y + 68, 15, COLOR_TEXT_DARK, true);
                    for (size_t i = 0; i < allSubjects.size() && i < 3; ++i) {
                        bool sel = (selectedSubjectIdx == (int)i);
                        if (GuiBtn({ contentRec.x + 680 + i * 100, contentRec.y + 62, 90, 32 }, allSubjects[i]->id, sel ? COLOR_PRIMARY : COLOR_SIDEBAR, COLOR_PRIMARY_HOVER, WHITE, 14, sel)) {
                            selectedSubjectIdx = (int)i;
                            reviewQPage = 0;
                        }
                    }

                    if (!sList.empty() && selectedStudentIdx < (int)sList.size() && !allSubjects.empty() && selectedSubjectIdx < (int)allSubjects.size()) {
                        Student* curStd = sList[selectedStudentIdx];
                        Subject* curSub = allSubjects[selectedSubjectIdx];
                        Score* scNode = findScore(curStd, curSub->id);

                        if (scNode == nullptr) {
                            DrawTxt(TextFormat("Sinh vien %s chua lam bai thi mon %s.", curStd->id, curSub->id), contentRec.x + 25, contentRec.y + 130, 17, COLOR_WARNING, true);
                        } else {
                            DrawTxt(TextFormat("Sinh vien: %s %s | Mon: %s | Ket qua: %.2f diem", curStd->ho, curStd->ten, curSub->name, scNode->score), contentRec.x + 25, contentRec.y + 115, 17, COLOR_SUCCESS, true);

                            std::vector<ExamDetail*> details;
                            ExamDetail* dCur = scNode->detailList;
                            while (dCur != nullptr) { details.push_back(dCur); dCur = dCur->next; }

                            int totalReviewPages = details.empty() ? 1 : ((int)details.size() + REVIEW_PER_PAGE - 1) / REVIEW_PER_PAGE;
                            if (reviewQPage >= totalReviewPages) reviewQPage = totalReviewPages - 1;
                            if (reviewQPage < 0) reviewQPage = 0;

                            int startQ = reviewQPage * REVIEW_PER_PAGE;
                            int endQ = std::min(startQ + REVIEW_PER_PAGE, (int)details.size());
                            int startY = (int)contentRec.y + 145;

                            for (int q = startQ; q < endQ; ++q) {
                                int offset = q - startQ;
                                Rectangle dCard = { contentRec.x + 25, (float)startY + offset * 145, contentRec.width - 50, 135 };
                                DrawCardPanel(dCard, MakeColor(248, 250, 252));

                                ExamDetail* d = details[q];
                                DrawTxt(TextFormat("Cau %d:", q + 1), dCard.x + 15, dCard.y + 12, 16, COLOR_PRIMARY, true);
                                DrawTxtWrapped(d->content, dCard.x + 75, dCard.y + 12, dCard.width - 90, 16, COLOR_TEXT_DARK, true);

                                DrawTxt(TextFormat("A. %s", d->answerA), dCard.x + 25, dCard.y + 42, 14, COLOR_TEXT_MUTED);
                                DrawTxt(TextFormat("B. %s", d->answerB), dCard.x + 25, dCard.y + 68, 14, COLOR_TEXT_MUTED);
                                DrawTxt(TextFormat("C. %s", d->answerC), dCard.x + 450, dCard.y + 42, 14, COLOR_TEXT_MUTED);
                                DrawTxt(TextFormat("D. %s", d->answerD), dCard.x + 450, dCard.y + 68, 14, COLOR_TEXT_MUTED);

                                bool isCorrect = (d->selectedAnswer == d->correctAnswer);
                                DrawTxt(TextFormat("Ban chon: %c | Dap an dung: %c", d->selectedAnswer ? d->selectedAnswer : '-', d->correctAnswer), dCard.x + 25, dCard.y + 100, 15, isCorrect ? COLOR_SUCCESS : COLOR_DANGER, true);
                                DrawTxt(isCorrect ? "[ CHINH XAC ]" : "[ SAI ]", dCard.x + 350, dCard.y + 100, 15, isCorrect ? COLOR_SUCCESS : COLOR_DANGER, true);
                            }

                            int pageBarY = (int)(contentRec.y + contentRec.height - 45);
                            DrawTxt(TextFormat("Trang %d / %d (Tong: %d cau)", reviewQPage + 1, totalReviewPages, (int)details.size()), contentRec.x + 40, (float)pageBarY + 6, 15, COLOR_TEXT_MUTED, true);

                            if (GuiBtnOutline({ contentRec.x + contentRec.width - 240, (float)pageBarY, 105, 30 }, "< Trang truoc", COLOR_PRIMARY, COLOR_PRIMARY, 13, false, reviewQPage == 0)) {
                                if (reviewQPage > 0) reviewQPage--;
                            }
                            if (GuiBtnOutline({ contentRec.x + contentRec.width - 125, (float)pageBarY, 105, 30 }, "Trang sau >", COLOR_PRIMARY, COLOR_PRIMARY, 13, false, reviewQPage >= totalReviewPages - 1)) {
                                if (reviewQPage < totalReviewPages - 1) reviewQPage++;
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
                if (GuiBtn(tabRec, stdTabs[i], tabBg, COLOR_SIDEBAR_HOVER, WHITE, 15, active)) {
                    studentTab = i;
                }
            }

            Rectangle contentRec = { 260, 85, (float)(screenWidth - 280), (float)(screenHeight - 105) };
            DrawCardPanel(contentRec);

            // ==================== TAB 0: VAO THI ====================
            if (studentTab == 0) {
                DrawTxt("CHUAN BI THI TRAC NGHIEM", contentRec.x + 30, contentRec.y + 30, 22, COLOR_TEXT_DARK, true);

                char pendSub[32] = {};
                int pendSec = 0;
                int formStartY = (int)contentRec.y + 80;

                if (checkExamSession(loggedStudent->id, pendSub, pendSec)) {
                    Rectangle warnCard = { contentRec.x + 30, (float)formStartY, contentRec.width - 60, 95 };
                    DrawCardPanel(warnCard, MakeColor(254, 243, 199), COLOR_WARNING);
                    DrawTxt(TextFormat("[CANH BAO] Ban dang co phien thi do dang mon: %s (con %d phut %d giay)", pendSub, pendSec / 60, pendSec % 60), warnCard.x + 20, warnCard.y + 18, 16, COLOR_WARNING, true);

                    if (GuiBtn({ warnCard.x + 20, warnCard.y + 50, 230, 34 }, ">> TIEP TUC PHIEN THI <<", COLOR_PRIMARY, COLOR_PRIMARY_HOVER, WHITE, 14, true)) {
                        if (loadExamSession(loggedStudent->id, pendSub, pendSec, examHead, root)) {
                            examList.clear();
                            ExamDetail* cur = examHead;
                            while (cur != nullptr) { examList.push_back(cur); cur = cur->next; }
                            activeExamSubId = pendSub;
                            examRemainingSeconds = pendSec;
                            currentExamQuestionIdx = 0;
                            currentScreen = SCREEN_STUDENT_EXAM;
                        } else {
                            snprintf(notifyMessage, sizeof(notifyMessage), "Khong the khoi phuc phien thi cu.");
                            notifyTimer = 3.0f;
                            notifyIsError = true;
                        }
                    }
                    if (GuiBtnOutline({ warnCard.x + 270, warnCard.y + 50, 160, 34 }, "Huy phien thi do dang", COLOR_DANGER, COLOR_DANGER, 14, true)) {
                        removeExamSession(loggedStudent->id);
                        snprintf(notifyMessage, sizeof(notifyMessage), "Da huy phien thi cu.");
                        notifyTimer = 2.5f;
                        notifyIsError = false;
                    }

                    formStartY += 120;
                }

                DrawTxt("1. Chon mon hoc:", contentRec.x + 30, (float)formStartY, 17, COLOR_TEXT_DARK, true);
                if (allSubjects.empty()) {
                    DrawTxt("He thong chua co mon hoc nao de thi.", contentRec.x + 30, (float)formStartY + 35, 16, COLOR_TEXT_MUTED);
                } else {
                    int maxVis = 4;
                    if (studentSubNavOffset > 0) {
                        if (GuiBtnOutline({ contentRec.x + 30, (float)formStartY + 30, 35, 36 }, "<", COLOR_PRIMARY, COLOR_PRIMARY, 14, true)) {
                            studentSubNavOffset--;
                        }
                    }

                    for (int s = 0; s < maxVis; ++s) {
                        int idx = studentSubNavOffset + s;
                        if (idx >= (int)allSubjects.size()) break;
                        bool sel = (studentSubChoice == idx);
                        Rectangle btnRec = { contentRec.x + 75 + s * 155, (float)formStartY + 30, 145, 36 };
                        if (GuiBtn(btnRec, allSubjects[idx]->id, sel ? COLOR_PRIMARY : COLOR_SIDEBAR, COLOR_PRIMARY_HOVER, WHITE, 15, sel)) {
                            studentSubChoice = idx;
                        }
                    }

                    if (studentSubNavOffset + maxVis < (int)allSubjects.size()) {
                        if (GuiBtnOutline({ contentRec.x + 75 + maxVis * 155, (float)formStartY + 30, 35, 36 }, ">", COLOR_PRIMARY, COLOR_PRIMARY, 14, true)) {
                            studentSubNavOffset++;
                        }
                    }
                }

                if (!allSubjects.empty() && studentSubChoice < (int)allSubjects.size()) {
                    Subject* chosenSub = allSubjects[studentSubChoice];
                    int totalQ = countQuestions(chosenSub->questionList);
                    DrawTxt(TextFormat("-> Mon: %s | Ngan hang hien co: %d cau hoi", chosenSub->name, totalQ), contentRec.x + 30, (float)formStartY + 85, 15, COLOR_TEXT_MUTED);

                    DrawTxt("2. So luong cau hoi thi:", contentRec.x + 30, (float)formStartY + 125, 17, COLOR_TEXT_DARK, true);
                    Rectangle numQRec = { contentRec.x + 30, (float)formStartY + 155, 200, 42 };
                    GuiTextBoxInput(numQRec, numQuestionsInput, sizeof(numQuestionsInput), activeNumQ, "Nhap so cau");

                    DrawTxt("3. Thoi gian thi (phut):", contentRec.x + 280, (float)formStartY + 125, 17, COLOR_TEXT_DARK, true);
                    Rectangle minRec = { contentRec.x + 280, (float)formStartY + 155, 200, 42 };
                    GuiTextBoxInput(minRec, examMinutesInput, sizeof(examMinutesInput), activeMinutes, "Nhap so phut");

                    if (IsKeyPressed(KEY_TAB)) {
                        if (activeNumQ) { activeNumQ = false; activeMinutes = true; }
                        else if (activeMinutes) { activeMinutes = false; activeNumQ = true; }
                    }

                    if (GuiBtn({ contentRec.x + 30, (float)formStartY + 230, 320, 52 }, ">> BAT DAU THI NGAY <<", COLOR_SUCCESS, COLOR_SUCCESS_HOVER, WHITE, 18, true)) {
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
                            notifyTimer = 3.5f;
                            notifyIsError = true;
                        }
                    }

                    if (notifyTimer > 0 && notifyIsError) {
                        DrawTxt(notifyMessage, contentRec.x + 30, (float)formStartY + 300, 15, COLOR_DANGER, true);
                    }
                }
            }

            // ==================== TAB 1: BANG DIEM CA NHAN ====================
            else if (studentTab == 1) {
                DrawTxt("KET QUA THI CAC MON HOC CUA BAN", contentRec.x + 30, contentRec.y + 30, 22, COLOR_TEXT_DARK, true);

                int tableY = (int)contentRec.y + 80;
                DrawRectangle((int)contentRec.x + 30, tableY, (int)contentRec.width - 60, 40, COLOR_SIDEBAR);
                DrawTxt("MA MON HOC", contentRec.x + 50, (float)tableY + 11, 15, WHITE, true);
                DrawTxt("TEN MON HOC", contentRec.x + 250, (float)tableY + 11, 15, WHITE, true);
                DrawTxt("DIEM THI (Thang 10)", contentRec.x + 700, (float)tableY + 11, 15, WHITE, true);

                Score* sc = loggedStudent->scoreList;
                int idx = 0;
                while (sc != nullptr) {
                    int rowY = tableY + 45 + idx * 45;
                    Color rowBg = (idx % 2 == 0) ? COLOR_CARD : MakeColor(248, 250, 252);
                    DrawRectangle((int)contentRec.x + 30, rowY, (int)contentRec.width - 60, 42, rowBg);
                    DrawRectangleLines((int)contentRec.x + 30, rowY, (int)contentRec.width - 60, 42, COLOR_BORDER);

                    DrawTxt(sc->subjectId, contentRec.x + 50, (float)rowY + 11, 15, COLOR_PRIMARY, true);
                    Subject* sNode = findSubject(root, sc->subjectId);
                    DrawTxt(sNode ? sNode->name : "Mon hoc", contentRec.x + 250, (float)rowY + 11, 15, COLOR_TEXT_DARK);
                    DrawTxt(TextFormat("%.2f diem", sc->score), contentRec.x + 700, (float)rowY + 11, 17, COLOR_SUCCESS, true);

                    sc = sc->next;
                    idx++;
                }

                if (idx == 0) {
                    DrawTxt("Ban chua co diem mon nao trong he thong. Hay vao tab 'Vao thi' de bat dau.", contentRec.x + 50, (float)tableY + 60, 16, COLOR_TEXT_MUTED);
                }
            }

            // ==================== TAB 2: XEM LAI BAI THI ====================
            else if (studentTab == 2) {
                DrawTxt("XEM LAI BAI THI DA HOAN THANH", contentRec.x + 30, contentRec.y + 30, 22, COLOR_TEXT_DARK, true);

                Score* sc = loggedStudent->scoreList;
                int sIdx = 0;
                DrawTxt("Chon Mon:", contentRec.x + 30, contentRec.y + 70, 15, COLOR_TEXT_DARK, true);
                while (sc != nullptr && sIdx < 4) {
                    bool sel = (studentSubChoice == sIdx);
                    if (GuiBtn({ contentRec.x + 120 + sIdx * 120, contentRec.y + 65, 110, 32 }, sc->subjectId, sel ? COLOR_PRIMARY : COLOR_SIDEBAR, COLOR_PRIMARY_HOVER, WHITE, 14, sel)) {
                        studentSubChoice = sIdx;
                        reviewQPage = 0;
                    }
                    sc = sc->next;
                    sIdx++;
                }

                if (sIdx == 0) {
                    DrawTxt("Ban chua co bai thi nao da hoan thanh de xem lai.", contentRec.x + 30, contentRec.y + 120, 16, COLOR_TEXT_MUTED);
                } else {
                    sc = loggedStudent->scoreList;
                    int curIdx = 0;
                    while (sc != nullptr && curIdx < studentSubChoice) {
                        sc = sc->next;
                        curIdx++;
                    }

                    if (sc != nullptr) {
                        DrawTxt(TextFormat("Ket qua: %.2f / 10.0 diem", sc->score), contentRec.x + 30, contentRec.y + 115, 17, COLOR_SUCCESS, true);

                        std::vector<ExamDetail*> details;
                        ExamDetail* dCur = sc->detailList;
                        while (dCur != nullptr) { details.push_back(dCur); dCur = dCur->next; }

                        int totalReviewPages = details.empty() ? 1 : ((int)details.size() + REVIEW_PER_PAGE - 1) / REVIEW_PER_PAGE;
                        if (reviewQPage >= totalReviewPages) reviewQPage = totalReviewPages - 1;
                        if (reviewQPage < 0) reviewQPage = 0;

                        int startQ = reviewQPage * REVIEW_PER_PAGE;
                        int endQ = std::min(startQ + REVIEW_PER_PAGE, (int)details.size());
                        int startY = (int)contentRec.y + 145;

                        for (int q = startQ; q < endQ; ++q) {
                            int offset = q - startQ;
                            Rectangle dCard = { contentRec.x + 30, (float)startY + offset * 145, contentRec.width - 60, 135 };
                            DrawCardPanel(dCard, MakeColor(248, 250, 252));

                            ExamDetail* d = details[q];
                            DrawTxt(TextFormat("Cau %d:", q + 1), dCard.x + 15, dCard.y + 12, 16, COLOR_PRIMARY, true);
                            DrawTxtWrapped(d->content, dCard.x + 75, dCard.y + 12, dCard.width - 90, 16, COLOR_TEXT_DARK, true);

                            DrawTxt(TextFormat("A. %s", d->answerA), dCard.x + 25, dCard.y + 42, 14, COLOR_TEXT_MUTED);
                            DrawTxt(TextFormat("B. %s", d->answerB), dCard.x + 25, dCard.y + 68, 14, COLOR_TEXT_MUTED);
                            DrawTxt(TextFormat("C. %s", d->answerC), dCard.x + 450, dCard.y + 42, 14, COLOR_TEXT_MUTED);
                            DrawTxt(TextFormat("D. %s", d->answerD), dCard.x + 450, dCard.y + 68, 14, COLOR_TEXT_MUTED);

                            bool isCorrect = (d->selectedAnswer == d->correctAnswer);
                            DrawTxt(TextFormat("Ban chon: %c | Dap an dung: %c", d->selectedAnswer ? d->selectedAnswer : '-', d->correctAnswer), dCard.x + 25, dCard.y + 100, 15, isCorrect ? COLOR_SUCCESS : COLOR_DANGER, true);
                            DrawTxt(isCorrect ? "[ CHINH XAC ]" : "[ SAI ]", dCard.x + 350, dCard.y + 100, 15, isCorrect ? COLOR_SUCCESS : COLOR_DANGER, true);
                        }

                        int pageBarY = (int)(contentRec.y + contentRec.height - 45);
                        DrawTxt(TextFormat("Trang %d / %d (Tong: %d cau)", reviewQPage + 1, totalReviewPages, (int)details.size()), contentRec.x + 40, (float)pageBarY + 6, 15, COLOR_TEXT_MUTED, true);

                        if (GuiBtnOutline({ contentRec.x + contentRec.width - 240, (float)pageBarY, 105, 30 }, "< Trang truoc", COLOR_PRIMARY, COLOR_PRIMARY, 13, false, reviewQPage == 0)) {
                            if (reviewQPage > 0) reviewQPage--;
                        }
                        if (GuiBtnOutline({ contentRec.x + contentRec.width - 125, (float)pageBarY, 105, 30 }, "Trang sau >", COLOR_PRIMARY, COLOR_PRIMARY, 13, false, reviewQPage >= totalReviewPages - 1)) {
                            if (reviewQPage < totalReviewPages - 1) reviewQPage++;
                        }
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

            if (GuiBtn({ (float)(screenWidth - 160), 15, 130, 40 }, "NOP BAI", COLOR_WARNING, COLOR_PRIMARY_HOVER, WHITE, 17, true)) {
                currentModal = MODAL_CONFIRM_SUBMIT;
            }

            Rectangle qPane = { 30, 95, 840, (float)(screenHeight - 125) };
            DrawCardPanel(qPane);

            ExamDetail* curQ = examList[currentExamQuestionIdx];
            DrawTxt(TextFormat("CAU HOI %d / %d", currentExamQuestionIdx + 1, (int)examList.size()), qPane.x + 30, qPane.y + 25, 20, COLOR_PRIMARY, true);
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
                if (!gModalOpen && CheckCollisionPointRec(mouse, ansRec) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    curQ->selectedAnswer = choices[i].letter;
                    saveExamSession(loggedStudent->id, activeExamSubId.c_str(), examRemainingSeconds, examHead);
                }
            }

            if (currentExamQuestionIdx > 0) {
                if (GuiBtnOutline({ qPane.x + 30, qPane.y + qPane.height - 60, 150, 42 }, "< Cau truoc", COLOR_PRIMARY, COLOR_PRIMARY, 15, true)) {
                    currentExamQuestionIdx--;
                }
            }
            if (currentExamQuestionIdx < (int)examList.size() - 1) {
                if (GuiBtn({ qPane.x + qPane.width - 180, qPane.y + qPane.height - 60, 150, 42 }, "Cau tiep >", COLOR_PRIMARY, COLOR_PRIMARY_HOVER, WHITE, 15, true)) {
                    currentExamQuestionIdx++;
                }
            }

            // Lưới danh sách các câu hỏi bên phải
            Rectangle gridPane = { 890, 95, (float)(screenWidth - 920), (float)(screenHeight - 125) };
            DrawCardPanel(gridPane);

            DrawTxt("DANH SACH CAU HOI", gridPane.x + 25, gridPane.y + 22, 17, COLOR_TEXT_DARK, true);
            DrawTxt("Xanh: Da lam | Trang: Chua lam", gridPane.x + 25, gridPane.y + 48, 13, COLOR_TEXT_MUTED);

            int btnSize = (examList.size() > 36) ? 36 : 42;
            int btnGap = (examList.size() > 36) ? 8 : 12;
            int cols = 6;
            for (size_t i = 0; i < examList.size(); ++i) {
                int col = (int)i % cols;
                int row = (int)i / cols;
                Rectangle numRec = { (float)(gridPane.x + 25 + col * (btnSize + btnGap)), (float)(gridPane.y + 80 + row * (btnSize + btnGap)), (float)btnSize, (float)btnSize };

                bool isAnswered = (examList[i]->selectedAnswer != '\0');
                bool isCurrent = (currentExamQuestionIdx == (int)i);

                Color numBg = isAnswered ? COLOR_SUCCESS : MakeColor(241, 245, 249);
                Color numText = isAnswered ? WHITE : COLOR_TEXT_DARK;

                DrawRectangleRounded(numRec, 0.2f, 4, numBg);
                if (isCurrent) {
                    DrawRectangleRoundedLines(numRec, 0.2f, 4, COLOR_PRIMARY);
                }

                const char* numStr = TextFormat("%d", (int)i + 1);
                DrawTxt(numStr, numRec.x + (btnSize - MeasureTxt(numStr, 15, true)) / 2.0f, numRec.y + (btnSize - 15) / 2.0f, 15, numText, true);

                Vector2 mouse = GetMousePosition();
                if (!gModalOpen && CheckCollisionPointRec(mouse, numRec) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    currentExamQuestionIdx = (int)i;
                }
            }
        }

        // ----------------------------------------------------
        // 5. SCREEN EXAM RESULT
        // ----------------------------------------------------
        else if (currentScreen == SCREEN_EXAM_RESULT) {
            DrawRectangle(0, 0, screenWidth, screenHeight, COLOR_BG);

            Rectangle card = { (float)(screenWidth / 2 - 250), (float)(screenHeight / 2 - 210), 500, 420 };
            DrawCardPanel(card);

            DrawTxt("HOAN THANH BAI THI!", card.x + (500 - MeasureTxt("HOAN THANH BAI THI!", 24, true)) / 2.0f, card.y + 35, 24, COLOR_SUCCESS, true);
            DrawTxt(TextFormat("Mon thi: %s", activeExamSubId.c_str()), card.x + 40, card.y + 90, 17, COLOR_TEXT_DARK);

            DrawTxt("DIEM SO DAT DUOC:", card.x + (500 - MeasureTxt("DIEM SO DAT DUOC:", 17, true)) / 2.0f, card.y + 130, 17, COLOR_TEXT_MUTED, true);
            const char* scoreTxt = TextFormat("%.2f", lastScore);
            DrawTxt(scoreTxt, card.x + (500 - MeasureTxt(scoreTxt, 48, true)) / 2.0f, card.y + 165, 48, COLOR_PRIMARY, true);

            DrawTxt(TextFormat("Ket qua: %d / %d cau dung", lastCorrect, lastTotal), card.x + (500 - MeasureTxt(TextFormat("Ket qua: %d / %d cau dung", lastCorrect, lastTotal), 17)) / 2.0f, card.y + 240, 17, COLOR_TEXT_DARK);

            bool passed = (lastScore >= 5.0f);
            const char* badge = passed ? "[ DAT ]" : "[ CHUA DAT ]";
            Color badgeC = passed ? COLOR_SUCCESS : COLOR_DANGER;
            DrawTxt(badge, card.x + (500 - MeasureTxt(badge, 17, true)) / 2.0f, card.y + 275, 17, badgeC, true);

            if (GuiBtn({ card.x + 40, card.y + 330, 420, 48 }, "VE MAN HINH CHINH", COLOR_PRIMARY, COLOR_PRIMARY_HOVER, WHITE, 17, true)) {
                currentScreen = SCREEN_STUDENT_DASHBOARD;
                studentTab = 1;
            }
        }

        // ==================== MODAL DIALOGS ====================
        if (currentModal != MODAL_NONE) {
            gInsideModal = true;
            // Lớp phủ nền mờ làm nổi bật Modal
            DrawRectangle(0, 0, screenWidth, screenHeight, MakeColor(0, 0, 0, 130));

            // Phím ESC hủy Modal
            if (IsKeyPressed(KEY_ESCAPE)) {
                currentModal = MODAL_NONE;
            }

            Rectangle modalRec = { (float)(screenWidth / 2 - 250), (float)(screenHeight / 2 - 180), 500, 360 };

            // Phím Tab điều hướng giữa các trường
            if (IsKeyPressed(KEY_TAB)) {
                if (currentModal == MODAL_ADD_SUBJECT || currentModal == MODAL_EDIT_SUBJECT || currentModal == MODAL_ADD_CLASS) {
                    activeFormBox = (activeFormBox == 1) ? 2 : 1;
                } else if (currentModal == MODAL_ADD_STUDENT || currentModal == MODAL_EDIT_STUDENT) {
                    activeFormBox++;
                    if (activeFormBox > 5) activeFormBox = (currentModal == MODAL_ADD_STUDENT) ? 1 : 2;
                } else if (currentModal == MODAL_ADD_QUESTION || currentModal == MODAL_EDIT_QUESTION) {
                    activeFormBox++;
                    if (activeFormBox > 5) activeFormBox = 1;
                }
            }

            // MODAL 1: MON HOC
            if (currentModal == MODAL_ADD_SUBJECT || currentModal == MODAL_EDIT_SUBJECT) {
                DrawCardPanel(modalRec);
                DrawTxt((currentModal == MODAL_ADD_SUBJECT) ? "THEM MON HOC MOI" : "HIEU CHINH MON HOC", modalRec.x + 30, modalRec.y + 25, 20, COLOR_TEXT_DARK, true);

                DrawTxt("Ma mon hoc (MAMH <= 15):", modalRec.x + 30, modalRec.y + 65, 14, COLOR_TEXT_DARK);
                bool actId = (activeFormBox == 1);
                GuiTextBoxInput({ modalRec.x + 30, modalRec.y + 88, 440, 38 }, formId, sizeof(formId), actId, "VD: CTDL");
                if (actId) activeFormBox = 1;

                DrawTxt("Ten mon hoc:", modalRec.x + 30, modalRec.y + 140, 14, COLOR_TEXT_DARK);
                bool actName = (activeFormBox == 2);
                GuiTextBoxInput({ modalRec.x + 30, modalRec.y + 163, 440, 38 }, formName, sizeof(formName), actName, "VD: Cau truc du lieu & Giai thuat");
                if (actName) activeFormBox = 2;

                if (modalError[0] != '\0') {
                    DrawTxt(modalError, modalRec.x + 30, modalRec.y + 215, 14, COLOR_DANGER, true);
                }

                bool submitSub = GuiBtn({ modalRec.x + 30, modalRec.y + 270, 200, 42 }, "Luu Mon Hoc", COLOR_SUCCESS, COLOR_SUCCESS_HOVER, WHITE, 16, true);
                if (IsKeyPressed(KEY_ENTER)) submitSub = true;

                if (submitSub) {
                    BusinessResult res = SUCCESS;
                    if (currentModal == MODAL_ADD_SUBJECT) {
                        res = addSubjectWorkflow(root, formId, formName);
                    } else {
                        res = editSubjectWorkflow(root, classes, classCount, formId, formId, formName);
                    }

                    if (res == SUCCESS) {
                        currentModal = MODAL_NONE;
                        snprintf(notifyMessage, sizeof(notifyMessage), "Da luu mon hoc thanh cong!");
                        notifyTimer = 3.0f;
                        notifyIsError = false;
                    } else {
                        snprintf(modalError, sizeof(modalError), "%s", getErrorMessage(res));
                    }
                }
                if (GuiBtnOutline({ modalRec.x + 270, modalRec.y + 270, 200, 42 }, "Huy bo", COLOR_DANGER, COLOR_DANGER, 16, true)) {
                    currentModal = MODAL_NONE;
                }
            }
            // MODAL 2: XOA MON HOC
            else if (currentModal == MODAL_DELETE_SUBJECT) {
                DrawCardPanel(modalRec);
                DrawTxt("XAC NHAN XOA MON HOC", modalRec.x + 30, modalRec.y + 30, 20, COLOR_DANGER, true);
                DrawTxt(TextFormat("Ban co chac chan muon xoa mon hoc [%s]?", formId), modalRec.x + 30, modalRec.y + 80, 16, COLOR_TEXT_DARK);
                DrawTxt("Luu y: Chi xoa duoc mon chua co diem thi trong he thong.", modalRec.x + 30, modalRec.y + 115, 14, COLOR_TEXT_MUTED);

                if (modalError[0] != '\0') {
                    DrawTxt(modalError, modalRec.x + 30, modalRec.y + 155, 15, COLOR_DANGER, true);
                }

                if (GuiBtn({ modalRec.x + 30, modalRec.y + 210, 200, 42 }, "Xac Nhan Xoa", COLOR_DANGER, COLOR_DANGER_HOVER, WHITE, 16, true)) {
                    BusinessResult res = deleteSubjectWorkflow(root, classes, classCount, formId);
                    if (res == SUCCESS) {
                        currentModal = MODAL_NONE;
                        snprintf(notifyMessage, sizeof(notifyMessage), "Da xoa mon hoc [%s]!", formId);
                        notifyTimer = 3.0f;
                        notifyIsError = false;
                    } else {
                        snprintf(modalError, sizeof(modalError), "%s", getErrorMessage(res));
                    }
                }
                if (GuiBtnOutline({ modalRec.x + 270, modalRec.y + 210, 200, 42 }, "Huy bo", COLOR_PRIMARY, COLOR_PRIMARY, 16, true)) {
                    currentModal = MODAL_NONE;
                }
            }
            // MODAL 3: THEM LOP HOC
            else if (currentModal == MODAL_ADD_CLASS) {
                DrawCardPanel(modalRec);
                DrawTxt("THEM LOP HOC MOI", modalRec.x + 30, modalRec.y + 25, 20, COLOR_TEXT_DARK, true);

                DrawTxt("Ma lop hoc (MALOP <= 19):", modalRec.x + 30, modalRec.y + 65, 14, COLOR_TEXT_DARK);
                bool actId = (activeFormBox == 1);
                GuiTextBoxInput({ modalRec.x + 30, modalRec.y + 88, 440, 38 }, formId, sizeof(formId), actId, "VD: D23CQCN01");
                if (actId) activeFormBox = 1;

                DrawTxt("Ten lop hoc:", modalRec.x + 30, modalRec.y + 140, 14, COLOR_TEXT_DARK);
                bool actName = (activeFormBox == 2);
                GuiTextBoxInput({ modalRec.x + 30, modalRec.y + 163, 440, 38 }, formName, sizeof(formName), actName, "VD: Cong nghe thong tin 1");
                if (actName) activeFormBox = 2;

                if (modalError[0] != '\0') {
                    DrawTxt(modalError, modalRec.x + 30, modalRec.y + 215, 14, COLOR_DANGER, true);
                }

                bool submitClass = GuiBtn({ modalRec.x + 30, modalRec.y + 270, 200, 42 }, "Luu Lop Hoc", COLOR_SUCCESS, COLOR_SUCCESS_HOVER, WHITE, 16, true);
                if (IsKeyPressed(KEY_ENTER)) submitClass = true;

                if (submitClass) {
                    BusinessResult res = addClassWorkflow(classes, classCount, formId, formName);
                    if (res == SUCCESS) {
                        currentModal = MODAL_NONE;
                        snprintf(notifyMessage, sizeof(notifyMessage), "Da them lop hoc thanh cong!");
                        notifyTimer = 3.0f;
                        notifyIsError = false;
                    } else {
                        snprintf(modalError, sizeof(modalError), "%s", getErrorMessage(res));
                    }
                }
                if (GuiBtnOutline({ modalRec.x + 270, modalRec.y + 270, 200, 42 }, "Huy bo", COLOR_DANGER, COLOR_DANGER, 16, true)) {
                    currentModal = MODAL_NONE;
                }
            }
            // MODAL 4: XOA LOP HOC
            else if (currentModal == MODAL_DELETE_CLASS) {
                DrawCardPanel(modalRec);
                DrawTxt("XAC NHAN XOA LOP HOC", modalRec.x + 30, modalRec.y + 30, 20, COLOR_DANGER, true);
                DrawTxt(TextFormat("Ban co chac chan muon xoa lop [%s]?", formId), modalRec.x + 30, modalRec.y + 80, 16, COLOR_TEXT_DARK);
                DrawTxt("Luu y: Khong duoc xoa neu lop hoc dang co sinh vien.", modalRec.x + 30, modalRec.y + 115, 14, COLOR_TEXT_MUTED);

                if (modalError[0] != '\0') {
                    DrawTxt(modalError, modalRec.x + 30, modalRec.y + 155, 15, COLOR_DANGER, true);
                }

                if (GuiBtn({ modalRec.x + 30, modalRec.y + 210, 200, 42 }, "Xac Nhan Xoa", COLOR_DANGER, COLOR_DANGER_HOVER, WHITE, 16, true)) {
                    BusinessResult res = deleteClassWorkflow(classes, classCount, formId);
                    if (res == SUCCESS) {
                        currentModal = MODAL_NONE;
                        snprintf(notifyMessage, sizeof(notifyMessage), "Da xoa lop hoc [%s]!", formId);
                        notifyTimer = 3.0f;
                        notifyIsError = false;
                    } else {
                        snprintf(modalError, sizeof(modalError), "%s", getErrorMessage(res));
                    }
                }
                if (GuiBtnOutline({ modalRec.x + 270, modalRec.y + 210, 200, 42 }, "Huy bo", COLOR_PRIMARY, COLOR_PRIMARY, 16, true)) {
                    currentModal = MODAL_NONE;
                }
            }
            // MODAL 5: SINH VIEN
            else if (currentModal == MODAL_ADD_STUDENT || currentModal == MODAL_EDIT_STUDENT) {
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

                if (modalError[0] != '\0') {
                    DrawTxt(modalError, bigModal.x + 30, bigModal.y + 270, 14, COLOR_DANGER, true);
                }

                bool submitStd = GuiBtn({ bigModal.x + 30, bigModal.y + 360, 230, 42 }, "Luu Sinh Vien", COLOR_SUCCESS, COLOR_SUCCESS_HOVER, WHITE, 16, true);
                if (IsKeyPressed(KEY_ENTER)) submitStd = true;

                if (submitStd) {
                    if (selectedClassIdx < classCount && classes[selectedClassIdx] != nullptr) {
                        BusinessResult res = SUCCESS;
                        if (currentModal == MODAL_ADD_STUDENT) {
                            res = addStudentWorkflow(classes, classCount, classes[selectedClassIdx]->id, formId, formHo, formTen, formGender, formPass);
                        } else {
                            res = editStudentWorkflow(classes, classCount, formId, formHo, formTen, formGender, formPass);
                        }

                        if (res == SUCCESS) {
                            currentModal = MODAL_NONE;
                            snprintf(notifyMessage, sizeof(notifyMessage), "Da luu sinh vien thanh cong!");
                            notifyTimer = 3.0f;
                            notifyIsError = false;
                        } else {
                            snprintf(modalError, sizeof(modalError), "%s", getErrorMessage(res));
                        }
                    }
                }
                if (GuiBtnOutline({ bigModal.x + 280, bigModal.y + 360, 230, 42 }, "Huy bo", COLOR_DANGER, COLOR_DANGER, 16, true)) {
                    currentModal = MODAL_NONE;
                }
            }
            // MODAL 6: XOA SINH VIEN
            else if (currentModal == MODAL_DELETE_STUDENT) {
                DrawCardPanel(modalRec);
                DrawTxt("XAC NHAN XOA SINH VIEN", modalRec.x + 30, modalRec.y + 30, 20, COLOR_DANGER, true);
                DrawTxt(TextFormat("Ban co chac chan muon xoa sinh vien [%s]?", formId), modalRec.x + 30, modalRec.y + 80, 16, COLOR_TEXT_DARK);
                DrawTxt("Luu y: Chi xoa duoc neu sinh vien chua co diem thi.", modalRec.x + 30, modalRec.y + 115, 14, COLOR_TEXT_MUTED);

                if (modalError[0] != '\0') {
                    DrawTxt(modalError, modalRec.x + 30, modalRec.y + 155, 15, COLOR_DANGER, true);
                }

                if (GuiBtn({ modalRec.x + 30, modalRec.y + 210, 200, 42 }, "Xac Nhan Xoa", COLOR_DANGER, COLOR_DANGER_HOVER, WHITE, 16, true)) {
                    if (selectedClassIdx < classCount && classes[selectedClassIdx] != nullptr) {
                        BusinessResult res = deleteStudentWorkflow(classes, classCount, classes[selectedClassIdx]->id, formId);
                        if (res == SUCCESS) {
                            currentModal = MODAL_NONE;
                            snprintf(notifyMessage, sizeof(notifyMessage), "Da xoa sinh vien [%s]!", formId);
                            notifyTimer = 3.0f;
                            notifyIsError = false;
                        } else {
                            snprintf(modalError, sizeof(modalError), "%s", getErrorMessage(res));
                        }
                    }
                }
                if (GuiBtnOutline({ modalRec.x + 270, modalRec.y + 210, 200, 42 }, "Huy bo", COLOR_PRIMARY, COLOR_PRIMARY, 16, true)) {
                    currentModal = MODAL_NONE;
                }
            }
            // MODAL 7: THEM HOAC SUA CAU HOI
            else if (currentModal == MODAL_ADD_QUESTION || currentModal == MODAL_EDIT_QUESTION) {
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

                if (modalError[0] != '\0') {
                    DrawTxt(modalError, qModal.x + 30, qModal.y + 420, 14, COLOR_DANGER, true);
                }

                bool submitQ = GuiBtn({ qModal.x + 30, qModal.y + 460, 270, 42 }, "Luu Cau Hoi", COLOR_SUCCESS, COLOR_SUCCESS_HOVER, WHITE, 16, true);
                if (IsKeyPressed(KEY_ENTER)) submitQ = true;

                if (submitQ) {
                    if (selectedSubjectIdx < (int)allSubjects.size()) {
                        Subject* sNode = allSubjects[selectedSubjectIdx];
                        BusinessResult res = SUCCESS;
                        if (currentModal == MODAL_ADD_QUESTION) {
                            int newId = 0;
                            res = addQuestionWorkflow(root, sNode->id, formContent, formA, formB, formC, formD, formCorrect[0], newId);
                        } else {
                            res = editQuestionWorkflow(root, sNode->id, formEditQId, formContent, formA, formB, formC, formD, formCorrect[0]);
                        }

                        if (res == SUCCESS) {
                            currentModal = MODAL_NONE;
                            snprintf(notifyMessage, sizeof(notifyMessage), "Da luu cau hoi thanh cong!");
                            notifyTimer = 3.0f;
                            notifyIsError = false;
                        } else {
                            snprintf(modalError, sizeof(modalError), "%s", getErrorMessage(res));
                        }
                    }
                }
                if (GuiBtnOutline({ qModal.x + 340, qModal.y + 460, 270, 42 }, "Huy bo", COLOR_DANGER, COLOR_DANGER, 16, true)) {
                    currentModal = MODAL_NONE;
                }
            }
            // MODAL 8: XOA CAU HOI THI
            else if (currentModal == MODAL_DELETE_QUESTION) {
                DrawCardPanel(modalRec);
                DrawTxt("XAC NHAN XOA CAU HOI THI", modalRec.x + 30, modalRec.y + 30, 20, COLOR_DANGER, true);
                DrawTxt(TextFormat("Ban co chac chan muon xoa cau hoi ID: %d?", formEditQId), modalRec.x + 30, modalRec.y + 80, 16, COLOR_TEXT_DARK);
                DrawTxt("Luu y: Chi xoa duoc cau hoi neu chua co sinh vien thi mon nay.", modalRec.x + 30, modalRec.y + 115, 14, COLOR_TEXT_MUTED);

                if (modalError[0] != '\0') {
                    DrawTxt(modalError, modalRec.x + 30, modalRec.y + 155, 15, COLOR_DANGER, true);
                }

                if (GuiBtn({ modalRec.x + 30, modalRec.y + 210, 200, 42 }, "Xac Nhan Xoa", COLOR_DANGER, COLOR_DANGER_HOVER, WHITE, 16, true)) {
                    if (selectedSubjectIdx < (int)allSubjects.size()) {
                        Subject* sNode = allSubjects[selectedSubjectIdx];
                        BusinessResult res = deleteQuestionWorkflow(root, sNode->id, formEditQId);
                        if (res == SUCCESS) {
                            currentModal = MODAL_NONE;
                            snprintf(notifyMessage, sizeof(notifyMessage), "Da xoa cau hoi ID %d!", formEditQId);
                            notifyTimer = 3.0f;
                            notifyIsError = false;
                        } else {
                            snprintf(modalError, sizeof(modalError), "%s", getErrorMessage(res));
                        }
                    }
                }
                if (GuiBtnOutline({ modalRec.x + 270, modalRec.y + 210, 200, 42 }, "Huy bo", COLOR_PRIMARY, COLOR_PRIMARY, 16, true)) {
                    currentModal = MODAL_NONE;
                }
            }
            // MODAL 9: XAC NHAN NOP BAI THI
            else if (currentModal == MODAL_CONFIRM_SUBMIT) {
                DrawCardPanel(modalRec);
                DrawTxt("XAC NHAN NOP BAI THI", modalRec.x + 30, modalRec.y + 30, 20, COLOR_WARNING, true);

                int answeredCount = 0;
                for (size_t i = 0; i < examList.size(); ++i) {
                    if (examList[i]->selectedAnswer != '\0') answeredCount++;
                }

                DrawTxt(TextFormat("Ban da tra loi: %d / %d cau hoi.", answeredCount, (int)examList.size()), modalRec.x + 30, modalRec.y + 80, 17, COLOR_TEXT_DARK);
                DrawTxt("Ban co chac chan muon nop bai ngay bay gio?", modalRec.x + 30, modalRec.y + 115, 15, COLOR_TEXT_MUTED);

                if (GuiBtn({ modalRec.x + 30, modalRec.y + 210, 200, 44 }, "Dong Y Nop Bai", COLOR_SUCCESS, COLOR_SUCCESS_HOVER, WHITE, 16, true)) {
                    currentModal = MODAL_NONE;
                    submitExamWorkflow(loggedStudent, activeExamSubId.c_str(), examHead, lastScore, lastCorrect, lastTotal);
                    removeExamSession(loggedStudent->id);
                    saveAllData(root, classes, classCount, "data");
                    examHead = nullptr;
                    examList.clear();
                    currentScreen = SCREEN_EXAM_RESULT;
                    snprintf(notifyMessage, sizeof(notifyMessage), "Da nop bai thi thanh cong!");
                    notifyTimer = 3.5f;
                    notifyIsError = false;
                }
                if (GuiBtnOutline({ modalRec.x + 270, modalRec.y + 210, 200, 44 }, "Quay Lai Lam Tiep", COLOR_PRIMARY, COLOR_PRIMARY, 16, true)) {
                    currentModal = MODAL_NONE;
                }
            }
        }

        // ==================== TOAST NOTIFICATION BANNER ====================
        if (notifyTimer > 0) {
            DrawToastNotification(notifyMessage, notifyTimer, notifyIsError);
        }

        EndDrawing();
    }

    // Cleanup khi thoat chuong trinh
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
