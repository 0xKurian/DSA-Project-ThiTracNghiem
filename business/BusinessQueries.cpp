#include "BusinessQueries.h"

#include "../core/ClassArray.h"
#include "../core/SubjectBST.h"

bool isPassingScore(float score) {
    return score >= 5.0f;
}

int countStudentsInClass(const Class* classroom) {
    if (classroom == nullptr) return 0;
    int count = 0;
    for (Student* student = classroom->studentList; student != nullptr;
         student = student->next) ++count;
    return count;
}

static void collectInOrder(Subject* root, Subject* out[], int capacity,
                           int& count) {
    if (root == nullptr || count >= capacity) return;
    collectInOrder(root->left, out, capacity, count);
    if (count < capacity) out[count++] = root;
    collectInOrder(root->right, out, capacity, count);
}

int collectSubjectsInOrder(Subject* root, Subject* out[], int capacity) {
    if (out == nullptr || capacity <= 0) return 0;
    int count = 0;
    collectInOrder(root, out, capacity, count);
    return count;
}

void freeAllData(Subject*& root, Class* classes[], int& classCount) {
    if (classes != nullptr) {
        while (classCount > 0) {
            if (classes[0] == nullptr) break;
            deleteClass(classes, classCount, classes[0]->id);
        }
    }
    while (root != nullptr) root = deleteSubject(root, root->id);
}
