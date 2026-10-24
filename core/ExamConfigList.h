#ifndef EXAM_CONFIG_LIST_H
#define EXAM_CONFIG_LIST_H

#include "Structures.h"

ExamConfig* createExamConfig(
    const char subjectId[],
    int minutes,
    const char createdAt[],
    const int questionIds[],
    int questionCount
);

bool addExamConfig(
    Class* classroom,
    ExamConfig* config
);

ExamConfig* findExamConfig(
    const Class* classroom,
    const char* subjectId
);

bool deleteExamConfig(
    Class* classroom,
    const char* subjectId
);

void deleteExamConfigList(
    ExamConfig*& head
);

#endif
