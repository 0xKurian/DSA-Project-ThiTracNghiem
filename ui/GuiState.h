#ifndef GUI_STATE_H
#define GUI_STATE_H

// trang thai cua UI do hoa, truyen tham chieu GuiState& cho cac module man hinh.
//phan chia man hinh theo file (ten ham cung ten):
//   dang nhap : GuiScreenLogin.cpp
//   gv        : GuiScreenAdmin.cpp + GuiAdmin<tab>.cpp (moi tab 1 cau truc du lieu)
//   sinh vien : GuiScreenStudent.cpp + GuiExam<buoc>.cpp
//   modal      : GuiModalDialogs.cpp | main loop: GuiApp.cpp
#include "../core/Constants.h"
#include "../core/ClassArray.h"   // MAX_CLASS
#include "../core/Structures.h"   // Subject, Class, Student, ExamDetail, LoginRole

#include <cstdio>
#include <string>
#include <vector>

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

struct GuiState {
    Subject* root;                     // goc BST mon hoc
    Class* classes[MAX_CLASS];         // toi da MAX_CLASS lop
    int classCount;
    std::vector<Subject*> subjects;    // danh sach mon xay dung moi frame (dung de phan trang)

    ScreenState currentScreen;
    LoginRole currentRole;
    Student* loggedStudent;

    char notifyMessage[128];
    float notifyTimer;
    bool notifyIsError;
    void showToast(const char* msg, float duration, bool err) {
        snprintf(notifyMessage, sizeof(notifyMessage), "%s", msg);
        notifyTimer = duration;
        notifyIsError = err;
    }

    char loginUser[32];
    char loginPass[32];
    bool activeUserBox;
    bool activePassBox;

    int teacherTab;
    int subjectPage;
    int classPage;
    int studentPage;
    int questionPage;
    int scorePage;
    int reviewQPage;
    int selectedSubjectIdx;
    int selectedClassIdx;
    int selectedStudentIdx;
    int teacherSubNavOffset;
    int teacherClassNavOffset;
    int scoreSubNavOffset;
    int reviewSubNavOffset;

    int studentTab;
    int studentSubChoice;
    int studentSubNavOffset;

    ExamDetail* examHead;                   // DSLK cac cau thi
    std::vector<ExamDetail*> examList;      // cau thi tra theo chi muc
    int currentExamQuestionIdx;
    std::string activeExamSubId;
    int examRemainingSeconds;
    float examTimerClock;
    char numQuestionsInput[16];
    char examMinutesInput[16];
    bool activeNumQ;
    bool activeMinutes;

    float lastScore;
    int lastCorrect;
    int lastTotal;

    ModalType currentModal;
    char modalError[128];
    char formId[SUBJECT_ID_LEN + 4];
    char formName[SUBJECT_NAME_LEN + 4];
    char formHo[HO_LEN + 4];
    char formTen[TEN_LEN + 4];
    char formGender[GENDER_LEN + 4];
    char formPass[PASSWORD_LEN];
    char formContent[QUESTION_CONTENT_LEN];
    char formA[ANSWER_CONTENT_LEN];
    char formB[ANSWER_CONTENT_LEN];
    char formC[ANSWER_CONTENT_LEN];
    char formD[ANSWER_CONTENT_LEN];
    char formCorrect[8];
    int formEditQId;
    int activeFormBox;
};

void runGraphicApp();

void drawLoginScreen(GuiState& g);

void drawAdminDashboard(GuiState& g);void drawAdminTabSubjects(GuiState& g);
void drawAdminTabQuestions(GuiState& g);
void drawAdminTabClasses(GuiState& g);
void drawAdminTabStudents(GuiState& g);
void drawAdminTabScores(GuiState& g);
void drawAdminTabExamReview(GuiState& g);

void drawStudentHome(GuiState& g);void drawStudentTabScores(GuiState& g);
void drawStudentTabReview(GuiState& g);

void drawExamPrepareTab(GuiState& g);void updateExamSession(GuiState& g, float dt);void drawExamRoom(GuiState& g);void drawExamSubmitConfirmDialog(GuiState& g);void drawExamResult(GuiState& g);
void renderModalDialogs(GuiState& g);

#endif
