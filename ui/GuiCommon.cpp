#include "GuiCommon.h"

#include "../core/QuestionList.h"
#include "../core/StudentList.h"
#include "../core/SubjectBST.h"
#include "../core/Structures.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <string>

static Font gFontRegular;
static int suggestScroll = 0;
static Font gFontBold;

static bool gModalOpen = false;
static bool gInsideModal = false;

static bool gShieldPrev = false;
static bool gShieldCur = false;
static Rectangle gShieldPrevRec = { 0, 0, 0, 0 };
static Rectangle gShieldCurRec = { 0, 0, 0, 0 };

void GuiBeginFrame() {
    gShieldPrev = gShieldCur;
    gShieldPrevRec = gShieldCurRec;
    gShieldCur = false;
}

static bool clickShielded(Vector2 mouse) {
    return gShieldPrev && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)
        && CheckCollisionPointRec(mouse, gShieldPrevRec);
}

void GuiSetModalState(bool modalOpen) {
    gModalOpen = modalOpen;
    gInsideModal = false;
}

void GuiMarkInsideModal() {
    gInsideModal = true;
}

bool GuiModalAbsorbsInput() {
    return gModalOpen && !gInsideModal;
}

static int* getVietnameseCodepoints(int& count) {
    static int cps[449];
    static int total = 0;
    if (total == 0) {
        static const int ranges[][2] = {
            {32, 126},
            {160, 255},
            {0x0100, 0x017F},
            {0x01A0, 0x01B0},
            {0x1EA0, 0x1EF9},
            {0x2010, 0x2026}
        };
        for (int r = 0; r < 6; ++r) {
            for (int i = ranges[r][0]; i <= ranges[r][1]; ++i) cps[total++] = i;
        }
    }
    count = total;
    return cps;
}

