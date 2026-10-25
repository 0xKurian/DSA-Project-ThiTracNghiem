#include "GuiCommon.h"
#include "GuiState.h"

#include <algorithm>
#include <cstdio>

void drawAdminTabQuestions(GuiState& g) {
    Rectangle contentRec = { 260, 85, (float)(GUI_WIDTH - 280), (float)(GUI_HEIGHT - 105) };

    DrawTxt("NGAN HANG CAU HOI THI TRAC NGHIEM", contentRec.x + 25, contentRec.y + 25, 20, COLOR_TEXT_DARK, true);

    DrawTxt("Chon Mon Hoc:", contentRec.x + 25, contentRec.y + 68, 15, COLOR_TEXT_DARK, true);
    Rectangle questionSearchRec = { contentRec.x + 150, contentRec.y + 57, 380, 40 };
    if (g.subjects.empty()) {
        DrawTxt("Chua co mon hoc nao de tao cau hoi.", contentRec.x + 150, contentRec.y + 68, 15, COLOR_TEXT_MUTED);
    }

    if (GuiBtnOutline({ contentRec.x + contentRec.width - 350, contentRec.y + 62, 155, 34 }, "<- Quay lai", COLOR_PRIMARY, COLOR_PRIMARY, 14, true)) {
        g.teacherTab = 0;
    }

    if (!g.subjects.empty() && g.selectedSubjectIdx < (int)g.subjects.size()) {
        Subject* curSub = g.subjects[g.selectedSubjectIdx];
        if (GuiBtn({ contentRec.x + contentRec.width - 180, contentRec.y + 62, 155, 34 }, "+ Them Cau Hoi", COLOR_SUCCESS, COLOR_SUCCESS_HOVER, WHITE, 14, true)) {
            g.currentModal = MODAL_ADD_QUESTION;
            g.formContent[0] = '\0';
            g.formA[0] = '\0';
            g.formB[0] = '\0';
            g.formC[0] = '\0';
            g.formD[0] = '\0';
            g.formCorrect[0] = 'A';
            g.modalError[0] = '\0';
            g.activeFormBox = 1;
        }

        Arr<Question*> qList;
        collectQuestions(curSub->questionList, qList);

        int totalQPages = qList.empty() ? 1 : ((int)qList.size() + QUESTIONS_PER_PAGE - 1) / QUESTIONS_PER_PAGE;
        if (g.questionPage >= totalQPages) g.questionPage = totalQPages - 1;
        if (g.questionPage < 0) g.questionPage = 0;

        DrawTxt(TextFormat("Mon: %s - Tong so: %d cau hoi", curSub->id, (int)qList.size()), contentRec.x + 25, contentRec.y + 110, 15, COLOR_TEXT_MUTED, true);

        if (qList.empty()) {
            DrawTxt("Mon hoc nay chua co cau hoi thi nao. Hay bam '+ Them Cau Hoi' de bo sung.", contentRec.x + 30, contentRec.y + 150, 16, COLOR_TEXT_MUTED);
        } else {
            int startQ = g.questionPage * QUESTIONS_PER_PAGE;
            int endQ = std::min(startQ + QUESTIONS_PER_PAGE, (int)qList.size());
            int startY = (int)contentRec.y + 135;

            for (int i = startQ; i < endQ; ++i) {
                int offset = i - startQ;
                Rectangle qCard = { contentRec.x + 25, (float)startY + offset * 155, contentRec.width - 50, 145 };
                DrawCardPanel(qCard, COLOR_ROW_ALT);

                DrawTxt(TextFormat("ID %d:", qList[i]->id), qCard.x + 15, qCard.y + 12, 16, COLOR_PRIMARY, true);
                DrawTxtWrapped(qList[i]->content, qCard.x + 75, qCard.y + 12, qCard.width - 160, 16, COLOR_TEXT_DARK, true);

                DrawTxt(TextFormat("A. %s", qList[i]->answerA), qCard.x + 25, qCard.y + 45, 14, (qList[i]->correctAnswer == 'A') ? COLOR_SUCCESS : COLOR_TEXT_MUTED);
                DrawTxt(TextFormat("B. %s", qList[i]->answerB), qCard.x + 25, qCard.y + 70, 14, (qList[i]->correctAnswer == 'B') ? COLOR_SUCCESS : COLOR_TEXT_MUTED);
                DrawTxt(TextFormat("C. %s", qList[i]->answerC), qCard.x + 450, qCard.y + 45, 14, (qList[i]->correctAnswer == 'C') ? COLOR_SUCCESS : COLOR_TEXT_MUTED);
                DrawTxt(TextFormat("D. %s", qList[i]->answerD), qCard.x + 450, qCard.y + 70, 14, (qList[i]->correctAnswer == 'D') ? COLOR_SUCCESS : COLOR_TEXT_MUTED);

                DrawTxt(TextFormat("Dap an dung: %c", qList[i]->correctAnswer), qCard.x + 25, qCard.y + 105, 15, COLOR_SUCCESS, true);

                if (GuiBtnOutline({ qCard.x + qCard.width - 150, qCard.y + 100, 65, 30 }, "Sua", COLOR_PRIMARY, COLOR_PRIMARY, 14)) {
                    g.currentModal = MODAL_EDIT_QUESTION;
                    g.formEditQId = qList[i]->id;
                    snprintf(g.formContent, sizeof(g.formContent), "%s", qList[i]->content);
                    snprintf(g.formA, sizeof(g.formA), "%s", qList[i]->answerA);
                    snprintf(g.formB, sizeof(g.formB), "%s", qList[i]->answerB);
                    snprintf(g.formC, sizeof(g.formC), "%s", qList[i]->answerC);
                    snprintf(g.formD, sizeof(g.formD), "%s", qList[i]->answerD);
                    g.formCorrect[0] = qList[i]->correctAnswer;
                    g.modalError[0] = '\0';
                    g.activeFormBox = 1;
                }
                if (GuiBtnOutline({ qCard.x + qCard.width - 75, qCard.y + 100, 65, 30 }, "Xoa", COLOR_DANGER, COLOR_DANGER, 14)) {
                    g.currentModal = MODAL_DELETE_QUESTION;
                    g.formEditQId = qList[i]->id;
                    g.modalError[0] = '\0';
                }
            }

            int pageBarY = (int)(contentRec.y + contentRec.height - 45);
            DrawTxt(TextFormat("Trang %d / %d (Tong: %d cau hoi)", g.questionPage + 1, totalQPages, (int)qList.size()), contentRec.x + 40, (float)pageBarY + 6, 15, COLOR_TEXT_MUTED, true);

            if (GuiBtnOutline({ contentRec.x + contentRec.width - 240, (float)pageBarY, 105, 30 }, "< Trang truoc", COLOR_PRIMARY, COLOR_PRIMARY, 13, false, g.questionPage == 0)) {
                if (g.questionPage > 0) g.questionPage--;
            }
            if (GuiBtnOutline({ contentRec.x + contentRec.width - 125, (float)pageBarY, 105, 30 }, "Trang sau >", COLOR_PRIMARY, COLOR_PRIMARY, 13, false, g.questionPage >= totalQPages - 1)) {
                if (g.questionPage < totalQPages - 1) g.questionPage++;
            }
        }
    }

    if (!g.subjects.empty()) {
        Arr<SuggestItem> items;
        GuiMakeSubjectItems(g.subjects, true, items);
        int picked = GuiSearchSuggest(questionSearchRec, g.searchQuestion, sizeof(g.searchQuestion),
                                      g.activeSearchQuestion, items,
                                      "Tim theo ma hoac ten mon...");
        if (picked >= 0) {
            g.selectedSubjectIdx = picked;
            g.questionPage = 0;
        }
    }
}
