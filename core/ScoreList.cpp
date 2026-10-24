#include "ScoreList.h"
#include "StringUtils.h"

#include <cstdlib>
#include <ctime>
#include <iostream>

using namespace std;

static void initializeRandomOnce() {
    static bool initialized = false;

    if (!initialized) {
        srand(static_cast<unsigned int>(time(nullptr)));
        initialized = true;
    }
}

static void swapAnswerText(
    char a[],
    char b[]
) {
    char temp[ANSWER_CONTENT_LEN];

    stringCopy(temp, a, ANSWER_CONTENT_LEN);
    stringCopy(a, b, ANSWER_CONTENT_LEN);
    stringCopy(b, temp, ANSWER_CONTENT_LEN);
}

void shuffleExamDetailAnswers(
    ExamDetail* detail
) {

    if (detail == nullptr || detail->selectedAnswer != '\0') {
        return;
    }

    int correctIndex = detail->correctAnswer - 'A';

    if (correctIndex < 0 || correctIndex > 3) {
        return;
    }

    initializeRandomOnce();

    char* answers[4] = {
        detail->answerA,
        detail->answerB,
        detail->answerC,
        detail->answerD
    };

    for (int i = 3; i > 0; --i) {
        int j = rand() % (i + 1);

        if (i == j) {
            continue;
        }

        swapAnswerText(
            answers[i],
            answers[j]
        );

        if (correctIndex == i) {
            correctIndex = j;
        }
        else if (correctIndex == j) {
            correctIndex = i;
        }
    }

    detail->correctAnswer =
        static_cast<char>('A' + correctIndex);
}

ExamDetail* createExamDetail(
    const Question* question
) {
    if (question == nullptr) {
        return nullptr;
    }

    if (
        question->correctAnswer < 'A' ||
        question->correctAnswer > 'D'
    ) {
        return nullptr;
    }

    ExamDetail* detail = new ExamDetail;

    detail->questionId = question->id;

    stringCopy(
        detail->content,
        question->content,
        QUESTION_CONTENT_LEN
    );

    stringCopy(
        detail->answerA,
        question->answerA,
        ANSWER_CONTENT_LEN
    );

    stringCopy(
        detail->answerB,
        question->answerB,
        ANSWER_CONTENT_LEN
    );

    stringCopy(
        detail->answerC,
        question->answerC,
        ANSWER_CONTENT_LEN
    );

    stringCopy(
        detail->answerD,
        question->answerD,
        ANSWER_CONTENT_LEN
    );

    detail->correctAnswer = question->correctAnswer;
    detail->selectedAnswer = '\0';
    detail->next = nullptr;

    shuffleExamDetailAnswers(detail);

    return detail;
}

void addExamDetail(
    ExamDetail*& head,
    ExamDetail* detail
) {
    if (detail == nullptr) {
        return;
    }

    detail->next = nullptr;

    if (head == nullptr) {
        head = detail;
        return;
    }

    ExamDetail* current = head;

    while (current->next != nullptr) {
        current = current->next;
    }

    current->next = detail;
}

void deleteExamDetailList(
    ExamDetail*& head
) {
    while (head != nullptr) {
        ExamDetail* temp = head;
        head = head->next;
        delete temp;
    }
}

Score* createScore(
    const char subjectId[],
    float score,
    ExamDetail* detailList
) {
    if (!isValidIdentifier(subjectId, SUBJECT_ID_LEN)) {
        return nullptr;
    }

    Score* newScore = new Score;

    stringCopy(
        newScore->subjectId,
        subjectId,
        SUBJECT_ID_LEN
    );

    newScore->score = score;
    newScore->detailList = detailList;
    newScore->next = nullptr;

    return newScore;
}

Score* createScore(
    const char subjectId[],
    float score
) {
    return createScore(
        subjectId,
        score,
        nullptr
    );
}

Score* findScore(
    Student* student,
    const char subjectId[]
) {
    if (student == nullptr || subjectId == nullptr) {
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
    float score,
    ExamDetail* detailList
) {
    if (student == nullptr) {
        return false;
    }

    if (
        !isValidIdentifier(subjectId, SUBJECT_ID_LEN)
    ) {
        return false;
    }

    if (findScore(student, subjectId) != nullptr) {
        return false;
    }

    Score* newScore = createScore(
        subjectId,
        score,
        detailList
    );

    if (newScore == nullptr) {
        return false;
    }

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

bool addScore(
    Student* student,
    const char subjectId[],
    float score
) {
    return addScore(
        student,
        subjectId,
        score,
        nullptr
    );
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
    if (student == nullptr || subjectId == nullptr) {
        return false;
    }

    Score*& head = student->scoreList;

    if (head == nullptr) {
        return false;
    }

    if (stringEqual(head->subjectId, subjectId)) {
        Score* temp = head;

        head = head->next;

        deleteExamDetailList(
            temp->detailList
        );

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

    deleteExamDetailList(
        temp->detailList
    );

    delete temp;

    return true;
}
