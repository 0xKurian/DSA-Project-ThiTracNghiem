#include "ClassArray.h"
#include "StudentList.h"
#include "StringUtils.h"

#include <iostream>

using namespace std;


// ==================== CREATE ====================

Class* createClass(
    const char id[],
    const char name[]
) {
    if (!isValidIdentifier(id, CLASS_ID_LEN)) {
        return nullptr;
    }

    Class* classroom = new Class;

    stringCopy(
        classroom->id,
        id,
        CLASS_ID_LEN
    );

    stringCopy(
        classroom->name,
        name,
        CLASS_NAME_LEN
    );

    classroom->studentList = nullptr;

    return classroom;
}


// ==================== FIND ====================

Class* findClass(
    Class* classes[],
    int classCount,
    const char id[]
) {
    for (int i = 0; i < classCount; ++i) {
        if (
            classes[i] != nullptr &&
            stringEqual(classes[i]->id, id)
        ) {
            return classes[i];
        }
    }

    return nullptr;
}


// ==================== ADD ====================

bool addClass(
    Class* classes[],
    int& classCount,
    Class* newClass
) {
    if (newClass == nullptr) {
        return false;
    }

    if (classCount >= MAX_CLASS) {
        return false;
    }

    if (findClass(
        classes,
        classCount,
        newClass->id
    ) != nullptr) {
        return false;
    }

    classes[classCount] = newClass;

    ++classCount;

    return true;
}


// ==================== CLEANUP HELPERS ====================

static void deleteExamDetailList(
    ExamDetail*& head
) {
    while (head != nullptr) {
        ExamDetail* temp = head;
        head = head->next;
        delete temp;
    }
}

static void deleteScoreListOfStudent(
    Score*& head
) {
    while (head != nullptr) {
        Score* temp = head;
        head = head->next;

        deleteExamDetailList(
            temp->detailList
        );

        delete temp;
    }
}

static void deleteStudentListOfClass(
    Student*& head
) {
    while (head != nullptr) {
        Student* temp = head;

        head = head->next;

        deleteScoreListOfStudent(
            temp->scoreList
        );

        delete temp;
    }
}


// ==================== DELETE ====================

bool deleteClass(
    Class* classes[],
    int& classCount,
    const char id[]
) {
    int index = -1;

    for (int i = 0; i < classCount; ++i) {
        if (
            classes[i] != nullptr &&
            stringEqual(classes[i]->id, id)
        ) {
            index = i;
            break;
        }
    }

    if (index == -1) {
        return false;
    }

    deleteStudentListOfClass(
        classes[index]->studentList
    );

    delete classes[index];

    for (
        int i = index;
        i < classCount - 1;
        ++i
    ) {
        classes[i] = classes[i + 1];
    }

    --classCount;

    classes[classCount] = nullptr;

    return true;
}


// ==================== PRINT ====================

void printClasses(
    Class* classes[],
    int classCount
) {
    if (classCount == 0) {
        cout << "Class list is empty\n";
        return;
    }

    for (int i = 0; i < classCount; ++i) {
        if (classes[i] != nullptr) {
            cout << classes[i]->id
                 << " - "
                 << classes[i]->name
                 << '\n';
        }
    }
}


// ==================== FIND STUDENT GLOBAL ====================

Student* findStudentGlobal(
    Class* classes[],
    int classCount,
    const char id[]
) {
    for (int i = 0; i < classCount; ++i) {
        if (classes[i] == nullptr) {
            continue;
        }

        Student* found = findStudent(
            classes[i]->studentList,
            id
        );

        if (found != nullptr) {
            return found;
        }
    }

    return nullptr;
}

// ==================== ADD STUDENT GLOBAL ====================

bool addStudentGlobal(
    Class* classes[],
    int classCount,
    Class* classroom,
    Student* student
) {
    if (classes == nullptr || classCount < 0 || classCount > MAX_CLASS ||
        classroom == nullptr || student == nullptr) {
        return false;
    }

    bool registered = false;
    for (int i = 0; i < classCount; ++i) {
        if (classes[i] == classroom) {
            registered = true;
            break;
        }
    }
    if (!registered) {
        return false;
    }

    if (findStudentGlobal(
        classes,
        classCount,
        student->id
    ) != nullptr) {
        return false;
    }

    return addStudent(
        classroom,
        student
    );
}

