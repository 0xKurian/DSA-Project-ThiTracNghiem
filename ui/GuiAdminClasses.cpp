// tab lop hoc + sinh vien theo lop: master-detail 2 cot (mang con tro classes[])
#include "GuiCommon.h"
#include "GuiState.h"

#include <algorithm>
#include <cstdio>
#include <cstring>

// cot trai: danh muc lop (loc + phan trang), cot phai: sinh vien cua lop dang chon
// hoac toan bo sinh vien khi bat "Tat ca cac lop"

static int countStudents(Class* classroom) {
    int n = 0;
    for (Student* s = classroom != nullptr ? classroom->studentList : nullptr; s != nullptr; s = s->next) n++;
    return n;
}

static bool studentMatches(Student* s, const char* query) {
    if (GuiMatchText(s->id, query)) return true;
    char fullName[HO_LEN + TEN_LEN + 2];
    snprintf(fullName, sizeof(fullName), "%s %s", s->ho, s->ten);
    return GuiMatchText(fullName, query);
}

struct StudentRowRef {
    Class* classroom;
    Student* student;
};

void drawAdminTabClasses(GuiState& g) {
    Rectangle contentRec = { 260, 85, (float)(GUI_WIDTH - 280), (float)(GUI_HEIGHT - 105) };
    const float leftW = 330;
    const float gap = 10;
    Rectangle leftPane = { contentRec.x + 20, contentRec.y + 20, leftW, contentRec.height - 40 };
    Rectangle rightPane = { leftPane.x + leftW + gap, contentRec.y + 20, contentRec.width - 40 - leftW - gap, contentRec.height - 40 };

    if (g.selectedClassIdx >= g.classCount) g.selectedClassIdx = g.classCount - 1;
    if (g.selectedClassIdx < 0 && g.classCount > 0) g.selectedClassIdx = 0;

    DrawCardPanel(leftPane);
    DrawCardPanel(rightPane);

    // ================= COT TRAI: DANH MUC LOP =================
    DrawTxt("DANH MUC LOP", leftPane.x + 12, leftPane.y + 14, 16, COLOR_TEXT_DARK, true);
    if (GuiBtn({ leftPane.x + 10, leftPane.y + 40, leftPane.width - 20, 34 }, "+ Them Lop Hoc", COLOR_SUCCESS, COLOR_SUCCESS_HOVER, WHITE, 14, true)) {
        g.currentModal = MODAL_ADD_CLASS;
        g.formId[0] = '\0';
        g.formName[0] = '\0';
        g.modalError[0] = '\0';
        g.activeFormBox = 1;
    }

    Rectangle classSearchRec = { leftPane.x + 10, leftPane.y + 82, leftPane.width - 20, 38 };
    size_t classLenBefore = strlen(g.searchClass);
    GuiTextBoxInput(classSearchRec, g.searchClass, sizeof(g.searchClass), g.activeSearchClass, "Tim theo ma hoac ten lop...");
    if (strlen(g.searchClass) != classLenBefore) g.classPage = 0;

    // loc + xep tang dan theo ma lop
    std::vector<int> classView;
    for (int i = 0; i < g.classCount; ++i) {
        if (g.classes[i] == nullptr) continue;
        if (GuiMatchText(g.classes[i]->id, g.searchClass) || GuiMatchText(g.classes[i]->name, g.searchClass)) {
            classView.push_back(i);
        }
    }
    std::stable_sort(classView.begin(), classView.end(), [&](int a, int b) {
        return strcmp(g.classes[a]->id, g.classes[b]->id) < 0;
    });

    Rectangle allRec = { leftPane.x + 10, leftPane.y + 128, leftPane.width - 20, 34 };
    if (GuiBtn(allRec, "Tat ca cac lop (Toan truong)", g.classShowAll ? COLOR_SIDEBAR_ACTIVE : COLOR_SIDEBAR, COLOR_SIDEBAR_HOVER, WHITE, 14, g.classShowAll)) {
        g.classShowAll = true;
        g.studentPage = 0;
        g.searchStudent[0] = '\0';
    }

    const int classRowsPerPage = 5;
    int totalClassPages = classView.empty() ? 1 : ((int)classView.size() + classRowsPerPage - 1) / classRowsPerPage;
    if (g.classPage >= totalClassPages) g.classPage = totalClassPages - 1;
    if (g.classPage < 0) g.classPage = 0;

    if (g.classCount == 0) {
        DrawTxt("Chua co lop hoc nao", leftPane.x + 12, leftPane.y + 180, 15, COLOR_TEXT_MUTED);
    } else if (classView.empty()) {
        DrawTxt("Khong tim thay lop khop tu khoa", leftPane.x + 12, leftPane.y + 180, 15, COLOR_TEXT_MUTED);
    }

    int listTop = (int)leftPane.y + 170;
    int listBottom = (int)(leftPane.y + leftPane.height - 46);
    const int rowH = 80;
    int maxRowsFit = (listBottom - listTop) / rowH;
    int rowsPerPage = classRowsPerPage < maxRowsFit ? classRowsPerPage : maxRowsFit;

    int clsStart = g.classPage * rowsPerPage;
    int clsEnd = std::min(clsStart + rowsPerPage, (int)classView.size());
    for (int v = clsStart; v < clsEnd; ++v) {
        int idx = classView[v];
        Class* cls = g.classes[idx];
        int rowIdx = v - clsStart;
        Rectangle row = { leftPane.x + 8, (float)(listTop + rowIdx * rowH), leftPane.width - 16, (float)(rowH - 6) };
        bool active = (!g.classShowAll && g.selectedClassIdx == idx);

        Color rowBg = active ? MakeColor(219, 234, 254) : COLOR_ROW_ALT;
        DrawRectangleRounded(row, 0.08f, 4, rowBg);
        DrawRectangleRoundedLines(row, 0.08f, 4, active ? COLOR_PRIMARY : COLOR_BORDER);

        DrawTxt(cls->id, row.x + 10, row.y + 8, 15, COLOR_PRIMARY, true);
        int siSo = countStudents(cls);
        const char* badge = TextFormat("%d SV", siSo);
        DrawTxt(badge, row.x + row.width - MeasureTxt(badge, 13, true) - 10, row.y + 9, 13, COLOR_TEXT_MUTED, true);

        BeginScissorMode((int)row.x + 8, (int)row.y + 30, (int)row.width - 16, 20);
        DrawTxt(cls->name, row.x + 10, row.y + 30, 13, COLOR_TEXT_MUTED);
        EndScissorMode();

        // nut Xoa o goc dau hang; khong con nut Xem (click ca hang de chon lop)
        Rectangle xoaRec = { row.x + 8, row.y + 48, 62, 24 };
        if (GuiBtnOutline(xoaRec, "Xoa", COLOR_DANGER, COLOR_DANGER, 13)) {
            g.currentModal = MODAL_DELETE_CLASS;
            snprintf(g.formId, sizeof(g.formId), "%s", cls->id);
            snprintf(g.formName, sizeof(g.formName), "%s", cls->name);
            g.modalError[0] = '\0';
        }

        // click vao vung trong cua row cung chon lop (khong danh cho nut Xoa)
        Vector2 mouse = GetMousePosition();
        Rectangle clickArea = { row.x, row.y, row.width, row.height };
        if (!GuiModalAbsorbsInput() && CheckCollisionPointRec(mouse, clickArea)
            && !CheckCollisionPointRec(mouse, xoaRec) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            g.classShowAll = false;
            g.selectedClassIdx = idx;
            g.studentPage = 0;
            g.searchStudent[0] = '\0';
        }
    }

    int classBarY = (int)(leftPane.y + leftPane.height - 36);
    DrawTxt(TextFormat("Trang %d / %d", g.classPage + 1, totalClassPages), leftPane.x + 12, (float)classBarY + 5, 14, COLOR_TEXT_MUTED, true);
    if (GuiBtnOutline({ leftPane.x + leftPane.width - 82, (float)classBarY, 32, 26 }, "<", COLOR_PRIMARY, COLOR_PRIMARY, 13, false, g.classPage == 0)) {
        if (g.classPage > 0) g.classPage--;
    }
    if (GuiBtnOutline({ leftPane.x + leftPane.width - 44, (float)classBarY, 32, 26 }, ">", COLOR_PRIMARY, COLOR_PRIMARY, 13, false, g.classPage >= totalClassPages - 1)) {
        if (g.classPage < totalClassPages - 1) g.classPage++;
    }

    // ================= COT PHAI: SINH VIEN =================
    bool haveClass = (!g.classShowAll && g.selectedClassIdx >= 0 && g.selectedClassIdx < g.classCount && g.classes[g.selectedClassIdx] != nullptr);
    Class* curClass = haveClass ? g.classes[g.selectedClassIdx] : nullptr;

    if (g.classShowAll) {
        DrawTxt("TAT CA SINH VIEN TOAN TRUONG", rightPane.x + 14, rightPane.y + 14, 16, COLOR_TEXT_DARK, true);
    } else if (curClass != nullptr) {
        BeginScissorMode((int)rightPane.x + 10, (int)rightPane.y + 8, (int)rightPane.width - 190, 26);
        DrawTxt(TextFormat("SINH VIEN LOP: %s - %s", curClass->id, curClass->name), rightPane.x + 14, rightPane.y + 14, 16, COLOR_TEXT_DARK, true);
        EndScissorMode();
    } else {
        DrawTxt("SINH VIEN", rightPane.x + 14, rightPane.y + 14, 16, COLOR_TEXT_DARK, true);
    }

    bool canAddStudent = (curClass != nullptr && !g.classShowAll);
    if (GuiBtn({ rightPane.x + rightPane.width - 164, rightPane.y + 8, 150, 34 }, "+ Them Sinh Vien", COLOR_SUCCESS, COLOR_SUCCESS_HOVER, WHITE, 14, true, !canAddStudent)) {
        g.currentModal = MODAL_ADD_STUDENT;
        g.formId[0] = '\0';
        g.formHo[0] = '\0';
        g.formTen[0] = '\0';
        snprintf(g.formGender, sizeof(g.formGender), "Nam");
        snprintf(g.formPass, sizeof(g.formPass), "123");
        g.modalError[0] = '\0';
        g.activeFormBox = 1;
    }

    Rectangle stdSearchRec = { rightPane.x + 14, rightPane.y + 52, 340, 38 };
    size_t stdLenBefore = strlen(g.searchStudent);
    GuiTextBoxInput(stdSearchRec, g.searchStudent, sizeof(g.searchStudent), g.activeSearchStudent, "Tim theo MASV hoac Ho ten...");
    if (strlen(g.searchStudent) != stdLenBefore) g.studentPage = 0;

    std::vector<StudentRowRef> rows;
    if (g.classShowAll) {
        for (int i = 0; i < g.classCount; ++i) {
            if (g.classes[i] == nullptr) continue;
            for (Student* s = g.classes[i]->studentList; s != nullptr; s = s->next) {
                if (studentMatches(s, g.searchStudent)) rows.push_back({ g.classes[i], s });
            }
        }
    } else if (curClass != nullptr) {
        for (Student* s = curClass->studentList; s != nullptr; s = s->next) {
            if (studentMatches(s, g.searchStudent)) rows.push_back({ curClass, s });
        }
    }

    int totalStdPages = rows.empty() ? 1 : ((int)rows.size() + STUDENTS_PER_PAGE - 1) / STUDENTS_PER_PAGE;
    if (g.studentPage >= totalStdPages) g.studentPage = totalStdPages - 1;
    if (g.studentPage < 0) g.studentPage = 0;

    bool showClassCol = g.classShowAll;
    int xStt = 10;
    int xMasv = 45;
    int xLop = 112;
    int xName = showClassCol ? 205 : 120;
    int xGender = showClassCol ? 345 : 340;
    int xPass = showClassCol ? 390 : 385;
    int xActions = (int)rightPane.width - 130;

    int tableY = (int)rightPane.y + 100;
    DrawRectangle((int)rightPane.x + 6, tableY, (int)rightPane.width - 12, 40, COLOR_SIDEBAR);
    DrawTxt("STT", rightPane.x + xStt, (float)tableY + 11, 15, WHITE, true);
    DrawTxt("MASV", rightPane.x + xMasv, (float)tableY + 11, 15, WHITE, true);
    if (showClassCol) DrawTxt("MA LOP", rightPane.x + xLop, (float)tableY + 11, 15, WHITE, true);
    DrawTxt("HO VA TEN", rightPane.x + xName, (float)tableY + 11, 15, WHITE, true);
    DrawTxt("PHAI", rightPane.x + xGender, (float)tableY + 11, 15, WHITE, true);
    DrawTxt("MAT KHAU", rightPane.x + xPass, (float)tableY + 11, 15, WHITE, true);
    DrawTxt("THAO TAC", rightPane.x + xActions, (float)tableY + 11, 15, WHITE, true);

    bool noClassAtAll = (g.classCount == 0);
    bool classHasNoStudents = (curClass != nullptr && curClass->studentList == nullptr);
    bool filterNoMatch = (!rows.empty() || g.searchStudent[0] == '\0') ? false : true;
    if (!g.classShowAll && curClass != nullptr) {
        int rawCount = countStudents(curClass);
        if (rawCount > 0 && rows.empty()) filterNoMatch = true;
    }

    if (noClassAtAll) {
        DrawTxt("Vui long them lop hoc o cot ben trai truoc.", rightPane.x + 20, (float)tableY + 60, 16, COLOR_TEXT_MUTED);
    } else if (g.classShowAll && rows.empty() && g.searchStudent[0] == '\0') {
        DrawTxt("Chua co sinh vien nao trong he thong.", rightPane.x + 20, (float)tableY + 60, 16, COLOR_TEXT_MUTED);
    } else if (classHasNoStudents) {
        DrawTxt(TextFormat("Lop %s hien chua co sinh vien nao.", curClass->id), rightPane.x + 20, (float)tableY + 60, 16, COLOR_TEXT_MUTED);
        DrawTxt("Nhan [+ Them Sinh Vien] o tren de them ngay.", rightPane.x + 20, (float)tableY + 88, 16, COLOR_TEXT_MUTED);
    } else if (filterNoMatch) {
        DrawTxt("Khong tim thay sinh vien khop tu khoa", rightPane.x + 20, (float)tableY + 60, 16, COLOR_TEXT_MUTED);
        if (GuiBtnOutline({ rightPane.x + 20, (float)tableY + 92, 140, 32 }, "Xoa tim kiem", COLOR_PRIMARY, COLOR_PRIMARY, 14)) {
            g.searchStudent[0] = '\0';
            g.studentPage = 0;
        }
    } else if (!rows.empty()) {
        int startIdx = g.studentPage * STUDENTS_PER_PAGE;
        int endIdx = std::min(startIdx + STUDENTS_PER_PAGE, (int)rows.size());

        for (int i = startIdx; i < endIdx; ++i) {
            int rowOffset = i - startIdx;
            int rowY = tableY + 45 + rowOffset * 45;
            Color rowBg = (rowOffset % 2 == 0) ? COLOR_CARD : COLOR_ROW_ALT;
            DrawRectangle((int)rightPane.x + 6, rowY, (int)rightPane.width - 12, 42, rowBg);
            DrawRectangleLines((int)rightPane.x + 6, rowY, (int)rightPane.width - 12, 42, COLOR_BORDER);

            Student* s = rows[i].student;
            DrawTxt(TextFormat("%d", i + 1), rightPane.x + xStt, (float)rowY + 11, 15, COLOR_TEXT_DARK);
            DrawTxt(s->id, rightPane.x + xMasv, (float)rowY + 11, 15, COLOR_PRIMARY, true);
            if (showClassCol) DrawTxt(rows[i].classroom->id, rightPane.x + xLop, (float)rowY + 11, 14, COLOR_TEXT_MUTED);
            DrawTxt(TextFormat("%s %s", s->ho, s->ten), rightPane.x + xName, (float)rowY + 11, 15, COLOR_TEXT_DARK);
            DrawTxt(s->gender, rightPane.x + xGender, (float)rowY + 11, 15, COLOR_TEXT_MUTED);
            DrawTxt("******", rightPane.x + xPass, (float)rowY + 11, 15, COLOR_TEXT_MUTED);

            if (GuiBtnOutline({ rightPane.x + xActions, (float)rowY + 6, 55, 30 }, "Sua", COLOR_PRIMARY, COLOR_PRIMARY, 14)) {
                g.currentModal = MODAL_EDIT_STUDENT;
                snprintf(g.formId, sizeof(g.formId), "%s", s->id);
                snprintf(g.formHo, sizeof(g.formHo), "%s", s->ho);
                snprintf(g.formTen, sizeof(g.formTen), "%s", s->ten);
                snprintf(g.formGender, sizeof(g.formGender), "%s", s->gender);
                snprintf(g.formPass, sizeof(g.formPass), "%s", s->password);
                snprintf(g.formClassId, sizeof(g.formClassId), "%s", rows[i].classroom->id);
                g.modalError[0] = '\0';
                g.activeFormBox = 2; // khong sua ID
            }
            if (GuiBtnOutline({ rightPane.x + xActions + 65, (float)rowY + 6, 55, 30 }, "Xoa", COLOR_DANGER, COLOR_DANGER, 14)) {
                g.currentModal = MODAL_DELETE_STUDENT;
                snprintf(g.formId, sizeof(g.formId), "%s", s->id);
                snprintf(g.formClassId, sizeof(g.formClassId), "%s", rows[i].classroom->id);
                g.modalError[0] = '\0';
            }
        }

        int pageBarY = (int)(rightPane.y + rightPane.height - 40);
        DrawTxt(TextFormat("Trang %d / %d (Tong: %d sinh vien)", g.studentPage + 1, totalStdPages, (int)rows.size()), rightPane.x + 14, (float)pageBarY + 6, 14, COLOR_TEXT_MUTED, true);

        if (GuiBtnOutline({ rightPane.x + rightPane.width - 235, (float)pageBarY, 105, 28 }, "< Trang truoc", COLOR_PRIMARY, COLOR_PRIMARY, 13, false, g.studentPage == 0)) {
            if (g.studentPage > 0) g.studentPage--;
        }
        if (GuiBtnOutline({ rightPane.x + rightPane.width - 125, (float)pageBarY, 105, 28 }, "Trang sau >", COLOR_PRIMARY, COLOR_PRIMARY, 13, false, g.studentPage >= totalStdPages - 1)) {
            if (g.studentPage < totalStdPages - 1) g.studentPage++;
        }
    }
}
