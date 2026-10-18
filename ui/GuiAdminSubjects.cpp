// tab mon hoc (BST, duyet in-order)
#include "GuiCommon.h"
#include "GuiState.h"

#include "../core/QuestionList.h"   // countQuestions

#include <algorithm>
#include <cstdio>

// mon hoc hien thi theo thu tu duyet cay (LNR)

void drawAdminTabSubjects(GuiState& g) {
    Rectangle contentRec = { 260, 85, (float)(GUI_WIDTH - 280), (float)(GUI_HEIGHT - 105) };

    DrawTxt("QUAN LY DANH MUC MON HOC (CAY NHI PHAN TIM KIEM BST)", contentRec.x + 25, contentRec.y + 25, 20, COLOR_TEXT_DARK, true);
    if (GuiBtn({ contentRec.x + contentRec.width - 180, contentRec.y + 20, 155, 38 }, "+ Them Mon Hoc", COLOR_SUCCESS, COLOR_SUCCESS_HOVER, WHITE, 15, true)) {
        g.currentModal = MODAL_ADD_SUBJECT;
        g.formId[0] = '\0';
        g.formName[0] = '\0';
        g.modalError[0] = '\0';
        g.activeFormBox = 1;
    }

    int totalSubPages = g.subjects.empty() ? 1 : ((int)g.subjects.size() + SUBJECTS_PER_PAGE - 1) / SUBJECTS_PER_PAGE;
    if (g.subjectPage >= totalSubPages) g.subjectPage = totalSubPages - 1;
    if (g.subjectPage < 0) g.subjectPage = 0;

    int tableY = (int)contentRec.y + 75;
    DrawRectangle((int)contentRec.x + 25, tableY, (int)contentRec.width - 50, 40, COLOR_SIDEBAR);
    DrawTxt("STT", contentRec.x + 40, (float)tableY + 11, 15, WHITE, true);
    DrawTxt("MA MON HOC (Key BST)", contentRec.x + 100, (float)tableY + 11, 15, WHITE, true);
    DrawTxt("TEN MON HOC", contentRec.x + 350, (float)tableY + 11, 15, WHITE, true);
    DrawTxt("SO CAU HOI", contentRec.x + 680, (float)tableY + 11, 15, WHITE, true);
    DrawTxt("THAO TAC", contentRec.x + 820, (float)tableY + 11, 15, WHITE, true);

    if (g.subjects.empty()) {
        DrawTxt("Chua co mon hoc nao. Hay bam '+ Them Mon Hoc' de them mon dau tien.", contentRec.x + 40, (float)tableY + 60, 16, COLOR_TEXT_MUTED);
    } else {
        int startIdx = g.subjectPage * SUBJECTS_PER_PAGE;
        int endIdx = std::min(startIdx + SUBJECTS_PER_PAGE, (int)g.subjects.size());

        for (int i = startIdx; i < endIdx; ++i) {
            int rowOffset = i - startIdx;
            int rowY = tableY + 45 + rowOffset * 45;
            Color rowBg = (rowOffset % 2 == 0) ? COLOR_CARD : COLOR_ROW_ALT;
            DrawRectangle((int)contentRec.x + 25, rowY, (int)contentRec.width - 50, 42, rowBg);
            DrawRectangleLines((int)contentRec.x + 25, rowY, (int)contentRec.width - 50, 42, COLOR_BORDER);

            DrawTxt(TextFormat("%d", i + 1), contentRec.x + 45, (float)rowY + 11, 15, COLOR_TEXT_DARK);
            DrawTxt(g.subjects[i]->id, contentRec.x + 100, (float)rowY + 11, 15, COLOR_PRIMARY, true);
            DrawTxt(g.subjects[i]->name, contentRec.x + 350, (float)rowY + 11, 15, COLOR_TEXT_DARK);

            int qCount = countQuestions(g.subjects[i]->questionList);
            DrawTxt(TextFormat("%d cau", qCount), contentRec.x + 690, (float)rowY + 11, 15, COLOR_TEXT_MUTED);

            if (GuiBtnOutline({ contentRec.x + 810, (float)rowY + 6, 65, 30 }, "Sua", COLOR_PRIMARY, COLOR_PRIMARY, 14)) {
                g.currentModal = MODAL_EDIT_SUBJECT;
                snprintf(g.formId, sizeof(g.formId), "%s", g.subjects[i]->id);
                snprintf(g.formName, sizeof(g.formName), "%s", g.subjects[i]->name);
                g.modalError[0] = '\0';
                g.activeFormBox = 2;
            }
            if (GuiBtnOutline({ contentRec.x + 885, (float)rowY + 6, 65, 30 }, "Xoa", COLOR_DANGER, COLOR_DANGER, 14)) {
                g.currentModal = MODAL_DELETE_SUBJECT;
                snprintf(g.formId, sizeof(g.formId), "%s", g.subjects[i]->id);
                snprintf(g.formName, sizeof(g.formName), "%s", g.subjects[i]->name);
                g.modalError[0] = '\0';
            }
        }

        int pageBarY = (int)(contentRec.y + contentRec.height - 50);
        DrawTxt(TextFormat("Trang %d / %d  (Tong: %d mon)", g.subjectPage + 1, totalSubPages, (int)g.subjects.size()), contentRec.x + 40, (float)pageBarY + 8, 15, COLOR_TEXT_MUTED, true);

        if (GuiBtnOutline({ contentRec.x + contentRec.width - 240, (float)pageBarY, 105, 32 }, "< Trang truoc", COLOR_PRIMARY, COLOR_PRIMARY, 13, false, g.subjectPage == 0)) {
            if (g.subjectPage > 0) g.subjectPage--;
        }
        if (GuiBtnOutline({ contentRec.x + contentRec.width - 125, (float)pageBarY, 105, 32 }, "Trang sau >", COLOR_PRIMARY, COLOR_PRIMARY, 13, false, g.subjectPage >= totalSubPages - 1)) {
            if (g.subjectPage < totalSubPages - 1) g.subjectPage++;
        }
    }
}
