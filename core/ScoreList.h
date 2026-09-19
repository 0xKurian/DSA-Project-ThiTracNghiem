#ifndef SCORE_LIST_H
#define SCORE_LIST_H

#include "Structures.h"

// ==================== EXAM DETAIL ====================

ExamDetail* createExamDetail(
    const Question* question
);

void shuffleExamDetailAnswers(
    ExamDetail* detail
);

void addExamDetail(
    ExamDetail*& head,
    ExamDetail* detail
);

void deleteExamDetailList(
    ExamDetail*& head
);

// ==================== SCORE ====================

Score* createScore(
    const char subjectId[],
    float score
);

Score* createScore(
    const char subjectId[],
    float score,
    ExamDetail* detailList
);

Score* findScore(
    Student* student,
    const char subjectId[]
);

bool addScore(
    Student* student,
    const char subjectId[],
    float score
);

bool addScore(
    Student* student,
    const char subjectId[],
    float score,
    ExamDetail* detailList
);

bool updateScore(
    Student* student,
    const char subjectId[],
    float score
);

void printScores(
    Student* student
);

bool deleteScore(
    Student* student,
    const char subjectId[]
);

#endif
