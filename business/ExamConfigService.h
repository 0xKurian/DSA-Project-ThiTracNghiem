#ifndef EXAM_CONFIG_SERVICE_H
#define EXAM_CONFIG_SERVICE_H

#include "../core/Structures.h"
#include "BusinessErrors.h"

int drawQuestionIds(Subject* subject, int count, int outIds[]);

Class* findStudentClass(Class* classes[], int classCount, const char studentId[]);
bool classHasTakenSubject(Class* classes[], int classCount,
                          const char classId[], const char subjectId[]);

BusinessResult setClassExamConfigWorkflow(
    Subject* root, Class* classes[], int classCount, const char classId[],
    const char subjectId[], int numberOfQuestions, int minutes);

BusinessResult deleteClassExamConfigWorkflow(
    Class* classes[], int classCount, const char classId[],
    const char subjectId[]);

BusinessResult prepareExamFromConfig(
    Subject* root, Class* classes[], int classCount, Student* student,
    const char subjectId[], ExamDetail*& examQuestionsHead);

BusinessResult canStudentTakeSubject(
    Subject* root, Class* classes[], int classCount, Student* student,
    const char subjectId[], const char** reasonOut);

#endif
