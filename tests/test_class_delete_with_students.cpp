#include "../core/Structures.h"
#include "../core/ClassArray.h"
#include "../core/StudentList.h"
#include "../core/ScoreList.h"
#include "../business/ClassService.h"
#include <cassert>
#include <iostream>

using namespace std;

static int countStudentsInClass(Student* head) {
    int count = 0;
    while (head != nullptr) {
        ++count;
        head = head->next;
    }
    return count;
}

int main() {
    cout << "=== Running test_class_delete_with_students ===\n";

    Class* classes[MAX_CLASS] = {};
    int classCount = 0;

    // 1. Create 2 classes
    Class* c1 = createClass("D23CN01", "Lop Cong nghe 1");
    Class* c2 = createClass("D23CN02", "Lop Cong nghe 2");
    assert(addClass(classes, classCount, c1));
    assert(addClass(classes, classCount, c2));
    assert(classCount == 2);

    // 2. Add students to C1
    Student* s1 = createStudent("SV001", "Nguyen Van", "An", "Nam", "pass1");
    Student* s2 = createStudent("SV002", "Tran Thi", "Binh", "Nu", "pass2");
    assert(s1 != nullptr && s2 != nullptr);
    assert(addStudent(c1, s1));
    assert(addStudent(c1, s2));

    // Add score with detail to s1
    assert(addScore(s1, "CTDL", 9.5f));

    // 3. Add student to C2
    Student* s3 = createStudent("SV003", "Le Van", "Cuong", "Nam", "pass3");
    assert(s3 != nullptr);
    assert(addStudent(c2, s3));

    // Verify initial counts
    assert(countStudentsInClass(c1->studentList) == 2);
    assert(countStudentsInClass(c2->studentList) == 1);

    // 4. Test nonexistent class before deletion
    BusinessResult rNonExist = deleteClassWorkflow(classes, classCount, "NONEXIST");
    assert(rNonExist == ERR_NOT_FOUND);
    assert(classCount == 2);

    BusinessResult rNull = deleteClassWorkflow(classes, classCount, nullptr);
    assert(rNull == ERR_NOT_FOUND);
    assert(classCount == 2);

    BusinessResult rEmpty = deleteClassWorkflow(classes, classCount, "");
    assert(rEmpty == ERR_NOT_FOUND);
    assert(classCount == 2);

    // 5. Delete class D23CN01 which has students -> MUST return SUCCESS
    BusinessResult rDel = deleteClassWorkflow(classes, classCount, "D23CN01");
    assert(rDel == SUCCESS);
    assert(classCount == 1);

    // Deleted class D23CN01 is no longer found
    assert(findClass(classes, classCount, "D23CN01") == nullptr);

    // Remaining class is D23CN02 at index 0
    assert(classes[0] != nullptr);
    assert(classes[0] == c2);
    assert(countStudentsInClass(classes[0]->studentList) == 1);
    assert(findStudent(classes[0]->studentList, "SV003") != nullptr);

    // 6. Delete remaining class D23CN02 (also has a student) -> returns SUCCESS
    rDel = deleteClassWorkflow(classes, classCount, "D23CN02");
    assert(rDel == SUCCESS);
    assert(classCount == 0);
    assert(findClass(classes, classCount, "D23CN02") == nullptr);

    // 7. Deleting from empty list returns ERR_NOT_FOUND
    assert(deleteClassWorkflow(classes, classCount, "D23CN02") == ERR_NOT_FOUND);

    cout << "PASS test_class_delete_with_students\n";
    return 0;
}
