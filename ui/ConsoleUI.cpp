#include "ConsoleUI.h"

#include "../business/AuthService.h"
#include "../business/BusinessErrors.h"
#include "../business/ClassService.h"
#include "../business/ExamService.h"
#include "../business/FileStorage.h"
#include "../business/QuestionService.h"
#include "../business/ReportService.h"
#include "../business/StudentService.h"
#include "../business/SubjectService.h"

#include "../core/ClassArray.h"
#include "../core/QuestionList.h"
#include "../core/StudentList.h"
#include "../core/SubjectBST.h"

#include <iostream>
#include <string>
#include <limits>
#include <ctime>

using namespace std;

static void pauseScreen() {
    cout << "\nNhan Enter de tiep tuc...";
    if (cin.eof()) return;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

static string readLine(const char* prompt) {
    cout << prompt;
    string input;
    if (cin.eof()) return "";
    getline(cin, input);
    return input;
}

static int readInt(const char* prompt) {
    while (true) {
        if (cin.eof()) return 0;
        cout << prompt;
        int val;
        if (cin >> val) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return val;
        } else {
            if (cin.eof()) return 0;
            cout << "Vui long nhap so nguyen hop le!\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    }
}

static int countSubjects(Subject* root) {
    if (root == nullptr) return 0;
    return 1 + countSubjects(root->left) + countSubjects(root->right);
}

static void collectSubjectsInOrder(Subject* root, Subject* arr[], int& index) {
    if (root == nullptr) return;
    collectSubjectsInOrder(root->left, arr, index);
    arr[index++] = root;
    collectSubjectsInOrder(root->right, arr, index);
}

static Subject* selectSubjectFromList(Subject* root) {
    if (root == nullptr) {
        cout << "-> Danh sach mon hoc hien dang trong!\n";
        return nullptr;
    }

    int total = countSubjects(root);
    Subject** subjectList = new Subject*[total];
    int count = 0;
    collectSubjectsInOrder(root, subjectList, count);

    cout << "\n=============================================================\n";
    cout << "                    DANH SACH MON HOC                        \n";
    cout << "=============================================================\n";
    for (int i = 0; i < count; ++i) {
        cout << "[" << (i + 1) << "] " 
             << subjectList[i]->id << " - " 
             << subjectList[i]->name << "\n";
    }
    cout << "[0] Huy bo / Quay lai\n";
    cout << "-------------------------------------------------------------\n";

    Subject* selected = nullptr;
    while (true) {
        int choice = readInt("Chon mon hoc (nhap so): ");
        if (choice == 0) {
            cout << "-> Da huy thao tac.\n";
            selected = nullptr;
            break;
        }
        if (choice >= 1 && choice <= count) {
            selected = subjectList[choice - 1];
            break;
        }
        cout << "Lua chon khong hop le! Vui long nhap tu 1 den " << count << " (hoac 0 de huy).\n";
    }

    delete[] subjectList;
    return selected;
}

static Class* selectClassFromList(Class* classes[], int classCount) {
    if (classes == nullptr || classCount == 0) {
        cout << "-> Danh sach lop hoc hien dang trong!\n";
        return nullptr;
    }

    cout << "\n=============================================================\n";
    cout << "                    DANH SACH LOP HOC                        \n";
    cout << "=============================================================\n";
    for (int i = 0; i < classCount; ++i) {
        if (classes[i] != nullptr) {
            cout << "[" << (i + 1) << "] " 
                 << classes[i]->id << " - " 
                 << classes[i]->name << "\n";
        }
    }
    cout << "[0] Huy bo / Quay lai\n";
    cout << "-------------------------------------------------------------\n";

    Class* selected = nullptr;
    while (true) {
        int choice = readInt("Chon lop hoc (nhap so): ");
        if (choice == 0) {
            cout << "-> Da huy thao tac.\n";
            return nullptr;
        }
        if (choice >= 1 && choice <= classCount && classes[choice - 1] != nullptr) {
            selected = classes[choice - 1];
            break;
        }
        cout << "Lua chon khong hop le! Vui long nhap tu 1 den " << classCount << " (hoac 0 de huy).\n";
    }

    return selected;
}

static int countStudentsInClass(const Class* c) {
    if (c == nullptr) return 0;
    int count = 0;
    Student* cur = c->studentList;
    while (cur != nullptr) {
        count++;
        cur = cur->next;
    }
    return count;
}

static Student* selectStudentFromClass(Class* c) {
    if (c == nullptr || c->studentList == nullptr) {
        cout << "-> Lop " << (c ? c->id : "") << " hien chua co sinh vien nao!\n";
        return nullptr;
    }

    int total = countStudentsInClass(c);
    Student** studentArr = new Student*[total];
    Student* cur = c->studentList;
    int idx = 0;
    while (cur != nullptr && idx < total) {
        studentArr[idx++] = cur;
        cur = cur->next;
    }

    cout << "\n=============================================================\n";
    cout << "          DANH SACH SINH VIEN LOP " << c->id << "\n";
    cout << "=============================================================\n";
    for (int i = 0; i < total; ++i) {
        cout << "[" << (i + 1) << "] " 
             << studentArr[i]->id << " - " 
             << studentArr[i]->ho << " " << studentArr[i]->ten 
             << " (" << studentArr[i]->gender << ")\n";
    }
    cout << "[0] Huy bo / Quay lai\n";
    cout << "-------------------------------------------------------------\n";

    Student* selected = nullptr;
    while (true) {
        int choice = readInt("Chon sinh vien (nhap so): ");
        if (choice == 0) {
            cout << "-> Da huy thao tac.\n";
            selected = nullptr;
            break;
        }
        if (choice >= 1 && choice <= total) {
            selected = studentArr[choice - 1];
            break;
        }
        cout << "Lua chon khong hop le! Vui long nhap tu 1 den " << total << " (hoac 0 de huy).\n";
    }

    delete[] studentArr;
    return selected;
}

static void teacherSubjectMenu(Subject*& root, Class* classes[], int classCount) {
    while (true) {
        cout << "\n-------------------------------------------------------------\n";
        cout << "                  QUAN LY MON HOC                            \n";
        cout << "-------------------------------------------------------------\n";
        cout << "1. Xem danh sach mon hoc\n";
        cout << "2. Them mon hoc moi\n";
        cout << "3. Hieu chinh mon hoc\n";
        cout << "4. Xoa mon hoc\n";
        cout << "0. Quay lai menu chinh\n";
        int choice = readInt("Lua chon cua ban: ");

        if (choice == 0) break;

        switch (choice) {
            case 1: {
                cout << "\n=== DANH SACH MON HOC (BST) ===\n";
                if (root == nullptr) {
                    cout << "Chua co mon hoc nao trong he thong.\n";
                } else {
                    printSubjects(root);
                }
                pauseScreen();
                break;
            }
            case 2: {
                string id = readLine("Nhap Ma mon hoc (MAMH <= 15): ");
                string name = readLine("Nhap Ten mon hoc: ");
                BusinessResult res = addSubjectWorkflow(root, id.c_str(), name.c_str());
                if (res == SUCCESS) {
                    cout << "-> Them mon hoc thanh cong!\n";
                } else {
                    cout << "-> Loi: " << getErrorMessage(res) << "\n";
                }
                pauseScreen();
                break;
            }
            case 3: {
                Subject* target = selectSubjectFromList(root);
                if (target == nullptr) {
                    pauseScreen();
                    break;
                }
                string oldId = target->id;
                cout << "\n-> Ban dang hieu chinh mon: [" << target->id << "] " << target->name << "\n";
                string newId = readLine("Nhap Ma mon hoc moi (nhap giong cu neu chi doi ten): ");
                string newName = readLine("Nhap Ten mon hoc moi: ");
                BusinessResult res = editSubjectWorkflow(root, classes, classCount, oldId.c_str(), newId.c_str(), newName.c_str());
                if (res == SUCCESS) {
                    renameSubjectQuestionFile("data", oldId.c_str(), newId.c_str());
                    cout << "-> Hieu chinh mon hoc thanh cong!\n";
                } else {
                    cout << "-> Loi: " << getErrorMessage(res) << "\n";
                }
                pauseScreen();
                break;
            }
            case 4: {
                Subject* target = selectSubjectFromList(root);
                if (target == nullptr) {
                    pauseScreen();
                    break;
                }
                string delId = target->id;
                string prompt = "Ban co chac chan muon xoa mon [" + delId + "] " + string(target->name) + "? (Y/N): ";
                string confirm = readLine(prompt.c_str());
                if (confirm.empty() || (confirm[0] != 'Y' && confirm[0] != 'y')) {
                    cout << "-> Da huy xoa mon hoc.\n";
                    pauseScreen();
                    break;
                }
                BusinessResult res = deleteSubjectWorkflow(root, classes, classCount, delId.c_str());
                if (res == SUCCESS) {
                    cout << "-> Xoa mon hoc thanh cong!\n";
                } else {
                    cout << "-> Loi: " << getErrorMessage(res) << "\n";
                }
                pauseScreen();
                break;
            }
            default:
                cout << "Lua chon khong hop le!\n";
                break;
        }
    }
}

static void teacherQuestionMenu(Subject* root) {
    while (true) {
        cout << "\n-------------------------------------------------------------\n";
        cout << "                  QUAN LY CAU HOI THI                        \n";
        cout << "-------------------------------------------------------------\n";
        cout << "1. Xem danh sach cau hoi theo mon\n";
        cout << "2. Them cau hoi thi moi\n";
        cout << "3. Hieu chinh cau hoi\n";
        cout << "4. Xoa cau hoi\n";
        cout << "0. Quay lai menu chinh\n";
        int choice = readInt("Lua chon cua ban: ");

        if (choice == 0) break;

        switch (choice) {
            case 1: {
                Subject* sub = selectSubjectFromList(root);
                if (sub == nullptr) {
                    pauseScreen();
                    break;
                }
                cout << "\n=== DANH SACH CAU HOI MON " << sub->id << " - " << sub->name << " ===\n";
                if (sub->questionList == nullptr) {
                    cout << "Mon nay chua co cau hoi nao.\n";
                } else {
                    Question* q = sub->questionList;
                    while (q != nullptr) {
                        cout << "ID: " << q->id << " - " << q->content << "\n";
                        cout << "   A. " << q->answerA << "\n";
                        cout << "   B. " << q->answerB << "\n";
                        cout << "   C. " << q->answerC << "\n";
                        cout << "   D. " << q->answerD << "\n";
                        cout << "   -> Dap an dung: " << q->correctAnswer << "\n\n";
                        q = q->next;
                    }
                }
                pauseScreen();
                break;
            }
            case 2: {
                Subject* sub = selectSubjectFromList(root);
                if (sub == nullptr) {
                    pauseScreen();
                    break;
                }
                cout << "\n-> Them cau hoi cho mon: [" << sub->id << "] " << sub->name << "\n";
                string content = readLine("Nhap Noi dung cau hoi: ");
                string a = readLine("Nhap Dap an A: ");
                string b = readLine("Nhap Dap an B: ");
                string c = readLine("Nhap Dap an C: ");
                string d = readLine("Nhap Dap an D: ");
                string correctStr = readLine("Nhap Dap an dung (A/B/C/D): ");
                char correct = correctStr.empty() ? 'A' : correctStr[0];

                int genId = 0;
                BusinessResult res = addQuestionWorkflow(
                    root, sub->id, content.c_str(),
                    a.c_str(), b.c_str(), c.c_str(), d.c_str(),
                    correct, genId
                );

                if (res == SUCCESS) {
                    cout << "-> Them cau hoi thanh cong! Ma ID tu sinh: " << genId << "\n";
                } else {
                    cout << "-> Loi: " << getErrorMessage(res) << "\n";
                }
                pauseScreen();
                break;
            }
            case 3: {
                Subject* sub = selectSubjectFromList(root);
                if (sub == nullptr) {
                    pauseScreen();
                    break;
                }
                cout << "\n-> Hieu chinh cau hoi thuoc mon: [" << sub->id << "] " << sub->name << "\n";
                int qId = readInt("Nhap ID cau hoi can sua: ");
                string content = readLine("Nhap Noi dung cau hoi moi: ");
                string a = readLine("Nhap Dap an A moi: ");
                string b = readLine("Nhap Dap an B moi: ");
                string c = readLine("Nhap Dap an C moi: ");
                string d = readLine("Nhap Dap an D moi: ");
                string correctStr = readLine("Nhap Dap an dung moi (A/B/C/D): ");
                char correct = correctStr.empty() ? 'A' : correctStr[0];

                BusinessResult res = editQuestionWorkflow(
                    root, sub->id, qId, content.c_str(),
                    a.c_str(), b.c_str(), c.c_str(), d.c_str(), correct
                );

                if (res == SUCCESS) {
                    cout << "-> Hieu chinh cau hoi thanh cong!\n";
                } else {
                    cout << "-> Loi: " << getErrorMessage(res) << "\n";
                }
                pauseScreen();
                break;
            }
            case 4: {
                Subject* sub = selectSubjectFromList(root);
                if (sub == nullptr) {
                    pauseScreen();
                    break;
                }
                cout << "\n-> Xoa cau hoi thuoc mon: [" << sub->id << "] " << sub->name << "\n";
                int qId = readInt("Nhap ID cau hoi can xoa: ");
                BusinessResult res = deleteQuestionWorkflow(root, sub->id, qId);
                if (res == SUCCESS) {
                    cout << "-> Xoa cau hoi thanh cong!\n";
                } else {
                    cout << "-> Loi: " << getErrorMessage(res) << "\n";
                }
                pauseScreen();
                break;
            }
            default:
                cout << "Lua chon khong hop le!\n";
                break;
        }
    }
}

static void teacherClassMenu(Class* classes[], int& classCount) {
    while (true) {
        cout << "\n-------------------------------------------------------------\n";
        cout << "                  QUAN LY LOP HOC                            \n";
        cout << "-------------------------------------------------------------\n";
        cout << "1. Xem danh sach lop hoc\n";
        cout << "2. Them lop hoc moi\n";
        cout << "3. Xoa lop hoc\n";
        cout << "0. Quay lai menu chinh\n";
        int choice = readInt("Lua chon cua ban: ");

        if (choice == 0) break;

        switch (choice) {
            case 1: {
                cout << "\n=== DANH SACH LOP HOC ===\n";
                printClasses(classes, classCount);
                pauseScreen();
                break;
            }
            case 2: {
                string id = readLine("Nhap Ma lop hoc (MALOP <= 19): ");
                string name = readLine("Nhap Ten lop hoc: ");
                BusinessResult res = addClassWorkflow(classes, classCount, id.c_str(), name.c_str());
                if (res == SUCCESS) {
                    cout << "-> Them lop hoc thanh cong!\n";
                } else {
                    cout << "-> Loi: " << getErrorMessage(res) << "\n";
                }
                pauseScreen();
                break;
            }
            case 3: {
                Class* target = selectClassFromList(classes, classCount);
                if (target == nullptr) {
                    pauseScreen();
                    break;
                }
                string delId = target->id;
                string prompt = "Ban co chac chan muon xoa lop [" + delId + "] " + string(target->name) + "? (Y/N): ";
                string confirm = readLine(prompt.c_str());
                if (confirm.empty() || (confirm[0] != 'Y' && confirm[0] != 'y')) {
                    cout << "-> Da huy xoa lop hoc.\n";
                    pauseScreen();
                    break;
                }
                BusinessResult res = deleteClassWorkflow(classes, classCount, delId.c_str());
                if (res == SUCCESS) {
                    cout << "-> Xoa lop hoc thanh cong!\n";
                } else {
                    cout << "-> Loi: " << getErrorMessage(res) << "\n";
                }
                pauseScreen();
                break;
            }
            default:
                cout << "Lua chon khong hop le!\n";
                break;
        }
    }
}

static void teacherStudentMenu(Class* classes[], int classCount) {
    while (true) {
        cout << "\n-------------------------------------------------------------\n";
        cout << "                  QUAN LY SINH VIEN                          \n";
        cout << "-------------------------------------------------------------\n";
        cout << "1. Xem danh sach sinh vien theo lop\n";
        cout << "2. Them sinh vien vao lop\n";
        cout << "3. Hieu chinh sinh vien\n";
        cout << "4. Xoa sinh vien\n";
        cout << "0. Quay lai menu chinh\n";
        int choice = readInt("Lua chon cua ban: ");

        if (choice == 0) break;

        switch (choice) {
            case 1: {
                Class* c = selectClassFromList(classes, classCount);
                if (c == nullptr) {
                    pauseScreen();
                    break;
                }
                cout << "\n=== DANH SACH SINH VIEN LOP " << c->id << " - " << c->name << " ===\n";
                printStudents(c);
                pauseScreen();
                break;
            }
            case 2: {
                Class* classroom = selectClassFromList(classes, classCount);
                if (classroom == nullptr) {
                    pauseScreen();
                    break;
                }

                cout << "\n>>> BAT DAU NHAP SINH VIEN VAO LOP " << classroom->id << " - " << classroom->name << " <<<\n";
                cout << "(Meo: Nhap '0' hoac de trong tai 'Ma sinh vien' de DUNG NHAP bat cu luc nao)\n\n";

                int addedCount = 0;
                while (true) {
                    cout << "--- Nhap thong tin sinh vien thu " << (addedCount + 1) << " ---\n";
                    string sId = readLine("Nhap Ma sinh vien (nhap 0 de DUNG): ");
                    if (sId.empty() || sId == "0") {
                        cout << "-> Da dung nhap sinh vien theo yeu cau cua ban.\n";
                        break;
                    }

                    string ho = readLine("Nhap Ho dem: ");
                    string ten = readLine("Nhap Ten: ");
                    string gender = readLine("Nhap Phai (Nam/Nu): ");
                    string pass = readLine("Nhap Mat khau: ");

                    BusinessResult res = addStudentWorkflow(
                        classes, classCount, classroom->id,
                        sId.c_str(), ho.c_str(), ten.c_str(),
                        gender.c_str(), pass.c_str()
                    );

                    if (res == SUCCESS) {
                        addedCount++;
                        cout << "-> Da them thanh cong SV: " << ho << " " << ten << " (" << sId << ")!\n\n";
                    } else {
                        cout << "-> Loi: " << getErrorMessage(res) << ". Vui long thu lai.\n\n";
                    }
                }

                cout << "=============================================================\n";
                cout << "Tong ket: Da them thanh cong " << addedCount << " sinh vien vao lop " << classroom->id << ".\n";
                cout << "=============================================================\n";
                pauseScreen();
                break;
            }
            case 3: {
                Class* c = selectClassFromList(classes, classCount);
                if (c == nullptr) {
                    pauseScreen();
                    break;
                }
                Student* s = selectStudentFromClass(c);
                if (s == nullptr) {
                    pauseScreen();
                    break;
                }
                cout << "\n-> Ban dang hieu chinh SV: [" << s->id << "] " << s->ho << " " << s->ten << "\n";
                string ho = readLine("Nhap Ho dem moi: ");
                string ten = readLine("Nhap Ten moi: ");
                string gender = readLine("Nhap Phai moi (Nam/Nu): ");
                string pass = readLine("Nhap Mat khau moi: ");

                BusinessResult res = editStudentWorkflow(
                    classes, classCount, s->id,
                    ho.c_str(), ten.c_str(), gender.c_str(), pass.c_str()
                );

                if (res == SUCCESS) {
                    cout << "-> Hieu chinh sinh vien thanh cong!\n";
                } else {
                    cout << "-> Loi: " << getErrorMessage(res) << "\n";
                }
                pauseScreen();
                break;
            }
            case 4: {
                Class* c = selectClassFromList(classes, classCount);
                if (c == nullptr) {
                    pauseScreen();
                    break;
                }
                Student* s = selectStudentFromClass(c);
                if (s == nullptr) {
                    pauseScreen();
                    break;
                }
                string delId = s->id;
                string prompt = "Ban co chac chan muon xoa sinh vien [" + delId + "] " + string(s->ho) + " " + string(s->ten) + "? (Y/N): ";
                string confirm = readLine(prompt.c_str());
                if (confirm.empty() || (confirm[0] != 'Y' && confirm[0] != 'y')) {
                    cout << "-> Da huy xoa sinh vien.\n";
                    pauseScreen();
                    break;
                }
                BusinessResult res = deleteStudentWorkflow(classes, classCount, c->id, delId.c_str());
                if (res == SUCCESS) {
                    cout << "-> Xoa sinh vien thanh cong!\n";
                } else {
                    cout << "-> Loi: " << getErrorMessage(res) << "\n";
                }
                pauseScreen();
                break;
            }
            default:
                cout << "Lua chon khong hop le!\n";
                break;
        }
    }
}

static void teacherMenu(Subject*& root, Class* classes[], int& classCount) {
    while (true) {
        cout << "\n=============================================================\n";
        cout << "                     MENU GIANG VIEN                         \n";
        cout << "=============================================================\n";
        cout << "1. Quan ly Mon hoc\n";
        cout << "2. Quan ly Cau hoi thi\n";
        cout << "3. Quan ly Lop hoc\n";
        cout << "4. Quan ly Sinh vien\n";
        cout << "5. In Bang diem lop theo mon hoc\n";
        cout << "6. Xem chi tiet bai thi cua mot sinh vien\n";
        cout << "0. Dang xuat (Quay lai man hinh chinh)\n";
        int choice = readInt("Lua chon cua ban: ");

        if (choice == 0) break;

        switch (choice) {
            case 1:
                teacherSubjectMenu(root, classes, classCount);
                break;
            case 2:
                teacherQuestionMenu(root);
                break;
            case 3:
                teacherClassMenu(classes, classCount);
                break;
            case 4:
                teacherStudentMenu(classes, classCount);
                break;
            case 5: {
                Class* c = selectClassFromList(classes, classCount);
                if (c == nullptr) {
                    pauseScreen();
                    break;
                }
                Subject* sub = selectSubjectFromList(root);
                if (sub == nullptr) {
                    pauseScreen();
                    break;
                }
                BusinessResult res = printClassScoreTable(classes, classCount, root, c->id, sub->id);
                if (res != SUCCESS) {
                    cout << "-> Loi: " << getErrorMessage(res) << "\n";
                }
                pauseScreen();
                break;
            }
            case 6: {
                Class* c = selectClassFromList(classes, classCount);
                if (c == nullptr) {
                    pauseScreen();
                    break;
                }
                Student* s = selectStudentFromClass(c);
                if (s == nullptr) {
                    pauseScreen();
                    break;
                }
                Subject* sub = selectSubjectFromList(root);
                if (sub != nullptr) {
                    BusinessResult res = printExamDetailReview(s, sub->id);
                    if (res != SUCCESS) {
                        cout << "-> Loi: " << getErrorMessage(res) << "\n";
                    }
                }
                pauseScreen();
                break;
            }
            default:
                cout << "Lua chon khong hop le!\n";
                break;
        }
    }
}

static void runExamLoop(
    Subject*,
    Student* student,
    const string& subId,
    ExamDetail* examHead,
    int initialRemainingSeconds,
    int totalQuestions
) {
    ExamTimer timer;
    timer.resume(initialRemainingSeconds);

    cout << "\n>>> DANG THI MON " << subId << " (" << totalQuestions << " cau, con " 
         << (initialRemainingSeconds / 60) << " phut " << (initialRemainingSeconds % 60) << " giay) <<<\n";
    cout << "Nhap A, B, C, D de chon dap an. Nhap N (Next), P (Prev), S (Submit - Nop bai).\n";

    ExamDetail** questionRefs = new ExamDetail*[totalQuestions];
    ExamDetail* cur = examHead;
    int count = 0;
    while (cur != nullptr && count < totalQuestions) {
        questionRefs[count++] = cur;
        cur = cur->next;
    }

    saveExamSession(student->id, subId.c_str(), timer.getRemainingSeconds(), examHead);

    int currentIdx = 0;
    while (true) {
        if (timer.isExpired()) {
            cout << "\n=============================================================\n";
            cout << ">>> DA HET THOI GIAN LAM BAI! He thong tu dong nop bai... <<<\n";
            cout << "=============================================================\n";
            break;
        }

        int remainingSeconds = timer.getRemainingSeconds();
        ExamDetail* q = questionRefs[currentIdx];
        char timeDisplay[64];
        formatExamTime(remainingSeconds, timeDisplay, sizeof(timeDisplay));

        cout << "\n-------------------------------------------------------------\n";
        cout << "[DONG HO DEM THOI GIAN: " << timeDisplay << "]\n";
        cout << "Cau " << (currentIdx + 1) << " / " << totalQuestions << ": " << q->content << "\n";
        cout << "  A. " << q->answerA << "\n";
        cout << "  B. " << q->answerB << "\n";
        cout << "  C. " << q->answerC << "\n";
        cout << "  D. " << q->answerD << "\n";
        cout << "  -> Da chon: " << (q->selectedAnswer == '\0' ? "(Chua chon)" : string(1, q->selectedAnswer)) << "\n";
        cout << "-------------------------------------------------------------\n";
        cout << "[A/B/C/D] Chon | [N] Cau tiep | [P] Cau truoc | [S] Nop bai\n";

        string act = readLine("Hanh dong cua ban: ");

        if (timer.isExpired()) {
            cout << "\n=============================================================\n";
            cout << ">>> DA HET THOI GIAN LAM BAI! He thong tu dong nop bai... <<<\n";
            cout << "=============================================================\n";
            break;
        }

        if (act.empty()) continue;

        char cmd = act[0];
        if (cmd >= 'a' && cmd <= 'z') cmd = (char)(cmd - ('a' - 'A'));

        if (cmd == 'A' || cmd == 'B' || cmd == 'C' || cmd == 'D') {
            q->selectedAnswer = cmd;
            saveExamSession(student->id, subId.c_str(), timer.getRemainingSeconds(), examHead);
            if (currentIdx < totalQuestions - 1) {
                currentIdx++;
                saveExamSession(student->id, subId.c_str(), timer.getRemainingSeconds(), examHead);
            }
        } else if (cmd == 'N') {
            if (currentIdx < totalQuestions - 1) {
                currentIdx++;
                saveExamSession(student->id, subId.c_str(), timer.getRemainingSeconds(), examHead);
            } else {
                cout << "Ban dang o cau cuoi cung!\n";
            }
        } else if (cmd == 'P') {
            if (currentIdx > 0) {
                currentIdx--;
                saveExamSession(student->id, subId.c_str(), timer.getRemainingSeconds(), examHead);
            } else {
                cout << "Ban dang o cau dau tien!\n";
            }
        } else if (cmd == 'S') {
            string confirm = readLine("Ban co chac chan muon nop bai? (Y/N): ");
            if (!confirm.empty() && (confirm[0] == 'Y' || confirm[0] == 'y')) {
                break;
            }
        }
    }

    delete[] questionRefs;

    float finalScore = 0.0f;
    int correctCount = 0;
    int totalCount = 0;

    BusinessResult subRes = submitExamWorkflow(student, subId.c_str(), examHead, finalScore, correctCount, totalCount);
    removeExamSession(student->id);

    if (subRes == SUCCESS) {
        cout << "\n=============================================================\n";
        cout << "                   KET QUA BAI THI CUA BAN                   \n";
        cout << "=============================================================\n";
        cout << "Mon thi       : " << subId << "\n";
        cout << "So cau dung   : " << correctCount << " / " << totalCount << "\n";
        cout << "Diem so dat duoc: " << finalScore << " / 10.0\n";
        cout << "=============================================================\n";
    } else {
        cout << "-> Loi khi ghi nhan ket qua: " << getErrorMessage(subRes) << "\n";
    }

    pauseScreen();
}

static void takeExamSession(Subject* root, Student* student) {
    char pendSubId[SUBJECT_ID_LEN] = {};
    int pendSeconds = 0;
    if (checkExamSession(student->id, pendSubId, pendSeconds)) {
        cout << "\n=============================================================\n";
        cout << " [THONG BAO] Tim thay phien thi do dang chua hoan thanh:\n";
        cout << " Mon thi: " << pendSubId << " | Thoi gian con lai: " 
             << (pendSeconds / 60) << " phut " << (pendSeconds % 60) << " giay\n";
        cout << "=============================================================\n";
        string resumeChoice = readLine("Ban co muon tiep tuc lam tiep phien thi nay? (Y/N): ");
        if (!resumeChoice.empty() && (resumeChoice[0] == 'Y' || resumeChoice[0] == 'y')) {
            ExamDetail* examHead = nullptr;
            if (loadExamSession(student->id, pendSubId, pendSeconds, examHead, root)) {
                int totalQ = 0;
                ExamDetail* t = examHead;
                while (t != nullptr) {
                    totalQ++;
                    t = t->next;
                }
                runExamLoop(root, student, pendSubId, examHead, pendSeconds, totalQ);
                return;
            } else {
                cout << "-> Khong the khoi phuc phien thi cu. Se tien hanh xoa file tam.\n";
                removeExamSession(student->id);
            }
        } else {
            string discardChoice = readLine("Ban co muon huy bo phien thi do dang nay de thi mon khac? (Y/N): ");
            if (!discardChoice.empty() && (discardChoice[0] == 'Y' || discardChoice[0] == 'y')) {
                removeExamSession(student->id);
                cout << "-> Da xoa phien thi do dang.\n";
            } else {
                return;
            }
        }
    }

    Subject* targetSub = selectSubjectFromList(root);
    if (targetSub == nullptr) {
        pauseScreen();
        return;
    }
    string subId = targetSub->id;
    int availableQ = countQuestions(targetSub->questionList);
    cout << "\n-> Mon [" << subId << " - " << targetSub->name << "] hien co " << availableQ << " cau hoi trong ngan hang de.\n";

    if (availableQ <= 0) {
        cout << "-> Mon nay hien chua co cau hoi nao! Khong the bat dau thi.\n";
        pauseScreen();
        return;
    }

    string promptQ = "Nhap So luong cau hoi (1 - " + to_string(availableQ) + " cau): ";
    int numQuestions = readInt(promptQ.c_str());
    if (numQuestions <= 0 || numQuestions > availableQ) {
        cout << "-> So luong cau hoi khong hop le (phai tu 1 den " << availableQ << " cau)!\n";
        pauseScreen();
        return;
    }

    int minutes = readInt("Nhap Thoi gian thi (so phut): ");
    if (minutes <= 0) {
        cout << "-> Thoi gian thi phai lon hon 0 phut!\n";
        pauseScreen();
        return;
    }

    ExamDetail* examHead = nullptr;
    BusinessResult prepRes = prepareExamWorkflow(root, student, subId.c_str(), numQuestions, minutes, examHead);

    if (prepRes != SUCCESS) {
        cout << "-> Khong the bat dau thi: " << getErrorMessage(prepRes) << "\n";
        pauseScreen();
        return;
    }

    runExamLoop(root, student, subId, examHead, minutes * 60, numQuestions);
}

static void studentMenu(Subject* root, Student* student) {
    while (true) {
        cout << "\n=============================================================\n";
        cout << " MENU SINH VIEN: " << student->id << " - " << student->ho << " " << student->ten << "\n";
        cout << "=============================================================\n";
        cout << "1. Vao thi trac nghiem\n";
        cout << "2. Xem bang diem ca nhan\n";
        cout << "3. Xem lai chi tiet bai thi da lam\n";
        cout << "0. Dang xuat (Quay lai man hinh chinh)\n";
        int choice = readInt("Lua chon cua ban: ");

        if (choice == 0) break;

        switch (choice) {
            case 1:
                takeExamSession(root, student);
                break;
            case 2:
                printStudentScores(student, root);
                pauseScreen();
                break;
            case 3: {
                Subject* sub = selectSubjectFromList(root);
                if (sub == nullptr) {
                    pauseScreen();
                    break;
                }
                BusinessResult res = printExamDetailReview(student, sub->id);
                if (res != SUCCESS) {
                    cout << "-> Loi: " << getErrorMessage(res) << "\n";
                }
                pauseScreen();
                break;
            }
            default:
                cout << "Lua chon khong hop le!\n";
                break;
        }
    }
}

void runConsoleApp() {
    Subject* root = nullptr;
    Class* classes[MAX_CLASS] = {};
    int classCount = 0;

    cout << "Dang nap du lieu tu thu muc 'data'...\n";
    if (loadAllData(root, classes, classCount, "data")) {
        cout << "-> Nap du lieu thanh cong!\n";
    } else {
        cout << "-> Khong tim thay du lieu cu hoac thu muc rong. Khoi tao he thong moi.\n";
    }

    while (true) {
        cout << "\n=============================================================\n";
        cout << "       HE THONG THI TRAC NGHIEM C++ (DATA STRUCTURES)        \n";
        cout << "=============================================================\n";
        cout << "1. Dang nhap\n";
        cout << "0. Luu du lieu va Thoat\n";
        int choice = readInt("Lua chon cua ban: ");

        if (choice == 0) {
            cout << "\nDang luu du lieu he thong vao thu muc 'data'...\n";
            if (saveAllData(root, classes, classCount, "data")) {
                cout << "-> Luu du lieu thanh cong!\n";
            } else {
                cout << "-> Canh bao: Khong the luu du lieu vao thu muc 'data'!\n";
            }
            break;
        }

        if (choice == 1) {
            string username = readLine("Ten dang nhap (GV hoac MASV): ");
            string password = readLine("Mat khau: ");

            LoginRole role = LOGIN_FAILED;
            Student* loggedStudent = nullptr;

            BusinessResult authRes = loginWorkflow(
                classes, classCount,
                username.c_str(), password.c_str(),
                role, loggedStudent
            );

            if (authRes != SUCCESS) {
                cout << "-> Dang nhap that bai: " << getErrorMessage(authRes) << "\n";
                pauseScreen();
                continue;
            }

            if (role == LOGIN_TEACHER) {
                cout << "\n-> Xin chao Giang Vien!\n";
                teacherMenu(root, classes, classCount);
            } else if (role == LOGIN_STUDENT && loggedStudent != nullptr) {
                cout << "\n-> Xin chao Sinh vien: " << loggedStudent->ho << " " << loggedStudent->ten << "!\n";

                char pendSub[SUBJECT_ID_LEN] = {};
                int pendSec = 0;
                if (checkExamSession(loggedStudent->id, pendSub, pendSec)) {
                    cout << "\n=============================================================\n";
                    cout << ">>> CANH BAO: Ban dang co 1 bai thi DO DANG chua hoan tat! <<<\n";
                    cout << "    Mon thi          : " << pendSub << "\n";
                    cout << "    Thoi gian con lai: " << (pendSec / 60) << " phut " << (pendSec % 60) << " giay\n";
                    cout << "    Vui long chon muc '1. Vao thi trac nghiem' de tiep tuc lam bai.\n";
                    cout << "=============================================================\n";
                }

                studentMenu(root, loggedStudent);
            }
        }
    }

    cout << "Dang giai phong bo nho RAM...\n";
    while (classCount > 0) {
        deleteClass(classes, classCount, classes[0]->id);
    }
    while (root != nullptr) {
        root = deleteSubject(root, root->id);
    }
    cout << "-> Da thoat chuong trinh an toan.\n";
}
