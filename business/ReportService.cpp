#include "ReportService.h"
#include "Validation.h"
#include "../core/ClassArray.h"
#include "../core/SubjectBST.h"
#include "../core/ScoreList.h"
#include "../core/StringUtils.h"

#include <iomanip>

Score* getStudentExamScore(
    Student* student,
    const char subjectId[]
) {
    if (student == nullptr || subjectId == nullptr) {
        return nullptr;
    }

    char cleanSubId[SUBJECT_ID_LEN];
    stringCopy(cleanSubId, subjectId, SUBJECT_ID_LEN);
    trimString(cleanSubId);
    toUpperString(cleanSubId);

    return findScore(student, cleanSubId);
}

ExamDetail* getStudentExamDetails(
    Student* student,
    const char subjectId[]
) {
    Score* score = getStudentExamScore(student, subjectId);
    return (score != nullptr) ? score->detailList : nullptr;
}

bool hasStudentTakenExam(
    Student* student,
    const char subjectId[],
    float& outScore
) {
    outScore = 0.0f;
    Score* score = getStudentExamScore(student, subjectId);
    if (score != nullptr) {
        outScore = score->score;
        return true;
    }

    return false;
}

BusinessResult printClassScoreTable(
    Class* classes[],
    int classCount,
    Subject* root,
    const char classId[],
    const char subjectId[],
    std::ostream& out
) {
    if (classes == nullptr || classId == nullptr || subjectId == nullptr) {
        return ERR_NOT_FOUND;
    }

    char cleanClassId[CLASS_ID_LEN];
    stringCopy(cleanClassId, classId, CLASS_ID_LEN);
    trimString(cleanClassId);
    toUpperString(cleanClassId);

    char cleanSubId[SUBJECT_ID_LEN];
    stringCopy(cleanSubId, subjectId, SUBJECT_ID_LEN);
    trimString(cleanSubId);
    toUpperString(cleanSubId);

    Class* classroom = findClass(classes, classCount, cleanClassId);
    if (classroom == nullptr) {
        return ERR_NOT_FOUND;
    }

    Subject* subject = (root != nullptr) ? findSubject(root, cleanSubId) : nullptr;
    const char* subjectName = (subject != nullptr) ? subject->name : "Khong xac dinh";

    out << "=============================================================\n";
    out << "                   BANG DIEM MON HOC                         \n";
    out << "Lop    : " << classroom->id << " - " << classroom->name << "\n";
    out << "Mon hoc: " << cleanSubId << " - " << subjectName << "\n";
    out << "=============================================================\n";
    out << std::left << std::setw(5) << "STT"
        << std::setw(15) << "MASV"
        << std::setw(30) << "HO VA TEN"
        << "DIEM\n";
    out << "-------------------------------------------------------------\n";

    if (classroom->studentList == nullptr) {
        out << "Lop chua co sinh vien nao.\n";
        out << "=============================================================\n";
        return SUCCESS;
    }

    Student* current = classroom->studentList;
    int stt = 1;

    while (current != nullptr) {
        char fullName[HO_LEN + TEN_LEN];
        stringCopy(fullName, current->ho, HO_LEN + TEN_LEN);
        int len = stringLength(fullName);
        if (len < HO_LEN + TEN_LEN - 2) {
            fullName[len] = ' ';
            fullName[len + 1] = '\0';
            stringCopy(fullName + len + 1, current->ten, TEN_LEN);
        }

        out << std::left << std::setw(5) << stt++
            << std::setw(15) << current->id
            << std::setw(30) << fullName;

        float scoreVal = 0.0f;
        if (hasStudentTakenExam(current, cleanSubId, scoreVal)) {
            out << std::fixed << std::setprecision(2) << scoreVal << "\n";
        } else {
            out << "Chua thi\n";
        }

        current = current->next;
    }

    out << "=============================================================\n";
    return SUCCESS;
}

BusinessResult printExamDetailReview(
    Student* student,
    const char subjectId[],
    std::ostream& out
) {
    if (student == nullptr || subjectId == nullptr) {
        return ERR_NOT_FOUND;
    }

    char cleanSubId[SUBJECT_ID_LEN];
    stringCopy(cleanSubId, subjectId, SUBJECT_ID_LEN);
    trimString(cleanSubId);
    toUpperString(cleanSubId);

    Score* score = findScore(student, cleanSubId);
    if (score == nullptr) {
        return ERR_NOT_FOUND; // Sinh vien chua thi mon nay
    }

    if (score->detailList == nullptr) {
        return ERR_NOT_FOUND; // Khong co chi tiet bai thi
    }

    out << "=============================================================\n";
    out << "              CHI TIET BAI THI TRAC NGHIEM                   \n";
    out << "Sinh vien: " << student->id << " - " << student->ho << " " << student->ten << "\n";
    out << "Mon thi  : " << cleanSubId << "\n";
    out << "Diem so  : " << std::fixed << std::setprecision(2) << score->score << "\n";
    out << "=============================================================\n";

    ExamDetail* cur = score->detailList;
    int qNo = 1;

    while (cur != nullptr) {
        out << "\nCau " << qNo++ << ": " << cur->content << "\n";
        out << "  A. " << cur->answerA << "\n";
        out << "  B. " << cur->answerB << "\n";
        out << "  C. " << cur->answerC << "\n";
        out << "  D. " << cur->answerD << "\n";

        out << "  -> Ban chon : ";
        if (cur->selectedAnswer != '\0') {
            out << cur->selectedAnswer;
        } else {
            out << "(Bo trong)";
        }

        out << " | Dap an dung: " << cur->correctAnswer;

        if (cur->selectedAnswer == cur->correctAnswer) {
            out << "  [DUNG]\n";
        } else {
            out << "  [SAI]\n";
        }

        cur = cur->next;
    }

    out << "\n=============================================================\n";
    return SUCCESS;
}

BusinessResult printStudentScores(
    Student* student,
    Subject* root,
    std::ostream& out
) {
    if (student == nullptr) {
        return ERR_NOT_FOUND;
    }

    out << "=============================================================\n";
    out << "               KET QUA THI CUA SINH VIEN                     \n";
    out << "Sinh vien: " << student->id << " - " << student->ho << " " << student->ten << "\n";
    out << "=============================================================\n";
    out << std::left << std::setw(15) << "MA MON"
        << std::setw(35) << "TEN MON HOC"
        << "DIEM\n";
    out << "-------------------------------------------------------------\n";

    if (student->scoreList == nullptr) {
        out << "Sinh vien chua co diem mon nao.\n";
        out << "=============================================================\n";
        return SUCCESS;
    }

    Score* cur = student->scoreList;
    while (cur != nullptr) {
        Subject* sub = (root != nullptr) ? findSubject(root, cur->subjectId) : nullptr;
        const char* subName = (sub != nullptr) ? sub->name : "Khong xac dinh";

        out << std::left << std::setw(15) << cur->subjectId
            << std::setw(35) << subName
            << std::fixed << std::setprecision(2) << cur->score << "\n";

        cur = cur->next;
    }

    out << "=============================================================\n";
    return SUCCESS;
}
