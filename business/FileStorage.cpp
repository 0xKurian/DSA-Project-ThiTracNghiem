#include "FileStorage.h"
#include "../core/ClassArray.h"
#include "../core/QuestionList.h"
#include "../core/ScoreList.h"
#include "../core/StudentList.h"
#include "../core/SubjectBST.h"
#include "../core/StringUtils.h"

#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>

#if defined(_WIN32)
#include <direct.h>
#define CREATE_DIR(d) _mkdir(d)
#else
#include <sys/stat.h>
#define CREATE_DIR(d) mkdir(d, 0755)
#endif

static void trimLine(std::string& s) {
    while (!s.empty() && (s.back() == '\r' || s.back() == '\n' || s.back() == ' ' || s.back() == '\t')) {
        s.pop_back();
    }
    size_t start = 0;
    while (start < s.size() && (s[start] == ' ' || s[start] == '\t' || s[start] == '\r' || s[start] == '\n')) {
        start++;
    }
    if (start > 0) {
        s = s.substr(start);
    }
}

static int splitPipe(const std::string& line, std::string tokens[], int maxTokens) {
    if (maxTokens <= 0) return 0;
    int count = 0;
    size_t start = 0;
    while (count < maxTokens) {
        size_t pos = line.find('|', start);
        if (pos == std::string::npos) {
            tokens[count++] = line.substr(start);
            break;
        } else {
            tokens[count++] = line.substr(start, pos - start);
            start = pos + 1;
        }
    }
    for (int i = 0; i < count; ++i) {
        while (!tokens[i].empty() && (tokens[i].back() == '\r' || tokens[i].back() == '\n')) {
            tokens[i].pop_back();
        }
    }
    return count;
}

static std::string extractDirectory(const std::string& filepath) {
    size_t lastSlash = filepath.find_last_of("/\\");
    if (lastSlash == std::string::npos) {
        return "";
    }
    return filepath.substr(0, lastSlash);
}

static std::string getSubjectQuestionFilePath(const std::string& dir, const char* subjectId) {
    if (dir.empty()) {
        return std::string(subjectId) + ".txt";
    }
    return dir + "/" + subjectId + ".txt";
}

static bool isQuestionIdUsedAnywhere(Subject* root, int id) {
    if (root == nullptr) return false;
    if (findQuestion(root->questionList, id) != nullptr) return true;
    if (isQuestionIdUsedAnywhere(root->left, id)) return true;
    if (isQuestionIdUsedAnywhere(root->right, id)) return true;
    return false;
}

static int countSubjectsInTree(Subject* root) {
    if (root == nullptr) return 0;
    return 1 + countSubjectsInTree(root->left) + countSubjectsInTree(root->right);
}

bool createEmptyQuestionFile(const char* dir, const char* subjectId) {
    if (subjectId == nullptr || stringLength(subjectId) == 0) return false;
    if (dir != nullptr && stringLength(dir) > 0) {
        CREATE_DIR(dir);
    }
    std::string path = getSubjectQuestionFilePath(dir ? dir : "", subjectId);
    std::ifstream check(path);
    if (check.is_open()) {
        return true;
    }
    std::ofstream out(path);
    return out.is_open();
}

bool renameSubjectQuestionFile(const char* dir, const char* oldId, const char* newId) {
    if (oldId == nullptr || newId == nullptr) return false;
    if (stringEqual(oldId, newId)) return true;

    std::string oldPath = getSubjectQuestionFilePath(dir ? dir : "", oldId);
    std::string newPath = getSubjectQuestionFilePath(dir ? dir : "", newId);

    std::ifstream checkNew(newPath);
    if (checkNew.is_open()) {
        checkNew.close();
        std::remove(newPath.c_str());
    }

    std::ifstream checkOld(oldPath);
    if (!checkOld.is_open()) {

        return createEmptyQuestionFile(dir, newId);
    }
    checkOld.close();

    return (std::rename(oldPath.c_str(), newPath.c_str()) == 0);
}

bool deleteSubjectQuestionFile(const char* dir, const char* subjectId) {
    if (subjectId == nullptr || stringLength(subjectId) == 0) return false;
    std::string path = getSubjectQuestionFilePath(dir ? dir : "", subjectId);
    std::ifstream check(path);
    if (!check.is_open()) {
        return true;
    }
    check.close();
    return (std::remove(path.c_str()) == 0);
}

