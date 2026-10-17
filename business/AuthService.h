#ifndef AUTH_SERVICE_H
#define AUTH_SERVICE_H

#include "../core/Structures.h"
#include "BusinessErrors.h"

// Ham dang nhap chuan theo FunctionPrototype
LoginRole login(
    Class* classes[],
    int classCount,
    const char username[],
    const char password[],
    Student*& loggedStudent
);

// Workflow dang nhap mo rong tra ve ma loi nghiep vu chi tiet cho UI
BusinessResult loginWorkflow(
    Class* classes[],
    int classCount,
    const char username[],
    const char password[],
    LoginRole& role,
    Student*& loggedStudent
);

#endif
