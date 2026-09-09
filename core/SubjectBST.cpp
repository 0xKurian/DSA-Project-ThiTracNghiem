#include "SubjectBST.h"
#include "StringUtils.h"
#include <iostream>

using namespace std;

Subject* createSubject(
    const char id[],
    const char name[]
) {
    Subject* subject = new Subject;

    stringCopy(subject->id, id);
    stringCopy(subject->name, name);

    subject->questionList = nullptr;

    subject->left = nullptr;
    subject->right = nullptr;

    return subject;
}

Subject* insertSubject(
    Subject* root,
    Subject* subject
) {
    if (subject == nullptr) {
        return root;
    }

    if (root == nullptr) {
        return subject;
    }

    int cmp = stringCompare(subject->id, root->id);

    if (cmp < 0) {
        root->left = insertSubject(root->left, subject);
    }
    else if (cmp > 0) {
        root->right = insertSubject(root->right, subject);
    }

    return root;
}

Subject* findSubject(
    Subject* root,
    const char id[]
) {
    if (root == nullptr) {
        return nullptr;
    }

    int cmp = stringCompare(id, root->id);

    if (cmp == 0) {
        return root;
    }

    if (cmp < 0) {
        return findSubject(root->left, id);
    }

    return findSubject(root->right, id);
}

void printSubjects(
    Subject* root
) {
    if (root == nullptr) {
        return;
    }

    printSubjects(root->left);

    cout << root->id
         << " - "
         << root->name
         << '\n';

    printSubjects(root->right);
}

static Subject* findMinSubject(
    Subject* root
) {
    if (root == nullptr) {
        return nullptr;
    }

    while (root->left != nullptr) {
        root = root->left;
    }

    return root;
}

static void deleteQuestionList(
    Question*& head
) {
    while (head != nullptr) {
        Question* temp = head;

        head = head->next;

        delete temp;
    }
}

Subject* deleteSubject(
    Subject* root,
    const char id[]
) {
    if (root == nullptr) {
        return nullptr;
    }

    int cmp = stringCompare(id, root->id);

    if (cmp < 0) {
        root->left = deleteSubject(
            root->left,
            id
        );
    }
    else if (cmp > 0) {
        root->right = deleteSubject(
            root->right,
            id
        );
    }
    else {
        // Khong co con trai
        if (root->left == nullptr) {
            Subject* temp = root->right;

            deleteQuestionList(
                root->questionList
            );

            delete root;

            return temp;
        }

        // Khong co con phai
        if (root->right == nullptr) {
            Subject* temp = root->left;

            deleteQuestionList(
                root->questionList
            );

            delete root;

            return temp;
        }

        // Co 2 con
        Subject* successor =
            findMinSubject(root->right);

        deleteQuestionList(
            root->questionList
        );

        stringCopy(
            root->id,
            successor->id
        );

        stringCopy(
            root->name,
            successor->name
        );

        root->questionList =
            successor->questionList;

        successor->questionList = nullptr;

        root->right = deleteSubject(
            root->right,
            successor->id
        );
    }

    return root;
}

bool editSubject(
    Subject*& root,
    const char oldId[],
    const char newId[],
    const char newName[]
) {
    Subject* subject = findSubject(root, oldId);

    if (subject == nullptr) {
        return false;
    }

    // Khong doi ma mon, chi cap nhat ten
    if (stringEqual(oldId, newId)) {
        stringCopy(subject->name, newName);
        return true;
    }

    // Ma mon moi da ton tai
    if (findSubject(root, newId) != nullptr) {
        return false;
    }

    // Tao node moi va giu lai danh sach cau hoi
    Subject* newSubject = createSubject(
        newId,
        newName
    );

    newSubject->questionList =
        subject->questionList;

    // Khong cho deleteSubject xoa questionList nay
    subject->questionList = nullptr;

    // Xoa node cu khoi BST
    root = deleteSubject(
        root,
        oldId
    );

    // Chen node moi vao dung vi tri BST
    root = insertSubject(
        root,
        newSubject
    );

    return true;
}