static void writeSubjectsRecursive(Subject* node, std::ostream& out, const std::string& dir) {
    if (node == nullptr) return;

    writeSubjectsRecursive(node->left, out, dir);

    int qCount = countQuestions(node->questionList);

    out << node->id << "|" << node->name << "|" << qCount << "|"
        << node->examQuestionCount << "|" << node->examMinutes << "\n";

    Question* q = node->questionList;
    while (q != nullptr) {

        out << q->id << "|"
            << q->content << "|"
            << q->answerA << "|"
            << q->answerB << "|"
            << q->answerC << "|"
            << q->answerD << "|"
            << q->correctAnswer << "\n";
        q = q->next;
    }

    writeSubjectsRecursive(node->right, out, dir);
}

bool saveSubjectsToFile(Subject* root, const char* filepath) {
    if (filepath == nullptr) return false;

    std::string dir = extractDirectory(filepath);
    if (!dir.empty()) {
        CREATE_DIR(dir.c_str());
    }

    std::ofstream out(filepath);
    if (!out.is_open()) return false;

    int totalSubjects = countSubjectsInTree(root);
    out << totalSubjects << "\n";

    writeSubjectsRecursive(root, out, dir);

    out.close();
    return true;
}

bool loadSubjectsFromFile(Subject*& root, const char* filepath) {
    if (filepath == nullptr) return false;

    std::ifstream in(filepath);
    if (!in.is_open()) return false;

    std::string dir = extractDirectory(filepath);

    std::string line;
    if (!std::getline(in, line)) return false;
    trimLine(line);

    int totalSubjects = 0;
    try {
        totalSubjects = std::stoi(line);
    } catch (...) {
        return false;
    }

    for (int i = 0; i < totalSubjects; ++i) {
        std::string subLine;
        while (std::getline(in, subLine)) {
            trimLine(subLine);
            if (!subLine.empty()) break;
        }
        if (subLine.empty()) break;

        if (subLine.find('|') != std::string::npos) {

            std::string tokens[5];
            int cnt = splitPipe(subLine, tokens, 5);
            std::string subId = tokens[0];
            std::string subName = (cnt >= 2) ? tokens[1] : "";
            int qCountInline = -1;
            if (cnt >= 3 && !tokens[2].empty()) {
                try {
                    qCountInline = std::stoi(tokens[2]);
                } catch (...) {
                    qCountInline = -1;
                }
            }
            int examQCount = 0;
            int examMins = 0;
            if (cnt >= 5) {
                try {
                    if (!tokens[3].empty()) {
                        examQCount = std::stoi(tokens[3]);
                    }
                } catch (...) {
                    examQCount = 0;
                }
                try {
                    if (!tokens[4].empty()) {
                        examMins = std::stoi(tokens[4]);
                    }
                } catch (...) {
                    examMins = 0;
                }
            }
            trimLine(subId);
            trimLine(subName);

            Subject* sub = createSubject(subId.c_str(), subName.c_str());
            if (sub != nullptr) {
                sub->examQuestionCount = examQCount;
                sub->examMinutes = examMins;
                root = insertSubject(root, sub);
            }

            Subject* insertedSub = findSubject(root, subId.c_str());
            if (insertedSub != nullptr) {
                insertedSub->examQuestionCount = examQCount;
                insertedSub->examMinutes = examMins;
            }

            if (qCountInline >= 0) {

                for (int j = 0; j < qCountInline; ++j) {
                    std::string qLine;
                    if (!std::getline(in, qLine)) break;
                    trimLine(qLine);
                    if (qLine.empty()) {
                        --j;
                        continue;
                    }

                    std::string qTokens[7];
                    int qTokensCount = splitPipe(qLine, qTokens, 7);
                    if (qTokensCount < 7) continue;

                    for (int k = 0; k < 7; ++k) {
                        while (!qTokens[k].empty() && (qTokens[k].back() == '\r' || qTokens[k].back() == '\n')) {
                            qTokens[k].pop_back();
                        }
                    }

                    int qId = 0;
                    try {
                        qId = std::stoi(qTokens[0]);
                    } catch (...) {
                        qId = 0;
                    }

                    if (qId <= 0 || isQuestionIdUsedAnywhere(root, qId)) {
                        qId = generateQuestionId();
                    } else {
                        updateQuestionIdCounter(qId);
                    }

                    std::string content = qTokens[1];
                    std::string a = qTokens[2];
                    std::string b = qTokens[3];
                    std::string c = qTokens[4];
                    std::string d = qTokens[5];
                    char correct = (qTokens[6].empty()) ? 'A' : qTokens[6][0];

                    if (insertedSub != nullptr) {
                        Question* q = createQuestion(
                            qId,
                            content.c_str(),
                            a.c_str(),
                            b.c_str(),
                            c.c_str(),
                            d.c_str(),
                            correct
                        );
                        if (q != nullptr) {
                            insertQuestion(insertedSub->questionList, q);
                        }
                    }
                }
            } else {

                std::string qPath = getSubjectQuestionFilePath(dir, subId.c_str());
                std::ifstream qIn(qPath);
                if (qIn.is_open()) {
                    std::string qLine;
                    while (std::getline(qIn, qLine)) {
                        trimLine(qLine);
                        if (qLine.empty()) continue;

                        std::string qTokens[7];
                        int qTokensCount = splitPipe(qLine, qTokens, 7);
                        if (qTokensCount < 7) continue;

                        for (int k = 0; k < 7; ++k) {
                            while (!qTokens[k].empty() && (qTokens[k].back() == '\r' || qTokens[k].back() == '\n')) {
                                qTokens[k].pop_back();
                            }
                        }

                        int qId = 0;
                        try {
                            qId = std::stoi(qTokens[0]);
                        } catch (...) {
                            qId = 0;
                        }

                        if (qId <= 0 || isQuestionIdUsedAnywhere(root, qId)) {
                            qId = generateQuestionId();
                        } else {
                            updateQuestionIdCounter(qId);
                        }

                        std::string content = qTokens[1];
                        std::string a = qTokens[2];
                        std::string b = qTokens[3];
                        std::string c = qTokens[4];
                        std::string d = qTokens[5];
                        char correct = (qTokens[6].empty()) ? 'A' : qTokens[6][0];

                        if (insertedSub != nullptr) {
                            Question* q = createQuestion(
                                qId,
                                content.c_str(),
                                a.c_str(),
                                b.c_str(),
                                c.c_str(),
                                d.c_str(),
                                correct
                            );
                            if (q != nullptr) {
                                insertQuestion(insertedSub->questionList, q);
                            }
                        }
                    }
                    qIn.close();
                }
            }
        } else {

            std::string subId = subLine;
            std::string subName, qCountStr;
            if (!std::getline(in, subName)) break;
            if (!std::getline(in, qCountStr)) break;
            trimLine(subId);
            trimLine(subName);
            trimLine(qCountStr);

            Subject* sub = createSubject(subId.c_str(), subName.c_str());
            if (sub != nullptr) {
                root = insertSubject(root, sub);
            }

            Subject* insertedSub = findSubject(root, subId.c_str());

            int qCount = 0;
            try {
                qCount = std::stoi(qCountStr);
            } catch (...) {
                qCount = 0;
            }

            for (int j = 0; j < qCount; ++j) {
                std::string qIdStr, content, a, b, c, d, correctStr;
                if (!std::getline(in, qIdStr)) break;
                if (!std::getline(in, content)) break;
                if (!std::getline(in, a)) break;
                if (!std::getline(in, b)) break;
                if (!std::getline(in, c)) break;
                if (!std::getline(in, d)) break;
                if (!std::getline(in, correctStr)) break;

                trimLine(qIdStr);
                trimLine(content);
                trimLine(a);
                trimLine(b);
                trimLine(c);
                trimLine(d);
                trimLine(correctStr);

                int qId = 0;
                try {
                    qId = std::stoi(qIdStr);
                } catch (...) {
                    qId = 0;
                }

                if (qId <= 0 || isQuestionIdUsedAnywhere(root, qId)) {
                    qId = generateQuestionId();
                } else {
                    updateQuestionIdCounter(qId);
                }

                char correct = (correctStr.empty()) ? 'A' : correctStr[0];

                if (insertedSub != nullptr) {
                    Question* q = createQuestion(
                        qId,
                        content.c_str(),
                        a.c_str(),
                        b.c_str(),
                        c.c_str(),
                        d.c_str(),
                        correct
                    );

                    if (q != nullptr) {
                        insertQuestion(insertedSub->questionList, q);
                    }
                }
            }
        }
    }

    in.close();
    return true;
}

