#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include "../core/StudentList.h"
#include "../core/ScoreList.h"

int main() {
    Student* student = createStudent("SV1", "Nguyen", "An", "Nam", "123");
    bool added = addScore(student, "ABCDEFGHIJKLMNO", 8.0f);
    assert(added);
    added = addScore(student, "ABCDEFGHIJKLMNOX", 9.0f);
    assert(!added);
    assert(student->scoreList->next == nullptr);
    assert(student->scoreList->score == 8.0f);
    bool deleted = deleteScore(student, "ABCDEFGHIJKLMNO");
    assert(deleted);
    delete student;
}
