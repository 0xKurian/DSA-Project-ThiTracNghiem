#ifndef FUNCTION_PROTOTYPE_H
#define FUNCTION_PROTOTYPE_H

// Core declarations are maintained in their module headers.
#include "ClassArray.h"
#include "StudentList.h"
#include "SubjectBST.h"
#include "QuestionList.h"
#include "ScoreList.h"
#include "StringUtils.h"

// The declarations below are integration contracts implemented by the business
// and persistence layers, not by this core package.
// ==================== AUTH ====================

LoginRole login(
    Class* classes[],
    int classCount,
    const char username[],
    const char password[],
    Student*& loggedStudent
);


// ==================== EXAM ====================

bool canTakeExam(
    Subject* subject,
    int numberOfQuestions,
    int minutes
);

float calculateScore(
    int correctAnswers,
    int totalQuestions
);


// ==================== FILE ====================

void loadAllData();
void saveAllData();

#endif
