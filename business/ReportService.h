#ifndef REPORT_SERVICE_H
#define REPORT_SERVICE_H

#include "../core/Structures.h"
#include "BusinessErrors.h"
#include <iostream>

bool hasStudentTakenExam(
    Student* student,
    const char subjectId[],
    float& outScore
);

Score* getStudentExamScore(
    Student* student,
    const char subjectId[]
);

ExamDetail* getStudentExamDetails(
    Student* student,
    const char subjectId[]
);

BusinessResult printClassScoreTable(
    Class* classes[],
    int classCount,
    Subject* root,
    const char classId[],
    const char subjectId[],
    std::ostream& out = std::cout
);

BusinessResult printExamDetailReview(
    Student* student,
    const char subjectId[],
    std::ostream& out = std::cout
);

BusinessResult printStudentScores(
    Student* student,
    Subject* root,
    std::ostream& out = std::cout
);

#endif
