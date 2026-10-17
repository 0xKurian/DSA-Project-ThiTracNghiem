#include "AuthService.h"
#include "Validation.h"
#include "../core/ClassArray.h"
#include "../core/StringUtils.h"

LoginRole login(
    Class* classes[],
    int classCount,
    const char username[],
    const char password[],
    Student*& loggedStudent
) {
    loggedStudent = nullptr;

    if (username == nullptr || password == nullptr) {
        return LOGIN_FAILED;
    }

    if (stringLength(username) == 0 || stringLength(password) == 0) {
        return LOGIN_FAILED;
    }

    if (stringLength(username) >= STUDENT_ID_LEN ||
        stringLength(password) >= PASSWORD_LEN) {
        return LOGIN_FAILED;
    }

    char cleanUsername[STUDENT_ID_LEN];
    stringCopy(cleanUsername, username, STUDENT_ID_LEN);
    trimString(cleanUsername);
    toUpperString(cleanUsername);

    char cleanPassword[PASSWORD_LEN];
    stringCopy(cleanPassword, password, PASSWORD_LEN);
    trimString(cleanPassword);

    // 1. Kiem tra tai khoan Giang vien co dinh: GV / GV
    if (stringEqual(cleanUsername, "GV") && stringEqual(cleanPassword, "GV")) {
        return LOGIN_TEACHER;
    }

    // 2. Kiem tra tai khoan Sinh vien (tim toan cuc theo MASV)
    if (classes != nullptr && classCount > 0) {
        Student* found = findStudentGlobal(classes, classCount, cleanUsername);
        if (found != nullptr) {
            // Mat khau duoc so sanh chinh xac (case-sensitive)
            if (stringEqual(found->password, cleanPassword)) {
                loggedStudent = found;
                return LOGIN_STUDENT;
            }
        }
    }

    return LOGIN_FAILED;
}

BusinessResult loginWorkflow(
    Class* classes[],
    int classCount,
    const char username[],
    const char password[],
    LoginRole& role,
    Student*& loggedStudent
) {
    role = LOGIN_FAILED;
    loggedStudent = nullptr;

    if (username == nullptr || password == nullptr) {
        return ERR_EMPTY_FIELD;
    }

    if (stringLength(username) == 0 || stringLength(password) == 0) {
        return ERR_EMPTY_FIELD;
    }

    if (stringLength(username) >= STUDENT_ID_LEN ||
        stringLength(password) >= PASSWORD_LEN) {
        return ERR_BUFFER_OVERFLOW;
    }

    char cleanUsername[STUDENT_ID_LEN];
    stringCopy(cleanUsername, username, STUDENT_ID_LEN);
    trimString(cleanUsername);

    char cleanPassword[PASSWORD_LEN];
    stringCopy(cleanPassword, password, PASSWORD_LEN);
    trimString(cleanPassword);

    if (stringLength(cleanUsername) == 0 || stringLength(cleanPassword) == 0) {
        return ERR_EMPTY_FIELD;
    }

    toUpperString(cleanUsername);

    // 1. Kiem tra Giang vien
    if (stringEqual(cleanUsername, "GV")) {
        if (stringEqual(cleanPassword, "GV")) {
            role = LOGIN_TEACHER;
            return SUCCESS;
        } else {
            return ERR_INVALID_FORMAT; // Sai mat khau
        }
    }

    // 2. Kiem tra Sinh vien
    if (classes == nullptr || classCount <= 0) {
        return ERR_NOT_FOUND;
    }

    Student* found = findStudentGlobal(classes, classCount, cleanUsername);
    if (found == nullptr) {
        return ERR_NOT_FOUND; // Khong tim thay MASV
    }

    if (!stringEqual(found->password, cleanPassword)) {
        return ERR_INVALID_FORMAT; // Sai mat khau sinh vien
    }

    role = LOGIN_STUDENT;
    loggedStudent = found;
    return SUCCESS;
}
