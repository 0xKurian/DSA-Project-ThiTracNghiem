#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include "../core/StudentList.h"
#include "../core/Constants.h"

int main() {
    const char longId[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    Student* student = createStudent(
        longId,
        "Nguyen Van",
        "An",
        "Nam",
        "123456"
    );

    assert(student == nullptr);

    delete student;
    return 0;
}
