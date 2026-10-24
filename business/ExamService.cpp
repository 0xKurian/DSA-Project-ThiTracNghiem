#include "ExamService.h"
#include "Validation.h"
#include "../core/SubjectBST.h"
#include "../core/QuestionList.h"
#include "../core/ScoreList.h"
#include "../core/StringUtils.h"

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <string>

bool canTakeExam(
    Subject* subject,
    int numberOfQuestions,
    int minutes
) {
    if (subject == nullptr) {
        return false;
    }

    if (numberOfQuestions <= 0 || minutes <= 0) {
        return false;
    }

    int totalAvailable = countQuestions(subject->questionList);
    if (numberOfQuestions > totalAvailable) {
        return false;
    }

    return true;
}

float calculateScore(
    int correctAnswers,
    int totalQuestions
) {
    if (totalQuestions <= 0) {
        return 0.0f;
    }

    if (correctAnswers <= 0) {
        return 0.0f;
    }

    if (correctAnswers > totalQuestions) {
        correctAnswers = totalQuestions;
    }

    return (static_cast<float>(correctAnswers) * 10.0f) / static_cast<float>(totalQuestions);
}

void cancelExam(
    ExamDetail*& examQuestionsHead
) {
    deleteExamDetailList(examQuestionsHead);
}

bool isExamTimeExpired(
    time_t startTime,
    int minutes
) {
    if (minutes <= 0) {
        return true;
    }
    return difftime(time(nullptr), startTime) >= static_cast<double>(minutes * 60);
}

ExamTimer::ExamTimer()
    : startTime(0), totalDurationSeconds(0), initialRemainingSeconds(0), isRunning(false) {}

ExamTimer::ExamTimer(int durationSeconds)
    : startTime(time(nullptr)), totalDurationSeconds(durationSeconds),
      initialRemainingSeconds(durationSeconds), isRunning(true) {}

void ExamTimer::start(int durationSeconds) {
    startTime = time(nullptr);
    totalDurationSeconds = durationSeconds;
    initialRemainingSeconds = durationSeconds;
    isRunning = true;
}

void ExamTimer::startMinutes(int minutes) {
    start(minutes * 60);
}

void ExamTimer::resume(int remainingSeconds) {
    startTime = time(nullptr);
    totalDurationSeconds = remainingSeconds;
    initialRemainingSeconds = remainingSeconds;
    isRunning = true;
}

int ExamTimer::getRemainingSeconds() const {
    if (!isRunning) return 0;
    double elapsed = difftime(time(nullptr), startTime);
    int remaining = initialRemainingSeconds - static_cast<int>(elapsed);
    return (remaining > 0) ? remaining : 0;
}

int ExamTimer::getElapsedSeconds() const {
    if (!isRunning) return 0;
    int elapsed = static_cast<int>(difftime(time(nullptr), startTime));
    return (elapsed >= initialRemainingSeconds) ? initialRemainingSeconds : elapsed;
}

bool ExamTimer::isExpired() const {
    return getRemainingSeconds() <= 0;
}

void ExamTimer::stop() {
    isRunning = false;
}

void ExamTimer::formatTime(char buffer[], int bufferSize) const {
    formatExamTime(getRemainingSeconds(), buffer, bufferSize);
}

int calculateExamRemainingSeconds(time_t startTime, int durationSeconds) {
    if (durationSeconds <= 0) return 0;
    int elapsed = static_cast<int>(difftime(time(nullptr), startTime));
    int remaining = durationSeconds - elapsed;
    return (remaining > 0) ? remaining : 0;
}

bool isExamTimeExpiredSeconds(time_t startTime, int durationSeconds) {
    if (durationSeconds <= 0) return true;
    return difftime(time(nullptr), startTime) >= static_cast<double>(durationSeconds);
}

void formatExamTime(int remainingSeconds, char buffer[], int bufferSize) {
    if (buffer == nullptr || bufferSize <= 0) return;
    if (remainingSeconds < 0) remainingSeconds = 0;
    int mins = remainingSeconds / 60;
    int secs = remainingSeconds % 60;
    std::snprintf(buffer, bufferSize, "%02d:%02d (%d phut %d giay)", mins, secs, mins, secs);
}

static void initializeRngOnce() {
    static bool rngInitialized = false;
    if (!rngInitialized) {
        srand(static_cast<unsigned int>(time(nullptr)));
        rngInitialized = true;
    }
}

