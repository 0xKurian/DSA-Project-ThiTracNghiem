#include "StringUtils.h"

int stringLength(const char str[]) {
    int length = 0;

    while (str[length] != '\0') {
        ++length;
    }

    return length;
}

void stringCopy(
    char destination[],
    const char source[]
) {
    int i = 0;

    while (source[i] != '\0') {
        destination[i] = source[i];
        ++i;
    }

    destination[i] = '\0';
}

int stringCompare(
    const char a[],
    const char b[]
) {
    int i = 0;

    while (
        a[i] != '\0' &&
        b[i] != '\0' &&
        a[i] == b[i]
    ) {
        ++i;
    }

    return a[i] - b[i];
}

bool stringEqual(
    const char a[],
    const char b[]
) {
    return stringCompare(a, b) == 0;
}