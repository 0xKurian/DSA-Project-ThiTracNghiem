#ifndef EXAM_SERVICE_H
#define EXAM_SERVICE_H

#include "../core/Structures.h"
#include "BusinessErrors.h"

#include <cstddef>
#include <ctime>

bool canTakeExam(
    Subject* subject,
    int numberOfQuestions,
    int minutes
);

float calculateScore(
    int correctAnswers,
    int totalQuestions
);

bool isExamTimeExpired(
    time_t startTime,
    int minutes
);

struct ExamTimer {
    time_t startTime;
    int totalDurationSeconds;
    int initialRemainingSeconds;
    bool isRunning;

    ExamTimer();
    explicit ExamTimer(int durationSeconds);

    void start(int durationSeconds);
    void startMinutes(int minutes);
    void resume(int remainingSeconds);
    int getRemainingSeconds() const;
    int getElapsedSeconds() const;
    bool isExpired() const;
    void stop();
    void formatTime(char buffer[], int bufferSize) const;
};

int calculateExamRemainingSeconds(
    time_t startTime,
    int durationSeconds
);

bool isExamTimeExpiredSeconds(
    time_t startTime,
    int durationSeconds
);

void formatExamTime(
    int remainingSeconds,
    char buffer[],
    int bufferSize
);

BusinessResult prepareExamWorkflow(
    Subject* root,
    Student* student,
    const char subjectId[],
    int numberOfQuestions,
    int minutes,
    ExamDetail*& examQuestionsHead
);

BusinessResult submitExamWorkflow(
    Student* student,
    const char subjectId[],
    ExamDetail* examQuestionsHead,
    float& finalScore,
    int& correctCount,
    int& totalCount
);

void cancelExam(
    ExamDetail*& examQuestionsHead
);

bool saveExamSession(
    const char studentId[],
    const char subjectId[],
    int remainingSeconds,
    ExamDetail* examQuestionsHead,
    const char* filepath = nullptr
);

bool saveExamSession(
    const char studentId[],
    int remainingSeconds,
    ExamDetail* examQuestionsHead,
    const char* filepath = nullptr
);

bool saveExamSession(
    Student* student,
    const char subjectId[],
    int remainingSeconds,
    ExamDetail* examQuestionsHead,
    const char* filepath = nullptr
);

bool saveExamSession(
    const char studentId[],
    const char subjectId[],
    int remainingSeconds,
    const int questionIds[],
    const char selectedAnswers[],
    int count,
    const char* filepath = nullptr
);

bool saveExamSession(
    const char studentId[],
    int remainingSeconds,
    const int questionIds[],
    const char selectedAnswers[],
    int count,
    const char* filepath = nullptr
);

bool checkExamSession(
    const char studentId[],
    const char* filepath = nullptr
);

bool checkExamSession(
    Student* student,
    const char* filepath = nullptr
);

bool checkExamSession(
    const char studentId[],
    char subjectIdOut[],
    int& remainingSecondsOut,
    const char* filepath = nullptr
);

bool checkExamSession(
    const char studentId[],
    char subjectIdOut[],
    int& remainingSecondsOut,
    int questionIdsOut[],
    char selectedAnswersOut[],
    int& countOut,
    int maxQuestions = 100,
    const char* filepath = nullptr
);

bool removeExamSession(
    const char studentId[],
    const char* filepath = nullptr
);

bool removeExamSession(
    Student* student,
    const char* filepath = nullptr
);

bool loadExamSession(
    const char studentId[],
    char subjectIdOut[],
    int& remainingSecondsOut,
    ExamDetail*& examQuestionsHead,
    Subject* root = nullptr,
    const char* filepath = nullptr
);

inline bool saveExamSession(std::nullptr_t, const char*, int, ExamDetail*, const char* = nullptr) {
    return false;
}

inline bool checkExamSession(std::nullptr_t, const char* = nullptr) {
    return false;
}

inline bool removeExamSession(std::nullptr_t, const char* = nullptr) {
    return false;
}

#endif
