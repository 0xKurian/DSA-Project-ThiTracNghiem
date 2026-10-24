#include "ClassArray.h"
#include "ExamConfigList.h"
#include "QuestionList.h"
#include "ScoreList.h"
#include "StudentList.h"
#include "SubjectBST.h"
#include "ExamService.h"
#include "FileStorage.h"
#include "QuestionService.h"

#include <cassert>
#include <cstring>

int main() {
    Subject* root = createSubject("MATH", "Math|Plus");
    Question* question = createQuestion(200, "What\\|now?", "A|one", "B",
                                        "C", "D", 'A');
    insertQuestion(root->questionList, question);
    Class* classes[MAX_CLASS] = {};
    int classCount = 0;
    Class* classroom = createClass("C1", "Class");
    assert(addClass(classes, classCount, classroom));
    Student* student = createStudent("SV1", "Smith|Jones", "One", "Nam", "pw");
    assert(addStudent(classroom, student));

    int ids[] = {200};
    ExamConfig* cfg = createExamConfig("MATH", 15, "2026-10-24 10:00", ids, 1);
    assert(addExamConfig(classroom, cfg));
    assert(deleteQuestionWorkflow(root, classes, classCount, "MATH", 200) ==
           ERR_CONSTRAINT_VIOLATION);
    assert(deleteExamConfig(classroom, "MATH"));

    ExamDetail* detail = createExamDetail(question);
    assert(detail != nullptr);
    assert(addScore(student, "MATH", 5.0f, detail));
    assert(deleteQuestionWorkflow(root, classes, classCount, "MATH", 200) ==
           ERR_CONSTRAINT_VIOLATION);

    const char* dir = "build/test_business_pipe_data";
    assert(saveAllData(root, classes, classCount, dir));
    Subject* loadedRoot = nullptr;
    Class* loadedClasses[MAX_CLASS] = {};
    int loadedCount = 0;
    assert(loadAllData(loadedRoot, loadedClasses, loadedCount, dir));
    Subject* loadedSubject = findSubject(loadedRoot, "MATH");
    assert(loadedSubject != nullptr);
    assert(std::strcmp(loadedSubject->name, "Math|Plus") == 0);
    Question* loadedQuestion = findQuestion(loadedSubject->questionList, 200);
    assert(loadedQuestion != nullptr);
    assert(std::strcmp(loadedQuestion->content, "What\\|now?") == 0);
    assert(std::strcmp(loadedQuestion->answerA, "A|one") == 0);
    Student* loadedStudent = loadedClasses[0]->studentList;
    assert(std::strcmp(loadedStudent->ho, "Smith|Jones") == 0);
    Score* loadedScore = findScore(loadedStudent, "MATH");
    assert(loadedScore != nullptr && loadedScore->detailList != nullptr);
    assert(std::strcmp(loadedScore->detailList->content, detail->content) == 0);

    const char* sessionFile = "build/test_business_pipe_session.dat";
    assert(saveExamSession("SV1", "MATH", 20, detail, sessionFile));
    ExamDetail* resumed = nullptr;
    char subjectId[SUBJECT_ID_LEN] = {};
    int remaining = 0;
    assert(loadExamSession("SV1", subjectId, remaining, resumed, loadedRoot,
                           sessionFile));
    assert(remaining == 20);
    assert(std::strcmp(resumed->content, detail->content) == 0);
    assert(std::strcmp(resumed->answerA, detail->answerA) == 0);
    cancelExam(resumed);
    assert(removeExamSession("SV1", sessionFile));

    assert(deleteScore(student, "MATH"));
    assert(deleteQuestionWorkflow(root, classes, classCount, "MATH", 200) == SUCCESS);
    assert(findQuestion(root->questionList, 200) == nullptr);
    while (loadedCount > 0)
        assert(deleteClass(loadedClasses, loadedCount, loadedClasses[0]->id));
    while (classCount > 0)
        assert(deleteClass(classes, classCount, classes[0]->id));
    loadedRoot = deleteSubject(loadedRoot, "MATH");
    root = deleteSubject(root, "MATH");
}
