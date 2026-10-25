#include "GuiCommon.h"
#include "GuiState.h"

#include "../business/BusinessErrors.h"
#include "../business/ClassService.h"
#include "../business/ExamService.h"
#include "../business/FileStorage.h"
#include "../business/QuestionService.h"
#include "../business/StudentService.h"
#include "../business/SubjectService.h"

#include "../core/ClassArray.h"
#include "../core/QuestionList.h"
#include "../core/SubjectBST.h"

#include <cstdio>
#include <cstdlib>

static Rectangle modalRec() {
    return { (float)(GUI_WIDTH / 2 - 250), (float)(GUI_HEIGHT / 2 - 180), 500, 360 };
}

static void drawSubjectAddEditModal(GuiState& g) {
    Rectangle rec = modalRec();
    DrawCardPanel(rec);
    DrawTxt((g.currentModal == MODAL_ADD_SUBJECT) ? "THEM MON HOC MOI" : "HIEU CHINH MON HOC", rec.x + 30, rec.y + 25, 20, COLOR_TEXT_DARK, true);

    DrawTxt("Ma mon hoc (MAMH <= 15):", rec.x + 30, rec.y + 65, 14, COLOR_TEXT_DARK);
    bool actId = (g.activeFormBox == 1);
    GuiTextBoxInput({ rec.x + 30, rec.y + 88, 440, 38 }, g.formId, sizeof(g.formId), actId, "VD: CTDL");
    if (actId) g.activeFormBox = 1;

    DrawTxt("Ten mon hoc:", rec.x + 30, rec.y + 140, 14, COLOR_TEXT_DARK);
    bool actName = (g.activeFormBox == 2);
    GuiTextBoxInput({ rec.x + 30, rec.y + 163, 440, 38 }, g.formName, sizeof(g.formName), actName, "VD: Cau truc du lieu & Giai thuat");
    if (actName) g.activeFormBox = 2;

    if (g.modalError[0] != '\0') {
        DrawTxt(g.modalError, rec.x + 30, rec.y + 215, 14, COLOR_DANGER, true);
    }

    bool submitSub = GuiBtn({ rec.x + 30, rec.y + 270, 200, 42 }, "Luu Mon Hoc", COLOR_SUCCESS, COLOR_SUCCESS_HOVER, WHITE, 16, true);
    if (IsKeyPressed(KEY_ENTER)) submitSub = true;

    if (submitSub) {
        BusinessResult res = SUCCESS;
        if (g.currentModal == MODAL_ADD_SUBJECT) {
            res = addSubjectWorkflow(g.root, g.formId, g.formName);
        } else {
            res = editSubjectWorkflow(g.root, g.classes, g.classCount, g.formId, g.formId, g.formName);
        }

        if (res == SUCCESS) {
            g.currentModal = MODAL_NONE;
            g.showToast("Da luu mon hoc thanh cong!", 3.0f, false);
        } else {
            snprintf(g.modalError, sizeof(g.modalError), "%s", getErrorMessage(res));
        }
    }
    if (GuiBtnOutline({ rec.x + 270, rec.y + 270, 200, 42 }, "Huy bo", COLOR_DANGER, COLOR_DANGER, 16, true)) {
        g.currentModal = MODAL_NONE;
    }
}

