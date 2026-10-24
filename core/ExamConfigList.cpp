#include "ExamConfigList.h"
#include "StringUtils.h"

ExamConfig* createExamConfig(
    const char subjectId[],
    int minutes,
    const char createdAt[],
    const int questionIds[],
    int questionCount
) {
    if (!isValidIdentifier(subjectId, SUBJECT_ID_LEN)) {
        return nullptr;
    }

    if (minutes <= 0 || questionCount <= 0 || questionIds == nullptr) {
        return nullptr;
    }

    ExamConfig* config = new ExamConfig;

    stringCopy(
        config->subjectId,
        subjectId,
        SUBJECT_ID_LEN
    );

    config->minutes = minutes;
    config->questionCount = questionCount;

    if (createdAt != nullptr) {
        stringCopy(
            config->createdAt,
            createdAt,
            sizeof(config->createdAt)
        );
    } else {
        config->createdAt[0] = '\0';
    }

    config->questionIds = new int[questionCount];
    for (int i = 0; i < questionCount; ++i) {
        config->questionIds[i] = questionIds[i];
    }

    config->next = nullptr;
    return config;
}

bool addExamConfig(
    Class* classroom,
    ExamConfig* config
) {
    if (classroom == nullptr || config == nullptr) {
        return false;
    }

    if (findExamConfig(classroom, config->subjectId) != nullptr) {
        return false;
    }

    config->next = nullptr;

    if (classroom->configList == nullptr) {
        classroom->configList = config;
        return true;
    }

    ExamConfig* current = classroom->configList;
    while (current->next != nullptr) {
        current = current->next;
    }

    current->next = config;
    return true;
}

ExamConfig* findExamConfig(
    const Class* classroom,
    const char* subjectId
) {
    if (classroom == nullptr || subjectId == nullptr) {
        return nullptr;
    }

    ExamConfig* current = classroom->configList;
    while (current != nullptr) {
        if (stringEqual(current->subjectId, subjectId)) {
            return current;
        }
        current = current->next;
    }

    return nullptr;
}

bool deleteExamConfig(
    Class* classroom,
    const char* subjectId
) {
    if (classroom == nullptr || classroom->configList == nullptr || subjectId == nullptr) {
        return false;
    }

    ExamConfig* current = classroom->configList;
    ExamConfig* prev = nullptr;

    while (current != nullptr && !stringEqual(current->subjectId, subjectId)) {
        prev = current;
        current = current->next;
    }

    if (current == nullptr) {
        return false;
    }

    if (prev == nullptr) {
        classroom->configList = current->next;
    } else {
        prev->next = current->next;
    }

    delete[] current->questionIds;
    delete current;

    return true;
}

void deleteExamConfigList(
    ExamConfig*& head
) {
    while (head != nullptr) {
        ExamConfig* temp = head;
        head = head->next;
        delete[] temp->questionIds;
        delete temp;
    }
    head = nullptr;
}
