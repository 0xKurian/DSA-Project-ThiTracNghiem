#include "ScoreList.h"
#include "StringUtils.h"
#include <iostream>

using namespace std;

Score* createScore(
    const char subjectId[],
    float score
) {
    Score* newScore = new Score;

    stringCopy(
        newScore->subjectId,
        subjectId,
        SUBJECT_ID_LEN
    );

    newScore->score = score;
    newScore->next = nullptr;

    return newScore;
}

Score* findScore(
    Student* student,
    const char subjectId[]
) {
    if (student == nullptr) {
        return nullptr;
    }

    Score* current = student->scoreList;

    while (current != nullptr) {
        if (stringEqual(
            current->subjectId,
            subjectId
        )) {
            return current;
        }

        current = current->next;
    }

    return nullptr;
}

bool addScore(
    Student* student,
    const char subjectId[],
    float score
) {
    if (student == nullptr) {
        return false;
    }

    if (findScore(student, subjectId) != nullptr) {
        return false;
    }

    Score* newScore = createScore(
        subjectId,
        score
    );

    if (student->scoreList == nullptr) {
        student->scoreList = newScore;
        return true;
    }

    Score* current = student->scoreList;

    while (current->next != nullptr) {
        current = current->next;
    }

    current->next = newScore;

    return true;
}

bool updateScore(
    Student* student,
    const char subjectId[],
    float score
) {
    Score* found = findScore(
        student,
        subjectId
    );

    if (found == nullptr) {
        return false;
    }

    found->score = score;

    return true;
}

void printScores(
    Student* student
) {
    if (student == nullptr) {
        return;
    }

    if (student->scoreList == nullptr) {
        cout << "Score list is empty\n";
        return;
    }

    Score* current = student->scoreList;

    while (current != nullptr) {
        cout << current->subjectId
             << " - "
             << current->score
             << '\n';

        current = current->next;
    }
}

bool deleteScore(
    Student* student,
    const char subjectId[]
) {
    if (student == nullptr) {
        return false;
    }

    Score*& head = student->scoreList;

    if (head == nullptr) {
        return false;
    }

    // Xoa node dau
    if (stringEqual(head->subjectId, subjectId)) {
        Score* temp = head;

        head = head->next;

        delete temp;

        return true;
    }

    Score* current = head;

    while (
        current->next != nullptr &&
        !stringEqual(
            current->next->subjectId,
            subjectId
        )
    ) {
        current = current->next;
    }

    if (current->next == nullptr) {
        return false;
    }

    Score* temp = current->next;

    current->next = temp->next;

    delete temp;

    return true;
}