static void drawSubjectDeleteModal(GuiState& g) {
    Rectangle rec = modalRec();
    DrawCardPanel(rec);
    DrawTxt("XAC NHAN XOA MON HOC", rec.x + 30, rec.y + 30, 20, COLOR_DANGER, true);
    DrawTxt(TextFormat("Ban co chac chan muon xoa mon hoc [%s]?", g.formId), rec.x + 30, rec.y + 80, 16, COLOR_TEXT_DARK);
    DrawTxt("Luu y: Chi xoa duoc mon chua co diem thi trong he thong.", rec.x + 30, rec.y + 115, 14, COLOR_TEXT_MUTED);

    if (g.modalError[0] != '\0') {
        DrawTxt(g.modalError, rec.x + 30, rec.y + 155, 15, COLOR_DANGER, true);
    }

    if (GuiBtn({ rec.x + 30, rec.y + 210, 200, 42 }, "Xac Nhan Xoa", COLOR_DANGER, COLOR_DANGER_HOVER, WHITE, 16, true)) {
        BusinessResult res = deleteSubjectWorkflow(g.root, g.classes, g.classCount, g.formId);
        if (res == SUCCESS) {
            g.configSelected = -1;
            g.currentModal = MODAL_NONE;
            g.showToast(TextFormat("Da xoa mon hoc [%s]!", g.formId), 3.0f, false);
        } else {
            snprintf(g.modalError, sizeof(g.modalError), "%s", getErrorMessage(res));
        }
    }
    if (GuiBtnOutline({ rec.x + 270, rec.y + 210, 200, 42 }, "Huy bo", COLOR_PRIMARY, COLOR_PRIMARY, 16, true)) {
        g.currentModal = MODAL_NONE;
    }
}

static void drawClassAddModal(GuiState& g) {
    Rectangle rec = modalRec();
    DrawCardPanel(rec);
    DrawTxt("THEM LOP HOC MOI", rec.x + 30, rec.y + 25, 20, COLOR_TEXT_DARK, true);

    DrawTxt("Ma lop hoc (MALOP <= 19):", rec.x + 30, rec.y + 65, 14, COLOR_TEXT_DARK);
    bool actId = (g.activeFormBox == 1);
    GuiTextBoxInput({ rec.x + 30, rec.y + 88, 440, 38 }, g.formId, sizeof(g.formId), actId, "VD: D23CQCN01");
    if (actId) g.activeFormBox = 1;

    DrawTxt("Ten lop hoc:", rec.x + 30, rec.y + 140, 14, COLOR_TEXT_DARK);
    bool actName = (g.activeFormBox == 2);
    GuiTextBoxInput({ rec.x + 30, rec.y + 163, 440, 38 }, g.formName, sizeof(g.formName), actName, "VD: Cong nghe thong tin 1");
    if (actName) g.activeFormBox = 2;

    if (g.modalError[0] != '\0') {
        DrawTxt(g.modalError, rec.x + 30, rec.y + 215, 14, COLOR_DANGER, true);
    }

    bool submitClass = GuiBtn({ rec.x + 30, rec.y + 270, 200, 42 }, "Luu Lop Hoc", COLOR_SUCCESS, COLOR_SUCCESS_HOVER, WHITE, 16, true);
    if (IsKeyPressed(KEY_ENTER)) submitClass = true;

    if (submitClass) {
        BusinessResult res = addClassWorkflow(g.classes, g.classCount, g.formId, g.formName);
        if (res == SUCCESS) {
            g.currentModal = MODAL_NONE;
            g.showToast("Da them lop hoc thanh cong!", 3.0f, false);
        } else {
            snprintf(g.modalError, sizeof(g.modalError), "%s", getErrorMessage(res));
        }
    }
    if (GuiBtnOutline({ rec.x + 270, rec.y + 270, 200, 42 }, "Huy bo", COLOR_DANGER, COLOR_DANGER, 16, true)) {
        g.currentModal = MODAL_NONE;
    }
}

