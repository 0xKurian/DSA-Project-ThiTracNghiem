#ifndef FILE_STORAGE_H
#define FILE_STORAGE_H

#include "../core/Structures.h"

// ==================== SUBJECT & QUESTION PERSISTENCE ====================

bool saveSubjectsToFile(
    Subject* root,
    const char* filepath
);

bool loadSubjectsFromFile(
    Subject*& root,
    const char* filepath
);

// ==================== SUBJECT QUESTION FILE UTILITIES ====================

bool createEmptyQuestionFile(const char* dir, const char* subjectId);
bool renameSubjectQuestionFile(const char* dir, const char* oldId, const char* newId);
bool deleteSubjectQuestionFile(const char* dir, const char* subjectId);

// ==================== CLASS, STUDENT & SCORE PERSISTENCE ====================

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

// ==================== GLOBAL PERSISTENCE ====================

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
