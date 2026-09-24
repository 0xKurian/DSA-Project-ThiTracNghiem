#ifndef QUESTION_LIST_H
#define QUESTION_LIST_H

// Contract: use generateQuestionId() for every NEW question across all subjects.
// Restoring data: validate IDs for uniqueness across ALL subjects, then call
// updateQuestionIdCounter(id) for each restored ID before generating new IDs.
// IDs must be positive and leave room for the next ID in int; this API does not
// maintain a global registry of imported IDs or detect counter exhaustion.
// createQuestion accepts valid null-terminated strings and an A-D correct answer.
// insertQuestion takes ownership of a fresh detached node; never insert it twice.
// The caller must ensure uniqueness: insertion itself does not validate the ID.

#include "Structures.h"

Question* createQuestion(
    int id,
    const char content[],
    const char answerA[],
    const char answerB[],
    const char answerC[],
    const char answerD[],
    char correctAnswer
);

void insertQuestion(
    Question*& head,
    Question* question
);

Question* findQuestion(
    Question* head,
    int id
);

bool deleteQuestion(
    Question*& head,
    int id
);

int countQuestions(
    Question* head
);

void printQuestions(
    Question* head
);

int generateQuestionId();

void updateQuestionIdCounter(
    int id
);

#endif
