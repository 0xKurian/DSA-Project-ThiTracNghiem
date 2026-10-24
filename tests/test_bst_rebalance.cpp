#include "SubjectBST.h"
#include "SubjectService.h"

#include <cassert>
#include <cstdio>
#include <cstring>

static int height(Subject* root) {
    if (root == nullptr) return 0;
    int left = height(root->left);
    int right = height(root->right);
    return 1 + (left > right ? left : right);
}

static void collect(Subject* root, char ids[][SUBJECT_ID_LEN], int& count) {
    if (root == nullptr) return;
    collect(root->left, ids, count);
    std::strcpy(ids[count++], root->id);
    collect(root->right, ids, count);
}

int main() {
    Subject* root = nullptr;
    rebalanceSubjectBST(root);
    assert(root == nullptr);
    for (int i = 1; i <= 100; ++i) {
        char id[SUBJECT_ID_LEN];
        std::snprintf(id, sizeof(id), "S%03d", i);
        root = insertSubject(root, createSubject(id, id));
    }
    assert(height(root) == 100);
    char before[100][SUBJECT_ID_LEN] = {};
    char after[100][SUBJECT_ID_LEN] = {};
    int count = 0;
    collect(root, before, count);
    assert(count == 100);
    rebalanceSubjectBST(root);
    assert(height(root) <= 14);
    count = 0;
    collect(root, after, count);
    assert(count == 100);
    for (int i = 0; i < 100; ++i) {
        assert(std::strcmp(before[i], after[i]) == 0);
        assert(findSubject(root, before[i]) != nullptr);
    }
    rebalanceSubjectBST(root);
    assert(height(root) <= 14);
    while (root != nullptr) root = deleteSubject(root, root->id);
    root = createSubject("S1", "One");
    rebalanceSubjectBST(root);
    assert(root != nullptr && height(root) == 1);
    root = insertSubject(root, createSubject("S2", "Two"));
    rebalanceSubjectBST(root);
    assert(height(root) == 2);
    while (root != nullptr) root = deleteSubject(root, root->id);
}
