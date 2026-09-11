#include <cassert>
#include "../core/SubjectBST.h"

int main() {
    Subject* root = nullptr;

    root = insertSubject(root, createSubject("CTDL", "Cau truc du lieu"));
    root = insertSubject(root, createSubject("CTDL", "Ten bi trung"));

    Subject* found = findSubject(root, "CTDL");
    assert(found != nullptr);

    root = deleteSubject(root, "CTDL");
    assert(root == nullptr);
    return 0;
}