static void drawClassDeleteModal(GuiState& g) {
    Rectangle rec = modalRec();
    DrawCardPanel(rec);
    DrawTxt("XAC NHAN XOA LOP HOC", rec.x + 30, rec.y + 30, 20, COLOR_DANGER, true);
    DrawTxt(TextFormat("Ban co chac chan muon xoa lop [%s]?", g.formId), rec.x + 30, rec.y + 80, 16, COLOR_TEXT_DARK);

    Class* target = findClass(g.classes, g.classCount, g.formId);
    int siSo = 0;
    for (Student* s = (target != nullptr ? target->studentList : nullptr); s != nullptr; s = s->next) siSo++;
    if (siSo > 0) {
        DrawTxt(TextFormat("Luu y: %d sinh vien cua lop nay se bi xoa kem theo.", siSo), rec.x + 30, rec.y + 115, 14, COLOR_DANGER, true);
    } else {
        DrawTxt("Lop hoc dang trong, co the xoa an toan.", rec.x + 30, rec.y + 115, 14, COLOR_TEXT_MUTED);
    }

    if (g.modalError[0] != '\0') {
        DrawTxt(g.modalError, rec.x + 30, rec.y + 155, 15, COLOR_DANGER, true);
    }

    if (GuiBtn({ rec.x + 30, rec.y + 210, 200, 42 }, "Xac Nhan Xoa", COLOR_DANGER, COLOR_DANGER_HOVER, WHITE, 16, true)) {
        bool wasSelected = (!g.classShowAll && g.selectedClassIdx >= 0 && g.selectedClassIdx < g.classCount
                            && g.classes[g.selectedClassIdx] != nullptr
                            && strcmp(g.classes[g.selectedClassIdx]->id, g.formId) == 0);
        BusinessResult res = deleteClassWorkflow(g.classes, g.classCount, g.formId);
        if (res == SUCCESS) {
            g.configSelected = -1;
            if (g.classCount > 0) {
                if (wasSelected) g.selectedClassIdx = 0;
                if (g.selectedClassIdx >= g.classCount) g.selectedClassIdx = g.classCount - 1;
            } else {
                g.selectedClassIdx = -1;
                g.classShowAll = false;
            }
            g.currentModal = MODAL_NONE;
            g.showToast(TextFormat("Da xoa lop hoc [%s]!", g.formId), 3.0f, false);
        } else {
            snprintf(g.modalError, sizeof(g.modalError), "%s", getErrorMessage(res));
        }
    }
    if (GuiBtnOutline({ rec.x + 270, rec.y + 210, 200, 42 }, "Huy bo", COLOR_PRIMARY, COLOR_PRIMARY, 16, true)) {
        g.currentModal = MODAL_NONE;
    }
}

