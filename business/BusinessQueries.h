#ifndef BUSINESS_QUERIES_H
#define BUSINESS_QUERIES_H

#include "../core/Structures.h"

bool isPassingScore(float score);
int countStudentsInClass(const Class* classroom);
int collectSubjectsInOrder(Subject* root, Subject* out[], int capacity);
void freeAllData(Subject*& root, Class* classes[], int& classCount);

#endif
