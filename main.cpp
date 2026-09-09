#include <iostream>

#include "core/QuestionList.h"

using namespace std;

int main() {
    Question* head = nullptr;

    cout << "Generated IDs:\n";

    cout << generateQuestionId() << '\n';
    cout << generateQuestionId() << '\n';
    cout << generateQuestionId() << '\n';

    updateQuestionIdCounter(100);

    cout << "After update to 100:\n";
    cout << generateQuestionId() << '\n';
    cout << generateQuestionId() << '\n';

    cout << '\n';

    Question* q1 = createQuestion(
        1,
        "2 + 2 = ?",
        "1",
        "2",
        "3",
        "4",
        'D'
    );

    Question* q2 = createQuestion(
        2,
        "3 + 3 = ?",
        "4",
        "5",
        "6",
        "7",
        'C'
    );

    Question* q3 = createQuestion(
        3,
        "5 - 2 = ?",
        "1",
        "2",
        "3",
        "4",
        'C'
    );

    insertQuestion(head, q1);
    insertQuestion(head, q2);
    insertQuestion(head, q3);

    printQuestions(head);

    Question* found = findQuestion(head, 2);

    if (found != nullptr) {
        cout << "Found: "
            << found->id << " - "
            << found->content << '\n';
    }
    else {
        cout << "Question not found\n";
    }

    found = findQuestion(head, 100);

    if (found == nullptr) {
        cout << "Question 100 not found\n";
    }

    cout << "Number of questions: "
     << countQuestions(head) << '\n';

    cout << "\nDelete question 2:\n";

    if (deleteQuestion(head, 2)) {
        cout << "Deleted successfully\n";
    }
    else {
        cout << "Question not found\n";
    }

    printQuestions(head);

    cout << "\nDelete question 1:\n";

    if (deleteQuestion(head, 1)) {
        cout << "Deleted successfully\n";
    }
    else {
        cout << "Question not found\n";
    }

    printQuestions(head);

    cout << "\nDelete question 100:\n";

    if (!deleteQuestion(head, 100)) {
        cout << "Question 100 not found\n";
    }

    cout << "Number of questions after delete: "
     << countQuestions(head) << '\n';

    while (head != nullptr) {
        Question* temp = head;
        head = head->next;
        delete temp;
    }

    return 0;
}

