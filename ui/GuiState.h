#ifndef GUI_STATE_H
#define GUI_STATE_H

#include "../core/Constants.h"
#include "../core/ClassArray.h"
#include "../core/Structures.h"
#include "../business/ExamService.h"
#include "GuiArr.h"

#include <cstdio>
#include <cstring>
#include <string>

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
    MODAL_CLASS_EXAM_CONFIG,
    MODAL_DELETE_CLASS_EXAM_CONFIG,
    MODAL_CONFIRM_SUBMIT,
    MODAL_CANCEL_EXAM
};

struct GuiState {
    Subject* root;
    Class* classes[MAX_CLASS];
    int classCount;
    Arr<Subject*> subjects;

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

    bool classShowAll;
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

    char reviewSubjectId[SUBJECT_ID_LEN];
    char reviewClassId[CLASS_ID_LEN];
    char reviewStudentId[STUDENT_ID_LEN];
    bool reviewDetailOpen;

    char formClassId[CLASS_ID_LEN + 4];
    char configClassId[CLASS_ID_LEN + 4];
    char configSubjectId[SUBJECT_ID_LEN];
    char configEditClass[CLASS_ID_LEN];
    char configEditSubject[SUBJECT_ID_LEN];
    int configSelected = -1;
    int configPage;
    int configPreviewPage;
    char configFilterClass[CLASS_ID_LEN];
    char configFilterSubject[SUBJECT_ID_LEN];
    char formExamQ[8];
    char formExamMinutes[16];
    bool activeConfigClassSearch;
    bool activeConfigSubjectSearch;
    bool activeConfigFilterClass;
    bool activeConfigFilterSubject;

    int studentTab;
    int studentSubChoice;
    int studentReviewScoreIdx = -1;

    ExamDetail* examHead;
    Arr<ExamDetail*> examList;
    int currentExamQuestionIdx;
    std::string activeExamSubId;
    ExamTimer examTimer;
    ExamSessionTracker examTracker;

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
void drawAdminTabExamConfigs(GuiState& g);
void drawClassExamConfigModal(GuiState& g);
void drawDeleteClassExamConfigModal(GuiState& g);

void drawStudentHome(GuiState& g);void drawStudentTabScores(GuiState& g);

void drawExamPrepareTab(GuiState& g);void updateExamSession(GuiState& g);void drawExamRoom(GuiState& g);void drawExamSubmitConfirmDialog(GuiState& g);void drawExamResult(GuiState& g);
void renderModalDialogs(GuiState& g);

#endif
