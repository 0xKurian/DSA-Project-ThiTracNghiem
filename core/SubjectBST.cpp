#include "SubjectBST.h"
#include "StringUtils.h"
#include <iostream>

using namespace std;

static void deleteQuestionList(Question*& head);

Subject* createSubject(
    const char id[],
    const char name[]
) {
    if (!isValidIdentifier(id, SUBJECT_ID_LEN)) {
        return nullptr;
    }

    Subject* subject = new Subject;

    stringCopy(subject->id, id, SUBJECT_ID_LEN);
    stringCopy(subject->name, name, SUBJECT_NAME_LEN);

    subject->questionList = nullptr;

    subject->examQuestionCount = 0;
    subject->examMinutes = 0;

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
    else if (subject != root) {

        deleteQuestionList(subject->questionList);
        delete subject;
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

        if (root->left == nullptr) {
            Subject* temp = root->right;

            deleteQuestionList(
                root->questionList
            );

            delete root;

            return temp;
        }

        if (root->right == nullptr) {
            Subject* temp = root->left;

            deleteQuestionList(
                root->questionList
            );

            delete root;

            return temp;
        }

        Subject* successor =
            findMinSubject(root->right);

        deleteQuestionList(
            root->questionList
        );

        stringCopy(
            root->id,
            successor->id,
            SUBJECT_ID_LEN
        );

        stringCopy(
            root->name,
            successor->name,
            SUBJECT_NAME_LEN
        );

        root->questionList =
            successor->questionList;

        root->examQuestionCount =
            successor->examQuestionCount;

        root->examMinutes =
            successor->examMinutes;

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

    if (!isValidIdentifier(newId, SUBJECT_ID_LEN)) {
        return false;
    }

    Subject* subject = findSubject(root, oldId);

    if (subject == nullptr) {
        return false;
    }

    if (stringEqual(oldId, newId)) {
        stringCopy(
            subject->name,
            newName,
            SUBJECT_NAME_LEN
        );
        return true;
    }

    if (findSubject(root, newId) != nullptr) {
        return false;
    }

    Subject* newSubject = createSubject(
        newId,
        newName
    );

    newSubject->questionList =
        subject->questionList;

    newSubject->examQuestionCount =
        subject->examQuestionCount;

    newSubject->examMinutes =
        subject->examMinutes;

    subject->questionList = nullptr;

    root = deleteSubject(
        root,
        oldId
    );

    root = insertSubject(
        root,
        newSubject
    );

    return true;
}
