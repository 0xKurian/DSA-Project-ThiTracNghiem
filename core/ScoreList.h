#ifndef SCORE_LIST_H
#define SCORE_LIST_H

// Contract: createScore/addScore reject null/empty/overlong subject IDs.
// createScore returns nullptr on rejection; addScore returns false.
// The caller validates the score against the chosen grading scale.
// A successful createScore/addScore takes exclusive ownership of detailList;
// on failure the caller retains it. Never share details between scores.
// addScore rejects an existing subject key; retake policy belongs to the caller.
// deleteScore frees its details. updateScore changes only the numeric score.
// createExamDetail copies and shuffles a question; invalid A-D answers are rejected.
// shuffleExamDetailAnswers does nothing once selectedAnswer is nonzero.
// addExamDetail takes a fresh detached node. Do not insert the same node twice.

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

