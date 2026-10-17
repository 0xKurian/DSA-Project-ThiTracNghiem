#ifndef CLASS_SERVICE_H
#define CLASS_SERVICE_H

#include "../core/Structures.h"
#include "BusinessErrors.h"

BusinessResult addClassWorkflow(
    Class* classes[],
    int& classCount,
    const char id[],
    const char name[]
);

BusinessResult deleteClassWorkflow(
    Class* classes[],
    int& classCount,
    const char id[]
);

#endif
