#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include "../core/ClassArray.h"
#include "../core/StudentList.h"


int main() {
    Class* classes[MAX_CLASS] = {};
    int classCount = 0;

    Class* c1 = createClass("C01", "Lop 1");
    Class* c2 = createClass("C02", "Lop 2");
    assert(addClass(classes, classCount, c1));
    assert(addClass(classes, classCount, c2));

    Student* s1 = createStudent("SV001", "Nguyen", "An", "Nam", "123");
    Student* duplicate = createStudent("SV001", "Tran", "Binh", "Nu", "456");

    assert(addStudentGlobal(classes, classCount, c1, s1));
    assert(!addStudentGlobal(classes, classCount, c2, duplicate));
    assert(findStudent(c2->studentList, "SV001") == nullptr);

    delete duplicate;

    while (classCount > 0) {
        assert(deleteClass(classes, classCount, classes[0]->id));
    }

    return 0;
}
