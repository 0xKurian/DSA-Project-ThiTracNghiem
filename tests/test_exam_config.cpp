#include "ClassArray.h"
#include "ExamConfigList.h"
#include "QuestionList.h"
#include "ScoreList.h"
#include "StudentList.h"
#include "SubjectBST.h"
#include "ExamConfigService.h"
#include "FileStorage.h"
#include "SubjectService.h"
#include "BusinessQueries.h"

#include <cassert>
#include <cstring>
#include <fstream>
#include <string>

static void addQuestions(Subject* subject, int first, int count) {
    for (int i = 0; i < count; ++i) {
        Question* question = createQuestion(first + i, "Question", "A", "B",
                                            "C", "D", 'A');
        assert(question != nullptr);
        insertQuestion(subject->questionList, question);
    }
}

static void cleanup(Subject*& root, Class* classes[], int& classCount) {
    while (classCount > 0) assert(deleteClass(classes, classCount, classes[0]->id));
    while (root != nullptr) root = deleteSubject(root, root->id);
}

int main() {
    Subject* root = createSubject("CTDL", "Structures");
    addQuestions(root, 100, 10);
    Class* classes[MAX_CLASS] = {};
    int classCount = 0;
    Class* first = createClass("C1", "Class 1");
    Class* second = createClass("C2", "Class 2");
    assert(addClass(classes, classCount, first));
    assert(addClass(classes, classCount, second));
    Student* a = createStudent("SV1", "A", "One", "Nam", "pw");
    Student* b = createStudent("SV2", "B", "Two", "Nam", "pw");
    assert(addStudent(first, a));
    assert(addStudent(first, b));
    assert(countStudentsInClass(first) == 2);
    assert(isPassingScore(5.0f) && !isPassingScore(4.99f));
    Subject* ordered[1] = {};
    assert(collectSubjectsInOrder(root, ordered, 1) == 1 && ordered[0] == root);
    assert(findStudentClass(classes, classCount, " sv1 ") == first);

    assert(setClassExamConfigWorkflow(root, classes, classCount, " c1 ",
                                      " ctdl ", 3, 15) == SUCCESS);
    ExamConfig* cfg = findExamConfig(first, "CTDL");
    assert(cfg != nullptr && cfg->questionCount == 3 && cfg->minutes == 15);
    int original[3] = {cfg->questionIds[0], cfg->questionIds[1], cfg->questionIds[2]};
    assert(setClassExamConfigWorkflow(root, classes, classCount, "C1", "CTDL",
                                      3, 20) == SUCCESS);
    assert(cfg->minutes == 20 && cfg->questionIds[0] == original[0] &&
           cfg->questionIds[1] == original[1] && cfg->questionIds[2] == original[2]);
    assert(setClassExamConfigWorkflow(root, classes, classCount, "C1", "CTDL",
                                      4, 20) == SUCCESS);
    assert(cfg->questionCount == 4 && cfg->questionIds != nullptr);
    assert(setClassExamConfigWorkflow(root, classes, classCount, "C1", "CTDL",
                                      3, 20) == SUCCESS);
    assert(setClassExamConfigWorkflow(root, classes, classCount, "C1", "CTDL",
                                      11, 20) == ERR_INVALID_FORMAT);
    assert(setClassExamConfigWorkflow(root, classes, classCount, "C1", "CTDL",
                                      3, 181) == ERR_INVALID_FORMAT);
    assert(cfg->questionCount == 3 && cfg->minutes == 20);
    assert(setClassExamConfigWorkflow(root, classes, classCount, "C2", "CTDL",
                                      2, 10) == SUCCESS);
    const char* reason = nullptr;
    assert(canStudentTakeSubject(root, classes, classCount, a, "CTDL",
                                 &reason) == SUCCESS && reason == nullptr);

    ExamDetail* examA = nullptr;
    ExamDetail* examB = nullptr;
    assert(prepareExamFromConfig(root, classes, classCount, a, "CTDL", examA) == SUCCESS);
    assert(prepareExamFromConfig(root, classes, classCount, b, "CTDL", examB) == SUCCESS);
    ExamDetail* qa = examA;
    ExamDetail* qb = examB;
    for (int i = 0; i < 3; ++i) {
        assert(qa != nullptr && qb != nullptr);
        assert(qa->questionId == cfg->questionIds[i]);
        assert(qb->questionId == cfg->questionIds[i]);
        qa = qa->next;
        qb = qb->next;
    }
    assert(qa == nullptr && qb == nullptr);
    deleteExamDetailList(examA);
    deleteExamDetailList(examB);

    const char* dir = "build/test_exam_config_data";
    assert(saveAllData(root, classes, classCount, dir));
    Subject* loadedRoot = nullptr;
    Class* loadedClasses[MAX_CLASS] = {};
    int loadedCount = 0;
    assert(loadAllData(loadedRoot, loadedClasses, loadedCount, dir));
    assert(loadedCount == 2);
    ExamConfig* loaded = findExamConfig(loadedClasses[0], "CTDL");
    assert(loaded != nullptr && loaded->questionCount == 3 && loaded->minutes == 20);
    for (int i = 0; i < 3; ++i) assert(loaded->questionIds[i] == cfg->questionIds[i]);
    assert(loadExamConfigsFromFile(loadedClasses, loadedCount,
                                   "build/no_such_exam_config_fixture.txt"));
    assert(findExamConfig(loadedClasses[0], "CTDL") == nullptr);
    const char* malformedFile = "build/test_malformed_exam_configs.txt";
    {
        std::ofstream out(malformedFile);
        out << "6\n";
        out << "bad line\n";
        out << "C1|CTDL|2|10|bad date|100,101\n";
        out << "C1|CTDL|2|10|2026-10-24 10:00|100,100\n";
        out << "C1|CTDL|2|10|2026-10-24 10:00|100,101\n";
        out << "C9|CTDL|1|10|2026-10-24 10:00|100\n";
        out << "C1|CTDL|2|10|2026-10-24 10:00|100,101\n";
    }
    assert(loadExamConfigsFromFile(loadedClasses, loadedCount, malformedFile));
    loaded = findExamConfig(loadedClasses[0], "CTDL");
    assert(loaded != nullptr && loaded->questionCount == 2);
    assert(deleteClass(loadedClasses, loadedCount, "C2"));
    const char* cleanFile = "build/test_clean_exam_configs.txt";
    assert(saveExamConfigsToFile(loadedClasses, loadedCount, cleanFile));
    {
        std::ifstream in(cleanFile);
        std::string header;
        assert(std::getline(in, header) && header == "1");
    }
    cleanup(loadedRoot, loadedClasses, loadedCount);

    assert(addScore(a, "CTDL", 5.0f));
    assert(canStudentTakeSubject(root, classes, classCount, a, "CTDL",
                                 &reason) == ERR_CONSTRAINT_VIOLATION);
    assert(classHasTakenSubject(classes, classCount, "C1", "CTDL"));
    assert(setClassExamConfigWorkflow(root, classes, classCount, "C1", "CTDL",
                                      4, 20) == ERR_CONSTRAINT_VIOLATION);
    assert(deleteClassExamConfigWorkflow(classes, classCount, "C1", "CTDL") ==
           ERR_CONSTRAINT_VIOLATION);
    assert(deleteClassExamConfigWorkflow(classes, classCount, "C2", "CTDL") == SUCCESS);
    assert(findExamConfig(second, "CTDL") == nullptr);
    Subject* otherSubject = createSubject("OLD", "Old subject");
    root = insertSubject(root, otherSubject);
    addQuestions(otherSubject, 200, 2);
    assert(setClassExamConfigWorkflow(root, classes, classCount, "C2", "OLD",
                                      1, 10) == SUCCESS);
    assert(editSubjectWorkflow(root, classes, classCount, "OLD", "NEW",
                               "New subject") == SUCCESS);
    assert(findExamConfig(second, "NEW") != nullptr);
    assert(deleteSubjectWorkflow(root, classes, classCount, "NEW") == SUCCESS);
    assert(findExamConfig(second, "NEW") == nullptr);
    assert(deleteQuestion(root->questionList, cfg->questionIds[0]));
    ExamDetail* missingBankQuestion = nullptr;
    assert(prepareExamFromConfig(root, classes, classCount, b, "CTDL",
                                 missingBankQuestion) == ERR_NOT_FOUND);
    assert(missingBankQuestion == nullptr);
    cleanup(root, classes, classCount);
}