bool saveClassesToFile(Class* classes[], int classCount, const char* filepath) {
    if (filepath == nullptr) return false;

    std::string dir = extractDirectory(filepath);
    if (!dir.empty()) {
        CREATE_DIR(dir.c_str());
    }

    std::ofstream out(filepath);
    if (!out.is_open()) return false;

    out << classCount << "\n";

    for (int i = 0; i < classCount; ++i) {
        Class* c = classes[i];
        if (c == nullptr) continue;

        out << c->id << "\n";
        out << c->name << "\n";

        int studentCount = 0;
        Student* s = c->studentList;
        while (s != nullptr) {
            studentCount++;
            s = s->next;
        }
        out << studentCount << "\n";

        s = c->studentList;
        while (s != nullptr) {

            int scoreCount = 0;
            Score* sc = s->scoreList;
            while (sc != nullptr) {
                scoreCount++;
                sc = sc->next;
            }

            out << s->id << "|"
                << s->ho << "|"
                << s->ten << "|"
                << s->gender << "|"
                << s->password << "|"
                << scoreCount << "\n";

            sc = s->scoreList;
            while (sc != nullptr) {
                out << sc->subjectId << "\n";
                out << sc->score << "\n";

                int detailCount = 0;
                ExamDetail* d = sc->detailList;
                while (d != nullptr) {
                    detailCount++;
                    d = d->next;
                }
                out << detailCount << "\n";

                d = sc->detailList;
                while (d != nullptr) {
                    char sel = (d->selectedAnswer == '\0') ? '-' : d->selectedAnswer;

                    out << d->questionId << "|"
                        << d->content << "|"
                        << d->answerA << "|"
                        << d->answerB << "|"
                        << d->answerC << "|"
                        << d->answerD << "|"
                        << d->correctAnswer << "|"
                        << sel << "\n";
                    d = d->next;
                }

                sc = sc->next;
            }

            s = s->next;
        }
    }

    out.close();
    return true;
}

