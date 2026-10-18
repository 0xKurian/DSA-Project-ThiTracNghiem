// cai dat widget trong GuiCommon.h
#include "GuiCommon.h"

#include "../core/QuestionList.h"
#include "../core/StudentList.h"
#include "../core/SubjectBST.h"

#include <cstring>
#include <string>

static Font gFontRegular;
static Font gFontBold;

static bool gModalOpen = false;
static bool gInsideModal = false;

void GuiSetModalState(bool modalOpen) {
    gModalOpen = modalOpen;
    gInsideModal = false;
}

void GuiMarkInsideModal() {
    gInsideModal = true;
}

static std::vector<int> getVietnameseCodepoints() {
    std::vector<int> cps;
    // ASCII co ban: 32..126
    for (int i = 32; i <= 126; ++i) cps.push_back(i);
    // Latin-1 Supplement: 160..255 (a, a, a, a, e, e, e, i, i, o, o, o, u, u, y, ...)
    for (int i = 160; i <= 255; ++i) cps.push_back(i);
    // Latin Extended-A: 0x0100..0x017F (D = 0x0110, d = 0x0111, a, A, ...)
    for (int i = 0x0100; i <= 0x017F; ++i) cps.push_back(i);
    // Latin Extended-B: 0x01A0..0x01B0 (o, u, O, U)
    for (int i = 0x01A0; i <= 0x01B0; ++i) cps.push_back(i);
    // Latin Extended Additional: 0x1EA0..0x1EF9 (ky tu tieng Viet co dau to hop san)
    for (int i = 0x1EA0; i <= 0x1EF9; ++i) cps.push_back(i);
    // Dau cau thong dung: 0x2010..0x2026
    for (int i = 0x2010; i <= 0x2026; ++i) cps.push_back(i);
    return cps;
}

void loadVietnameseFonts() {
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
}

void unloadVietnameseFonts() {
    if (gFontRegular.texture.id != GetFontDefault().texture.id) UnloadFont(gFontRegular);
    if (gFontBold.texture.id != GetFontDefault().texture.id) UnloadFont(gFontBold);
}

void DrawTxt(const char* text, float posX, float posY, float fontSize, Color color, bool bold) {
    if (text == nullptr || text[0] == '\0') return;
    Font f = bold ? gFontBold : gFontRegular;
    DrawTextEx(f, text, Vector2{ posX, posY }, fontSize, 1.0f, color);
}

float MeasureTxt(const char* text, float fontSize, bool bold) {
    if (text == nullptr || text[0] == '\0') return 0.0f;
    Font f = bold ? gFontBold : gFontRegular;
    Vector2 sz = MeasureTextEx(f, text, fontSize, 1.0f);
    return sz.x;
}

float DrawTxtWrapped(const char* text, float posX, float posY, float maxWidth, float fontSize, Color color, bool bold, float lineSpacing) {
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

bool GuiBtn(Rectangle rec, const char* text, Color bg, Color hoverBg, Color textColor, float fontSize, bool bold, bool disabled) {
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

bool GuiBtnOutline(Rectangle rec, const char* text, Color borderColor, Color textColor, float fontSize, bool bold, bool disabled) {
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

void GuiTextBoxInput(Rectangle rec, char textBuffer[], int maxLen, bool& isActive, const char* placeholder, bool isPassword) {
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

void DrawCardPanel(Rectangle rec, Color bg, Color border) {
    DrawRectangleRounded(rec, 0.08f, 6, bg);
    DrawRectangleRoundedLines(rec, 0.08f, 6, border);
}

void DrawToastNotification(const char* message, float timer, bool isError) {
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
    float cardX = ((float)GUI_WIDTH - cardW) / 2.0f;
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

void collectSubjects(Subject* root, std::vector<Subject*>& list) {
    if (root == nullptr) return;
    collectSubjects(root->left, list);
    list.push_back(root);
    collectSubjects(root->right, list);
}

void collectQuestions(Question* head, std::vector<Question*>& list) {
    Question* cur = head;
    while (cur != nullptr) {
        list.push_back(cur);
        cur = cur->next;
    }
}

void collectStudents(Student* head, std::vector<Student*>& list) {
    Student* cur = head;
    while (cur != nullptr) {
        list.push_back(cur);
        cur = cur->next;
    }
}
