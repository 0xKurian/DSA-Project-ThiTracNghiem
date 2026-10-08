#ifndef EXAM_SERVICE_H
#define EXAM_SERVICE_H

#include "../core/Structures.h"
#include "BusinessErrors.h"

#include <ctime>

// ==================== CORE EXAM PROTOTYPES ====================

bool canTakeExam(
    Subject* subject,
    int numberOfQuestions,
    int minutes
);

float calculateScore(
    int correctAnswers,
    int totalQuestions
);

bool isExamTimeExpired(
    time_t startTime,
    int minutes
);

// ==================== WORKFLOWS ====================

BusinessResult prepareExamWorkflow(
    Subject* root,
    Student* student,
    const char subjectId[],
    int numberOfQuestions,
    int minutes,
    ExamDetail*& examQuestionsHead
);

BusinessResult submitExamWorkflow(
    Student* student,
    const char subjectId[],
    ExamDetail* examQuestionsHead,
    float& finalScore,
    int& correctCount,
    int& totalCount
);

void cancelExam(
    ExamDetail*& examQuestionsHead
);

#endif
