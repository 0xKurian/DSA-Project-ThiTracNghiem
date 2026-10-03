#include "ClassService.h"
#include "Validation.h"
#include "../core/ClassArray.h"
#include "../core/StringUtils.h"

BusinessResult addClassWorkflow(
    Class* classes[],
    int& classCount,
    const char id[],
    const char name[]
) {
    if (classCount >= MAX_CLASS) {
        return ERR_LIST_FULL;
    }

    BusinessResult rId = validateClassId(id);
    if (rId != SUCCESS) return rId;

    BusinessResult rName = validateClassName(name);
    if (rName != SUCCESS) return rName;

    char cleanId[CLASS_ID_LEN];
    stringCopy(cleanId, id, CLASS_ID_LEN);
    trimString(cleanId);
    toUpperString(cleanId);

    char cleanName[CLASS_NAME_LEN];
    stringCopy(cleanName, name, CLASS_NAME_LEN);
    trimString(cleanName);

    if (findClass(classes, classCount, cleanId) != nullptr) {
        return ERR_DUPLICATE;
    }

    Class* newClass = createClass(cleanId, cleanName);
    if (newClass == nullptr) {
        return ERR_UNKNOWN;
    }

    if (!addClass(classes, classCount, newClass)) {
        delete newClass;
        return ERR_LIST_FULL;
    }

    return SUCCESS;
}

BusinessResult deleteClassWorkflow(
    Class* classes[],
    int& classCount,
    const char id[]
) {
    if (id == nullptr) {
        return ERR_NOT_FOUND;
    }

    char cleanId[CLASS_ID_LEN];
    stringCopy(cleanId, id, CLASS_ID_LEN);
    trimString(cleanId);
    toUpperString(cleanId);

    Class* classroom = findClass(classes, classCount, cleanId);
    if (classroom == nullptr) {
        return ERR_NOT_FOUND;
    }

    // Khong duoc xoa lop neu dang co sinh vien
    if (classroom->studentList != nullptr) {
        return ERR_CONSTRAINT_VIOLATION;
    }

    bool deleted = deleteClass(classes, classCount, cleanId);
    return deleted ? SUCCESS : ERR_UNKNOWN;
}