static void drawStudentAddEditModal(GuiState& g) {
    Rectangle bigModal = { (float)(GUI_WIDTH / 2 - 270), (float)(GUI_HEIGHT / 2 - 230), 540, 460 };
    DrawCardPanel(bigModal);

    DrawTxt((g.currentModal == MODAL_ADD_STUDENT) ? "THEM SINH VIEN MOI" : "HIEU CHINH SINH VIEN", bigModal.x + 30, bigModal.y + 25, 20, COLOR_TEXT_DARK, true);

    DrawTxt("Ma sinh vien (MASV):", bigModal.x + 30, bigModal.y + 65, 14, COLOR_TEXT_DARK);
    bool actId = (g.activeFormBox == 1);
    GuiTextBoxInput({ bigModal.x + 30, bigModal.y + 85, 480, 34 }, g.formId, sizeof(g.formId), actId, "VD: SV004");
    if (actId) g.activeFormBox = 1;

    DrawTxt("Ho dem:", bigModal.x + 30, bigModal.y + 130, 14, COLOR_TEXT_DARK);
    bool actHo = (g.activeFormBox == 2);
    GuiTextBoxInput({ bigModal.x + 30, bigModal.y + 150, 230, 34 }, g.formHo, sizeof(g.formHo), actHo, "VD: Nguyen Van");
    if (actHo) g.activeFormBox = 2;

    DrawTxt("Ten:", bigModal.x + 280, bigModal.y + 130, 14, COLOR_TEXT_DARK);
    bool actTen = (g.activeFormBox == 3);
    GuiTextBoxInput({ bigModal.x + 280, bigModal.y + 150, 230, 34 }, g.formTen, sizeof(g.formTen), actTen, "VD: Dung");
    if (actTen) g.activeFormBox = 3;

    DrawTxt("Phai (Nam / Nu):", bigModal.x + 30, bigModal.y + 195, 14, COLOR_TEXT_DARK);
    bool actGen = (g.activeFormBox == 4);
    GuiTextBoxInput({ bigModal.x + 30, bigModal.y + 215, 230, 34 }, g.formGender, sizeof(g.formGender), actGen, "Nam");
    if (actGen) g.activeFormBox = 4;

    DrawTxt("Mat khau:", bigModal.x + 280, bigModal.y + 195, 14, COLOR_TEXT_DARK);
    bool actP = (g.activeFormBox == 5);
    GuiTextBoxInput({ bigModal.x + 280, bigModal.y + 215, 230, 34 }, g.formPass, sizeof(g.formPass), actP, "123");
    if (actP) g.activeFormBox = 5;

    if (g.modalError[0] != '\0') {
        DrawTxt(g.modalError, bigModal.x + 30, bigModal.y + 270, 14, COLOR_DANGER, true);
    }

    bool submitStd = GuiBtn({ bigModal.x + 30, bigModal.y + 360, 230, 42 }, "Luu Sinh Vien", COLOR_SUCCESS, COLOR_SUCCESS_HOVER, WHITE, 16, true);
    if (IsKeyPressed(KEY_ENTER)) submitStd = true;

    if (submitStd) {
        if (g.selectedClassIdx < g.classCount && g.classes[g.selectedClassIdx] != nullptr) {
            BusinessResult res = SUCCESS;
            if (g.currentModal == MODAL_ADD_STUDENT) {
                res = addStudentWorkflow(g.classes, g.classCount, g.classes[g.selectedClassIdx]->id, g.formId, g.formHo, g.formTen, g.formGender, g.formPass);
            } else {
                res = editStudentWorkflow(g.classes, g.classCount, g.formId, g.formHo, g.formTen, g.formGender, g.formPass);
            }

            if (res == SUCCESS) {
                g.currentModal = MODAL_NONE;
                g.showToast("Da luu sinh vien thanh cong!", 3.0f, false);
            } else {
                snprintf(g.modalError, sizeof(g.modalError), "%s", getErrorMessage(res));
            }
        }
    }
    if (GuiBtnOutline({ bigModal.x + 280, bigModal.y + 360, 230, 42 }, "Huy bo", COLOR_DANGER, COLOR_DANGER, 16, true)) {
        g.currentModal = MODAL_NONE;
    }
}

static void drawStudentDeleteModal(GuiState& g) {
    Rectangle rec = modalRec();
    DrawCardPanel(rec);
    DrawTxt("XAC NHAN XOA SINH VIEN", rec.x + 30, rec.y + 30, 20, COLOR_DANGER, true);
    DrawTxt(TextFormat("Ban co chac chan muon xoa sinh vien [%s]?", g.formId), rec.x + 30, rec.y + 80, 16, COLOR_TEXT_DARK);
    DrawTxt("Luu y: Chi xoa duoc neu sinh vien chua co diem thi.", rec.x + 30, rec.y + 115, 14, COLOR_TEXT_MUTED);

    if (g.modalError[0] != '\0') {
        DrawTxt(g.modalError, rec.x + 30, rec.y + 155, 15, COLOR_DANGER, true);
    }

    if (GuiBtn({ rec.x + 30, rec.y + 210, 200, 42 }, "Xac Nhan Xoa", COLOR_DANGER, COLOR_DANGER_HOVER, WHITE, 16, true)) {
        BusinessResult res = deleteStudentWorkflow(g.classes, g.classCount, g.formClassId, g.formId);
        if (res == SUCCESS) {
            g.currentModal = MODAL_NONE;
            g.showToast(TextFormat("Da xoa sinh vien [%s]!", g.formId), 3.0f, false);
        } else {
            snprintf(g.modalError, sizeof(g.modalError), "%s", getErrorMessage(res));
        }
    }
    if (GuiBtnOutline({ rec.x + 270, rec.y + 210, 200, 42 }, "Huy bo", COLOR_PRIMARY, COLOR_PRIMARY, 16, true)) {
        g.currentModal = MODAL_NONE;
    }
}

