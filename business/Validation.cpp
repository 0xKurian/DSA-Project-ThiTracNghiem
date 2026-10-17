#include "Validation.h"
#include "../core/Constants.h"
#include "../core/StringUtils.h"

// ==================== STRING SANITIZATION ====================

static bool isSpace(char c) {
  return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

static bool isAllWhitespace(const char str[]) {
  if (str == nullptr)
    return true;
  for (int i = 0; str[i] != '\0'; ++i) {
    if (!isSpace(str[i])) {
      return false;
    }
  }
  return true;
}

void trimString(char str[]) {
  if (str == nullptr)
    return;

  int len = stringLength(str);
  if (len == 0)
    return;

  int start = 0;
  while (start < len && isSpace(str[start])) {
    start++;
  }

  if (start == len) {
    str[0] = '\0';
    return;
  }

  int end = len - 1;
  while (end >= start && isSpace(str[end])) {
    end--;
  }

  int idx = 0;
  for (int i = start; i <= end; ++i) {
    str[idx++] = str[i];
  }
  str[idx] = '\0';
}

void toUpperString(char str[]) {
  if (str == nullptr)
    return;

  for (int i = 0; str[i] != '\0'; ++i) {
    if (str[i] >= 'a' && str[i] <= 'z') {
      str[i] = (char)(str[i] - ('a' - 'A'));
    }
  }
}

bool containsWhitespace(const char str[]) {
  if (str == nullptr)
    return false;

  for (int i = 0; str[i] != '\0'; ++i) {
    if (isSpace(str[i])) {
      return true;
    }
  }
  return false;
}

// ==================== VALIDATION HELPERS ====================

BusinessResult validateSubjectId(const char id[]) {
  if (id == nullptr || isAllWhitespace(id)) {
    return ERR_EMPTY_FIELD;
  }
  if (stringLength(id) >= SUBJECT_ID_LEN) {
    return ERR_BUFFER_OVERFLOW;
  }
  return SUCCESS;
}

BusinessResult validateSubjectName(const char name[]) {
  if (name == nullptr || isAllWhitespace(name)) {
    return ERR_EMPTY_FIELD;
  }
  if (stringLength(name) >= SUBJECT_NAME_LEN) {
    return ERR_BUFFER_OVERFLOW;
  }
  return SUCCESS;
}

BusinessResult validateClassId(const char id[]) {
  if (id == nullptr || isAllWhitespace(id)) {
    return ERR_EMPTY_FIELD;
  }
  if (stringLength(id) >= CLASS_ID_LEN) {
    return ERR_BUFFER_OVERFLOW;
  }
  return SUCCESS;
}

BusinessResult validateClassName(const char name[]) {
  if (name == nullptr || isAllWhitespace(name)) {
    return ERR_EMPTY_FIELD;
  }
  if (stringLength(name) >= CLASS_NAME_LEN) {
    return ERR_BUFFER_OVERFLOW;
  }
  return SUCCESS;
}

BusinessResult validateStudentId(const char id[]) {
  if (id == nullptr || isAllWhitespace(id)) {
    return ERR_EMPTY_FIELD;
  }
  if (stringLength(id) >= STUDENT_ID_LEN) {
    return ERR_BUFFER_OVERFLOW;
  }
  return SUCCESS;
}

BusinessResult validateStudentName(const char ho[], const char ten[]) {
  if (ho == nullptr || isAllWhitespace(ho) || ten == nullptr ||
      isAllWhitespace(ten)) {
    return ERR_EMPTY_FIELD;
  }
  if (stringLength(ho) >= HO_LEN || stringLength(ten) >= TEN_LEN) {
    return ERR_BUFFER_OVERFLOW;
  }
  return SUCCESS;
}

BusinessResult validateGender(char gender[]) {
  if (gender == nullptr || isAllWhitespace(gender)) {
    return ERR_EMPTY_FIELD;
  }
  trimString(gender);
  toUpperString(gender);

  if (stringEqual(gender, "NAM")) {
    stringCopy(gender, "Nam", GENDER_LEN);
    return SUCCESS;
  }
  if (stringEqual(gender, "NU")) {
    stringCopy(gender, "Nu", GENDER_LEN);
    return SUCCESS;
  }
  return ERR_INVALID_FORMAT;
}

BusinessResult validatePassword(const char pass[]) {
  if (pass == nullptr || isAllWhitespace(pass)) {
    return ERR_EMPTY_FIELD;
  }
  if (containsWhitespace(pass)) {
    return ERR_INVALID_FORMAT;
  }
  if (stringLength(pass) >= PASSWORD_LEN) {
    return ERR_BUFFER_OVERFLOW;
  }
  return SUCCESS;
}

BusinessResult validateQuestionContent(const char content[]) {
  if (content == nullptr || isAllWhitespace(content)) {
    return ERR_EMPTY_FIELD;
  }
  if (stringLength(content) >= QUESTION_CONTENT_LEN) {
    return ERR_BUFFER_OVERFLOW;
  }
  return SUCCESS;
}

BusinessResult validateAnswerContent(const char ans[]) {
  if (ans == nullptr || isAllWhitespace(ans)) {
    return ERR_EMPTY_FIELD;
  }
  if (stringLength(ans) >= ANSWER_CONTENT_LEN) {
    return ERR_BUFFER_OVERFLOW;
  }
  return SUCCESS;
}

BusinessResult validateCorrectAnswer(char &correct) {
  if (correct >= 'a' && correct <= 'd') {
    correct = (char)(correct - ('a' - 'A'));
  }
  if (correct == 'A' || correct == 'B' || correct == 'C' || correct == 'D') {
    return SUCCESS;
  }
  return ERR_INVALID_FORMAT;
}
