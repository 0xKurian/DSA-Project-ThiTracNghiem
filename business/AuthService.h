#ifndef AUTH_SERVICE_H
#define AUTH_SERVICE_H

#include "../core/Structures.h"
#include "BusinessErrors.h"

LoginRole login(
    Class* classes[],
    int classCount,
    const char username[],
    const char password[],
    Student*& loggedStudent
);

BusinessResult loginWorkflow(
    Class* classes[],
    int classCount,
    const char username[],
    const char password[],
    LoginRole& role,
    Student*& loggedStudent
);

#endif
