#ifndef FILE_STORAGE_H
#define FILE_STORAGE_H

#include "../core/Structures.h"

bool saveSubjectsToFile(
    Subject* root,
    const char* filepath
);

bool loadSubjectsFromFile(
    Subject*& root,
    const char* filepath
);

bool createEmptyQuestionFile(const char* dir, const char* subjectId);
bool renameSubjectQuestionFile(const char* dir, const char* oldId, const char* newId);
bool deleteSubjectQuestionFile(const char* dir, const char* subjectId);

bool saveClassesToFile(
    Class* classes[],
    int classCount,
    const char* filepath
);

bool loadClassesFromFile(
    Class* classes[],
    int& classCount,
    const char* filepath
);

bool saveExamConfigsToFile(Class* classes[], int classCount, const char* filepath);
bool loadExamConfigsFromFile(Class* classes[], int classCount, const char* filepath);

bool saveAllData(
    Subject* root,
    Class* classes[],
    int classCount,
    const char* dir = "data"
);

bool loadAllData(
    Subject*& root,
    Class* classes[],
    int& classCount,
    const char* dir = "data"
);

#endif
