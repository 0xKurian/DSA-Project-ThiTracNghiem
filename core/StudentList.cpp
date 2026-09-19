#include "StudentList.h"
#include "StringUtils.h"
#include <iostream>

using namespace std;

Student* createStudent(
    const char id[],
    const char ho[],
    const char ten[],
    const char gender[],
    const char password[]
) {
    Student* student = new Student;

    stringCopy(student->id, id, STUDENT_ID_LEN);
    stringCopy(student->ho, ho, HO_LEN);
    stringCopy(student->ten, ten, TEN_LEN);
    stringCopy(student->gender, gender, GENDER_LEN);
    stringCopy(student->password, password, PASSWORD_LEN);

    student->scoreList = nullptr;

    student->next = nullptr;

    return student;
}

Student* findStudent(
    Student* head,
    const char id[]
) {
    Student* current = head;

    while (current != nullptr) {
        if (stringEqual(current->id, id)) {
            return current;
        }

        current = current->next;
    }

    return nullptr;
}

bool addStudent(
    Class* classroom,
    Student* student
) {
    if (classroom == nullptr || student == nullptr) {
        return false;
    }

    if (findStudent(classroom->studentList, student->id) != nullptr) {
        return false;
    }

    if (classroom->studentList == nullptr) {
        classroom->studentList = student;
        return true;
    }

    Student* current = classroom->studentList;

    while (current->next != nullptr) {
        current = current->next;
    }

    current->next = student;

    return true;
}

static void deleteExamDetailList(
    ExamDetail*& head
) {
    while (head != nullptr) {
        ExamDetail* temp = head;
        head = head->next;
        delete temp;
    }
}

static void deleteScoreList(
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

bool deleteStudent(
    Class* classroom,
    const char studentId[]
) {
    if (classroom == nullptr) {
        return false;
    }

    Student*& head = classroom->studentList;

    if (head == nullptr) {
        return false;
    }

    // Xoa node dau
    if (stringEqual(head->id, studentId)) {
        Student* temp = head;

        head = head->next;

        deleteScoreList(
            temp->scoreList
        );

        delete temp;

        return true;
    }

    Student* current = head;

    while (
        current->next != nullptr &&
        !stringEqual(
            current->next->id,
            studentId
        )
    ) {
        current = current->next;
    }

    if (current->next == nullptr) {
        return false;
    }

    Student* temp = current->next;

    current->next = temp->next;

    deleteScoreList(
        temp->scoreList
    );

    delete temp;

    return true;
}

bool editStudent(
    Student* student,
    const char ho[],
    const char ten[],
    const char gender[],
    const char password[]
) {
    if (student == nullptr) {
        return false;
    }

    stringCopy(student->ho, ho, HO_LEN);
    stringCopy(student->ten, ten, TEN_LEN);
    stringCopy(student->gender, gender, GENDER_LEN);
    stringCopy(student->password, password, PASSWORD_LEN);

    return true;
}

void printStudents(
    Class* classroom
) {
    if (classroom == nullptr) {
        return;
    }

    if (classroom->studentList == nullptr) {
        cout << "Student list is empty\n";
        return;
    }

    Student* current = classroom->studentList;

    while (current != nullptr) {
        cout << current->id
             << " - "
             << current->ho
             << " "
             << current->ten
             << " - "
             << current->gender
             << '\n';

        current = current->next;
    }
}