bool loadClassesFromFile(Class* classes[], int& classCount, const char* filepath) {
    if (filepath == nullptr) return false;

    std::ifstream in(filepath);
    if (!in.is_open()) return false;

    std::string line;
    if (!std::getline(in, line)) return false;
    trimLine(line);

    int totalClasses = 0;
    try {
        totalClasses = std::stoi(line);
    } catch (...) {
        return false;
    }

    for (int i = 0; i < totalClasses; ++i) {
        std::string classId, className, sCountStr;
        while (std::getline(in, classId)) {
            trimLine(classId);
            if (!classId.empty()) break;
        }
        if (classId.empty()) break;

        if (!std::getline(in, className)) break;
        trimLine(className);

        if (!std::getline(in, sCountStr)) break;
        trimLine(sCountStr);

        Class* classroom = createClass(classId.c_str(), className.c_str());
        bool classAdded = false;
        if (classroom != nullptr) {
            classAdded = addClass(classes, classCount, classroom);
            if (!classAdded) {
                delete classroom;
                classroom = nullptr;
            }
        }

        int studentCount = 0;
        try {
            studentCount = std::stoi(sCountStr);
        } catch (...) {
            studentCount = 0;
        }

        for (int j = 0; j < studentCount; ++j) {
            std::string sLine;
            while (std::getline(in, sLine)) {
                trimLine(sLine);
                if (!sLine.empty()) break;
            }
            if (sLine.empty()) break;

            std::string sId, ho, ten, gender, pass, scoreCountStr;
            if (sLine.find('|') != std::string::npos) {

                std::string sTokens[6];
                int sTokCount = splitPipe(sLine, sTokens, 6);
                sId = sTokens[0];
                ho = (sTokCount > 1) ? sTokens[1] : "";
                ten = (sTokCount > 2) ? sTokens[2] : "";
                gender = (sTokCount > 3) ? sTokens[3] : "";
                pass = (sTokCount > 4) ? sTokens[4] : "";
                scoreCountStr = (sTokCount > 5) ? sTokens[5] : "0";
            } else {

                sId = sLine;
                if (!std::getline(in, ho)) break;
                if (!std::getline(in, ten)) break;
                if (!std::getline(in, gender)) break;
                if (!std::getline(in, pass)) break;
                if (!std::getline(in, scoreCountStr)) break;
            }

            trimLine(sId);
            trimLine(ho);
            trimLine(ten);
            trimLine(gender);
            trimLine(pass);
            trimLine(scoreCountStr);

            Student* student = nullptr;
            if (classroom != nullptr) {
                student = createStudent(
                    sId.c_str(),
                    ho.c_str(),
                    ten.c_str(),
                    gender.c_str(),
                    pass.c_str()
                );
                if (student != nullptr) {
                    if (!addStudent(classroom, student)) {
                        delete student;
                        student = nullptr;
                    }
                }
            }

            int scoreCount = 0;
            try {
                scoreCount = std::stoi(scoreCountStr);
            } catch (...) {
                scoreCount = 0;
            }

            for (int k = 0; k < scoreCount; ++k) {
                std::string subId, scoreValStr, detailCountStr;
                while (std::getline(in, subId)) {
                    trimLine(subId);
                    if (!subId.empty()) break;
                }
                if (subId.empty()) break;

                if (!std::getline(in, scoreValStr)) break;
                trimLine(scoreValStr);

                if (!std::getline(in, detailCountStr)) break;
                trimLine(detailCountStr);

                float scoreVal = 0.0f;
                try {
                    scoreVal = std::stof(scoreValStr);
                } catch (...) {
                    scoreVal = 0.0f;
                }

                int detailCount = 0;
                try {
                    detailCount = std::stoi(detailCountStr);
                } catch (...) {
                    detailCount = 0;
                }

                ExamDetail* detailHead = nullptr;

                for (int m = 0; m < detailCount; ++m) {
                    std::string dLine;
                    while (std::getline(in, dLine)) {
                        trimLine(dLine);
                        if (!dLine.empty()) break;
                    }
                    if (dLine.empty()) break;

                    std::string qIdStr, content, a, b, c, d, correctStr, selectedStr;
                    if (dLine.find('|') != std::string::npos) {

                        std::string dTokens[8];
                        int dTokCount = splitPipe(dLine, dTokens, 8);
                        qIdStr = dTokens[0];
                        content = (dTokCount > 1) ? dTokens[1] : "";
                        a = (dTokCount > 2) ? dTokens[2] : "";
                        b = (dTokCount > 3) ? dTokens[3] : "";
                        c = (dTokCount > 4) ? dTokens[4] : "";
                        d = (dTokCount > 5) ? dTokens[5] : "";
                        correctStr = (dTokCount > 6) ? dTokens[6] : "A";
                        selectedStr = (dTokCount > 7) ? dTokens[7] : "-";
                    } else {

                        qIdStr = dLine;
                        if (!std::getline(in, content)) break;
                        if (!std::getline(in, a)) break;
                        if (!std::getline(in, b)) break;
                        if (!std::getline(in, c)) break;
                        if (!std::getline(in, d)) break;
                        if (!std::getline(in, correctStr)) break;
                        if (!std::getline(in, selectedStr)) break;
                    }

                    trimLine(qIdStr);
                    trimLine(content);
                    trimLine(a);
                    trimLine(b);
                    trimLine(c);
                    trimLine(d);
                    trimLine(correctStr);
                    trimLine(selectedStr);

                    ExamDetail* detail = new ExamDetail;
                    try {
                        detail->questionId = std::stoi(qIdStr);
                    } catch (...) {
                        detail->questionId = 0;
                    }
                    stringCopy(detail->content, content.c_str(), QUESTION_CONTENT_LEN);
                    stringCopy(detail->answerA, a.c_str(), ANSWER_CONTENT_LEN);
                    stringCopy(detail->answerB, b.c_str(), ANSWER_CONTENT_LEN);
                    stringCopy(detail->answerC, c.c_str(), ANSWER_CONTENT_LEN);
                    stringCopy(detail->answerD, d.c_str(), ANSWER_CONTENT_LEN);
                    detail->correctAnswer = (correctStr.empty()) ? 'A' : correctStr[0];

                    char sel = (selectedStr.empty() || selectedStr[0] == '-') ? '\0' : selectedStr[0];
                    detail->selectedAnswer = sel;
                    detail->next = nullptr;

                    addExamDetail(detailHead, detail);
                }

                if (student != nullptr) {
                    if (!addScore(student, subId.c_str(), scoreVal, detailHead)) {
                        deleteExamDetailList(detailHead);
                    }
                } else {
                    deleteExamDetailList(detailHead);
                }
            }
        }
    }

    in.close();
    return true;
}

bool saveAllData(
    Subject* root,
    Class* classes[],
    int classCount,
    const char* dir
) {
    if (dir != nullptr && stringLength(dir) > 0) {
        CREATE_DIR(dir);
    }

    std::string baseDir = (dir != nullptr && stringLength(dir) > 0) ? (std::string(dir) + "/") : "";
    std::string subFile = baseDir + "subjects.txt";
    std::string classFile = baseDir + "classes.txt";

    bool s1 = saveSubjectsToFile(root, subFile.c_str());
    bool s2 = saveClassesToFile(classes, classCount, classFile.c_str());

    return s1 && s2;
}

bool loadAllData(
    Subject*& root,
    Class* classes[],
    int& classCount,
    const char* dir
) {
    std::string baseDir = (dir != nullptr && stringLength(dir) > 0) ? (std::string(dir) + "/") : "";
    std::string subFile = baseDir + "subjects.txt";
    std::string classFile = baseDir + "classes.txt";

    bool s1 = loadSubjectsFromFile(root, subFile.c_str());
    bool s2 = loadClassesFromFile(classes, classCount, classFile.c_str());

    return s1 || s2;
}
