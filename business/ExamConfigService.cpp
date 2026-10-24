#include "ExamConfigService.h"

#include "../core/ClassArray.h"
#include "../core/ExamConfigList.h"
#include "../core/QuestionList.h"
#include "../core/ScoreList.h"
#include "../core/StringUtils.h"
#include "../core/SubjectBST.h"
#include "ExamService.h"
#include "Validation.h"

#include <cstdio>
#include <ctime>

static void normalizeId(char out[], const char in[], int capacity) {
    stringCopy(out, in, capacity);
    trimString(out);
    toUpperString(out);
}

static void currentDateTime(char out[20]) {
    std::time_t now = std::time(nullptr);
    std::tm local = {};
#if defined(_WIN32)
    localtime_s(&local, &now);
#else
    localtime_r(&now, &local);
#endif
    std::strftime(out, 20, "%Y-%m-%d %H:%M", &local);
}

Class* findStudentClass(Class* classes[], int classCount,
                        const char studentId[]) {
    if (classes == nullptr || studentId == nullptr || classCount < 0) {
        return nullptr;
    }
    char cleanId[STUDENT_ID_LEN];
    if (validateStudentId(studentId) != SUCCESS) return nullptr;
    normalizeId(cleanId, studentId, STUDENT_ID_LEN);
    for (int i = 0; i < classCount; ++i) {
        if (classes[i] == nullptr) continue;
        for (Student* student = classes[i]->studentList; student != nullptr;
             student = student->next) {
            if (stringEqual(student->id, cleanId)) return classes[i];
        }
    }
    return nullptr;
}

bool classHasTakenSubject(Class* classes[], int classCount,
                          const char classId[], const char subjectId[]) {
    if (classes == nullptr || validateClassId(classId) != SUCCESS ||
        validateSubjectId(subjectId) != SUCCESS) return false;
    char cleanClass[CLASS_ID_LEN];
    char cleanSubject[SUBJECT_ID_LEN];
    normalizeId(cleanClass, classId, CLASS_ID_LEN);
    normalizeId(cleanSubject, subjectId, SUBJECT_ID_LEN);
    Class* classroom = findClass(classes, classCount, cleanClass);
    if (classroom == nullptr) return false;
    for (Student* student = classroom->studentList; student != nullptr;
         student = student->next) {
        if (findScore(student, cleanSubject) != nullptr) return true;
    }
    return false;
}

BusinessResult setClassExamConfigWorkflow(
    Subject* root, Class* classes[], int classCount, const char classId[],
    const char subjectId[], int numberOfQuestions, int minutes) {
    if (root == nullptr || classes == nullptr ||
        validateClassId(classId) != SUCCESS ||
        validateSubjectId(subjectId) != SUCCESS) return ERR_NOT_FOUND;

    char cleanClass[CLASS_ID_LEN];
    char cleanSubject[SUBJECT_ID_LEN];
    normalizeId(cleanClass, classId, CLASS_ID_LEN);
    normalizeId(cleanSubject, subjectId, SUBJECT_ID_LEN);
    Class* classroom = findClass(classes, classCount, cleanClass);
    Subject* subject = findSubject(root, cleanSubject);
    if (classroom == nullptr || subject == nullptr) return ERR_NOT_FOUND;
    if (numberOfQuestions < 1 ||
        numberOfQuestions > countQuestions(subject->questionList) ||
        minutes < 1 || minutes > 180) return ERR_INVALID_FORMAT;
    if (classHasTakenSubject(classes, classCount, cleanClass, cleanSubject))
        return ERR_CONSTRAINT_VIOLATION;

    ExamConfig* existing = findExamConfig(classroom, cleanSubject);
    if (existing != nullptr && existing->questionCount == numberOfQuestions) {
        existing->minutes = minutes;
        return SUCCESS;
    }

    int* ids = new int[numberOfQuestions];
    if (drawQuestionIds(subject, numberOfQuestions, ids) != numberOfQuestions) {
        delete[] ids;
        return ERR_INVALID_FORMAT;
    }
    char createdAt[20] = {};
    currentDateTime(createdAt);
    if (existing != nullptr) {
        int* oldIds = existing->questionIds;
        existing->questionIds = ids;
        existing->questionCount = numberOfQuestions;
        existing->minutes = minutes;
        stringCopy(existing->createdAt, createdAt, sizeof(existing->createdAt));
        delete[] oldIds;
        return SUCCESS;
    }

    ExamConfig* config = createExamConfig(cleanSubject, minutes, createdAt,
                                          ids, numberOfQuestions);
    delete[] ids;
    if (config == nullptr) return ERR_UNKNOWN;
    if (!addExamConfig(classroom, config)) {
        deleteExamConfigList(config);
        return ERR_UNKNOWN;
    }
    return SUCCESS;
}

