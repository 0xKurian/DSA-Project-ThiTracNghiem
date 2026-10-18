// tab lop hoc (mang con tro classes[MAX_CLASS])
#include "GuiCommon.h"
#include "GuiState.h"

#include <algorithm>
#include <cstdio>

// xoa chi duoc khi lop chua co sinh vien (dd cua add/delete nam trong ClassService)

void drawAdminTabClasses(GuiState& g) {
    Rectangle contentRec = { 260, 85, (float)(GUI_WIDTH - 280), (float)(GUI_HEIGHT - 105) };

    DrawTxt("QUAN LY DANH MUC LOP HOC (MANG CON TRO)", contentRec.x + 25, contentRec.y + 25, 20, COLOR_TEXT_DARK, true);
    if (GuiBtn({ contentRec.x + contentRec.width - 180, contentRec.y + 20, 155, 38 }, "+ Them Lop Hoc", COLOR_SUCCESS, COLOR_SUCCESS_HOVER, WHITE, 15, true)) {
        g.currentModal = MODAL_ADD_CLASS;
        g.formId[0] = '\0';
        g.formName[0] = '\0';
        g.modalError[0] = '\0';
        g.activeFormBox = 1;
    }

    int totalClassPages = (g.classCount == 0) ? 1 : (g.classCount + CLASSES_PER_PAGE - 1) / CLASSES_PER_PAGE;
    if (g.classPage >= totalClassPages) g.classPage = totalClassPages - 1;
    if (g.classPage < 0) g.classPage = 0;

    int tableY = (int)contentRec.y + 75;
    DrawRectangle((int)contentRec.x + 25, tableY, (int)contentRec.width - 50, 40, COLOR_SIDEBAR);
    DrawTxt("STT", contentRec.x + 40, (float)tableY + 11, 15, WHITE, true);
    DrawTxt("MA LOP", contentRec.x + 100, (float)tableY + 11, 15, WHITE, true);
    DrawTxt("TEN LOP HOC", contentRec.x + 350, (float)tableY + 11, 15, WHITE, true);
    DrawTxt("SI SO", contentRec.x + 680, (float)tableY + 11, 15, WHITE, true);
    DrawTxt("THAO TAC", contentRec.x + 820, (float)tableY + 11, 15, WHITE, true);

    if (g.classCount == 0) {
        DrawTxt("Chua co lop hoc nao trong he thong. Hay bam '+ Them Lop Hoc' de them moi.", contentRec.x + 40, (float)tableY + 60, 16, COLOR_TEXT_MUTED);
    } else {
        int startIdx = g.classPage * CLASSES_PER_PAGE;
        int endIdx = std::min(startIdx + CLASSES_PER_PAGE, g.classCount);

        for (int i = startIdx; i < endIdx; ++i) {
            if (g.classes[i] == nullptr) continue;
            int rowOffset = i - startIdx;
            int rowY = tableY + 45 + rowOffset * 45;
            Color rowBg = (rowOffset % 2 == 0) ? COLOR_CARD : COLOR_ROW_ALT;
            DrawRectangle((int)contentRec.x + 25, rowY, (int)contentRec.width - 50, 42, rowBg);
            DrawRectangleLines((int)contentRec.x + 25, rowY, (int)contentRec.width - 50, 42, COLOR_BORDER);

            DrawTxt(TextFormat("%d", i + 1), contentRec.x + 45, (float)rowY + 11, 15, COLOR_TEXT_DARK);
            DrawTxt(g.classes[i]->id, contentRec.x + 100, (float)rowY + 11, 15, COLOR_PRIMARY, true);
            DrawTxt(g.classes[i]->name, contentRec.x + 350, (float)rowY + 11, 15, COLOR_TEXT_DARK);

                    int count = 0;
            Student* s = g.classes[i]->studentList;
            while (s != nullptr) { count++; s = s->next; }
            DrawTxt(TextFormat("%d SV", count), contentRec.x + 690, (float)rowY + 11, 15, COLOR_TEXT_MUTED);

            if (GuiBtnOutline({ contentRec.x + 810, (float)rowY + 6, 75, 30 }, "Xoa", COLOR_DANGER, COLOR_DANGER, 14)) {
                g.currentModal = MODAL_DELETE_CLASS;
                snprintf(g.formId, sizeof(g.formId), "%s", g.classes[i]->id);
                snprintf(g.formName, sizeof(g.formName), "%s", g.classes[i]->name);
                g.modalError[0] = '\0';
            }
        }

        int pageBarY = (int)(contentRec.y + contentRec.height - 50);
        DrawTxt(TextFormat("Trang %d / %d (Tong: %d lop hoc)", g.classPage + 1, totalClassPages, g.classCount), contentRec.x + 40, (float)pageBarY + 8, 15, COLOR_TEXT_MUTED, true);

        if (GuiBtnOutline({ contentRec.x + contentRec.width - 240, (float)pageBarY, 105, 32 }, "< Trang truoc", COLOR_PRIMARY, COLOR_PRIMARY, 13, false, g.classPage == 0)) {
            if (g.classPage > 0) g.classPage--;
        }
        if (GuiBtnOutline({ contentRec.x + contentRec.width - 125, (float)pageBarY, 105, 32 }, "Trang sau >", COLOR_PRIMARY, COLOR_PRIMARY, 13, false, g.classPage >= totalClassPages - 1)) {
            if (g.classPage < totalClassPages - 1) g.classPage++;
        }
    }
}
