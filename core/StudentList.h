#ifndef STUDENT_LIST_H
#define STUDENT_LIST_H

// Contract: createStudent rejects null/empty/overlong IDs with nullptr.
// Other text inputs must be valid null-terminated strings.
// addStudent accepts a fresh detached student, taking ownership only on success.
// It checks uniqueness in one class; use addStudentGlobal (ClassArray.h) for
// uniqueness across the registered classes.
// deleteStudent also frees owned scores and exam details.

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
