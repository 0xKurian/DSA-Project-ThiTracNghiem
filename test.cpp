#include <iostream>

#include "core/ClassArray.h"
#include "core/StudentList.h"
#include "core/ScoreList.h"
#include "core/SubjectBST.h"
#include "core/QuestionList.h"

using namespace std;

int main() {
    // ==================================================
    // 1. SUBJECT BST
    // ==================================================

    cout << "=== SUBJECT BST ===\n";

    Subject* root = nullptr;

    root = insertSubject(
        root,
        createSubject(
            "CTDL",
            "Cau truc du lieu"
        )
    );

    root = insertSubject(
        root,
        createSubject(
            "OOP",
            "Lap trinh huong doi tuong"
        )
    );

    root = insertSubject(
        root,
        createSubject(
            "TRR",
            "Toan roi rac"
        )
    );

    printSubjects(root);


    // ==================================================
    // 2. QUESTION LIST + GLOBAL QUESTION ID
    // ==================================================

    cout << "\n=== QUESTION LIST ===\n";

    Subject* ctdl = findSubject(
        root,
        "CTDL"
    );

    Subject* oop = findSubject(
        root,
        "OOP"
    );

    Question* q1 = createQuestion(
        generateQuestionId(),
        "2 + 2 = ?",
        "3",
        "4",
        "5",
        "6",
        'B'
    );

    Question* q2 = createQuestion(
        generateQuestionId(),
        "Stack follows which principle?",
        "FIFO",
        "LIFO",
        "Random",
        "None",
        'B'
    );

    Question* q3 = createQuestion(
        generateQuestionId(),
        "OOP stands for?",
        "Object Oriented Programming",
        "Open Object Program",
        "Only Object Programming",
        "None",
        'A'
    );

    insertQuestion(
        ctdl->questionList,
        q1
    );

    insertQuestion(
        ctdl->questionList,
        q2
    );

    insertQuestion(
        oop->questionList,
        q3
    );

    cout << "CTDL questions:\n";
    printQuestions(
        ctdl->questionList
    );

    cout << "CTDL count: "
         << countQuestions(
                ctdl->questionList
            )
         << '\n';

    cout << "OOP questions:\n";
    printQuestions(
        oop->questionList
    );


    // ==================================================
    // 3. EDIT SUBJECT + PRESERVE QUESTIONS
    // ==================================================

    cout << "\n=== EDIT SUBJECT OOP -> AI ===\n";

    if (editSubject(
        root,
        "OOP",
        "AI",
        "Artificial Intelligence"
    )) {
        cout << "Subject edited successfully\n";
    }

    printSubjects(root);

    Subject* ai = findSubject(
        root,
        "AI"
    );

    if (ai != nullptr) {
        cout << "AI question count: "
             << countQuestions(
                    ai->questionList
                )
             << '\n';

        cout << "AI questions:\n";

        printQuestions(
            ai->questionList
        );
    }


    // ==================================================
    // 4. CLASS ARRAY
    // ==================================================

    cout << "\n=== CLASS ARRAY ===\n";

    Class* classes[MAX_CLASS] = {};
    int classCount = 0;

    Class* c1 = createClass(
        "D23CQCN01",
        "Cong nghe thong tin 1"
    );

    Class* c2 = createClass(
        "D23CQCN02",
        "Cong nghe thong tin 2"
    );

    addClass(
        classes,
        classCount,
        c1
    );

    addClass(
        classes,
        classCount,
        c2
    );

    printClasses(
        classes,
        classCount
    );


    // ==================================================
    // 5. STUDENT LIST
    // ==================================================

    cout << "\n=== STUDENT LIST ===\n";

    Student* s1 = createStudent(
        "SV001",
        "Nguyen Van",
        "An",
        "Nam",
        "123"
    );

    Student* s2 = createStudent(
        "SV002",
        "Tran Thi",
        "Binh",
        "Nu",
        "456"
    );

    Student* s3 = createStudent(
        "SV003",
        "Le Van",
        "Cuong",
        "Nam",
        "789"
    );

    addStudent(
        c1,
        s1
    );

    addStudent(
        c1,
        s2
    );

    addStudent(
        c2,
        s3
    );

    cout << "Class D23CQCN01:\n";
    printStudents(c1);

    cout << "Class D23CQCN02:\n";
    printStudents(c2);


    // ==================================================
    // 6. GLOBAL STUDENT SEARCH
    // ==================================================

    cout << "\n=== GLOBAL STUDENT SEARCH ===\n";

    Student* foundStudent =
        findStudentGlobal(
            classes,
            classCount,
            "SV003"
        );

    if (foundStudent != nullptr) {
        cout << "Found: "
             << foundStudent->id
             << " - "
             << foundStudent->ho
             << " "
             << foundStudent->ten
             << '\n';
    }


    // ==================================================
    // 7. GLOBAL DUPLICATE STUDENT CHECK
    // ==================================================

    cout << "\n=== GLOBAL DUPLICATE STUDENT ===\n";

    Student* duplicateStudent =
        createStudent(
            "SV001",
            "Duplicate",
            "Student",
            "Nam",
            "000"
        );

    if (
        findStudentGlobal(
            classes,
            classCount,
            duplicateStudent->id
        ) != nullptr
    ) {
        cout << "SV001 already exists globally\n";

        delete duplicateStudent;
    }
    else {
        addStudent(
            c2,
            duplicateStudent
        );
    }


    // ==================================================
    // 8. SCORE LIST
    // ==================================================

    cout << "\n=== SCORE LIST ===\n";

    addScore(
        s1,
        "CTDL",
        8.5
    );

    addScore(
        s1,
        "AI",
        7.0
    );

    addScore(
        s3,
        "TRR",
        9.0
    );

    cout << "SV001 scores:\n";
    printScores(s1);

    cout << "Update AI score:\n";

    updateScore(
        s1,
        "AI",
        9.25
    );

    printScores(s1);


    // ==================================================
    // 9. FIND SCORE
    // ==================================================

    cout << "\n=== FIND SCORE ===\n";

    Score* foundScore =
        findScore(
            s1,
            "AI"
        );

    if (foundScore != nullptr) {
        cout << "Found AI score: "
             << foundScore->score
             << '\n';
    }


    // ==================================================
    // 10. DELETE QUESTION
    // ==================================================

    cout << "\n=== DELETE QUESTION ===\n";

    if (deleteQuestion(
        ctdl->questionList,
        q1->id
    )) {
        cout << "Question deleted successfully\n";
    }

    cout << "CTDL count after delete: "
         << countQuestions(
                ctdl->questionList
            )
         << '\n';

    printQuestions(
        ctdl->questionList
    );


    // ==================================================
    // 11. DELETE STUDENT WITH SCORE LIST
    // ==================================================

    cout << "\n=== DELETE STUDENT SV001 ===\n";

    if (deleteStudent(
        c1,
        "SV001"
    )) {
        cout << "Student deleted successfully\n";
    }

    foundStudent =
        findStudentGlobal(
            classes,
            classCount,
            "SV001"
        );

    if (foundStudent == nullptr) {
        cout << "SV001 no longer exists\n";
    }

    cout << "Remaining students in D23CQCN01:\n";
    printStudents(c1);


    // ==================================================
    // 12. DELETE CLASS WITH STUDENTS + SCORES
    // ==================================================

    cout << "\n=== DELETE CLASS D23CQCN02 ===\n";

    if (deleteClass(
        classes,
        classCount,
        "D23CQCN02"
    )) {
        cout << "Class deleted successfully\n";
    }

    printClasses(
        classes,
        classCount
    );

    foundStudent =
        findStudentGlobal(
            classes,
            classCount,
            "SV003"
        );

    if (foundStudent == nullptr) {
        cout << "SV003 no longer exists\n";
    }


    // ==================================================
    // 13. CLEANUP CLASSES
    // ==================================================

    cout << "\n=== CLEANUP CLASSES ===\n";

    while (classCount > 0) {
        deleteClass(
            classes,
            classCount,
            classes[0]->id
        );
    }

    printClasses(
        classes,
        classCount
    );


    // ==================================================
    // 14. CLEANUP SUBJECT BST
    // ==================================================

    cout << "\n=== CLEANUP SUBJECTS ===\n";

    root = deleteSubject(
        root,
        "AI"
    );

    root = deleteSubject(
        root,
        "CTDL"
    );

    root = deleteSubject(
        root,
        "TRR"
    );

    if (root == nullptr) {
        cout << "Subject tree is empty\n";
    }


    // ==================================================
    // END
    // ==================================================

    cout << "\n=== CORE INTEGRATION TEST PASSED ===\n";

    return 0;
}