#ifndef STUDENT_LIST_H
#define STUDENT_LIST_H

#include "Structures.h"

Student* createStudent(
    const char id[],
    const char ho[],
    const char ten[],
    const char gender[],
    const char password[]
);

Student* findStudent(
    Student* head,
    const char id[]
);

Student* findStudentGlobal(
    Class* classes[],
    int classCount,
    const char id[]
);

bool addStudent(
    Class* classroom,
    Student* student
);

bool deleteStudent(
    Class* classroom,
    const char studentId[]
);

void printStudents(
    Class* classroom
);

bool editStudent(
    Student* student,
    const char ho[],
    const char ten[],
    const char gender[],
    const char password[]
);

#endif