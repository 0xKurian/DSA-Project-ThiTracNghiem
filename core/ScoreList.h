#ifndef SCORE_LIST_H
#define SCORE_LIST_H

#include "Structures.h"

Score* createScore(
    const char subjectId[],
    float score
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