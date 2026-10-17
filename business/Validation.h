#ifndef VALIDATION_H
#define VALIDATION_H

#include "BusinessErrors.h"

// ==================== STRING SANITIZATION ====================

void trimString(char str[]);
void toUpperString(char str[]);
bool containsWhitespace(const char str[]);

// ==================== VALIDATION HELPERS ====================

BusinessResult validateSubjectId(const char id[]);
BusinessResult validateSubjectName(const char name[]);

BusinessResult validateClassId(const char id[]);
BusinessResult validateClassName(const char name[]);

BusinessResult validateStudentId(const char id[]);
BusinessResult validateStudentName(const char ho[], const char ten[]);
BusinessResult validateGender(char gender[]);
BusinessResult validatePassword(const char pass[]);

BusinessResult validateQuestionContent(const char content[]);
BusinessResult validateAnswerContent(const char ans[]);
BusinessResult validateCorrectAnswer(char &correct);

#endif
