#ifndef REPORT_SERVICE_H
#define REPORT_SERVICE_H

#include "../core/Structures.h"
#include "BusinessErrors.h"
#include <iostream>

// ==================== DATA QUERY APIS (Dành cho UI tự thiết kế giao diện) ====================

// 1. Kiểm tra sinh viên đã thi môn học chưa và lấy điểm số
bool hasStudentTakenExam(
    Student* student,
    const char subjectId[],
    float& outScore
);

// 2. Lấy con trỏ Score của sinh viên theo môn học (trả về nullptr nếu chưa thi)
Score* getStudentExamScore(
    Student* student,
    const char subjectId[]
);

// 3. Lấy con trỏ đầu danh sách snapshot ExamDetail để UI duyệt xem lại bài thi (nullptr nếu chưa thi)
ExamDetail* getStudentExamDetails(
    Student* student,
    const char subjectId[]
);

// ==================== PRINT HELPERS (Tiện ích in sẵn nếu UI muốn gọi nhanh) ====================

// In bảng điểm của lớp theo môn học (tự động điền điểm hoặc "Chưa thi")
BusinessResult printClassScoreTable(
    Class* classes[],
    int classCount,
    Subject* root,
    const char classId[],
    const char subjectId[],
    std::ostream& out = std::cout
);

// In chi tiết từng câu hỏi trong bài thi đã làm của sinh viên kèm đáp án đã chọn và đáp án đúng
BusinessResult printExamDetailReview(
    Student* student,
    const char subjectId[],
    std::ostream& out = std::cout
);

// In danh sách các môn đã thi và điểm số của một sinh viên
BusinessResult printStudentScores(
    Student* student,
    Subject* root,
    std::ostream& out = std::cout
);

#endif
