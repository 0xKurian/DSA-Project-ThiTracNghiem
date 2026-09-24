#ifndef CLASS_ARRAY_H
#define CLASS_ARRAY_H

// Contract: createClass rejects null/empty/overlong IDs with nullptr.
// Other text inputs must be valid null-terminated strings.
// Supply an array of MAX_CLASS slots and a count in [0, MAX_CLASS].
// addClass takes ownership only on success; caller owns a rejected node.
// addStudentGlobal requires classroom to belong to classes[0..classCount).
// It takes ownership of a fresh student only on success.
// deleteClass frees its students, scores and exam details.

#include "Structures.h"

Class* createClass(
    const char id[],
    const char name[]
);

Class* findClass(
    Class* classes[],
    int classCount,
    const char id[]
);

bool addClass(
    Class* classes[],
    int& classCount,
    Class* newClass
);

bool deleteClass(
    Class* classes[],
    int& classCount,
    const char id[]
);

void printClasses(
    Class* classes[],
    int classCount
);

Student* findStudentGlobal(
    Class* classes[],
    int classCount,
    const char id[]
);

bool addStudentGlobal(
    Class* classes[],
    int classCount,
    Class* classroom,
    Student* student
);

#endif
