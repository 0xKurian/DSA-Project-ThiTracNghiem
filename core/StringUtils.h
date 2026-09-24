#ifndef STRING_UTILS_H
#define STRING_UTILS_H

// Text functions below require valid null-terminated input strings and valid
// destinations. stringCopy truncates TEXT to capacity-1 and terminates it when
// capacity > 0; validate identifiers with isValidIdentifier before copying.

// True for a non-null, non-empty ID fitting capacity including the terminator.
// Does not truncate or normalize the ID.
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

