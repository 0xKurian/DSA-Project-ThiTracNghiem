#ifndef GUI_COMMON_H
#define GUI_COMMON_H
#include "raylib.h"

#include "GuiArr.h"

#include <string>

struct Subject;
struct Question;
struct Student;
struct Class;

constexpr int GUI_WIDTH  = 1280;
constexpr int GUI_HEIGHT = 768;

constexpr int SUBJECTS_PER_PAGE   = 8;
constexpr int CLASSES_PER_PAGE    = 8;
constexpr int STUDENTS_PER_PAGE   = 8;
constexpr int QUESTIONS_PER_PAGE  = 3;
constexpr int SCORES_PER_PAGE     = 8;
constexpr int REVIEW_PER_PAGE     = 3;

inline const Color COLOR_BG             = { 245, 247, 250, 255 };
inline const Color COLOR_CARD           = { 255, 255, 255, 255 };
inline const Color COLOR_SIDEBAR        = { 30, 41, 59, 255 };
inline const Color COLOR_SIDEBAR_HOVER  = { 51, 65, 85, 255 };
inline const Color COLOR_SIDEBAR_ACTIVE = { 37, 99, 235, 255 };
inline const Color COLOR_PRIMARY        = { 37, 99, 235, 255 };
inline const Color COLOR_PRIMARY_HOVER  = { 29, 78, 216, 255 };
inline const Color COLOR_SUCCESS        = { 22, 163, 74, 255 };
inline const Color COLOR_SUCCESS_HOVER  = { 21, 128, 61, 255 };
inline const Color COLOR_DANGER         = { 220, 38, 38, 255 };
inline const Color COLOR_DANGER_HOVER   = { 185, 28, 28, 255 };
inline const Color COLOR_WARNING        = { 234, 88, 12, 255 };
inline const Color COLOR_TEXT_DARK      = { 15, 23, 42, 255 };
inline const Color COLOR_TEXT_MUTED     = { 100, 116, 139, 255 };
inline const Color COLOR_BORDER         = { 226, 232, 240, 255 };
inline const Color COLOR_ROW_ALT        = { 248, 250, 252, 255 };

inline Color MakeColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255) {
    Color c;
    c.r = r; c.g = g; c.b = b; c.a = a;
    return c;
}

void loadVietnameseFonts();
void unloadVietnameseFonts();

void DrawTxt(const char* text, float posX, float posY, float fontSize, Color color = COLOR_TEXT_DARK, bool bold = false);
float MeasureTxt(const char* text, float fontSize, bool bold = false);
float DrawTxtWrapped(const char* text, float posX, float posY, float maxWidth, float fontSize,
                     Color color = COLOR_TEXT_DARK, bool bold = false, float lineSpacing = 4.0f);

void GuiSetModalState(bool modalOpen);
void GuiMarkInsideModal();
bool GuiModalAbsorbsInput();

bool GuiBtn(Rectangle rec, const char* text,
            Color bg = COLOR_PRIMARY, Color hoverBg = COLOR_PRIMARY_HOVER,
            Color textColor = WHITE, float fontSize = 16.0f,
            bool bold = false, bool disabled = false);

bool GuiBtnOutline(Rectangle rec, const char* text,
                   Color borderColor = COLOR_PRIMARY, Color textColor = COLOR_PRIMARY,
                   float fontSize = 15.0f, bool bold = false, bool disabled = false);

void GuiTextBoxInput(Rectangle rec, char textBuffer[], int maxLen, bool& isActive,
                     const char* placeholder = "", bool isPassword = false);

struct SuggestItem {
    int value = -1;
    std::string id;
    std::string name;
    std::string meta;
    bool highlight = false;
};

void GuiBeginFrame();
bool GuiMatchText(const char* text, const char* query);

int GuiSearchSuggest(Rectangle rec, char* query, int capacity, bool& active,
                     const Arr<SuggestItem>& items, int currentValue,
                     const char* placeholder = "", bool* textChanged = nullptr);

void GuiMakeSubjectItems(const Arr<Subject*>& subjects, bool withCount, Arr<SuggestItem>& out);
void GuiMakeClassItems(Class* const classes[], int classCount, Arr<SuggestItem>& out);
void GuiMakeStudentItems(const Arr<Student*>& students, Arr<SuggestItem>& out);

void DrawCardPanel(Rectangle rec, Color bg = COLOR_CARD, Color border = COLOR_BORDER);

void DrawToastNotification(const char* message, float timer, bool isError = false);

void collectSubjects(Subject* root, Arr<Subject*>& list);
void collectQuestions(Question* head, Arr<Question*>& list);
void collectStudents(Student* head, Arr<Student*>& list);
Class* GuiFindStudentClass(Class* const classes[], int classCount, const char* studentId);

#endif
