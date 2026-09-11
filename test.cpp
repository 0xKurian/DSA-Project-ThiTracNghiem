#include <cassert>
#include <iostream>

#include "core/ClassArray.h"
#include "core/StudentList.h"
#include "core/ScoreList.h"
#include "core/SubjectBST.h"
#include "core/QuestionList.h"
#include "core/StringUtils.h"

using namespace std;

int main() {
    // ==================== SUBJECT + QUESTION ====================
    Subject* root = nullptr;

    root = insertSubject(root, createSubject("CTDL", "Cau truc du lieu"));
    root = insertSubject(root, createSubject("OOP", "Lap trinh huong doi tuong"));
    root = insertSubject(root, createSubject("TRR", "Toan roi rac"));

    assert(findSubject(root, "CTDL") != nullptr);
    assert(findSubject(root, "OOP") != nullptr);
    assert(findSubject(root, "TRR") != nullptr);

    Subject* oop = findSubject(root, "OOP");
    Question* q1 = createQuestion(
        generateQuestionId(),
        "OOP stands for?",
        "Object Oriented Programming",
        "Open Object Program",
        "Only Object Programming",
        "None",
        'A'
    );
    insertQuestion(oop->questionList, q1);
    assert(countQuestions(oop->questionList) == 1);

    // Hieu chinh mon hoc: cho phep doi ma va ten,
    // dong thoi giu lai danh sach cau hoi cua mon.
    assert(editSubject(root, "OOP", "AI", "Artificial Intelligence"));
    assert(findSubject(root, "OOP") == nullptr);
    assert(findSubject(root, "AI") != nullptr);
    assert(countQuestions(findSubject(root, "AI")->questionList) == 1);

    assert(editSubject(
        root,
        "AI",
        "AI",
        "Artificial Intelligence - updated"
    ));
    assert(stringEqual(
        findSubject(root, "AI")->name,
        "Artificial Intelligence - updated"
    ));

    // Insert trung ma mon khong tao them node.
    root = insertSubject(root, createSubject("CTDL", "Duplicate"));
    assert(findSubject(root, "CTDL") != nullptr);

    // ==================== CLASS + STUDENT ====================
    Class* classes[MAX_CLASS] = {};
    int classCount = 0;

    Class* c1 = createClass("D23CQCN01", "Cong nghe thong tin 1");
    Class* c2 = createClass("D23CQCN02", "Cong nghe thong tin 2");

    assert(addClass(classes, classCount, c1));
    assert(addClass(classes, classCount, c2));
    assert(classCount == 2);

    Student* s1 = createStudent("SV001", "Nguyen Van", "An", "Nam", "123");
    Student* s2 = createStudent("SV002", "Tran Thi", "Binh", "Nu", "456");
    Student* duplicate = createStudent("SV001", "Le", "Trung", "Nam", "789");

    assert(addStudentGlobal(classes, classCount, c1, s1));
    assert(addStudentGlobal(classes, classCount, c2, s2));
    assert(!addStudentGlobal(classes, classCount, c2, duplicate));
    delete duplicate;

    assert(findStudentGlobal(classes, classCount, "SV001") == s1);
    assert(findStudentGlobal(classes, classCount, "SV002") == s2);

    // ==================== SCORE ====================
    assert(addScore(s1, "CTDL", 8.5f));
    assert(!addScore(s1, "CTDL", 9.0f));
    assert(findScore(s1, "CTDL") != nullptr);
    assert(updateScore(s1, "CTDL", 9.25f));
    assert(findScore(s1, "CTDL")->score == 9.25f);
    assert(deleteScore(s1, "CTDL"));
    assert(findScore(s1, "CTDL") == nullptr);

    // ==================== SAFE FIXED-SIZE STRING ====================
    const char longId[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    Student* longIdStudent = createStudent(
        longId,
        "Nguyen Van",
        "An",
        "Nam",
        "123456"
    );
    assert(longIdStudent->id[STUDENT_ID_LEN - 1] == '\0');
    delete longIdStudent;

    // ==================== CLEANUP ====================
    while (classCount > 0) {
        assert(deleteClass(classes, classCount, classes[0]->id));
    }

    root = deleteSubject(root, "AI");
    root = deleteSubject(root, "CTDL");
    root = deleteSubject(root, "TRR");
    assert(root == nullptr);

    cout << "ALL CORE TESTS PASSED\n";
    return 0;
}