BusinessResult prepareExamWorkflow(
    Subject* root,
    Student* student,
    const char subjectId[],
    int numberOfQuestions,
    int minutes,
    ExamDetail*& examQuestionsHead
) {
    examQuestionsHead = nullptr;

    if (root == nullptr || student == nullptr || subjectId == nullptr) {
        return ERR_NOT_FOUND;
    }

    BusinessResult rSubId = validateSubjectId(subjectId);
    if (rSubId != SUCCESS) return rSubId;

    char cleanSubId[SUBJECT_ID_LEN];
    stringCopy(cleanSubId, subjectId, SUBJECT_ID_LEN);
    trimString(cleanSubId);
    toUpperString(cleanSubId);

    if (findScore(student, cleanSubId) != nullptr) {
        return ERR_CONSTRAINT_VIOLATION;
    }

    Subject* subject = findSubject(root, cleanSubId);
    if (subject == nullptr) {
        return ERR_NOT_FOUND;
    }

    if (!canTakeExam(subject, numberOfQuestions, minutes)) {
        return ERR_INVALID_FORMAT;
    }

    int totalAvailable = countQuestions(subject->questionList);
    if (numberOfQuestions > totalAvailable) {
        return ERR_INVALID_FORMAT;
    }

    const Question** questionPool = new const Question*[totalAvailable];

    Question* currentQ = subject->questionList;
    int idx = 0;
    while (currentQ != nullptr) {
        questionPool[idx++] = currentQ;
        currentQ = currentQ->next;
    }

    initializeRngOnce();

    for (int i = totalAvailable - 1; i > 0; --i) {
        int j = rand() % (i + 1);
        if (i != j) {
            const Question* temp = questionPool[i];
            questionPool[i] = questionPool[j];
            questionPool[j] = temp;
        }
    }

    for (int i = 0; i < numberOfQuestions; ++i) {
        ExamDetail* detail = createExamDetail(questionPool[i]);
        if (detail != nullptr) {
            addExamDetail(examQuestionsHead, detail);
        }
    }

    delete[] questionPool;

    return SUCCESS;
}

BusinessResult submitExamWorkflow(
    Student* student,
    const char subjectId[],
    ExamDetail* examQuestionsHead,
    float& finalScore,
    int& correctCount,
    int& totalCount
) {
    finalScore = 0.0f;
    correctCount = 0;
    totalCount = 0;

    if (student == nullptr || subjectId == nullptr) {
        if (examQuestionsHead != nullptr) {
            cancelExam(examQuestionsHead);
        }
        return ERR_NOT_FOUND;
    }

    if (examQuestionsHead == nullptr) {
        return ERR_EMPTY_FIELD;
    }

    char cleanSubId[SUBJECT_ID_LEN];
    stringCopy(cleanSubId, subjectId, SUBJECT_ID_LEN);
    trimString(cleanSubId);
    toUpperString(cleanSubId);

    if (findScore(student, cleanSubId) != nullptr) {
        cancelExam(examQuestionsHead);
        return ERR_CONSTRAINT_VIOLATION;
    }

    ExamDetail* current = examQuestionsHead;
    while (current != nullptr) {
        totalCount++;

        char selected = current->selectedAnswer;
        if (selected >= 'a' && selected <= 'd') {
            selected = static_cast<char>(selected - ('a' - 'A'));
            current->selectedAnswer = selected;
        }

        if (selected == current->correctAnswer) {
            correctCount++;
        }

        current = current->next;
    }

    finalScore = calculateScore(correctCount, totalCount);

    if (!addScore(student, cleanSubId, finalScore, examQuestionsHead)) {
        cancelExam(examQuestionsHead);
        return ERR_CONSTRAINT_VIOLATION;
    }

    removeExamSession(student->id);

    return SUCCESS;
}

static std::string getExamSessionFilePath(const char studentId[], const char* filepath = nullptr) {
    if (filepath != nullptr && stringLength(filepath) > 0) {
        return std::string(filepath);
    }
    char cleanId[STUDENT_ID_LEN] = {};
    if (studentId != nullptr) {
        stringCopy(cleanId, studentId, STUDENT_ID_LEN);
        trimString(cleanId);
        toUpperString(cleanId);
    }
    return "temp_" + std::string(cleanId) + ".dat";
}

