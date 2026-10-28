#include "ClassArray.h"
#include "QuestionList.h"
#include "ScoreList.h"
#include "StudentList.h"
#include "SubjectBST.h"
#include "ExamService.h"

#include <cassert>
#include <cstring>
#include <fstream>

int main() {
    ExamTimer timer;
    assert(timer.isExpired());
    assert(timer.getRemainingSeconds() == 0);
    timer.start(0);
    assert(timer.isExpired());
    timer.resume(0);
    assert(timer.isExpired());
    timer.start(10);
    assert(!timer.isExpired());
    char clock[16] = {};
    formatExamClock(0, clock, sizeof(clock));
    assert(std::strcmp(clock, "00:00") == 0);
    formatExamClock(65, clock, sizeof(clock));
    assert(std::strcmp(clock, "01:05") == 0);

    Subject* root = createSubject("CTDL", "Structures");
    Question* question = createQuestion(1, "Q", "A", "B", "C", "D", 'A');
    insertQuestion(root->questionList, question);
    Class* classes[MAX_CLASS] = {};
    int classCount = 0;
    Class* classroom = createClass("C1", "Class 1");
    assert(addClass(classes, classCount, classroom));
    Student* student = createStudent("SV1", "A", "One", "Nam", "pw");
    assert(addStudent(classroom, student));

    ExamDetail* head = createExamDetail(question);
    assert(head != nullptr);
    for (char answer = 'A'; answer <= 'D'; ++answer) {
        assert(recordSelectedAnswer(head, answer) == SUCCESS);
        assert(head->selectedAnswer == answer);
    }
    assert(recordSelectedAnswer(head, 'E') == ERR_INVALID_FORMAT);
    assert(head->selectedAnswer == 'D');
    assert(recordSelectedAnswer(head, 'a') == ERR_INVALID_FORMAT);
    assert(head->selectedAnswer == 'D');
    assert(recordSelectedAnswer(nullptr, 'A') == ERR_NOT_FOUND);
    assert(!saveExamSession("../SV1", "CTDL", 5, head));
    assert(!checkExamSession("../SV1"));
    ExamSessionTracker tracker;
    const char* sessionFile = "build/test_timer_session.dat";
    ExamTickResult tick = tickExamSession(timer, tracker, student->id, "CTDL",
                                          head, sessionFile);
    assert(!tick.expired && tick.saved);
    tick = tickExamSession(timer, tracker, student->id, "CTDL", head,
                           sessionFile);
    assert(!tick.expired && !tick.saved);
    assert(checkExamSession(student->id, sessionFile));

    ExamDetail* resumed = nullptr;
    char subject[SUBJECT_ID_LEN] = {};
    int remaining = 0;
    assert(resumeExamWorkflow(root, classes, classCount, student, resumed,
                              subject, remaining, sessionFile) == SUCCESS);
    assert(std::strcmp(subject, "CTDL") == 0 && resumed != nullptr);
    assert(resumed->selectedAnswer == 'D');
    cancelExam(resumed);

    float score = 0.0f;
    int correct = 0;
    int total = 0;
    bool saveFailed = true;
    assert(finishExamWorkflow(root, classes, classCount, student, "CTDL",
                              head, score, correct, total, &saveFailed,
                              "build/test_finish_data", sessionFile) == SUCCESS);
    assert(head == nullptr && !saveFailed && total == 1);
    assert(findScore(student, "CTDL") != nullptr);
    assert(findScore(student, "CTDL")->detailList->selectedAnswer == 'D');
    assert(!checkExamSession(student->id, sessionFile));

    Student* other = createStudent("SV2", "B", "Two", "Nam", "pw");
    assert(addStudent(classroom, other));
    ExamDetail* pending = createExamDetail(question);
    assert(saveExamSession(other->id, "CTDL", 5, pending, sessionFile));
    cancelExam(pending);
    assert(cancelPendingExamWorkflow(root, classes, classCount, other,
                                     &saveFailed, "build/test_finish_data",
                                     sessionFile) == SUCCESS);
    assert(!saveFailed && findScore(other, "CTDL") != nullptr);
    assert(!checkExamSession(other->id, sessionFile));

    Student* saveFailureStudent = createStudent("SV3", "C", "Three", "Nam", "pw");
    assert(addStudent(classroom, saveFailureStudent));
    ExamDetail* saveFailureHead = createExamDetail(question);
    assert(saveExamSession(saveFailureStudent->id, "CTDL", 5,
                           saveFailureHead, sessionFile));
    assert(finishExamWorkflow(root, classes, classCount, saveFailureStudent,
                              "CTDL", saveFailureHead, score, correct, total,
                              &saveFailed, "build/test_finish_data/subjects.txt",
                              sessionFile) == SUCCESS);
    assert(saveFailed && saveFailureHead == nullptr);
    assert(findScore(saveFailureStudent, "CTDL") != nullptr);
    assert(checkExamSession(saveFailureStudent->id, sessionFile));
    assert(removeExamSession(saveFailureStudent->id, sessionFile));

    Student* brokenSessionStudent = createStudent("SV4", "D", "Four", "Nam", "pw");
    assert(addStudent(classroom, brokenSessionStudent));
    {
        std::ofstream broken(sessionFile);
        broken << "broken session\n";
    }
    assert(checkExamSession(brokenSessionStudent->id, sessionFile));
    saveFailed = true;
    assert(cancelPendingExamWorkflow(root, classes, classCount,
                                     brokenSessionStudent, &saveFailed,
                                     "build/test_finish_data", sessionFile) == ERR_NOT_FOUND);
    assert(!saveFailed);
    assert(!checkExamSession(brokenSessionStudent->id, sessionFile));
    assert(cancelPendingExamWorkflow(root, classes, classCount,
                                     brokenSessionStudent, &saveFailed,
                                     "build/test_finish_data", sessionFile) == ERR_NOT_FOUND);

    timer.stop();
    assert(timer.isExpired());
    assert(timer.getRemainingSeconds() == 0);
    while (classCount > 0) assert(deleteClass(classes, classCount, classes[0]->id));
    root = deleteSubject(root, "CTDL");
}
