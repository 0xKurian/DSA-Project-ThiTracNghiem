#ifndef CLASS_ARRAY_H
#define CLASS_ARRAY_H

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