static void drawQuestionAddEditModal(GuiState& g) {
    Rectangle qModal = { (float)(GUI_WIDTH / 2 - 320), (float)(GUI_HEIGHT / 2 - 270), 640, 540 };
    DrawCardPanel(qModal);

    DrawTxt((g.currentModal == MODAL_ADD_QUESTION) ? "THEM CAU HOI THI MOI" : "HIEU CHINH CAU HOI", qModal.x + 30, qModal.y + 20, 20, COLOR_TEXT_DARK, true);

    DrawTxt("Noi dung cau hoi:", qModal.x + 30, qModal.y + 55, 14, COLOR_TEXT_DARK);
    bool actC = (g.activeFormBox == 1);
    GuiTextBoxInput({ qModal.x + 30, qModal.y + 75, 580, 36 }, g.formContent, sizeof(g.formContent), actC, "VD: 1 byte bang bao nhieu bit?");
    if (actC) g.activeFormBox = 1;

    DrawTxt("Dap an A:", qModal.x + 30, qModal.y + 120, 14, COLOR_TEXT_DARK);
    bool actA = (g.activeFormBox == 2);
    GuiTextBoxInput({ qModal.x + 30, qModal.y + 140, 580, 34 }, g.formA, sizeof(g.formA), actA, "A");
    if (actA) g.activeFormBox = 2;

    DrawTxt("Dap an B:", qModal.x + 30, qModal.y + 185, 14, COLOR_TEXT_DARK);
    bool actB = (g.activeFormBox == 3);
    GuiTextBoxInput({ qModal.x + 30, qModal.y + 205, 580, 34 }, g.formB, sizeof(g.formB), actB, "B");
    if (actB) g.activeFormBox = 3;

    DrawTxt("Dap an C:", qModal.x + 30, qModal.y + 250, 14, COLOR_TEXT_DARK);
    bool actCc = (g.activeFormBox == 4);
    GuiTextBoxInput({ qModal.x + 30, qModal.y + 270, 580, 34 }, g.formC, sizeof(g.formC), actCc, "C");
    if (actCc) g.activeFormBox = 4;

    DrawTxt("Dap an D:", qModal.x + 30, qModal.y + 315, 14, COLOR_TEXT_DARK);
    bool actD = (g.activeFormBox == 5);
    GuiTextBoxInput({ qModal.x + 30, qModal.y + 335, 580, 34 }, g.formD, sizeof(g.formD), actD, "D");
    if (actD) g.activeFormBox = 5;

    DrawTxt("Dap an dung (A / B / C / D):", qModal.x + 30, qModal.y + 380, 14, COLOR_TEXT_DARK);
    for (int c = 0; c < 4; ++c) {
        char l = (char)('A' + c);
        bool isCur = (g.formCorrect[0] == l);
        if (GuiBtn({ qModal.x + 230 + c * 60, qModal.y + 375, 45, 30 }, TextFormat("%c", l), isCur ? COLOR_SUCCESS : COLOR_SIDEBAR, COLOR_SUCCESS_HOVER, WHITE, 14, isCur)) {
            g.formCorrect[0] = l;
        }
    }

    if (g.modalError[0] != '\0') {
        DrawTxt(g.modalError, qModal.x + 30, qModal.y + 420, 14, COLOR_DANGER, true);
    }

    bool submitQ = GuiBtn({ qModal.x + 30, qModal.y + 460, 270, 42 }, "Luu Cau Hoi", COLOR_SUCCESS, COLOR_SUCCESS_HOVER, WHITE, 16, true);
    if (IsKeyPressed(KEY_ENTER)) submitQ = true;

    if (submitQ) {
        if (g.selectedSubjectIdx < (int)g.subjects.size()) {
            Subject* sNode = g.subjects[g.selectedSubjectIdx];
            BusinessResult res = SUCCESS;
            if (g.currentModal == MODAL_ADD_QUESTION) {
                int newId = 0;
                res = addQuestionWorkflow(g.root, sNode->id, g.formContent, g.formA, g.formB, g.formC, g.formD, g.formCorrect[0], newId);
            } else {
                res = editQuestionWorkflow(g.root, sNode->id, g.formEditQId, g.formContent, g.formA, g.formB, g.formC, g.formD, g.formCorrect[0]);
            }

            if (res == SUCCESS) {
                g.currentModal = MODAL_NONE;
                g.showToast("Da luu cau hoi thanh cong!", 3.0f, false);
            } else {
                snprintf(g.modalError, sizeof(g.modalError), "%s", getErrorMessage(res));
            }
        }
    }
    if (GuiBtnOutline({ qModal.x + 340, qModal.y + 460, 270, 42 }, "Huy bo", COLOR_DANGER, COLOR_DANGER, 16, true)) {
        g.currentModal = MODAL_NONE;
    }
}

