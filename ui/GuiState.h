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
#include <cstring>
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
    MODAL_EXAM_CONFIG,
    MODAL_CONFIRM_SUBMIT
};

// cau hinh thi theo mon: tam thoi giu trong bo nho UI, cho den khi core/business co field that
struct ExamConfigUi {
    char subjectId[SUBJECT_ID_LEN];
    int questionCount;
    int minutes;
};

inline ExamConfigUi* findExamConfig(std::vector<ExamConfigUi>& configs, const char* subjectId) {
    for (ExamConfigUi& cfg : configs) {
        if (subjectId != nullptr && strcmp(cfg.subjectId, subjectId) == 0) return &cfg;
    }
    return nullptr;
}

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
    int selectedClassIdx;              // cot trai master-detail; -1 = chua chon lop

    bool classShowAll;                 // cot phai: xem toan bo sinh vien
    char searchSubject[64];
    bool activeSearchSubject;
    char searchClass[64];
    bool activeSearchClass;
    char searchStudent[64];
    bool activeSearchStudent;
    char searchQuestion[64];
    bool activeSearchQuestion;
    char searchReviewSubject[64];
    bool activeSearchReviewSubject;
    char searchReviewClass[64];
    bool activeSearchReviewClass;
    char searchPrepare[64];
    bool activeSearchPrepare;

    char reviewSubjectId[SUBJECT_ID_LEN];   // mon dang chon o tab diem
    char reviewClassId[CLASS_ID_LEN];       // lop dang chon
    char reviewStudentId[STUDENT_ID_LEN];   // sinh vien dang chon
    bool reviewDetailOpen;

    char formClassId[CLASS_ID_LEN + 4];     // lop cua sinh vien trong modal sua/xoa
    std::vector<ExamConfigUi> examConfigs;  // cau hinh thi tam thoi (xem ExamConfigUi)
    char configSubjectId[SUBJECT_ID_LEN];
    char formExamQ[8];
    char formExamMinutes[16];

    int studentTab;
    int studentSubChoice;
    int studentReviewScoreIdx = -1;        // -1: dang xem bang diem, >=0: dang mo chi tiet bai thi

    ExamDetail* examHead;                   // DSLK cac cau thi
    std::vector<ExamDetail*> examList;      // cau thi tra theo chi muc
    int currentExamQuestionIdx;
    std::string activeExamSubId;
    int examRemainingSeconds;
    float examTimerClock;

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
void drawAdminTabScores(GuiState& g);

void drawStudentHome(GuiState& g);void drawStudentTabScores(GuiState& g);

void drawExamPrepareTab(GuiState& g);void updateExamSession(GuiState& g, float dt);void drawExamRoom(GuiState& g);void drawExamSubmitConfirmDialog(GuiState& g);void drawExamResult(GuiState& g);
void renderModalDialogs(GuiState& g);

#endif
