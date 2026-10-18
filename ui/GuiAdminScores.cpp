// tab bao cao diem theo lop + xem lai chi tiet bai thi (DSLK ExamDetail)
// luu y: tab xem lai chi tiet doc Score (findScore) roi duyet detailList
#include "GuiCommon.h"
#include "GuiState.h"

#include "../business/ReportService.h"   // hasStudentTakenExam

#include "../core/ScoreList.h"           // findScore
#include "../core/SubjectBST.h"          // Subject (curS)

#include <algorithm>
#include <cstdio>

void drawAdminTabScores(GuiState& g) {
    Rectangle contentRec = { 260, 85, (float)(GUI_WIDTH - 280), (float)(GUI_HEIGHT - 105) };

    DrawTxt("BANG DIEM MON HOC THEO LOP", contentRec.x + 25, contentRec.y + 25, 20, COLOR_TEXT_DARK, true);

    DrawTxt("Lop:", contentRec.x + 25, contentRec.y + 68, 15, COLOR_TEXT_DARK, true);
    for (int i = 0; i < g.classCount && i < 4; ++i) {
        if (g.classes[i] == nullptr) continue;
        bool sel = (g.selectedClassIdx == i);
        if (GuiBtn({ contentRec.x + 75 + i * 130, contentRec.y + 62, 120, 32 }, g.classes[i]->id, sel ? COLOR_PRIMARY : COLOR_SIDEBAR, COLOR_PRIMARY_HOVER, WHITE, 14, sel)) {
            g.selectedClassIdx = i;
            g.scorePage = 0;
        }
    }

    DrawTxt("Mon:", contentRec.x + 620, contentRec.y + 68, 15, COLOR_TEXT_DARK, true);
    // 3 nut + 2 moc trang moi vua khoang ben phai; subject thu 4 bi cat di canh cua so
    int subMax = 3;
    if (g.scoreSubNavOffset >= (int)g.subjects.size()) g.scoreSubNavOffset = 0;
    if (g.scoreSubNavOffset > 0 && GuiBtnOutline({ contentRec.x + 666, contentRec.y + 62, 30, 32 }, "<", COLOR_PRIMARY, COLOR_PRIMARY, 14, true)) {
        g.scoreSubNavOffset--;
    }
    for (int c = 0; c < subMax; ++c) {
        size_t idx = g.scoreSubNavOffset + c;
        if (idx >= g.subjects.size()) break;
        bool sel = (g.selectedSubjectIdx == (int)idx);
        if (GuiBtn({ contentRec.x + 704 + c * 92, contentRec.y + 62, 88, 32 }, g.subjects[idx]->id, sel ? COLOR_PRIMARY : COLOR_SIDEBAR, COLOR_PRIMARY_HOVER, WHITE, 14, sel)) {
            g.selectedSubjectIdx = (int)idx;
        }
    }
    if (g.scoreSubNavOffset + subMax < (int)g.subjects.size()) {
        if (GuiBtnOutline({ contentRec.x + 704 + subMax * 92, contentRec.y + 62, 30, 32 }, ">", COLOR_PRIMARY, COLOR_PRIMARY, 14, true)) {
            g.scoreSubNavOffset++;
        }
    }

    if (g.classCount > 0 && g.selectedClassIdx < g.classCount && g.classes[g.selectedClassIdx] != nullptr && !g.subjects.empty() && g.selectedSubjectIdx < (int)g.subjects.size()) {
        Class* curC = g.classes[g.selectedClassIdx];
        Subject* curS = g.subjects[g.selectedSubjectIdx];

        std::vector<Student*> sList;
        collectStudents(curC->studentList, sList);

        int totalScorePages = sList.empty() ? 1 : ((int)sList.size() + SCORES_PER_PAGE - 1) / SCORES_PER_PAGE;
        if (g.scorePage >= totalScorePages) g.scorePage = totalScorePages - 1;
        if (g.scorePage < 0) g.scorePage = 0;

        int tableY = (int)contentRec.y + 115;
        DrawRectangle((int)contentRec.x + 25, tableY, (int)contentRec.width - 50, 40, COLOR_SIDEBAR);
        DrawTxt("STT", contentRec.x + 40, (float)tableY + 11, 15, WHITE, true);
        DrawTxt("MASV", contentRec.x + 120, (float)tableY + 11, 15, WHITE, true);
        DrawTxt("HO VA TEN", contentRec.x + 320, (float)tableY + 11, 15, WHITE, true);
        DrawTxt("DIEM THI (Thang 10)", contentRec.x + 720, (float)tableY + 11, 15, WHITE, true);

        if (sList.empty()) {
            DrawTxt("Lop nay chua co sinh vien.", contentRec.x + 40, (float)tableY + 60, 16, COLOR_TEXT_MUTED);
        } else {
            int startIdx = g.scorePage * SCORES_PER_PAGE;
            int endIdx = std::min(startIdx + SCORES_PER_PAGE, (int)sList.size());

            for (int i = startIdx; i < endIdx; ++i) {
                int rowOffset = i - startIdx;
                int rowY = tableY + 45 + rowOffset * 45;
                Color rowBg = (rowOffset % 2 == 0) ? COLOR_CARD : COLOR_ROW_ALT;
                DrawRectangle((int)contentRec.x + 25, rowY, (int)contentRec.width - 50, 42, rowBg);
                DrawRectangleLines((int)contentRec.x + 25, rowY, (int)contentRec.width - 50, 42, COLOR_BORDER);

                DrawTxt(TextFormat("%d", i + 1), contentRec.x + 45, (float)rowY + 11, 15, COLOR_TEXT_DARK);
                DrawTxt(sList[i]->id, contentRec.x + 120, (float)rowY + 11, 15, COLOR_PRIMARY, true);
                DrawTxt(TextFormat("%s %s", sList[i]->ho, sList[i]->ten), contentRec.x + 320, (float)rowY + 11, 15, COLOR_TEXT_DARK);

                float sc = 0.0f;
                if (hasStudentTakenExam(sList[i], curS->id, sc)) {
                    DrawTxt(TextFormat("%.2f diem", sc), contentRec.x + 720, (float)rowY + 11, 16, COLOR_SUCCESS, true);
                } else {
                    DrawTxt("Chua thi", contentRec.x + 720, (float)rowY + 11, 15, COLOR_TEXT_MUTED);
                }
            }

            int pageBarY = (int)(contentRec.y + contentRec.height - 50);
            DrawTxt(TextFormat("Trang %d / %d (Tong: %d sinh vien)", g.scorePage + 1, totalScorePages, (int)sList.size()), contentRec.x + 40, (float)pageBarY + 8, 15, COLOR_TEXT_MUTED, true);

            if (GuiBtnOutline({ contentRec.x + contentRec.width - 240, (float)pageBarY, 105, 32 }, "< Trang truoc", COLOR_PRIMARY, COLOR_PRIMARY, 13, false, g.scorePage == 0)) {
                if (g.scorePage > 0) g.scorePage--;
            }
            if (GuiBtnOutline({ contentRec.x + contentRec.width - 125, (float)pageBarY, 105, 32 }, "Trang sau >", COLOR_PRIMARY, COLOR_PRIMARY, 13, false, g.scorePage >= totalScorePages - 1)) {
                if (g.scorePage < totalScorePages - 1) g.scorePage++;
            }
        }
    }
}

