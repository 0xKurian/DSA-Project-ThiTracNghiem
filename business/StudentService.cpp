#include "StudentService.h"
#include "Validation.h"
#include "../core/ClassArray.h"
#include "../core/StudentList.h"
#include "../core/StringUtils.h"

BusinessResult addStudentWorkflow(
    Class* classes[],
    int classCount,
    const char classId[],
    const char id[],
    const char ho[],
    const char ten[],
    const char gender[],
    const char password[]
) {
    if (classes == nullptr || classId == nullptr) {
        return ERR_NOT_FOUND;
    }

    char cleanClassId[CLASS_ID_LEN];
    stringCopy(cleanClassId, classId, CLASS_ID_LEN);
    trimString(cleanClassId);
    toUpperString(cleanClassId);

    Class* classroom = findClass(classes, classCount, cleanClassId);
    if (classroom == nullptr) {
        return ERR_NOT_FOUND;
    }

    BusinessResult rId = validateStudentId(id);
    if (rId != SUCCESS) return rId;

    BusinessResult rName = validateStudentName(ho, ten);
    if (rName != SUCCESS) return rName;

    char cleanGender[GENDER_LEN];
    stringCopy(cleanGender, gender, GENDER_LEN);
    BusinessResult rGender = validateGender(cleanGender);
    if (rGender != SUCCESS) return rGender;

    BusinessResult rPass = validatePassword(password);
    if (rPass != SUCCESS) return rPass;

    char cleanId[STUDENT_ID_LEN];
    stringCopy(cleanId, id, STUDENT_ID_LEN);
    trimString(cleanId);
    toUpperString(cleanId);

    char cleanHo[HO_LEN];
    stringCopy(cleanHo, ho, HO_LEN);
    trimString(cleanHo);

    char cleanTen[TEN_LEN];
    stringCopy(cleanTen, ten, TEN_LEN);
    trimString(cleanTen);

    // Kiem tra trung lap MASV tren TOAN BO he thong (khong chi trong lop)
    if (findStudentGlobal(classes, classCount, cleanId) != nullptr) {
        return ERR_DUPLICATE;
    }

    Student* newStudent = createStudent(
        cleanId,
        cleanHo,
        cleanTen,
        cleanGender,
        password
    );

    if (newStudent == nullptr) {
        return ERR_UNKNOWN;
    }

    if (!addStudentGlobal(classes, classCount, classroom, newStudent)) {
        delete newStudent;
        return ERR_DUPLICATE;
    }

    return SUCCESS;
}

BusinessResult editStudentWorkflow(
    Class* classes[],
    int classCount,
    const char studentId[],
    const char ho[],
    const char ten[],
    const char gender[],
    const char password[]
) {
    if (classes == nullptr || studentId == nullptr) {
        return ERR_NOT_FOUND;
    }

    char cleanStudentId[STUDENT_ID_LEN];
    stringCopy(cleanStudentId, studentId, STUDENT_ID_LEN);
    trimString(cleanStudentId);
    toUpperString(cleanStudentId);

    Student* student = findStudentGlobal(classes, classCount, cleanStudentId);
    if (student == nullptr) {
        return ERR_NOT_FOUND;
    }

    BusinessResult rName = validateStudentName(ho, ten);
    if (rName != SUCCESS) return rName;

    char cleanGender[GENDER_LEN];
    stringCopy(cleanGender, gender, GENDER_LEN);
    BusinessResult rGender = validateGender(cleanGender);
    if (rGender != SUCCESS) return rGender;

    BusinessResult rPass = validatePassword(password);
    if (rPass != SUCCESS) return rPass;

    char cleanHo[HO_LEN];
    stringCopy(cleanHo, ho, HO_LEN);
    trimString(cleanHo);

    char cleanTen[TEN_LEN];
    stringCopy(cleanTen, ten, TEN_LEN);
    trimString(cleanTen);

    bool updated = editStudent(
        student,
        cleanHo,
        cleanTen,
        cleanGender,
        password
    );

    return updated ? SUCCESS : ERR_UNKNOWN;
}

BusinessResult deleteStudentWorkflow(
    Class* classes[],
    int classCount,
    const char classId[],
    const char studentId[]
) {
    if (classes == nullptr || classId == nullptr || studentId == nullptr) {
        return ERR_NOT_FOUND;
    }

    char cleanClassId[CLASS_ID_LEN];
    stringCopy(cleanClassId, classId, CLASS_ID_LEN);
    trimString(cleanClassId);
    toUpperString(cleanClassId);

    Class* classroom = findClass(classes, classCount, cleanClassId);
    if (classroom == nullptr) {
        return ERR_NOT_FOUND;
    }

    char cleanStudentId[STUDENT_ID_LEN];
    stringCopy(cleanStudentId, studentId, STUDENT_ID_LEN);
    trimString(cleanStudentId);
    toUpperString(cleanStudentId);

    Student* student = findStudent(classroom->studentList, cleanStudentId);
    if (student == nullptr) {
        return ERR_NOT_FOUND;
    }

    bool deleted = deleteStudent(classroom, cleanStudentId);
    return deleted ? SUCCESS : ERR_UNKNOWN;
}
