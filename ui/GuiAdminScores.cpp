#include "GuiCommon.h"
#include "GuiState.h"

#include "../business/BusinessQueries.h"

#include "../core/ClassArray.h"
#include "../core/ScoreList.h"
#include "../core/StudentList.h"
#include "../core/SubjectBST.h"

#include <algorithm>
#include <cstdio>
#include <cstring>

void drawAdminTabScores(GuiState& g) {
    Rectangle contentRec = { 260, 85, (float)(GUI_WIDTH - 280), (float)(GUI_HEIGHT - 105) };

    Subject* curSub = (g.reviewSubjectId[0] != '\0') ? findSubject(g.root, g.reviewSubjectId) : nullptr;
    if (g.reviewSubjectId[0] != '\0' && curSub == nullptr) g.reviewSubjectId[0] = '\0';

    Class* curClass = (g.reviewClassId[0] != '\0') ? findClass(g.classes, g.classCount, g.reviewClassId) : nullptr;
    if (g.reviewClassId[0] != '\0' && curClass == nullptr) g.reviewClassId[0] = '\0';

    Student* curStudent = nullptr;
    Score* curScore = nullptr;
    if (curClass != nullptr && g.reviewStudentId[0] != '\0') {
        curStudent = findStudent(curClass->studentList, g.reviewStudentId);
        if (curStudent == nullptr) g.reviewStudentId[0] = '\0';
    }
    if (curStudent != nullptr && curSub != nullptr) {
        curScore = findScore(curStudent, curSub->id);
    }
    if (g.reviewDetailOpen && curScore == nullptr) g.reviewDetailOpen = false;

    if (g.reviewDetailOpen && curStudent != nullptr && curSub != nullptr && curScore != nullptr) {
        DrawTxt(TextFormat("CHI TIET BAI THI MON %s", curSub->id), contentRec.x + 25, contentRec.y + 25, 20, COLOR_TEXT_DARK, true);
        if (GuiBtnOutline({ contentRec.x + contentRec.width - 240, contentRec.y + 20, 210, 36 }, "<- Quay lai bang diem", COLOR_PRIMARY, COLOR_PRIMARY, 14, true)) {
            g.reviewDetailOpen = false;
            g.reviewQPage = 0;
        }

        DrawTxt(TextFormat("Sinh vien: %s %s %s | Lop: %s | Ket qua: %.2f diem", curStudent->ho, curStudent->ten, curStudent->id, curClass->id, curScore->score),
                contentRec.x + 25, contentRec.y + 68, 16, COLOR_SUCCESS, true);
        DrawTxt(TextFormat("Mon: %s - %s", curSub->id, curSub->name), contentRec.x + 25, contentRec.y + 92, 15, COLOR_TEXT_MUTED);

        Arr<ExamDetail*> details;
        for (ExamDetail* d = curScore->detailList; d != nullptr; d = d->next) details.push_back(d);

        int totalReviewPages = details.empty() ? 1 : ((int)details.size() + REVIEW_PER_PAGE - 1) / REVIEW_PER_PAGE;
        if (g.reviewQPage >= totalReviewPages) g.reviewQPage = totalReviewPages - 1;
        if (g.reviewQPage < 0) g.reviewQPage = 0;

        int startQ = g.reviewQPage * REVIEW_PER_PAGE;
        int endQ = std::min(startQ + REVIEW_PER_PAGE, (int)details.size());
        int startY = (int)contentRec.y + 125;

        for (int q = startQ; q < endQ; ++q) {
            int offset = q - startQ;
            Rectangle dCard = { contentRec.x + 25, (float)startY + offset * 145, contentRec.width - 50, 135 };
            DrawCardPanel(dCard, COLOR_ROW_ALT);

            ExamDetail* d = details[q];
            GuiDrawTakenExamCard(d, q, dCard);
        }

        int pageBarY = (int)(contentRec.y + contentRec.height - 45);
        DrawTxt(TextFormat("Trang %d / %d (Tong: %d cau)", g.reviewQPage + 1, totalReviewPages, (int)details.size()), contentRec.x + 40, (float)pageBarY + 6, 15, COLOR_TEXT_MUTED, true);

        if (GuiBtnOutline({ contentRec.x + contentRec.width - 240, (float)pageBarY, 105, 30 }, "< Trang truoc", COLOR_PRIMARY, COLOR_PRIMARY, 13, false, g.reviewQPage == 0)) {
            if (g.reviewQPage > 0) g.reviewQPage--;
        }
        if (GuiBtnOutline({ contentRec.x + contentRec.width - 125, (float)pageBarY, 105, 30 }, "Trang sau >", COLOR_PRIMARY, COLOR_PRIMARY, 13, false, g.reviewQPage >= totalReviewPages - 1)) {
            if (g.reviewQPage < totalReviewPages - 1) g.reviewQPage++;
        }
        return;
    }

    DrawTxt("BANG DIEM & CHI TIET BAI THI", contentRec.x + 25, contentRec.y + 25, 20, COLOR_TEXT_DARK, true);

    DrawTxt("Mon:", contentRec.x + 25, contentRec.y + 68, 15, COLOR_TEXT_DARK, true);
    Rectangle subjectSearchRec = { contentRec.x + 75, contentRec.y + 57, 350, 38 };

    DrawTxt("Lop:", contentRec.x + 460, contentRec.y + 68, 15, COLOR_TEXT_DARK, true);
    Rectangle classSearchRec = { contentRec.x + 505, contentRec.y + 57, 280, 38 };

    if (curSub == nullptr) {
        DrawTxt("Chon mon hoc o thanh tim kiem phia tren de bat dau.", contentRec.x + 25, contentRec.y + 118, 16, COLOR_TEXT_MUTED);
        DrawTxt("Quy trinh: Mon hoc -> Lop hoc -> Sinh vien.", contentRec.x + 25, contentRec.y + 145, 15, COLOR_TEXT_MUTED);
    } else if (curClass == nullptr) {
        DrawTxt(TextFormat("Mon: %s - %s. Chon tiep lop hoc.", curSub->id, curSub->name), contentRec.x + 25, contentRec.y + 118, 16, COLOR_TEXT_MUTED);
    } else {
        Arr<Student*> sList;
        for (Student* s = curClass->studentList; s != nullptr; s = s->next) sList.push_back(s);

        int totalScorePages = sList.empty() ? 1 : ((int)sList.size() + SCORES_PER_PAGE - 1) / SCORES_PER_PAGE;
        if (g.scorePage >= totalScorePages) g.scorePage = totalScorePages - 1;
        if (g.scorePage < 0) g.scorePage = 0;

        DrawTxt(TextFormat("Mon: %s - %s | Lop: %s - %s | Si so: %d", curSub->id, curSub->name, curClass->id, curClass->name, countStudentsInClass(curClass)),
                contentRec.x + 25, contentRec.y + 112, 15, COLOR_SUCCESS, true);

        int tableY = (int)contentRec.y + 140;
        DrawRectangle((int)contentRec.x + 25, tableY, (int)contentRec.width - 50, 40, COLOR_SIDEBAR);
        DrawTxt("STT", contentRec.x + 40, (float)tableY + 11, 15, WHITE, true);
        DrawTxt("MASV", contentRec.x + 120, (float)tableY + 11, 15, WHITE, true);
        DrawTxt("HO VA TEN", contentRec.x + 320, (float)tableY + 11, 15, WHITE, true);
        DrawTxt("DIEM (Thang 10)", contentRec.x + 720, (float)tableY + 11, 15, WHITE, true);
        DrawTxt("THAO TAC", contentRec.x + 880, (float)tableY + 11, 15, WHITE, true);

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

                Student* s = sList[i];
                Score* sc = findScore(s, curSub->id);
                DrawTxt(TextFormat("%d", i + 1), contentRec.x + 45, (float)rowY + 11, 15, COLOR_TEXT_DARK);
                DrawTxt(s->id, contentRec.x + 120, (float)rowY + 11, 15, COLOR_PRIMARY, true);
                DrawTxt(TextFormat("%s %s", s->ho, s->ten), contentRec.x + 320, (float)rowY + 11, 15, COLOR_TEXT_DARK);
                if (sc != nullptr) {
                    DrawTxt(TextFormat("%.2f diem", sc->score), contentRec.x + 720, (float)rowY + 11, 17, COLOR_SUCCESS, true);
                } else {
                    DrawTxt("Chua thi", contentRec.x + 720, (float)rowY + 11, 15, COLOR_TEXT_MUTED);
                }

                if (GuiBtnOutline({ contentRec.x + 880, (float)rowY + 6, 65, 30 }, "Xem", COLOR_PRIMARY, COLOR_PRIMARY, 14, false, sc == nullptr)) {
                    g.reviewStudentId[0] = '\0';
                    snprintf(g.reviewStudentId, sizeof(g.reviewStudentId), "%s", s->id);
                    g.reviewDetailOpen = true;
                    g.reviewQPage = 0;
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

    Arr<SuggestItem> subItems;
    GuiMakeSubjectItems(g.subjects, true, subItems);
    int pickedSub = GuiSearchSuggest(subjectSearchRec, g.searchReviewSubject, sizeof(g.searchReviewSubject),
                                     g.activeSearchReviewSubject, subItems,
                                     "Tim mon hoc...");
    if (pickedSub >= 0 && pickedSub < (int)g.subjects.size()) {
        snprintf(g.reviewSubjectId, sizeof(g.reviewSubjectId), "%s", g.subjects[pickedSub]->id);
        g.scorePage = 0;
        g.reviewDetailOpen = false;
    }

    Arr<SuggestItem> classItems;
    GuiMakeClassItems(g.classes, g.classCount, classItems);
    int pickedClass = GuiSearchSuggest(classSearchRec, g.searchReviewClass, sizeof(g.searchReviewClass),
                                       g.activeSearchReviewClass, classItems,
                                       "Tim lop hoc...");
    if (pickedClass >= 0 && pickedClass < g.classCount && g.classes[pickedClass] != nullptr) {
        snprintf(g.reviewClassId, sizeof(g.reviewClassId), "%s", g.classes[pickedClass]->id);
        g.scorePage = 0;
        g.reviewDetailOpen = false;
    }
}
