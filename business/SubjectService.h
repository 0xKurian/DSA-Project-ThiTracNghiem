#ifndef SUBJECT_SERVICE_H
#define SUBJECT_SERVICE_H

#include "../core/Structures.h"
#include "BusinessErrors.h"

bool isSubjectUsedInScores(
    Class* classes[],
    int classCount,
    const char subjectId[]
);

BusinessResult addSubjectWorkflow(
    Subject*& root,
    const char id[],
    const char name[]
);

BusinessResult editSubjectWorkflow(
    Subject*& root,
    Class* classes[],
    int classCount,
    const char oldId[],
    const char newId[],
    const char newName[]
);

BusinessResult deleteSubjectWorkflow(
    Subject*& root,
    Class* classes[],
    int classCount,
    const char id[]
);

#endif