BusinessResult deleteClassExamConfigWorkflow(
    Class* classes[], int classCount, const char classId[],
    const char subjectId[]) {
    if (classes == nullptr || validateClassId(classId) != SUCCESS ||
        validateSubjectId(subjectId) != SUCCESS) return ERR_NOT_FOUND;
    char cleanClass[CLASS_ID_LEN];
    char cleanSubject[SUBJECT_ID_LEN];
    normalizeId(cleanClass, classId, CLASS_ID_LEN);
    normalizeId(cleanSubject, subjectId, SUBJECT_ID_LEN);
    Class* classroom = findClass(classes, classCount, cleanClass);
    if (classroom == nullptr ||
        findExamConfig(classroom, cleanSubject) == nullptr) return ERR_NOT_FOUND;
    if (classHasTakenSubject(classes, classCount, cleanClass, cleanSubject))
        return ERR_CONSTRAINT_VIOLATION;
    return deleteExamConfig(classroom, cleanSubject) ? SUCCESS : ERR_UNKNOWN;
}

BusinessResult canStudentTakeSubject(
    Subject* root, Class* classes[], int classCount, Student* student,
    const char subjectId[], const char** reasonOut) {
    if (reasonOut != nullptr) *reasonOut = nullptr;
    if (student == nullptr || validateSubjectId(subjectId) != SUCCESS) {
        if (reasonOut != nullptr) *reasonOut = "Sinh vien hoac ma mon khong hop le";
        return ERR_NOT_FOUND;
    }
    char cleanSubject[SUBJECT_ID_LEN];
    normalizeId(cleanSubject, subjectId, SUBJECT_ID_LEN);
    Class* classroom = findStudentClass(classes, classCount, student->id);
    if (classroom == nullptr) {
        if (reasonOut != nullptr) *reasonOut = "Khong tim thay lop cua sinh vien";
        return ERR_NOT_FOUND;
    }
    Subject* subject = findSubject(root, cleanSubject);
    ExamConfig* config = findExamConfig(classroom, cleanSubject);
    if (subject == nullptr || config == nullptr) {
        if (reasonOut != nullptr) *reasonOut = "Lop chua co cau hinh thi mon nay";
        return ERR_NOT_FOUND;
    }
    if (findScore(student, cleanSubject) != nullptr) {
        if (reasonOut != nullptr) *reasonOut = "Sinh vien da thi mon nay";
        return ERR_CONSTRAINT_VIOLATION;
    }
    if (config->questionIds == nullptr || config->questionCount < 1 ||
        config->minutes < 1 || config->minutes > 180) {
        if (reasonOut != nullptr) *reasonOut = "Cau hinh thi khong hop le";
        return ERR_INVALID_FORMAT;
    }
    for (int i = 0; i < config->questionCount; ++i) {
        if (findQuestion(subject->questionList, config->questionIds[i]) == nullptr) {
            if (reasonOut != nullptr) *reasonOut = "Cau hoi trong cau hinh da bi xoa";
            return ERR_NOT_FOUND;
        }
    }
    return SUCCESS;
}

BusinessResult prepareExamFromConfig(
    Subject* root, Class* classes[], int classCount, Student* student,
    const char subjectId[], ExamDetail*& examQuestionsHead) {
    if (examQuestionsHead != nullptr) return ERR_CONSTRAINT_VIOLATION;
    BusinessResult result = canStudentTakeSubject(root, classes, classCount,
                                                  student, subjectId, nullptr);
    if (result != SUCCESS) return result;
    char cleanSubject[SUBJECT_ID_LEN];
    normalizeId(cleanSubject, subjectId, SUBJECT_ID_LEN);
    Subject* subject = findSubject(root, cleanSubject);
    Class* classroom = findStudentClass(classes, classCount, student->id);
    ExamConfig* config = findExamConfig(classroom, cleanSubject);
    for (int i = 0; i < config->questionCount; ++i) {
        Question* question = findQuestion(subject->questionList,
                                          config->questionIds[i]);
        if (question == nullptr) {
            cancelExam(examQuestionsHead);
            return ERR_NOT_FOUND;
        }
        ExamDetail* detail = createExamDetail(question);
        if (detail == nullptr) {
            cancelExam(examQuestionsHead);
            return ERR_INVALID_FORMAT;
        }
        addExamDetail(examQuestionsHead, detail);
    }
    return SUCCESS;
}
