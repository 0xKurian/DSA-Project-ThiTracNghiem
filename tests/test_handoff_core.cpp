#include "ClassArray.h"
#include "ExamConfigList.h"
#include "SubjectBST.h"

#include <cassert>
#include <cstring>
#include <iostream>

using namespace std;

void test_c01_exam_config_crud() {
    int qids1[3] = {101, 102, 103};
    ExamConfig* cfg1 = createExamConfig("CTDL", 45, "2026-10-24 10:00", qids1, 3);
    assert(cfg1 != nullptr);
    assert(strcmp(cfg1->subjectId, "CTDL") == 0);
    assert(cfg1->minutes == 45);
    assert(cfg1->questionCount == 3);
    assert(strcmp(cfg1->createdAt, "2026-10-24 10:00") == 0);
    assert(cfg1->questionIds[0] == 101 && cfg1->questionIds[1] == 102 && cfg1->questionIds[2] == 103);

    assert(createExamConfig("", 45, "2026", qids1, 3) == nullptr);
    assert(createExamConfig("CTDL", 0, "2026", qids1, 3) == nullptr);
    assert(createExamConfig("CTDL", -5, "2026", qids1, 3) == nullptr);
    assert(createExamConfig("CTDL", 45, "2026", qids1, 0) == nullptr);
    assert(createExamConfig("CTDL", 45, "2026", qids1, -1) == nullptr);
    assert(createExamConfig("CTDL", 45, "2026", nullptr, 3) == nullptr);
    assert(createExamConfig("WAYTOOLONGFORSUBJECTIDLIMITEXCEEDED", 45, "2026", qids1, 3) == nullptr);

    Class* cls = createClass("D23CQCN01", "CNTT 1");
    assert(cls != nullptr);
    assert(cls->configList == nullptr);

    assert(addExamConfig(cls, cfg1) == true);
    assert(cls->configList == cfg1);

    int qids_dup[2] = {201, 202};
    ExamConfig* cfg_dup = createExamConfig("CTDL", 30, "2026-10-24 11:00", qids_dup, 2);
    assert(addExamConfig(cls, cfg_dup) == false);
    ExamConfig* dupHead = cfg_dup;
    deleteExamConfigList(dupHead);

    int qids2[2] = {501, 502};
    ExamConfig* cfg2 = createExamConfig("CSDL", 30, "2026-10-24 12:00", qids2, 2);
    assert(addExamConfig(cls, cfg2) == true);

    ExamConfig* found = findExamConfig(cls, "CTDL");
    assert(found != nullptr);
    assert(found->questionCount == 3);

    found = findExamConfig(cls, "CSDL");
    assert(found != nullptr);
    assert(found->questionCount == 2);

    assert(findExamConfig(cls, "NONEXISTENT") == nullptr);
    assert(findExamConfig(nullptr, "CTDL") == nullptr);

    assert(deleteExamConfig(cls, "CTDL") == true);
    assert(findExamConfig(cls, "CTDL") == nullptr);
    assert(findExamConfig(cls, "CSDL") != nullptr);
    assert(deleteExamConfig(cls, "NONEXISTENT") == false);

    Class* classes[10] = { cls };
    int classCount = 1;
    assert(deleteClass(classes, classCount, "D23CQCN01") == true);
    assert(classCount == 0);

    cout << "PASS C01: ExamConfig CRUD, duplicate rejection, and deleteClass cascade\n";
}

void test_c02_delete_subject_preserves_successor_config() {
    Subject* root = nullptr;

    Subject* subB = createSubject("B", "Subject B");
    subB->examQuestionCount = 7;
    subB->examMinutes = 33;
    root = insertSubject(root, subB);

    Subject* subA = createSubject("A", "Subject A");
    subA->examQuestionCount = 5;
    subA->examMinutes = 20;
    root = insertSubject(root, subA);

    Subject* subC = createSubject("C", "Subject C");
    subC->examQuestionCount = 9;
    subC->examMinutes = 44;
    root = insertSubject(root, subC);

    root = deleteSubject(root, "B");
    assert(root != nullptr);
    assert(strcmp(root->id, "C") == 0);
    assert(root->examQuestionCount == 9);
    assert(root->examMinutes == 44);

    while (root != nullptr) {
        root = deleteSubject(root, root->id);
    }

    cout << "PASS C02: deleteSubject preserves successor config (7/33 -> 9/44)\n";
}

void test_c03_edit_subject_preserves_config() {
    Subject* root = nullptr;
    Subject* sub = createSubject("OLD_CODE", "Ten Mon");
    sub->examQuestionCount = 25;
    sub->examMinutes = 40;
    root = insertSubject(root, sub);

    bool ok = editSubject(root, "OLD_CODE", "NEW_CODE", "Ten Mon Moi");
    assert(ok == true);

    Subject* updated = findSubject(root, "NEW_CODE");
    assert(updated != nullptr);
    assert(strcmp(updated->name, "Ten Mon Moi") == 0);
    assert(updated->examQuestionCount == 25);
    assert(updated->examMinutes == 40);

    while (root != nullptr) {
        root = deleteSubject(root, root->id);
    }

    cout << "PASS C03: editSubject preserves config when changing subjectId\n";
}

int main() {
    test_c01_exam_config_crud();
    test_c02_delete_subject_preserves_successor_config();
    test_c03_edit_subject_preserves_config();
    cout << "ALL HANDOFF_CORE ACCEPTANCE TESTS PASSED!\n";
    return 0;
}
