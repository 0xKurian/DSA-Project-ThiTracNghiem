#include "ExamService.h"
#include "Validation.h"
#include "../core/SubjectBST.h"
#include "../core/QuestionList.h"
#include "../core/ScoreList.h"
#include "../core/StringUtils.h"

#include <cstdlib>
#include <ctime>

// ==================== CORE EXAM PROTOTYPES ====================

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

// ==================== WORKFLOWS ====================

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

    // Moi sinh vien chi duoc thi moi mon 1 lan duy nhat
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

    // Cap phat mang dong con tro de shuffle ngau nhien (Khong dung std::vector)
    const Question** questionPool = new const Question*[totalAvailable];

    Question* currentQ = subject->questionList;
    int idx = 0;
    while (currentQ != nullptr) {
        questionPool[idx++] = currentQ;
        currentQ = currentQ->next;
    }

    initializeRngOnce();

    // Fisher-Yates Shuffle
    for (int i = totalAvailable - 1; i > 0; --i) {
        int j = rand() % (i + 1);
        if (i != j) {
            const Question* temp = questionPool[i];
            questionPool[i] = questionPool[j];
            questionPool[j] = temp;
        }
    }

    // Lay N cau hoi dau tien va tao ExamDetail snapshot
    for (int i = 0; i < numberOfQuestions; ++i) {
        ExamDetail* detail = createExamDetail(questionPool[i]);
        if (detail != nullptr) {
            addExamDetail(examQuestionsHead, detail);
        }
    }

    // Giai phong ngay mang con tro phu (khong delete cac Question goc)
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

    // Kiem tra lai neu sinh vien da co diem mon nay
    if (findScore(student, cleanSubId) != nullptr) {
        cancelExam(examQuestionsHead);
        return ERR_CONSTRAINT_VIOLATION;
    }

    // Cham diem
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

    // Ghi nhan Score va gan quyen so huu examQuestionsHead vao Score->detailList
    if (!addScore(student, cleanSubId, finalScore, examQuestionsHead)) {
        cancelExam(examQuestionsHead);
        return ERR_CONSTRAINT_VIOLATION;
    }

    return SUCCESS;
}
