#ifndef SUBJECT_BST_H
#define SUBJECT_BST_H

// Contract: createSubject returns nullptr for null/empty/overlong IDs.
// Other text inputs must be valid null-terminated strings.
// insertSubject takes ownership of a fresh, detached node, including on duplicate
// rejection (the rejected node is deleted). Always assign the returned root.
// editSubject/deleteSubject only update the tree; callers handle Score references.
// Re-find subjects after mutation instead of retaining pointers to changed nodes.

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