static void drawQuestionDeleteModal(GuiState& g) {
    Rectangle rec = modalRec();
    DrawCardPanel(rec);
    DrawTxt("XAC NHAN XOA CAU HOI THI", rec.x + 30, rec.y + 30, 20, COLOR_DANGER, true);
    DrawTxt(TextFormat("Ban co chac chan muon xoa cau hoi ID: %d?", g.formEditQId), rec.x + 30, rec.y + 80, 16, COLOR_TEXT_DARK);
    DrawTxt("Luu y: Chi xoa duoc khi mon chua co diem thi va cau hoi", rec.x + 30, rec.y + 110, 14, COLOR_TEXT_MUTED);
    DrawTxt("chua nam trong cau hinh thi cua lop nao.", rec.x + 30, rec.y + 130, 14, COLOR_TEXT_MUTED);

    if (g.modalError[0] != '\0') {
        DrawTxt(g.modalError, rec.x + 30, rec.y + 155, 15, COLOR_DANGER, true);
    }

    if (GuiBtn({ rec.x + 30, rec.y + 210, 200, 42 }, "Xac Nhan Xoa", COLOR_DANGER, COLOR_DANGER_HOVER, WHITE, 16, true)) {
        if (g.selectedSubjectIdx < (int)g.subjects.size()) {
            Subject* sNode = g.subjects[g.selectedSubjectIdx];
            BusinessResult res = deleteQuestionWorkflow(g.root, g.classes, g.classCount, sNode->id, g.formEditQId);
            if (res == SUCCESS) {
                g.currentModal = MODAL_NONE;
                g.showToast(TextFormat("Da xoa cau hoi ID %d!", g.formEditQId), 3.0f, false);
            } else {
                snprintf(g.modalError, sizeof(g.modalError), "%s", getErrorMessage(res));
            }
        }
    }
    if (GuiBtnOutline({ rec.x + 270, rec.y + 210, 200, 42 }, "Huy bo", COLOR_PRIMARY, COLOR_PRIMARY, 16, true)) {
        g.currentModal = MODAL_NONE;
    }
}

