// dashboard sv: chuan bi thi / diem ca nhan / xem lai bai thi
#include "GuiCommon.h"
#include "GuiState.h"

#include "../core/SubjectBST.h"   // findSubject (hien ten mon trong bang diem)

#include <algorithm>
#include <cstdio>


void drawStudentHome(GuiState& g) {
    DrawRectangle(0, 0, GUI_WIDTH, 65, COLOR_SIDEBAR);
    DrawTxt(TextFormat("SINH VIEN: %s - %s %s", g.loggedStudent->id, g.loggedStudent->ho, g.loggedStudent->ten), 30, 20, 22, WHITE, true);

    if (GuiBtn({ (float)(GUI_WIDTH - 130), 15, 100, 35 }, "Dang xuat", COLOR_DANGER, COLOR_DANGER_HOVER, WHITE, 15, true)) {
        g.currentScreen = SCREEN_LOGIN;
        g.currentRole = LOGIN_FAILED;
        g.loggedStudent = nullptr;
    }

    DrawRectangle(0, 65, 240, GUI_HEIGHT - 65, COLOR_SIDEBAR);
    const char* stdTabs[] = {
        "Vao thi trac nghiem",
        "Bang diem ca nhan",
        "Xem lai bai thi"
    };

    for (int i = 0; i < 3; ++i) {
        Rectangle tabRec = { 10, (float)(85 + i * 55), 220, 45 };
        bool active = (g.studentTab == i);
        Color tabBg = active ? COLOR_SIDEBAR_ACTIVE : COLOR_SIDEBAR;
        if (GuiBtn(tabRec, stdTabs[i], tabBg, COLOR_SIDEBAR_HOVER, WHITE, 15, active)) {
            g.studentTab = i;
        }
    }

    Rectangle contentRec = { 260, 85, (float)(GUI_WIDTH - 280), (float)(GUI_HEIGHT - 105) };
    DrawCardPanel(contentRec);

    switch (g.studentTab) {
    case 0:
        drawExamPrepareTab(g);
        break;
    case 1:
        drawStudentTabScores(g);
        break;
    default:
        drawStudentTabReview(g);
        break;
    }
}

void drawStudentTabScores(GuiState& g) {
    Rectangle contentRec = { 260, 85, (float)(GUI_WIDTH - 280), (float)(GUI_HEIGHT - 105) };

    DrawTxt("KET QUA THI CAC MON HOC CUA BAN", contentRec.x + 30, contentRec.y + 30, 22, COLOR_TEXT_DARK, true);

    int tableY = (int)contentRec.y + 80;
    DrawRectangle((int)contentRec.x + 30, tableY, (int)contentRec.width - 60, 40, COLOR_SIDEBAR);
    DrawTxt("MA MON HOC", contentRec.x + 50, (float)tableY + 11, 15, WHITE, true);
    DrawTxt("TEN MON HOC", contentRec.x + 250, (float)tableY + 11, 15, WHITE, true);
    DrawTxt("DIEM THI (Thang 10)", contentRec.x + 700, (float)tableY + 11, 15, WHITE, true);

    Score* sc = g.loggedStudent->scoreList;
    int idx = 0;
    while (sc != nullptr) {
        int rowY = tableY + 45 + idx * 45;
        Color rowBg = (idx % 2 == 0) ? COLOR_CARD : COLOR_ROW_ALT;
        DrawRectangle((int)contentRec.x + 30, rowY, (int)contentRec.width - 60, 42, rowBg);
        DrawRectangleLines((int)contentRec.x + 30, rowY, (int)contentRec.width - 60, 42, COLOR_BORDER);

        DrawTxt(sc->subjectId, contentRec.x + 50, (float)rowY + 11, 15, COLOR_PRIMARY, true);
        Subject* sNode = findSubject(g.root, sc->subjectId);
        DrawTxt(sNode ? sNode->name : "Mon hoc", contentRec.x + 250, (float)rowY + 11, 15, COLOR_TEXT_DARK);
        DrawTxt(TextFormat("%.2f diem", sc->score), contentRec.x + 700, (float)rowY + 11, 17, COLOR_SUCCESS, true);

        sc = sc->next;
        idx++;
    }

    if (idx == 0) {
        DrawTxt("Ban chua co diem mon nao trong he thong. Hay vao tab 'Vao thi' de bat dau.", contentRec.x + 50, (float)tableY + 60, 16, COLOR_TEXT_MUTED);
    }
}

void drawStudentTabReview(GuiState& g) {
    Rectangle contentRec = { 260, 85, (float)(GUI_WIDTH - 280), (float)(GUI_HEIGHT - 105) };

    DrawTxt("XEM LAI BAI THI DA HOAN THANH", contentRec.x + 30, contentRec.y + 30, 22, COLOR_TEXT_DARK, true);

    Score* sc = g.loggedStudent->scoreList;
    int sIdx = 0;
    DrawTxt("Chon Mon:", contentRec.x + 30, contentRec.y + 70, 15, COLOR_TEXT_DARK, true);
    while (sc != nullptr && sIdx < 4) {
        bool sel = (g.studentSubChoice == sIdx);
        if (GuiBtn({ contentRec.x + 120 + sIdx * 120, contentRec.y + 65, 110, 32 }, sc->subjectId, sel ? COLOR_PRIMARY : COLOR_SIDEBAR, COLOR_PRIMARY_HOVER, WHITE, 14, sel)) {
            g.studentSubChoice = sIdx;
            g.reviewQPage = 0;
        }
        sc = sc->next;
        sIdx++;
    }

    if (sIdx == 0) {
        DrawTxt("Ban chua co bai thi nao da hoan thanh de xem lai.", contentRec.x + 30, contentRec.y + 120, 16, COLOR_TEXT_MUTED);
    } else {
        sc = g.loggedStudent->scoreList;
        int curIdx = 0;
        while (sc != nullptr && curIdx < g.studentSubChoice) {
            sc = sc->next;
            curIdx++;
        }

        if (sc != nullptr) {
            DrawTxt(TextFormat("Ket qua: %.2f / 10.0 diem", sc->score), contentRec.x + 30, contentRec.y + 115, 17, COLOR_SUCCESS, true);

            std::vector<ExamDetail*> details;
            ExamDetail* dCur = sc->detailList;
            while (dCur != nullptr) { details.push_back(dCur); dCur = dCur->next; }

            int totalReviewPages = details.empty() ? 1 : ((int)details.size() + REVIEW_PER_PAGE - 1) / REVIEW_PER_PAGE;
            if (g.reviewQPage >= totalReviewPages) g.reviewQPage = totalReviewPages - 1;
            if (g.reviewQPage < 0) g.reviewQPage = 0;

            int startQ = g.reviewQPage * REVIEW_PER_PAGE;
            int endQ = std::min(startQ + REVIEW_PER_PAGE, (int)details.size());
            int startY = (int)contentRec.y + 145;

            for (int q = startQ; q < endQ; ++q) {
                int offset = q - startQ;
                Rectangle dCard = { contentRec.x + 30, (float)startY + offset * 145, contentRec.width - 60, 135 };
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