bool saveExamSession(
    const char studentId[],
    const char subjectId[],
    int remainingSeconds,
    ExamDetail* examQuestionsHead,
    const char* filepath
) {
    if (studentId == nullptr || stringLength(studentId) == 0) {
        return false;
    }
    if (examQuestionsHead == nullptr) {
        return false;
    }

    std::string path = getExamSessionFilePath(studentId, filepath);
    std::ofstream out(path);
    if (!out.is_open()) {
        return false;
    }

    char cleanSubId[SUBJECT_ID_LEN] = {};
    if (subjectId != nullptr) {
        stringCopy(cleanSubId, subjectId, SUBJECT_ID_LEN);
        trimString(cleanSubId);
        toUpperString(cleanSubId);
    }

    char cleanStdId[STUDENT_ID_LEN] = {};
    stringCopy(cleanStdId, studentId, STUDENT_ID_LEN);
    trimString(cleanStdId);
    toUpperString(cleanStdId);

    int count = 0;
    ExamDetail* cur = examQuestionsHead;
    while (cur != nullptr) {
        count++;
        cur = cur->next;
    }

    out << cleanStdId << "|" << cleanSubId << "|" << (remainingSeconds >= 0 ? remainingSeconds : 0) << "|" << count << "\n";

    cur = examQuestionsHead;
    while (cur != nullptr) {
        char sel = (cur->selectedAnswer == '\0') ? '-' : cur->selectedAnswer;
        out << cur->questionId << "|"
            << sel << "|"
            << cur->correctAnswer << "|"
            << cur->content << "|"
            << cur->answerA << "|"
            << cur->answerB << "|"
            << cur->answerC << "|"
            << cur->answerD << "\n";
        cur = cur->next;
    }

    out.close();
    return true;
}

bool saveExamSession(
    const char studentId[],
    int remainingSeconds,
    ExamDetail* examQuestionsHead,
    const char* filepath
) {
    return saveExamSession(studentId, "", remainingSeconds, examQuestionsHead, filepath);
}

bool saveExamSession(
    Student* student,
    const char subjectId[],
    int remainingSeconds,
    ExamDetail* examQuestionsHead,
    const char* filepath
) {
    if (student == nullptr) return false;
    return saveExamSession(student->id, subjectId, remainingSeconds, examQuestionsHead, filepath);
}

bool saveExamSession(
    const char studentId[],
    const char subjectId[],
    int remainingSeconds,
    const int questionIds[],
    const char selectedAnswers[],
    int count,
    const char* filepath
) {
    if (studentId == nullptr || stringLength(studentId) == 0) return false;
    if (questionIds == nullptr || count <= 0) return false;

    std::string path = getExamSessionFilePath(studentId, filepath);
    std::ofstream out(path);
    if (!out.is_open()) return false;

    char cleanSubId[SUBJECT_ID_LEN] = {};
    if (subjectId != nullptr) {
        stringCopy(cleanSubId, subjectId, SUBJECT_ID_LEN);
        trimString(cleanSubId);
        toUpperString(cleanSubId);
    }

    char cleanStdId[STUDENT_ID_LEN] = {};
    stringCopy(cleanStdId, studentId, STUDENT_ID_LEN);
    trimString(cleanStdId);
    toUpperString(cleanStdId);

    out << cleanStdId << "|" << cleanSubId << "|" << (remainingSeconds >= 0 ? remainingSeconds : 0) << "|" << count << "\n";

    for (int i = 0; i < count; ++i) {
        char sel = (selectedAnswers != nullptr && selectedAnswers[i] != '\0') ? selectedAnswers[i] : '-';
        out << questionIds[i] << "|" << sel << "\n";
    }

    out.close();
    return true;
}

bool saveExamSession(
    const char studentId[],
    int remainingSeconds,
    const int questionIds[],
    const char selectedAnswers[],
    int count,
    const char* filepath
) {
    return saveExamSession(studentId, "", remainingSeconds, questionIds, selectedAnswers, count, filepath);
}

bool checkExamSession(
    const char studentId[],
    const char* filepath
) {
    if (studentId == nullptr || stringLength(studentId) == 0) return false;
    std::string path = getExamSessionFilePath(studentId, filepath);
    std::ifstream in(path);
    if (!in.is_open()) return false;

    std::string line;
    if (!std::getline(in, line)) return false;
    while (!line.empty() && (line.back() == '\r' || line.back() == '\n' || line.back() == ' ')) line.pop_back();
    return !line.empty();
}

