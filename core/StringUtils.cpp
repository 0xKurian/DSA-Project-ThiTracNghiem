#include "StringUtils.h"

bool isValidIdentifier(const char id[], int capacity) {
    if (id == nullptr || capacity <= 1 || id[0] == '\0') {
        return false;
    }
    for (int i = 0; i < capacity; ++i) {
        if (id[i] == '\0') return true;
    }
    return false;
}

int stringLength(const char str[]) {
    int length = 0;

    while (str[length] != '\0') {
        ++length;
    }

    return length;
}

void stringCopy(
    char destination[],
    const char source[],
    int destinationSize
) {
    if (destinationSize <= 0) {
        return;
    }

    int i = 0;

    while (
        source[i] != '\0' &&
        i < destinationSize - 1
    ) {
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

