#ifndef SUBJECT_BST_H
#define SUBJECT_BST_H

#include "Structures.h"

Subject* createSubject(
    const char id[],
    const char name[]
);

Subject* insertSubject(
    Subject* root,
    Subject* subject
);

Subject* findSubject(
    Subject* root,
    const char id[]
);

Subject* deleteSubject(
    Subject* root,
    const char id[]
);

bool editSubject(
    Subject*& root,
    const char oldId[],
    const char newId[],
    const char newName[]
);

void printSubjects(
    Subject* root
);

#endif