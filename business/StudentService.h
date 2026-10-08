#ifndef STUDENT_SERVICE_H
#define STUDENT_SERVICE_H

#include "../core/Structures.h"
#include "BusinessErrors.h"

BusinessResult addStudentWorkflow(
    Class* classes[],
    int classCount,
    const char classId[],
    const char id[],
    const char ho[],
    const char ten[],
    const char gender[],
    const char password[]
);

BusinessResult editStudentWorkflow(
    Class* classes[],
    int classCount,
    const char studentId[],
    const char ho[],
    const char ten[],
    const char gender[],
    const char password[]
);

BusinessResult deleteStudentWorkflow(
    Class* classes[],
    int classCount,
    const char classId[],
    const char studentId[]
);

#endif