bool checkExamSession(
    Student* student,
    const char* filepath
) {
    if (student == nullptr) return false;
    return checkExamSession(student->id, filepath);
}

bool checkExamSession(
    const char studentId[],
    char subjectIdOut[],
    int& remainingSecondsOut,
    const char* filepath
) {
    remainingSecondsOut = 0;
    if (subjectIdOut != nullptr) subjectIdOut[0] = '\0';
    if (studentId == nullptr || stringLength(studentId) == 0) return false;

    std::string path = getExamSessionFilePath(studentId, filepath);
    std::ifstream in(path);
    if (!in.is_open()) return false;

    std::string line;
    if (!std::getline(in, line)) return false;
    while (!line.empty() && (line.back() == '\r' || line.back() == '\n' || line.back() == ' ')) line.pop_back();
    if (line.empty()) return false;

    std::string tokens[8];
    int tokenCount = 0;
    size_t start = 0;
    while (tokenCount < 8) {
        size_t pos = line.find('|', start);
        if (pos == std::string::npos) {
            tokens[tokenCount++] = line.substr(start);
            break;
        } else {
            tokens[tokenCount++] = line.substr(start, pos - start);
            start = pos + 1;
        }
    }

    if (tokenCount >= 4) {
        if (subjectIdOut != nullptr) {
            stringCopy(subjectIdOut, tokens[1].c_str(), SUBJECT_ID_LEN);
        }
        try {
            remainingSecondsOut = std::stoi(tokens[2]);
        } catch (...) {
            remainingSecondsOut = 0;
        }
    } else if (tokenCount == 3) {
        if (subjectIdOut != nullptr) {
            stringCopy(subjectIdOut, tokens[0].c_str(), SUBJECT_ID_LEN);
        }
        try {
            remainingSecondsOut = std::stoi(tokens[1]);
        } catch (...) {
            remainingSecondsOut = 0;
        }
    } else if (tokenCount >= 1) {
        try {
            remainingSecondsOut = std::stoi(tokens[0]);
        } catch (...) {
            remainingSecondsOut = 0;
        }
    }

    return true;
}

bool checkExamSession(
    const char studentId[],
    char subjectIdOut[],
    int& remainingSecondsOut,
    int questionIdsOut[],
    char selectedAnswersOut[],
    int& countOut,
    int maxQuestions,
    const char* filepath
) {
    countOut = 0;
    if (!checkExamSession(studentId, subjectIdOut, remainingSecondsOut, filepath)) {
        return false;
    }

    std::string path = getExamSessionFilePath(studentId, filepath);
    std::ifstream in(path);
    if (!in.is_open()) return false;

    std::string line;
    if (!std::getline(in, line)) return false;

    int idx = 0;
    while (std::getline(in, line) && idx < maxQuestions) {
        while (!line.empty() && (line.back() == '\r' || line.back() == '\n' || line.back() == ' ')) line.pop_back();
        if (line.empty()) continue;

        size_t pipe = line.find('|');
        if (pipe != std::string::npos) {
            try {
                questionIdsOut[idx] = std::stoi(line.substr(0, pipe));
            } catch (...) {
                questionIdsOut[idx] = 0;
            }
            char sel = line[pipe + 1];
            selectedAnswersOut[idx] = (sel == '-') ? '\0' : sel;
            idx++;
        }
    }
    countOut = idx;
    return true;
}

bool removeExamSession(
    const char studentId[],
    const char* filepath
) {
    if (studentId == nullptr || stringLength(studentId) == 0) return false;
    std::string path = getExamSessionFilePath(studentId, filepath);

    std::ifstream check(path);
    if (!check.is_open()) {
        return false;
    }
    check.close();

    return (std::remove(path.c_str()) == 0);
}

bool removeExamSession(
    Student* student,
    const char* filepath
) {
    if (student == nullptr) return false;
    return removeExamSession(student->id, filepath);
}

