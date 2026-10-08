#ifndef QUESTION_SERVICE_H
#define QUESTION_SERVICE_H

#include "../core/Structures.h"
#include "BusinessErrors.h"

BusinessResult addQuestionWorkflow(
    Subject* root,
    const char subjectId[],
    const char content[],
    const char answerA[],
    const char answerB[],
    const char answerC[],
    const char answerD[],
    char correctAnswer,
    int& generatedId
);

BusinessResult editQuestionWorkflow(
    Subject* root,
    const char subjectId[],
    int questionId,
    const char content[],
    const char answerA[],
    const char answerB[],
    const char answerC[],
    const char answerD[],
    char correctAnswer
);

BusinessResult deleteQuestionWorkflow(
    Subject* root,
    const char subjectId[],
    int questionId
);

#endif
