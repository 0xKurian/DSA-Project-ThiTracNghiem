#include "SubjectService.h"
#include "Validation.h"
#include "../core/SubjectBST.h"
#include "../core/ScoreList.h"
#include "../core/StringUtils.h"

bool isSubjectUsedInScores(
    Class* classes[],
    int classCount,
    const char subjectId[]
) {
    if (classes == nullptr || subjectId == nullptr) {
        return false;
    }

    char cleanSubId[SUBJECT_ID_LEN];
    stringCopy(cleanSubId, subjectId, SUBJECT_ID_LEN);
    trimString(cleanSubId);
    toUpperString(cleanSubId);

    for (int i = 0; i < classCount; ++i) {
        if (classes[i] == nullptr) continue;

        Student* student = classes[i]->studentList;
        while (student != nullptr) {
            if (findScore(student, cleanSubId) != nullptr) {
                return true;
            }
            student = student->next;
        }
    }
    return false;
}

BusinessResult addSubjectWorkflow(
    Subject*& root,
    const char id[],
    const char name[]
) {
    BusinessResult rId = validateSubjectId(id);
    if (rId != SUCCESS) return rId;

    BusinessResult rName = validateSubjectName(name);
    if (rName != SUCCESS) return rName;

    char cleanId[SUBJECT_ID_LEN];
    stringCopy(cleanId, id, SUBJECT_ID_LEN);
    trimString(cleanId);
    toUpperString(cleanId);

    char cleanName[SUBJECT_NAME_LEN];
    stringCopy(cleanName, name, SUBJECT_NAME_LEN);
    trimString(cleanName);

    if (findSubject(root, cleanId) != nullptr) {
        return ERR_DUPLICATE;
    }

    Subject* newSubject = createSubject(cleanId, cleanName);
    if (newSubject == nullptr) {
        return ERR_UNKNOWN;
    }

    root = insertSubject(root, newSubject);
    return SUCCESS;
}

BusinessResult editSubjectWorkflow(
    Subject*& root,
    Class* classes[],
    int classCount,
    const char oldId[],
    const char newId[],
    const char newName[]
) {
    if (oldId == nullptr) {
        return ERR_NOT_FOUND;
    }

    char cleanOldId[SUBJECT_ID_LEN];
    stringCopy(cleanOldId, oldId, SUBJECT_ID_LEN);
    trimString(cleanOldId);
    toUpperString(cleanOldId);

    if (findSubject(root, cleanOldId) == nullptr) {
        return ERR_NOT_FOUND;
    }

    BusinessResult rId = validateSubjectId(newId);
    if (rId != SUCCESS) return rId;

    BusinessResult rName = validateSubjectName(newName);
    if (rName != SUCCESS) return rName;

    char cleanNewId[SUBJECT_ID_LEN];
    stringCopy(cleanNewId, newId, SUBJECT_ID_LEN);
    trimString(cleanNewId);
    toUpperString(cleanNewId);

    char cleanNewName[SUBJECT_NAME_LEN];
    stringCopy(cleanNewName, newName, SUBJECT_NAME_LEN);
    trimString(cleanNewName);

    // Neu thay doi ma mon, kiem tra rang buoc toan ven va trung lap
    if (!stringEqual(cleanOldId, cleanNewId)) {
        if (isSubjectUsedInScores(classes, classCount, cleanOldId)) {
            return ERR_CONSTRAINT_VIOLATION;
        }

        if (findSubject(root, cleanNewId) != nullptr) {
            return ERR_DUPLICATE;
        }
    }

    bool success = editSubject(root, cleanOldId, cleanNewId, cleanNewName);
    return success ? SUCCESS : ERR_UNKNOWN;
}

BusinessResult deleteSubjectWorkflow(
    Subject*& root,
    Class* classes[],
    int classCount,
    const char id[]
) {
    if (id == nullptr) {
        return ERR_NOT_FOUND;
    }

    char cleanId[SUBJECT_ID_LEN];
    stringCopy(cleanId, id, SUBJECT_ID_LEN);
    trimString(cleanId);
    toUpperString(cleanId);

    if (findSubject(root, cleanId) == nullptr) {
        return ERR_NOT_FOUND;
    }

    // Khong duoc xoa mon hoc khi da phat sinh diem thi trong he thong
    if (isSubjectUsedInScores(classes, classCount, cleanId)) {
        return ERR_CONSTRAINT_VIOLATION;
    }

    root = deleteSubject(root, cleanId);
    return SUCCESS;
}
