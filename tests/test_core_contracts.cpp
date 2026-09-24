#include <iostream>
#include <cstdlib>
#include "../core/ClassArray.h"
#include "../core/SubjectBST.h"
#include "../core/StudentList.h"
#include "../core/QuestionList.h"
#include "../core/ScoreList.h"
#include "../core/StringUtils.h"

int failures = 0;
void check(bool ok, const char* name) {
    std::cout << (ok ? "PASS " : "FAIL ") << name << '\n';
    if (!ok) ++failures;
}
int main(int argc, char** argv) {
    if (argc > 1) {
        Subject* a = createSubject(nullptr, "Name");
        Class* b = createClass(nullptr, "Name");
        Student* c = createStudent(nullptr, "Ho", "Ten", "Nam", "pw");
        Score* d = createScore(nullptr, 8);
        check(!a && !b && !c && !d, "null identifiers rejected");
        delete a; delete b; delete c; delete d;
        return failures ? 1 : 0;
    }
    (void)argv;
    Subject* a = createSubject("1234567890123456", "Name");
    Class* b = createClass("12345678901234567890", "Name");
    Student* c = createStudent("12345678901234567890", "Ho", "Ten", "Nam", "pw");
    Score* d = createScore("1234567890123456", 8);
    check(!a, "subject rejects overlong ID");
    check(!b, "class rejects overlong ID");
    check(!c, "student rejects overlong ID");
    check(!d, "score rejects overlong ID");
    delete a; delete b; delete c; delete d;
    a = createSubject("", "Name"); b = createClass("", "Name");
    c = createStudent("", "Ho", "Ten", "Nam", "pw"); d = createScore("", 8);
    check(!a && !b && !c && !d, "empty IDs rejected");
    delete a; delete b; delete c; delete d;
    a = createSubject("123456789012345", "Name");
    b = createClass("1234567890123456789", "Name");
    c = createStudent("1234567890123456789", "Ho", "Ten", "Nam", "pw");
    d = createScore("123456789012345", 8);
    check(a && b && c && d, "maximum valid ID lengths accepted");
    check(!editSubject(a, "123456789012345", "", "Name"), "rename rejects empty ID");
    check(!addScore(c, "", 8), "addScore rejects empty ID");
    if (findScore(c, "")) deleteScore(c, "");
    delete a; delete b; delete c; delete d;

    Class* classes[MAX_CLASS] = {};
    int count = 0;
    Class* owned = createClass("C1", "Class");
    Class* outsider = createClass("C2", "Outside");
    Student* st = createStudent("S1", "Ho", "Ten", "Nam", "pw");
    check(addClass(classes, count, owned), "add class");
    bool outsideAdded = addStudentGlobal(classes, count, outsider, st);
    check(!outsideAdded && outsider->studentList == nullptr, "global add rejects unregistered class without mutation");
    if (outsideAdded) outsider->studentList = nullptr;
    check(addStudentGlobal(classes, count, owned, st), "global add accepts registered class");
    delete outsider;
    deleteClass(classes, count, "C1");

    Question* q = createQuestion(1, "Question", "correct", "b", "c", "d", 'A');
    ExamDetail* detail = createExamDetail(q);
    detail->selectedAnswer = 'A';
    char saved[ANSWER_CONTENT_LEN]; stringCopy(saved, detail->answerA, ANSWER_CONTENT_LEN);
    char correct = detail->correctAnswer;
    std::srand(123);
    bool unchanged = true;
    for (int i=0;i<100;++i) {
        shuffleExamDetailAnswers(detail);
        unchanged = unchanged && stringEqual(saved, detail->answerA) && correct == detail->correctAnswer;
    }
    check(unchanged, "answered snapshot cannot be reshuffled");
    deleteExamDetailList(detail);
    bool mapping = true;
    for (int i=0;i<1000;++i) {
        detail = createExamDetail(q);
        const char* answers[] = {detail->answerA,detail->answerB,detail->answerC,detail->answerD};
        mapping = mapping && stringEqual(answers[detail->correctAnswer-'A'], "correct") && q->correctAnswer=='A' && stringEqual(q->answerA,"correct");
        deleteExamDetailList(detail);
    }
    check(mapping, "shuffle preserves correct answer and original question");
    delete q;
    return failures ? 1 : 0;
}
