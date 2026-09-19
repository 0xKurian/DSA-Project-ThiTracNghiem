#ifndef STRUCTURES_H
#define STRUCTURES_H

#include "Constants.h"

// Forward declarations
struct Question;
struct Subject;
struct ExamDetail;
struct Score;
struct Student;
struct Class;

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


// ==================== EXAM DETAIL ====================

struct ExamDetail {
    int questionId;

    char content[QUESTION_CONTENT_LEN];

    // These answers are stored in the exact order shown to the student.
    char answerA[ANSWER_CONTENT_LEN];
    char answerB[ANSWER_CONTENT_LEN];
    char answerC[ANSWER_CONTENT_LEN];
    char answerD[ANSWER_CONTENT_LEN];

    // correctAnswer is updated after shuffling A/B/C/D.
    char correctAnswer;
    char selectedAnswer;

    ExamDetail* next;
};


// ==================== SCORE ====================

struct Score {
    char subjectId[SUBJECT_ID_LEN];
    float score;

    // One subject can be taken only once, so the exam detail belongs here.
    ExamDetail* detailList;

    Score* next;
};


// ==================== STUDENT ====================

struct Student {
    char id[STUDENT_ID_LEN];

    char ho[HO_LEN];
    char ten[TEN_LEN];

    char gender[GENDER_LEN];
    char password[PASSWORD_LEN];

    Score* scoreList;

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
