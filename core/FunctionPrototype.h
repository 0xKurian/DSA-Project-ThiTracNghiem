#ifndef FUNCTION_PROTOTYPE_H
#define FUNCTION_PROTOTYPE_H

#include "Structures.h"
#include "StringUtils.h"
#include "QuestionList.h"
#include "SubjectBST.h"
#include "StudentList.h"
#include "ScoreList.h"
#include "ClassArray.h"

// Future Business Logic

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