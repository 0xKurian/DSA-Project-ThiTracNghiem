#include "QuestionList.h"
#include "StringUtils.h"
#include <iostream>

using namespace std;
static int nextQuestionId = 1;

Question* createQuestion(
    int id,
    const char content[],
    const char answerA[],
    const char answerB[],
    const char answerC[],
    const char answerD[],
    char correctAnswer
) {
    Question* question = new Question;

    question->id = id;

    stringCopy(question->content, content);

    stringCopy(question->answerA, answerA);
    stringCopy(question->answerB, answerB);
    stringCopy(question->answerC, answerC);
    stringCopy(question->answerD, answerD);

    question->correctAnswer = correctAnswer;

    question->next = nullptr;

    return question;
}

void insertQuestion(
    Question*& head,
    Question* question
) {
    if (question == nullptr) {
        return;
    }

    if (head == nullptr) {
        head = question;
        return;
    }

    Question* current = head;

    while (current->next != nullptr) {
        current = current->next;
    }

    current->next = question;
}

Question* findQuestion(
    Question* head,
    int id
) {
    Question* current = head;

    while (current != nullptr) {
        if (current->id == id) {
            return current;
        }

        current = current->next;
    }

    return nullptr;
}

int countQuestions(
    Question* head
) {
    int count = 0;

    Question* current = head;

    while (current != nullptr) {
        ++count;
        current = current->next;
    }

    return count;
}

bool deleteQuestion(
    Question*& head,
    int id
) {
    if (head == nullptr) {
        return false;
    }

    // Xoa node dau
    if (head->id == id) {
        Question* temp = head;
        head = head->next;

        delete temp;

        return true;
    }

    Question* current = head;

    while (
        current->next != nullptr &&
        current->next->id != id
    ) {
        current = current->next;
    }

    // Khong tim thay
    if (current->next == nullptr) {
        return false;
    }

    // Xoa node current->next
    Question* temp = current->next;

    current->next = temp->next;

    delete temp;

    return true;
}

void printQuestions(
    Question* head
) {
    if (head == nullptr) {
        cout << "Question list is empty\n";
        return;
    }

    Question* current = head;

    while (current != nullptr) {
        cout << current->id << " - "
             << current->content << '\n';

        current = current->next;
    }
}

int generateQuestionId() {
    return nextQuestionId++;
}

void updateQuestionIdCounter(int id) {
    if (id >= nextQuestionId) {
        nextQuestionId = id + 1;
    }
}