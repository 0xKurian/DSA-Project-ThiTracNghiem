#include "../core/Structures.h"
#include "../core/SubjectBST.h"
#include "../core/QuestionList.h"
#include "../business/SubjectService.h"
#include "../business/FileStorage.h"
#include <cassert>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>

using namespace std;

static void cleanupTree(Subject*& root) {
    while (root != nullptr) {
        root = deleteSubject(root, root->id);
    }
}

int main() {
    cout << "=== Running test_subject_exam_config ===\n";

    Subject* s1 = createSubject("MATH", "Toan hoc");
    assert(s1 != nullptr);
    assert(s1->examQuestionCount == 0);
    assert(s1->examMinutes == 0);

    Subject* root = nullptr;
    root = insertSubject(root, s1);

    for (int i = 1; i <= 20; ++i) {
        Question* q = createQuestion(
            i,
            "Question content",
            "A", "B", "C", "D",
            'A'
        );
        assert(q != nullptr);
        insertQuestion(s1->questionList, q);
    }
    assert(countQuestions(s1->questionList) == 20);

    BusinessResult r = setExamConfigWorkflow(root, "MATH", 10, 15);
    assert(r == SUCCESS);
    assert(s1->examQuestionCount == 10);
    assert(s1->examMinutes == 15);

    r = setExamConfigWorkflow(root, "MATH", 0, 15);
    assert(r == ERR_CONSTRAINT_VIOLATION);
    assert(s1->examQuestionCount == 10);
    assert(s1->examMinutes == 15);

    r = setExamConfigWorkflow(root, "MATH", -5, 15);
    assert(r == ERR_CONSTRAINT_VIOLATION);
    assert(s1->examQuestionCount == 10);
    assert(s1->examMinutes == 15);

    r = setExamConfigWorkflow(root, "MATH", 21, 15);
    assert(r == ERR_CONSTRAINT_VIOLATION);
    assert(s1->examQuestionCount == 10);
    assert(s1->examMinutes == 15);

    r = setExamConfigWorkflow(root, "MATH", 10, 0);
    assert(r == ERR_CONSTRAINT_VIOLATION);
    assert(s1->examQuestionCount == 10);
    assert(s1->examMinutes == 15);

    r = setExamConfigWorkflow(root, "MATH", 10, 181);
    assert(r == ERR_CONSTRAINT_VIOLATION);
    assert(s1->examQuestionCount == 10);
    assert(s1->examMinutes == 15);

    r = setExamConfigWorkflow(root, "NONEXIST", 10, 15);
    assert(r == ERR_NOT_FOUND);

    r = setExamConfigWorkflow(root, nullptr, 10, 15);
    assert(r == ERR_NOT_FOUND);

    r = setExamConfigWorkflow(root, "", 10, 15);
    assert(r == ERR_NOT_FOUND);

    r = setExamConfigWorkflow(root, "   ", 10, 15);
    assert(r == ERR_NOT_FOUND);

    Subject* nullRoot = nullptr;
    r = setExamConfigWorkflow(nullRoot, "MATH", 10, 15);
    assert(r == ERR_NOT_FOUND);

    const char* temp5ColPath = "test_fixture_5col.txt";
    bool saved = saveSubjectsToFile(root, temp5ColPath);
    assert(saved);

    {
        ifstream checkFile(temp5ColPath);
        assert(checkFile.is_open());
        string totalLine, subHeaderLine;
        getline(checkFile, totalLine);
        getline(checkFile, subHeaderLine);
        assert(subHeaderLine == "MATH|Toan hoc|20|10|15");
    }

    Subject* loadedRoot5 = nullptr;
    bool loaded = loadSubjectsFromFile(loadedRoot5, temp5ColPath);
    assert(loaded);
    assert(loadedRoot5 != nullptr);
    Subject* loadedMath = findSubject(loadedRoot5, "MATH");
    assert(loadedMath != nullptr);
    assert(loadedMath->examQuestionCount == 10);
    assert(loadedMath->examMinutes == 15);
    assert(countQuestions(loadedMath->questionList) == 20);

    cleanupTree(loadedRoot5);
    remove(temp5ColPath);

    const char* temp3ColPath = "test_fixture_3col.txt";
    {
        ofstream out(temp3ColPath);
        out << "1\n";
        out << "PHYS|Vat ly|2\n";
        out << "101|Question 1|A|B|C|D|A\n";
        out << "102|Question 2|A|B|C|D|B\n";
        out.close();
    }

    Subject* loadedRoot3 = nullptr;
    loaded = loadSubjectsFromFile(loadedRoot3, temp3ColPath);
    assert(loaded);
    assert(loadedRoot3 != nullptr);
    Subject* loadedPhys = findSubject(loadedRoot3, "PHYS");
    assert(loadedPhys != nullptr);
    assert(loadedPhys->examQuestionCount == 0);
    assert(loadedPhys->examMinutes == 0);
    assert(countQuestions(loadedPhys->questionList) == 2);

    const char* tempResavedPath = "test_fixture_resaved.txt";
    saved = saveSubjectsToFile(loadedRoot3, tempResavedPath);
    assert(saved);

    {
        ifstream checkResaved(tempResavedPath);
        assert(checkResaved.is_open());
        string totalLine, subHeaderLine;
        getline(checkResaved, totalLine);
        getline(checkResaved, subHeaderLine);
        assert(subHeaderLine == "PHYS|Vat ly|2|0|0");
    }

    Subject* reloadedRoot = nullptr;
    loaded = loadSubjectsFromFile(reloadedRoot, tempResavedPath);
    assert(loaded);
    Subject* reloadedPhys = findSubject(reloadedRoot, "PHYS");
    assert(reloadedPhys != nullptr);
    assert(reloadedPhys->examQuestionCount == 0);
    assert(reloadedPhys->examMinutes == 0);
    assert(countQuestions(reloadedPhys->questionList) == 2);

    cleanupTree(loadedRoot3);
    cleanupTree(reloadedRoot);
    cleanupTree(root);
    remove(temp3ColPath);
    remove(tempResavedPath);

    cout << "PASS test_subject_exam_config\n";
    return 0;
}