void loadVietnameseFonts() {
    int cpCount = 0;
    int* cps = getVietnameseCodepoints(cpCount);
    if (FileExists("assets/fonts/regular.ttf")) {
        gFontRegular = LoadFontEx("assets/fonts/regular.ttf", 36, cps, cpCount);
        SetTextureFilter(gFontRegular.texture, TEXTURE_FILTER_BILINEAR);
    } else {
        gFontRegular = GetFontDefault();
    }

    if (FileExists("assets/fonts/bold.ttf")) {
        gFontBold = LoadFontEx("assets/fonts/bold.ttf", 40, cps, cpCount);
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

void DrawTxtClipped(const char* text, float x, float y, float fontSize, Color color, bool bold, float maxW) {
    std::string out;
    if (MeasureTxt(text, fontSize, bold) > maxW) {
        out = text;
        while (!out.empty() && MeasureTxt((out + "...").c_str(), fontSize, bold) > maxW) out.pop_back();
        out += "...";
    }
    BeginScissorMode((int)x, (int)y - 1, (int)maxW, (int)fontSize + 4);
    DrawTxt(out.empty() ? text : out.c_str(), x, y, fontSize, color, bold);
    EndScissorMode();
}

static bool drawBtnShell(Rectangle rec, const char* text, float fontSize, bool bold,
                         Color fillColor, Color borderColor, Color textColor, bool disabledLike) {
    DrawRectangleRounded(rec, 0.15f, 4, fillColor);
    DrawRectangleRoundedLines(rec, 0.15f, 4, borderColor);
    float textW = MeasureTxt(text, fontSize, bold);
    float posX = rec.x + (rec.width - textW) / 2.0f;
    float posY = rec.y + (rec.height - fontSize) / 2.0f;
    DrawTxt(text, posX, posY, fontSize, textColor, bold);
    if (disabledLike) return false;
    Vector2 mouse = GetMousePosition();
    if (CheckCollisionPointRec(mouse, rec) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return !clickShielded(mouse);
    return false;
}

bool GuiBtn(Rectangle rec, const char* text, Color bg, Color hoverBg, Color textColor, float fontSize, bool bold, bool disabled) {
    if (disabled || (gModalOpen && !gInsideModal)) {
        return drawBtnShell(rec, text, fontSize, bold,
                            MakeColor(bg.r, bg.g, bg.b, 160), MakeColor(bg.r, bg.g, bg.b, 160),
                            MakeColor(textColor.r, textColor.g, textColor.b, 160), true);
    }

    Vector2 mouse = GetMousePosition();
    bool hovered = CheckCollisionPointRec(mouse, rec);
    Color currentBg = hovered ? hoverBg : bg;

    return drawBtnShell(rec, text, fontSize, bold, currentBg, currentBg, textColor, false);
}

bool GuiBtnOutline(Rectangle rec, const char* text, Color borderColor, Color textColor, float fontSize, bool bold, bool disabled) {
    if (disabled || (gModalOpen && !gInsideModal)) {
        return drawBtnShell(rec, text, fontSize, bold, COLOR_CARD, COLOR_BORDER, COLOR_TEXT_MUTED, true);
    }

    Vector2 mouse = GetMousePosition();
    bool hovered = CheckCollisionPointRec(mouse, rec);
    Color currentBg = hovered ? MakeColor(borderColor.r, borderColor.g, borderColor.b, 35) : COLOR_CARD;

    return drawBtnShell(rec, text, fontSize, bold, currentBg, borderColor, textColor, false);
}

static int utf8CharLen(const char* s) {
    unsigned char c = (unsigned char)s[0];
    if (c < 0x80) return 1;
    if ((c >> 5) == 0x06) return 2;
    if ((c >> 4) == 0x0E) return 3;
    if ((c >> 3) == 0x1E) return 4;
    return 1;
}

static int utf8PrevPos(const char* s, int pos) {
    if (pos <= 0) return 0;
    pos--;
    while (pos > 0 && ((s[pos] & 0xC0) == 0x80)) pos--;
    return pos;
}

static int utf8NextPos(const char* s, int pos) {
    int len = (int)strlen(s);
    if (pos >= len) return len;
    int next = pos + utf8CharLen(s + pos);
    return next > len ? len : next;
}

static int utf8CountBefore(const char* s, int pos) {
    int cps = 0, i = 0;
    while (i < pos && s[i] != '\0') {
        i += utf8CharLen(s + i);
        cps++;
    }
    return cps;
}

static float displayWidthBefore(const char* buf, int bytePos, bool isPassword) {
    if (!isPassword) {
        std::string prefix(buf, bytePos);
        return MeasureTxt(prefix.c_str(), 16.0f);
    }
    return (float)utf8CountBefore(buf, bytePos) * MeasureTxt("*", 16.0f);
}

static char* gCaretBuf = nullptr;
static int gCaretByte = 0;

void GuiTextBoxInput(Rectangle rec, char textBuffer[], int maxLen, bool& isActive, const char* placeholder, bool isPassword) {
    if (gModalOpen && !gInsideModal) {
        isActive = false;
    } else {
        Vector2 mouse = GetMousePosition();
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            isActive = CheckCollisionPointRec(mouse, rec) && !clickShielded(mouse);
            if (isActive) {
                int len = (int)strlen(textBuffer);
                float textMaxW = rec.width - 24.0f;
                float caretW = displayWidthBefore(textBuffer, (gCaretBuf == textBuffer ? gCaretByte : len), isPassword);
                float scroll = caretW > textMaxW ? caretW - textMaxW : 0.0f;
                float localX = mouse.x - (rec.x + 12.0f) + scroll;

                int chosen = 0;
                float bestDist = -1.0f;
                int i = 0;
                while (true) {
                    float w = displayWidthBefore(textBuffer, i, isPassword);
                    float d = w - localX;
                    if (d < 0) d = -d;
                    if (bestDist < 0 || d < bestDist) { bestDist = d; chosen = i; }
                    if (i >= len) break;
                    i += utf8CharLen(textBuffer + i);
                }
                gCaretBuf = textBuffer;
                gCaretByte = chosen;
            }
        }
    }

    if (isActive) {
        int len = (int)strlen(textBuffer);
        if (gCaretBuf != textBuffer) {
            gCaretBuf = textBuffer;
            gCaretByte = len;
        }
        if (gCaretByte < 0) gCaretByte = 0;
        if (gCaretByte > len) gCaretByte = len;

        int key = GetCharPressed();
        while (key > 0) {
            int bytesLen = 0;
            const char* utf8Bytes = CodepointToUTF8(key, &bytesLen);
            if (key >= 32 && utf8Bytes != nullptr && (int)strlen(textBuffer) + bytesLen < maxLen) {
                len = (int)strlen(textBuffer);
                memmove(textBuffer + gCaretByte + bytesLen, textBuffer + gCaretByte, len - gCaretByte + 1);
                memcpy(textBuffer + gCaretByte, utf8Bytes, bytesLen);
                gCaretByte += bytesLen;
            }
            key = GetCharPressed();
        }

        if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) {
            if (gCaretByte > 0) {
                int prev = utf8PrevPos(textBuffer, gCaretByte);
                len = (int)strlen(textBuffer);
                memmove(textBuffer + prev, textBuffer + gCaretByte, len - gCaretByte + 1);
                gCaretByte = prev;
            }
        }
        if (IsKeyPressed(KEY_DELETE) || IsKeyPressedRepeat(KEY_DELETE)) {
            len = (int)strlen(textBuffer);
            if (gCaretByte < len) {
                int next = utf8NextPos(textBuffer, gCaretByte);
                memmove(textBuffer + gCaretByte, textBuffer + next, len - next + 1);
            }
        }
        if (IsKeyPressed(KEY_LEFT) || IsKeyPressedRepeat(KEY_LEFT)) gCaretByte = utf8PrevPos(textBuffer, gCaretByte);
        if (IsKeyPressed(KEY_RIGHT) || IsKeyPressedRepeat(KEY_RIGHT)) gCaretByte = utf8NextPos(textBuffer, gCaretByte);
        if (IsKeyPressed(KEY_HOME)) gCaretByte = 0;
        if (IsKeyPressed(KEY_END)) gCaretByte = (int)strlen(textBuffer);
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
        int len = (int)strlen(textBuffer);
        std::string displayText;
        if (isPassword) {
            displayText.assign(utf8CountBefore(textBuffer, len), '*');
        } else {
            displayText = textBuffer;
        }

        float textW = MeasureTxt(displayText.c_str(), fontSize);
        float scroll = 0.0f;
        if (isActive) {
            float caretW = displayWidthBefore(textBuffer, gCaretByte, isPassword);
            if (caretW > textMaxW) scroll = caretW - textMaxW;
        } else if (textW > textMaxW) {
            scroll = textW - textMaxW;
        }
        float renderX = posX - scroll;

        DrawTxt(displayText.c_str(), renderX, posY, fontSize, COLOR_TEXT_DARK);

        if (isActive && ((GetTime() - (int)GetTime()) < 0.5)) {
            float caretW = displayWidthBefore(textBuffer, gCaretByte, isPassword);
            DrawTxt("|", posX + caretW - scroll, posY, fontSize, COLOR_PRIMARY, true);
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

    DrawTxt(fullMsg.c_str(), cardX + (cardW - textW) / 2.0f, cardY + (cardH - fontSize) / 2.0f, fontSize, textC, true);
}

void GuiDrawTakenExamCard(const ExamDetail* d, int index, Rectangle dCard) {
    char qLabel[32];
    snprintf(qLabel, sizeof(qLabel), "Cau %d:", index + 1);
    DrawTxt(qLabel, dCard.x + 15, dCard.y + 12, 16, COLOR_PRIMARY, true);
    float qx = std::max(dCard.x + 80.0f, dCard.x + 15 + MeasureTxt(qLabel, 16, true) + 10);
    DrawTxtWrapped(d->content, qx, dCard.y + 12, dCard.x + dCard.width - 15 - qx, 16, COLOR_TEXT_DARK, true);

    float ansMid = dCard.x + dCard.width * 0.5f;
    DrawTxtClipped(TextFormat("A. %s", d->answerA), dCard.x + 25, dCard.y + 42, 14, COLOR_TEXT_MUTED, false, ansMid - dCard.x - 40);
    DrawTxtClipped(TextFormat("B. %s", d->answerB), dCard.x + 25, dCard.y + 68, 14, COLOR_TEXT_MUTED, false, ansMid - dCard.x - 40);
    DrawTxtClipped(TextFormat("C. %s", d->answerC), ansMid, dCard.y + 42, 14, COLOR_TEXT_MUTED, false, dCard.x + dCard.width - ansMid - 45);
    DrawTxtClipped(TextFormat("D. %s", d->answerD), ansMid, dCard.y + 68, 14, COLOR_TEXT_MUTED, false, dCard.x + dCard.width - ansMid - 45);

    bool isCorrect = (d->selectedAnswer == d->correctAnswer);
    DrawTxt(TextFormat("Ban chon: %c | Dap an dung: %c", d->selectedAnswer ? d->selectedAnswer : '-', d->correctAnswer),
            dCard.x + 25, dCard.y + 100, 15, isCorrect ? COLOR_SUCCESS : COLOR_DANGER, true);
    DrawTxt(isCorrect ? "[ CHINH XAC ]" : "[ SAI ]", dCard.x + 350, dCard.y + 100, 15,
            isCorrect ? COLOR_SUCCESS : COLOR_DANGER, true);
}

void collectSubjects(Subject* root, Arr<Subject*>& list) {
    if (root == nullptr) return;
    collectSubjects(root->left, list);
    list.push_back(root);
    collectSubjects(root->right, list);
}

void collectQuestions(Question* head, Arr<Question*>& list) {
    Question* cur = head;
    while (cur != nullptr) {
        list.push_back(cur);
        cur = cur->next;
    }
}

void GuiLeaveOnlyDigits(char textBuffer[], int bufferSize) {
    if (textBuffer == nullptr || bufferSize <= 0) return;
    int w = 0;
    for (int i = 0; textBuffer[i] != '\0' && w < bufferSize - 1; ++i) {
        if (textBuffer[i] >= '0' && textBuffer[i] <= '9') textBuffer[w++] = textBuffer[i];
    }
    textBuffer[w] = '\0';
}

bool GuiMatchText(const char* text, const char* query) {
    if (query == nullptr || query[0] == '\0') return true;

    auto trim = [](const std::string& in) {
        size_t b = 0, e = in.size();
        while (b < e && (in[b] == ' ' || in[b] == '\t')) b++;
        while (e > b && (in[e - 1] == ' ' || in[e - 1] == '\t')) e--;
        return in.substr(b, e - b);
    };

    std::string hay = trim((text != nullptr) ? text : "");
    std::string needle = trim(query);
    if (needle.empty()) return true;
    for (char& c : hay) c = (char)tolower((unsigned char)c);
    for (char& c : needle) c = (char)tolower((unsigned char)c);
    return hay.find(needle) != std::string::npos;
}

int GuiSearchSuggest(Rectangle rec, char* query, int capacity, bool& active,
                     const Arr<SuggestItem>& items, const char* placeholder) {
    bool wasActive = active;
    GuiTextBoxInput(rec, query, capacity, active, placeholder);
    if (!active && !wasActive) return -1;
    if (active && !wasActive) suggestScroll = 0;

    Arr<const SuggestItem*> matches;
    for (const SuggestItem& it : items) {
        if (GuiMatchText(it.id.c_str(), query) || GuiMatchText(it.name.c_str(), query)) {
            matches.push_back(&it);
        }
    }

    const int maxRows = 6;
    int shown = (int)matches.size();
    if (shown > maxRows) shown = maxRows;

    const float rowH = 34.0f;
    float panelW = rec.width < 420.0f ? 420.0f : rec.width;
    if (rec.x + panelW > GUI_WIDTH - 10.0f) panelW = GUI_WIDTH - 10.0f - rec.x;
    Rectangle panel = { rec.x, rec.y + rec.height + 2.0f, panelW, shown * rowH + 8.0f };
    if (matches.empty()) panel.height = rowH + 8.0f;

    gShieldCur = true;
    gShieldCurRec = panel;

    DrawRectangleRounded(panel, 0.06f, 6, COLOR_CARD);
    DrawRectangleRoundedLines(panel, 0.06f, 6, COLOR_BORDER);

    if (matches.empty()) {
        DrawTxt("Khong tim thay ket qua phu hop.", panel.x + 12, panel.y + 10, 14, COLOR_TEXT_MUTED);
        return -1;
    }

    Vector2 mouse = GetMousePosition();
    bool pressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    int picked = -1;

    if (shown > 0 && CheckCollisionPointRec(mouse, panel)) {
        suggestScroll += (int)GetMouseWheelMove() * -1;
    }
    int scrollMax = (int)matches.size() - maxRows;
    if (scrollMax < 0) scrollMax = 0;
    if (suggestScroll > scrollMax) suggestScroll = scrollMax;
    if (suggestScroll < 0) suggestScroll = 0;

    for (int r = 0; r < shown; ++r) {
        const SuggestItem* it = matches[suggestScroll + r];
        Rectangle row = { panel.x + 4, panel.y + 4 + r * rowH, panel.width - 8, rowH };
        bool hover = CheckCollisionPointRec(mouse, row);

        Color rowBg = COLOR_CARD;
        if (it->highlight) rowBg = MakeColor(220, 252, 231);
        if (hover) rowBg = MakeColor(226, 232, 240);

        DrawRectangleRounded(row, 0.10f, 4, rowBg);
        DrawTxt(it->id.c_str(), row.x + 10, row.y + 8, 15, COLOR_PRIMARY, true);

        float metaW = it->meta.empty() ? 0.0f : MeasureTxt(it->meta.c_str(), 14, true) + 20.0f;
        float nameX = row.x + 10 + MeasureTxt(it->id.c_str(), 15, true) + 16.0f;
        if (nameX < row.x + 104) nameX = row.x + 104;
        if (nameX > row.x + row.width - metaW - 20) nameX = row.x + row.width - metaW - 20;
        BeginScissorMode((int)nameX, (int)row.y, (int)(row.x + row.width - metaW - nameX), (int)row.height);
        DrawTxt(it->name.c_str(), nameX, row.y + 8, 15, COLOR_TEXT_DARK);
        EndScissorMode();

        if (!it->meta.empty()) {
            float mw = MeasureTxt(it->meta.c_str(), 14, true);
            DrawTxt(it->meta.c_str(), row.x + row.width - mw - 12, row.y + 9, 14,
                    it->highlight ? COLOR_SUCCESS : COLOR_TEXT_MUTED, true);
        }

        if (pressed && CheckCollisionPointRec(mouse, row)) {
            snprintf(query, capacity, "%s", it->id.c_str());
            picked = it->value;
        }
    }

    if (picked >= 0) active = false;
    return picked;
}

void GuiMakeSubjectItems(const Arr<Subject*>& subjects, bool withCount, Arr<SuggestItem>& out) {
    out.clear();
    for (size_t i = 0; i < subjects.size(); ++i) {
        SuggestItem it;
        it.value = (int)i;
        it.id = subjects[i]->id;
        it.name = subjects[i]->name;
        if (withCount) {
            int n = countQuestions(subjects[i]->questionList);
            char meta[24];
            snprintf(meta, sizeof(meta), "%d cau", n);
            it.meta = meta;
            it.highlight = (n > 0);
        }
        out.push_back(it);
    }
}

void GuiMakeClassItems(Class* const classes[], int classCount, Arr<SuggestItem>& out) {
    out.clear();
    for (int i = 0; i < classCount; ++i) {
        if (classes[i] == nullptr) continue;
        SuggestItem it;
        it.value = i;
        it.id = classes[i]->id;
        it.name = classes[i]->name;
        int n = 0;
        for (Student* s = classes[i]->studentList; s != nullptr; s = s->next) n++;
        char meta[24];
        snprintf(meta, sizeof(meta), "%d SV", n);
        it.meta = meta;
        it.highlight = (n > 0);
        out.push_back(it);
    }
}
