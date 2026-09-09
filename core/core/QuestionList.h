#ifndef QUESTION_LIST_H
#define QUESTION_LIST_H

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