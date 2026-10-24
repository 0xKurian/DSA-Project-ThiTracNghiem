#ifndef FUNCTION_PROTOTYPE_H
#define FUNCTION_PROTOTYPE_H

#include "ClassArray.h"
#include "StudentList.h"
#include "SubjectBST.h"
#include "QuestionList.h"
#include "ScoreList.h"
#include "StringUtils.h"

LoginRole login(
    Class* classes[],
    int classCount,
    const char username[],
    const char password[],
    Student*& loggedStudent
);

bool canTakeExam(
    Subject* subject,
    int numberOfQuestions,
    int minutes
);

float calculateScore(
    int correctAnswers,
    int totalQuestions
);

void loadAllData();
void saveAllData();

#endif