static void drawCancelExamSessionModal(GuiState& g) {
    Rectangle rec = { (float)(GUI_WIDTH / 2 - 250), (float)(GUI_HEIGHT / 2 - 180), 500, 360 };
    DrawCardPanel(rec);
    DrawTxt("HUY PHIEN THI DO DANG", rec.x + 30, rec.y + 25, 20, COLOR_DANGER, true);

    DrawTxt("Huy phien = TU DONG NOP BAI dang do dang.", rec.x + 30, rec.y + 85, 15, COLOR_TEXT_DARK);
    DrawTxt("- Cau chua chon dap an se bi cham la SAI.", rec.x + 30, rec.y + 118, 14, COLOR_TEXT_MUTED);
    DrawTxt("- Diem duoc ghi vao bang diem, khong the thi lai mon nay.", rec.x + 30, rec.y + 145, 14, COLOR_TEXT_MUTED);
    DrawTxt("Ban co chac chan?", rec.x + 30, rec.y + 180, 15, COLOR_TEXT_DARK);

    if (GuiBtn({ rec.x + 30, rec.y + 240, 200, 44 }, "Nop bai & Huy", COLOR_DANGER, COLOR_DANGER_HOVER, WHITE, 16, true)) {
        g.currentModal = MODAL_NONE;
        bool saveFailed = false;
        BusinessResult res = cancelPendingExamWorkflow(g.root, g.classes, g.classCount,
                                                       g.loggedStudent, &saveFailed);
        char msg[160];
        if (res == SUCCESS) {
            snprintf(msg, sizeof(msg), "Da tu dong nop phien thi do dang%s.",
                     saveFailed ? " (chua luu duoc file diem)" : "");
            g.showToast(msg, 4.0f, saveFailed);
        } else {
            snprintf(msg, sizeof(msg), "Khong huy duoc phien thi: %s", getErrorMessage(res));
            g.showToast(msg, 4.0f, true);
        }
    }
    if (GuiBtnOutline({ rec.x + 270, rec.y + 240, 200, 44 }, "Quay Lai Thi Tiep", COLOR_PRIMARY, COLOR_PRIMARY, 16, true)) {
        g.currentModal = MODAL_NONE;
    }
}

void renderModalDialogs(GuiState& g) {
    GuiMarkInsideModal();

    DrawRectangle(0, 0, GUI_WIDTH, GUI_HEIGHT, MakeColor(0, 0, 0, 130));

    if (IsKeyPressed(KEY_ESCAPE)) {
        g.currentModal = MODAL_NONE;
        return;
    }

    if (IsKeyPressed(KEY_TAB)) {
        if (g.currentModal == MODAL_ADD_SUBJECT || g.currentModal == MODAL_EDIT_SUBJECT || g.currentModal == MODAL_ADD_CLASS) {
            g.activeFormBox = (g.activeFormBox == 1) ? 2 : 1;
        } else if (g.currentModal == MODAL_ADD_STUDENT || g.currentModal == MODAL_EDIT_STUDENT) {
            g.activeFormBox++;
            if (g.activeFormBox > 5) g.activeFormBox = (g.currentModal == MODAL_ADD_STUDENT) ? 1 : 2;
        } else if (g.currentModal == MODAL_ADD_QUESTION || g.currentModal == MODAL_EDIT_QUESTION) {
            g.activeFormBox++;
            if (g.activeFormBox > 5) g.activeFormBox = 1;
        }
    }

    switch (g.currentModal) {
    case MODAL_ADD_SUBJECT:
    case MODAL_EDIT_SUBJECT:
        drawSubjectAddEditModal(g);
        break;
    case MODAL_DELETE_SUBJECT:
        drawSubjectDeleteModal(g);
        break;
    case MODAL_ADD_CLASS:
        drawClassAddModal(g);
        break;
    case MODAL_DELETE_CLASS:
        drawClassDeleteModal(g);
        break;
    case MODAL_ADD_STUDENT:
    case MODAL_EDIT_STUDENT:
        drawStudentAddEditModal(g);
        break;
    case MODAL_DELETE_STUDENT:
        drawStudentDeleteModal(g);
        break;
    case MODAL_ADD_QUESTION:
    case MODAL_EDIT_QUESTION:
        drawQuestionAddEditModal(g);
        break;
    case MODAL_DELETE_QUESTION:
        drawQuestionDeleteModal(g);
        break;
    case MODAL_CLASS_EXAM_CONFIG:
        drawClassExamConfigModal(g);
        break;
    case MODAL_DELETE_CLASS_EXAM_CONFIG:
        drawDeleteClassExamConfigModal(g);
        break;
    case MODAL_CONFIRM_SUBMIT:
        drawExamSubmitConfirmDialog(g);
        break;
    case MODAL_CANCEL_EXAM:
        drawCancelExamSessionModal(g);
        break;
    default:
        g.currentModal = MODAL_NONE;
        break;
    }
}
