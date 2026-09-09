#ifndef STRING_UTILS_H
#define STRING_UTILS_H

int stringLength(const char str[]);

void stringCopy(
    char destination[],
    const char source[]
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