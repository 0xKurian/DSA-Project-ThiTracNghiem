// tab sinh vien theo lop (DSLK don)
#include "GuiCommon.h"
#include "GuiState.h"

#include <algorithm>
#include <cstdio>

// chon lop truoc (toi da 4 nut, mo rong bang < >); thao tac sinh vien go modal

void drawAdminTabStudents(GuiState& g) {
    Rectangle contentRec = { 260, 85, (float)(GUI_WIDTH - 280), (float)(GUI_HEIGHT - 105) };

    DrawTxt("QUAN LY SINH VIEN THEO LOP (DANH SACH LIEN KET)", contentRec.x + 25, contentRec.y + 25, 20, COLOR_TEXT_DARK, true);

    DrawTxt("Chon Lop:", contentRec.x + 25, contentRec.y + 68, 15, COLOR_TEXT_DARK, true);
    if (g.classCount == 0) {
        DrawTxt("Chua co lop hoc nao de quan ly sinh vien.", contentRec.x + 120, contentRec.y + 68, 15, COLOR_TEXT_MUTED);
    } else {
        int maxVis = 4;
        if (g.teacherClassNavOffset > 0) {
            if (GuiBtnOutline({ contentRec.x + 110, contentRec.y + 62, 35, 32 }, "<", COLOR_PRIMARY, COLOR_PRIMARY, 14, true)) {
                g.teacherClassNavOffset--;
            }
        }

        for (int c = 0; c < maxVis; ++c) {
            int idx = g.teacherClassNavOffset + c;
            if (idx >= g.classCount || g.classes[idx] == nullptr) break;
            bool sel = (g.selectedClassIdx == idx);
            Rectangle btnRec = { contentRec.x + 155 + c * 135, contentRec.y + 62, 125, 32 };
            if (GuiBtn(btnRec, g.classes[idx]->id, sel ? COLOR_PRIMARY : COLOR_SIDEBAR, COLOR_PRIMARY_HOVER, WHITE, 14, sel)) {
                g.selectedClassIdx = idx;
                g.studentPage = 0;
            }
        }

        if (g.teacherClassNavOffset + maxVis < g.classCount) {
            if (GuiBtnOutline({ contentRec.x + 155 + maxVis * 135, contentRec.y + 62, 35, 32 }, ">", COLOR_PRIMARY, COLOR_PRIMARY, 14, true)) {
                g.teacherClassNavOffset++;
            }
        }
    }

    if (g.classCount > 0 && g.selectedClassIdx < g.classCount && g.classes[g.selectedClassIdx] != nullptr) {
        Class* curClass = g.classes[g.selectedClassIdx];
        if (GuiBtn({ contentRec.x + contentRec.width - 180, contentRec.y + 62, 155, 34 }, "+ Them Sinh Vien", COLOR_SUCCESS, COLOR_SUCCESS_HOVER, WHITE, 14, true)) {
            g.currentModal = MODAL_ADD_STUDENT;
            g.formId[0] = '\0';
            g.formHo[0] = '\0';
            g.formTen[0] = '\0';
            snprintf(g.formGender, sizeof(g.formGender), "Nam");
            snprintf(g.formPass, sizeof(g.formPass), "123");
            g.modalError[0] = '\0';
            g.activeFormBox = 1;
        }

        std::vector<Student*> sList;
        collectStudents(curClass->studentList, sList);

        int totalStdPages = sList.empty() ? 1 : ((int)sList.size() + STUDENTS_PER_PAGE - 1) / STUDENTS_PER_PAGE;
        if (g.studentPage >= totalStdPages) g.studentPage = totalStdPages - 1;
        if (g.studentPage < 0) g.studentPage = 0;

        int tableY = (int)contentRec.y + 115;
        DrawRectangle((int)contentRec.x + 25, tableY, (int)contentRec.width - 50, 40, COLOR_SIDEBAR);
        DrawTxt("STT", contentRec.x + 40, (float)tableY + 11, 15, WHITE, true);
        DrawTxt("MASV", contentRec.x + 100, (float)tableY + 11, 15, WHITE, true);
        DrawTxt("HO VA TEN", contentRec.x + 250, (float)tableY + 11, 15, WHITE, true);
        DrawTxt("PHAI", contentRec.x + 550, (float)tableY + 11, 15, WHITE, true);
        DrawTxt("MAT KHAU", contentRec.x + 680, (float)tableY + 11, 15, WHITE, true);
        DrawTxt("THAO TAC", contentRec.x + 820, (float)tableY + 11, 15, WHITE, true);

        if (sList.empty()) {
            DrawTxt("Lop nay hien chua co sinh vien nao. Hay bam '+ Them Sinh Vien' de them.", contentRec.x + 40, (float)tableY + 60, 16, COLOR_TEXT_MUTED);
        } else {
            int startIdx = g.studentPage * STUDENTS_PER_PAGE;
            int endIdx = std::min(startIdx + STUDENTS_PER_PAGE, (int)sList.size());

            for (int i = startIdx; i < endIdx; ++i) {
                int rowOffset = i - startIdx;
                int rowY = tableY + 45 + rowOffset * 45;
                Color rowBg = (rowOffset % 2 == 0) ? COLOR_CARD : COLOR_ROW_ALT;
                DrawRectangle((int)contentRec.x + 25, rowY, (int)contentRec.width - 50, 42, rowBg);
                DrawRectangleLines((int)contentRec.x + 25, rowY, (int)contentRec.width - 50, 42, COLOR_BORDER);

                DrawTxt(TextFormat("%d", i + 1), contentRec.x + 45, (float)rowY + 11, 15, COLOR_TEXT_DARK);
                DrawTxt(sList[i]->id, contentRec.x + 100, (float)rowY + 11, 15, COLOR_PRIMARY, true);
                DrawTxt(TextFormat("%s %s", sList[i]->ho, sList[i]->ten), contentRec.x + 250, (float)rowY + 11, 15, COLOR_TEXT_DARK);
                DrawTxt(sList[i]->gender, contentRec.x + 550, (float)rowY + 11, 15, COLOR_TEXT_MUTED);
                DrawTxt("******", contentRec.x + 680, (float)rowY + 11, 15, COLOR_TEXT_MUTED);

                if (GuiBtnOutline({ contentRec.x + 810, (float)rowY + 6, 65, 30 }, "Sua", COLOR_PRIMARY, COLOR_PRIMARY, 14)) {
                    g.currentModal = MODAL_EDIT_STUDENT;
                    snprintf(g.formId, sizeof(g.formId), "%s", sList[i]->id);
                    snprintf(g.formHo, sizeof(g.formHo), "%s", sList[i]->ho);
                    snprintf(g.formTen, sizeof(g.formTen), "%s", sList[i]->ten);
                    snprintf(g.formGender, sizeof(g.formGender), "%s", sList[i]->gender);
                    snprintf(g.formPass, sizeof(g.formPass), "%s", sList[i]->password);
                    g.modalError[0] = '\0';
                    g.activeFormBox = 2; // Khong sua ID
                }
                if (GuiBtnOutline({ contentRec.x + 885, (float)rowY + 6, 65, 30 }, "Xoa", COLOR_DANGER, COLOR_DANGER, 14)) {
                    g.currentModal = MODAL_DELETE_STUDENT;
                    snprintf(g.formId, sizeof(g.formId), "%s", sList[i]->id);
                    g.modalError[0] = '\0';
                }
            }

            int pageBarY = (int)(contentRec.y + contentRec.height - 50);
            DrawTxt(TextFormat("Trang %d / %d (Tong: %d sinh vien)", g.studentPage + 1, totalStdPages, (int)sList.size()), contentRec.x + 40, (float)pageBarY + 8, 15, COLOR_TEXT_MUTED, true);

            if (GuiBtnOutline({ contentRec.x + contentRec.width - 240, (float)pageBarY, 105, 32 }, "< Trang truoc", COLOR_PRIMARY, COLOR_PRIMARY, 13, false, g.studentPage == 0)) {
                if (g.studentPage > 0) g.studentPage--;
            }
            if (GuiBtnOutline({ contentRec.x + contentRec.width - 125, (float)pageBarY, 105, 32 }, "Trang sau >", COLOR_PRIMARY, COLOR_PRIMARY, 13, false, g.studentPage >= totalStdPages - 1)) {
                if (g.studentPage < totalStdPages - 1) g.studentPage++;
            }
        }
    }
}