void drawAdminTabExamReview(GuiState& g) {
    Rectangle contentRec = { 260, 85, (float)(GUI_WIDTH - 280), (float)(GUI_HEIGHT - 105) };

    DrawTxt("XEM LAI CHI TIET BAI THI CUA SINH VIEN", contentRec.x + 25, contentRec.y + 25, 20, COLOR_TEXT_DARK, true);

    if (g.classCount > 0 && g.selectedClassIdx < g.classCount && g.classes[g.selectedClassIdx] != nullptr) {
        Class* curC = g.classes[g.selectedClassIdx];
        std::vector<Student*> sList;
        collectStudents(curC->studentList, sList);

        DrawTxt("Chon SV:", contentRec.x + 25, contentRec.y + 68, 15, COLOR_TEXT_DARK, true);
        for (size_t i = 0; i < sList.size() && i < 4; ++i) {
            bool sel = (g.selectedStudentIdx == (int)i);
            if (GuiBtn({ contentRec.x + 105 + i * 125, contentRec.y + 62, 115, 32 }, sList[i]->id, sel ? COLOR_PRIMARY : COLOR_SIDEBAR, COLOR_PRIMARY_HOVER, WHITE, 14, sel)) {
                g.selectedStudentIdx = (int)i;
                g.reviewQPage = 0;
            }
        }

        DrawTxt("Mon:", contentRec.x + 630, contentRec.y + 68, 15, COLOR_TEXT_DARK, true);
        // hang mon cung phan trang giong tab bang diem, môn thu 4 tro di nam ngoai man hinh
        int reviewSubMax = 3;
        if (g.reviewSubNavOffset >= (int)g.subjects.size()) g.reviewSubNavOffset = 0;
        if (g.reviewSubNavOffset > 0 && GuiBtnOutline({ contentRec.x + 676, contentRec.y + 62, 30, 32 }, "<", COLOR_PRIMARY, COLOR_PRIMARY, 14, true)) {
            g.reviewSubNavOffset--;
        }
        for (int c = 0; c < reviewSubMax; ++c) {
            size_t idx = g.reviewSubNavOffset + c;
            if (idx >= g.subjects.size()) break;
            bool sel = (g.selectedSubjectIdx == (int)idx);
            if (GuiBtn({ contentRec.x + 714 + c * 84, contentRec.y + 62, 80, 32 }, g.subjects[idx]->id, sel ? COLOR_PRIMARY : COLOR_SIDEBAR, COLOR_PRIMARY_HOVER, WHITE, 14, sel)) {
                g.selectedSubjectIdx = (int)idx;
                g.reviewQPage = 0;
            }
        }
        if (g.reviewSubNavOffset + reviewSubMax < (int)g.subjects.size()) {
            if (GuiBtnOutline({ contentRec.x + 714 + reviewSubMax * 84, contentRec.y + 62, 30, 32 }, ">", COLOR_PRIMARY, COLOR_PRIMARY, 14, true)) {
                g.reviewSubNavOffset++;
            }
        }

        if (!sList.empty() && g.selectedStudentIdx < (int)sList.size() && !g.subjects.empty() && g.selectedSubjectIdx < (int)g.subjects.size()) {
            Student* curStd = sList[g.selectedStudentIdx];
            Subject* curSub = g.subjects[g.selectedSubjectIdx];
            Score* scNode = findScore(curStd, curSub->id);

            if (scNode == nullptr) {
                DrawTxt(TextFormat("Sinh vien %s chua lam bai thi mon %s.", curStd->id, curSub->id), contentRec.x + 25, contentRec.y + 130, 17, COLOR_WARNING, true);
            } else {
                DrawTxt(TextFormat("Sinh vien: %s %s | Mon: %s | Ket qua: %.2f diem", curStd->ho, curStd->ten, curSub->name, scNode->score), contentRec.x + 25, contentRec.y + 115, 17, COLOR_SUCCESS, true);

                std::vector<ExamDetail*> details;
                ExamDetail* dCur = scNode->detailList;
                while (dCur != nullptr) { details.push_back(dCur); dCur = dCur->next; }

                int totalReviewPages = details.empty() ? 1 : ((int)details.size() + REVIEW_PER_PAGE - 1) / REVIEW_PER_PAGE;
                if (g.reviewQPage >= totalReviewPages) g.reviewQPage = totalReviewPages - 1;
                if (g.reviewQPage < 0) g.reviewQPage = 0;

                int startQ = g.reviewQPage * REVIEW_PER_PAGE;
                int endQ = std::min(startQ + REVIEW_PER_PAGE, (int)details.size());
                int startY = (int)contentRec.y + 145;

                for (int q = startQ; q < endQ; ++q) {
                    int offset = q - startQ;
                    Rectangle dCard = { contentRec.x + 25, (float)startY + offset * 145, contentRec.width - 50, 135 };
                    DrawCardPanel(dCard, COLOR_ROW_ALT);

                    ExamDetail* d = details[q];
                    DrawTxt(TextFormat("Cau %d:", q + 1), dCard.x + 15, dCard.y + 12, 16, COLOR_PRIMARY, true);
                    DrawTxtWrapped(d->content, dCard.x + 75, dCard.y + 12, dCard.width - 90, 16, COLOR_TEXT_DARK, true);

                    DrawTxt(TextFormat("A. %s", d->answerA), dCard.x + 25, dCard.y + 42, 14, COLOR_TEXT_MUTED);
                    DrawTxt(TextFormat("B. %s", d->answerB), dCard.x + 25, dCard.y + 68, 14, COLOR_TEXT_MUTED);
                    DrawTxt(TextFormat("C. %s", d->answerC), dCard.x + 450, dCard.y + 42, 14, COLOR_TEXT_MUTED);
                    DrawTxt(TextFormat("D. %s", d->answerD), dCard.x + 450, dCard.y + 68, 14, COLOR_TEXT_MUTED);

                    bool isCorrect = (d->selectedAnswer == d->correctAnswer);
                    DrawTxt(TextFormat("Ban chon: %c | Dap an dung: %c", d->selectedAnswer ? d->selectedAnswer : '-', d->correctAnswer), dCard.x + 25, dCard.y + 100, 15, isCorrect ? COLOR_SUCCESS : COLOR_DANGER, true);
                    DrawTxt(isCorrect ? "[ CHINH XAC ]" : "[ SAI ]", dCard.x + 350, dCard.y + 100, 15, isCorrect ? COLOR_SUCCESS : COLOR_DANGER, true);
                }

                int pageBarY = (int)(contentRec.y + contentRec.height - 45);
                DrawTxt(TextFormat("Trang %d / %d (Tong: %d cau)", g.reviewQPage + 1, totalReviewPages, (int)details.size()), contentRec.x + 40, (float)pageBarY + 6, 15, COLOR_TEXT_MUTED, true);

                if (GuiBtnOutline({ contentRec.x + contentRec.width - 240, (float)pageBarY, 105, 30 }, "< Trang truoc", COLOR_PRIMARY, COLOR_PRIMARY, 13, false, g.reviewQPage == 0)) {
                    if (g.reviewQPage > 0) g.reviewQPage--;
                }
                if (GuiBtnOutline({ contentRec.x + contentRec.width - 125, (float)pageBarY, 105, 30 }, "Trang sau >", COLOR_PRIMARY, COLOR_PRIMARY, 13, false, g.reviewQPage >= totalReviewPages - 1)) {
                    if (g.reviewQPage < totalReviewPages - 1) g.reviewQPage++;
                }
            }
        }
    }
}
