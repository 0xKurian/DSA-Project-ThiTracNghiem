#ifndef STRING_UTILS_H
#define STRING_UTILS_H

bool isValidIdentifier(const char id[], int capacity);

int stringLength(const char str[]);

void stringCopy(
    char destination[],
    const char source[],
    int destinationSize
);

int stringCompare(
    const char a[],
    const char b[]
);

bool stringEqual(
    const char a[],
    const char b[]
);

#endif
