#ifndef STRUCTURES_H
#define STRUCTURES_H

#include "Constants.h"

struct Question;
struct Subject;
struct ExamDetail;
struct Score;
struct Student;
struct Class;

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

struct Subject {
    char id[SUBJECT_ID_LEN];
    char name[SUBJECT_NAME_LEN];

    Question* questionList;

    int examQuestionCount;
    int examMinutes;

    Subject* left;
    Subject* right;
};

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

struct Score {
    char subjectId[SUBJECT_ID_LEN];
    float score;

    ExamDetail* detailList;

    Score* next;
};

struct Student {
    char id[STUDENT_ID_LEN];

    char ho[HO_LEN];
    char ten[TEN_LEN];

    char gender[GENDER_LEN];
    char password[PASSWORD_LEN];

    Score* scoreList;

    Student* next;
};

struct Class {
    char id[CLASS_ID_LEN];
    char name[CLASS_NAME_LEN];

    Student* studentList;
};

enum LoginRole {
    LOGIN_FAILED,
    LOGIN_TEACHER,
    LOGIN_STUDENT
};

#endif
