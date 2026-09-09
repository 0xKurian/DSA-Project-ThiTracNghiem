#ifndef STRUCTURES_H
#define STRUCTURES_H

#include "Constants.h"

// Forward declarations
struct Question;
struct Subject;
struct Score;
struct Student;
struct Class;
struct ExamDetail;
struct ExamHistory;

// ==================== QUESTION ====================

struct Question {
    int id;

    char content[QUESTION_CONTENT_LEN];

    char answerA[ANSWER_CONTENT_LEN];
    char answerB[ANSWER_CONTENT_LEN];
    char answerC[ANSWER_CONTENT_LEN];
    char answerD[ANSWER_CONTENT_LEN];

    char correctAnswer;

    Question* next;
};


// ==================== SUBJECT ====================

struct Subject {
    char id[SUBJECT_ID_LEN];
    char name[SUBJECT_NAME_LEN];

    Question* questionList;

    Subject* left;
    Subject* right;
};


// ==================== SCORE ====================

struct Score {
    char subjectId[SUBJECT_ID_LEN];
    float score;

    Score* next;
};


// ==================== EXAM DETAIL ====================

struct ExamDetail {
    int questionId;

    char content[QUESTION_CONTENT_LEN];

    char answerA[ANSWER_CONTENT_LEN];
    char answerB[ANSWER_CONTENT_LEN];
    char answerC[ANSWER_CONTENT_LEN];
    char answerD[ANSWER_CONTENT_LEN];

    char correctAnswer;
    char selectedAnswer;

    ExamDetail* next;
};


// ==================== EXAM HISTORY ====================

struct ExamHistory {
    char subjectId[SUBJECT_ID_LEN];

    int numberOfQuestions;
    float score;

    ExamDetail* detailList;

    ExamHistory* next;
};


// ==================== STUDENT ====================

struct Student {
    char id[STUDENT_ID_LEN];

    char ho[HO_LEN];
    char ten[TEN_LEN];

    char gender[GENDER_LEN];
    char password[PASSWORD_LEN];

    Score* scoreList;
    ExamHistory* examHistoryList;

    Student* next;
};


// ==================== CLASS ====================

struct Class {
    char id[CLASS_ID_LEN];
    char name[CLASS_NAME_LEN];

    Student* studentList;
};

// ==================== LOGIN ROLE ====================

enum LoginRole {
    LOGIN_FAILED,
    LOGIN_TEACHER,
    LOGIN_STUDENT
};

#endif