bool loadExamSession(
    const char studentId[],
    char subjectIdOut[],
    int& remainingSecondsOut,
    ExamDetail*& examQuestionsHead,
    Subject* root,
    const char* filepath
) {
    examQuestionsHead = nullptr;
    remainingSecondsOut = 0;
    if (subjectIdOut != nullptr) subjectIdOut[0] = '\0';
    if (studentId == nullptr || stringLength(studentId) == 0) return false;

    std::string path = getExamSessionFilePath(studentId, filepath);
    std::ifstream in(path);
    if (!in.is_open()) return false;

    std::string headerLine;
    if (!std::getline(in, headerLine)) return false;
    while (!headerLine.empty() && (headerLine.back() == '\r' || headerLine.back() == '\n' || headerLine.back() == ' ')) headerLine.pop_back();
    if (headerLine.empty()) return false;

    std::string headerTokens[8];
    int hCount = 0;
    size_t start = 0;
    while (hCount < 8) {
        size_t pos = headerLine.find('|', start);
        if (pos == std::string::npos) {
            headerTokens[hCount++] = headerLine.substr(start);
            break;
        } else {
            headerTokens[hCount++] = headerLine.substr(start, pos - start);
            start = pos + 1;
        }
    }

    std::string subId = "";
    if (hCount >= 4) {
        subId = headerTokens[1];
        try { remainingSecondsOut = std::stoi(headerTokens[2]); } catch (...) { remainingSecondsOut = 0; }
    } else if (hCount == 3) {
        subId = headerTokens[0];
        try { remainingSecondsOut = std::stoi(headerTokens[1]); } catch (...) { remainingSecondsOut = 0; }
    } else if (hCount >= 1) {
        try { remainingSecondsOut = std::stoi(headerTokens[0]); } catch (...) { remainingSecondsOut = 0; }
    }

    if (subjectIdOut != nullptr) {
        stringCopy(subjectIdOut, subId.c_str(), SUBJECT_ID_LEN);
    }

    Subject* subjectNode = nullptr;
    if (root != nullptr && !subId.empty()) {
        subjectNode = findSubject(root, subId.c_str());
    }

    std::string line;
    while (std::getline(in, line)) {
        while (!line.empty() && (line.back() == '\r' || line.back() == '\n' || line.back() == ' ')) line.pop_back();
        if (line.empty()) continue;

        std::string tokens[9];
        int tCount = 0;
        size_t sPos = 0;
        while (tCount < 9) {
            size_t pos = line.find('|', sPos);
            if (pos == std::string::npos) {
                tokens[tCount++] = line.substr(sPos);
                break;
            } else {
                tokens[tCount++] = line.substr(sPos, pos - sPos);
                sPos = pos + 1;
            }
        }

        if (tCount >= 8) {

            int qId = 0;
            try { qId = std::stoi(tokens[0]); } catch (...) { continue; }
            char sel = tokens[1].empty() ? '\0' : tokens[1][0];
            if (sel == '-') sel = '\0';
            char correct = tokens[2].empty() ? 'A' : tokens[2][0];

            ExamDetail* d = new ExamDetail;
            d->questionId = qId;
            d->selectedAnswer = sel;
            d->correctAnswer = correct;
            stringCopy(d->content, tokens[3].c_str(), QUESTION_CONTENT_LEN);
            stringCopy(d->answerA, tokens[4].c_str(), ANSWER_CONTENT_LEN);
            stringCopy(d->answerB, tokens[5].c_str(), ANSWER_CONTENT_LEN);
            stringCopy(d->answerC, tokens[6].c_str(), ANSWER_CONTENT_LEN);
            stringCopy(d->answerD, tokens[7].c_str(), ANSWER_CONTENT_LEN);
            d->next = nullptr;

            addExamDetail(examQuestionsHead, d);
        } else if (tCount >= 2 && subjectNode != nullptr) {

            int qId = 0;
            try { qId = std::stoi(tokens[0]); } catch (...) { continue; }
            char sel = tokens[1].empty() ? '\0' : tokens[1][0];
            if (sel == '-') sel = '\0';

            Question* q = findQuestion(subjectNode->questionList, qId);
            if (q != nullptr) {
                ExamDetail* d = createExamDetail(q);
                if (d != nullptr) {
                    d->selectedAnswer = sel;
                    addExamDetail(examQuestionsHead, d);
                }
            }
        }
    }

    in.close();
    if (examQuestionsHead == nullptr) {
        return false;
    }
    return true;
}
