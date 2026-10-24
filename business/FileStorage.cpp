#include "FileStorage.h"
#include "../core/ClassArray.h"
#include "../core/ExamConfigList.h"
#include "../core/QuestionList.h"
#include "../core/ScoreList.h"
#include "../core/StringUtils.h"
#include "../core/StudentList.h"
#include "../core/SubjectBST.h"
#include "Validation.h"
#include "SubjectService.h"
#include "PipeCodec.h"


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

static void trimLine(std::string &s) {
  while (!s.empty() && (s.back() == '\r' || s.back() == '\n' ||
                        s.back() == ' ' || s.back() == '\t')) {
    s.pop_back();
  }
  size_t start = 0;
  while (start < s.size() && (s[start] == ' ' || s[start] == '\t' ||
                              s[start] == '\r' || s[start] == '\n')) {
    start++;
  }
  if (start > 0) {
    s = s.substr(start);
  }
}

static int splitPipe(const std::string &line, std::string tokens[],
                     int maxTokens) {
  int count = splitEscapedPipe(line, tokens, maxTokens);
  if (count < 0) return 0;
  for (int i = 0; i < count; ++i) {
    while (!tokens[i].empty() &&
           (tokens[i].back() == '\r' || tokens[i].back() == '\n')) {
      tokens[i].pop_back();
    }
  }
  return count;
}

static std::string extractDirectory(const std::string &filepath) {
  size_t lastSlash = filepath.find_last_of("/\\");
  if (lastSlash == std::string::npos) {
    return "";
  }
  return filepath.substr(0, lastSlash);
}

static std::string getSubjectQuestionFilePath(const std::string &dir,
                                              const char *subjectId) {
  if (dir.empty()) {
    return std::string(subjectId) + ".txt";
  }
  return dir + "/" + subjectId + ".txt";
}

static bool isQuestionIdUsedAnywhere(Subject *root, int id) {
  if (root == nullptr)
    return false;
  if (findQuestion(root->questionList, id) != nullptr)
    return true;
  if (isQuestionIdUsedAnywhere(root->left, id))
    return true;
  if (isQuestionIdUsedAnywhere(root->right, id))
    return true;
  return false;
}

static int countSubjectsInTree(Subject *root) {
  if (root == nullptr)
    return 0;
  return 1 + countSubjectsInTree(root->left) + countSubjectsInTree(root->right);
}

bool createEmptyQuestionFile(const char *dir, const char *subjectId) {
  if (subjectId == nullptr || stringLength(subjectId) == 0)
    return false;
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

bool renameSubjectQuestionFile(const char *dir, const char *oldId,
                               const char *newId) {
  if (oldId == nullptr || newId == nullptr)
    return false;
  if (stringEqual(oldId, newId))
    return true;

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

bool deleteSubjectQuestionFile(const char *dir, const char *subjectId) {
  if (subjectId == nullptr || stringLength(subjectId) == 0)
    return false;
  std::string path = getSubjectQuestionFilePath(dir ? dir : "", subjectId);
  std::ifstream check(path);
  if (!check.is_open()) {
    return true;
  }
  check.close();
  return (std::remove(path.c_str()) == 0);
}

static void writeSubjectsRecursive(Subject *node, std::ostream &out,
                                   const std::string &dir) {
  if (node == nullptr)
    return;

  writeSubjectsRecursive(node->left, out, dir);

  int qCount = countQuestions(node->questionList);

  out << node->id << "|" << escapePipe(node->name) << "|" << qCount << "\n";

  Question *q = node->questionList;
  while (q != nullptr) {

    out << q->id << "|" << escapePipe(q->content) << "|"
        << escapePipe(q->answerA) << "|" << escapePipe(q->answerB)
        << "|" << escapePipe(q->answerC) << "|" << escapePipe(q->answerD)
        << "|" << q->correctAnswer
        << "\n";
    q = q->next;
  }

  writeSubjectsRecursive(node->right, out, dir);
}

bool saveSubjectsToFile(Subject *root, const char *filepath) {
  if (filepath == nullptr)
    return false;

  std::string dir = extractDirectory(filepath);
  if (!dir.empty()) {
    CREATE_DIR(dir.c_str());
  }

  std::ofstream out(filepath);
  if (!out.is_open())
    return false;

  int totalSubjects = countSubjectsInTree(root);
  out << totalSubjects << "\n";

  writeSubjectsRecursive(root, out, dir);

  out.close();
  return true;
}

bool loadSubjectsFromFile(Subject *&root, const char *filepath) {
  if (filepath == nullptr)
    return false;

  std::ifstream in(filepath);
  if (!in.is_open())
    return false;

  std::string dir = extractDirectory(filepath);

  std::string line;
  if (!std::getline(in, line))
    return false;
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
      if (!subLine.empty())
        break;
    }
    if (subLine.empty())
      break;

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
      trimLine(subId);
      trimLine(subName);

      Subject *sub = createSubject(subId.c_str(), subName.c_str());
      if (sub != nullptr) {
        root = insertSubject(root, sub);
      }

      Subject *insertedSub = findSubject(root, subId.c_str());

      if (qCountInline >= 0) {

        for (int j = 0; j < qCountInline; ++j) {
          std::string qLine;
          if (!std::getline(in, qLine))
            break;
          trimLine(qLine);
          if (qLine.empty()) {
            --j;
            continue;
          }

          std::string qTokens[7];
          int qTokensCount = splitPipe(qLine, qTokens, 7);
          if (qTokensCount < 7)
            continue;

          for (int k = 0; k < 7; ++k) {
            while (!qTokens[k].empty() &&
                   (qTokens[k].back() == '\r' || qTokens[k].back() == '\n')) {
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
            Question *q =
                createQuestion(qId, content.c_str(), a.c_str(), b.c_str(),
                               c.c_str(), d.c_str(), correct);
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
            if (qLine.empty())
              continue;

            std::string qTokens[7];
            int qTokensCount = splitPipe(qLine, qTokens, 7);
            if (qTokensCount < 7)
              continue;

            for (int k = 0; k < 7; ++k) {
              while (!qTokens[k].empty() &&
                     (qTokens[k].back() == '\r' || qTokens[k].back() == '\n')) {
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
              Question *q =
                  createQuestion(qId, content.c_str(), a.c_str(), b.c_str(),
                                 c.c_str(), d.c_str(), correct);
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
      if (!std::getline(in, subName))
        break;
      if (!std::getline(in, qCountStr))
        break;
      trimLine(subId);
      trimLine(subName);
      trimLine(qCountStr);

      Subject *sub = createSubject(subId.c_str(), subName.c_str());
      if (sub != nullptr) {
        root = insertSubject(root, sub);
      }

      Subject *insertedSub = findSubject(root, subId.c_str());

      int qCount = 0;
      try {
        qCount = std::stoi(qCountStr);
      } catch (...) {
        qCount = 0;
      }

      for (int j = 0; j < qCount; ++j) {
        std::string qIdStr, content, a, b, c, d, correctStr;
        if (!std::getline(in, qIdStr))
          break;
        if (!std::getline(in, content))
          break;
        if (!std::getline(in, a))
          break;
        if (!std::getline(in, b))
          break;
        if (!std::getline(in, c))
          break;
        if (!std::getline(in, d))
          break;
        if (!std::getline(in, correctStr))
          break;

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
          Question *q =
              createQuestion(qId, content.c_str(), a.c_str(), b.c_str(),
                             c.c_str(), d.c_str(), correct);

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

bool saveClassesToFile(Class *classes[], int classCount, const char *filepath) {
  if (filepath == nullptr)
    return false;

  std::string dir = extractDirectory(filepath);
  if (!dir.empty()) {
    CREATE_DIR(dir.c_str());
  }

  std::ofstream out(filepath);
  if (!out.is_open())
    return false;

  out << classCount << "\n";

  for (int i = 0; i < classCount; ++i) {
    Class *c = classes[i];
    if (c == nullptr)
      continue;

    out << c->id << "\n";
    out << c->name << "\n";

    int studentCount = 0;
    Student *s = c->studentList;
    while (s != nullptr) {
      studentCount++;
      s = s->next;
    }
    out << studentCount << "\n";

    s = c->studentList;
    while (s != nullptr) {

      int scoreCount = 0;
      Score *sc = s->scoreList;
      while (sc != nullptr) {
        scoreCount++;
        sc = sc->next;
      }

      out << s->id << "|" << escapePipe(s->ho) << "|" << escapePipe(s->ten)
          << "|" << escapePipe(s->gender) << "|" << escapePipe(s->password)
          << "|" << scoreCount << "\n";

      sc = s->scoreList;
      while (sc != nullptr) {
        out << sc->subjectId << "\n";
        out << sc->score << "\n";

        int detailCount = 0;
        ExamDetail *d = sc->detailList;
        while (d != nullptr) {
          detailCount++;
          d = d->next;
        }
        out << detailCount << "\n";

        d = sc->detailList;
        while (d != nullptr) {
          char sel = (d->selectedAnswer == '\0') ? '-' : d->selectedAnswer;

          out << d->questionId << "|" << escapePipe(d->content) << "|"
              << escapePipe(d->answerA) << "|" << escapePipe(d->answerB) << "|"
              << escapePipe(d->answerC) << "|" << escapePipe(d->answerD) << "|"
              << d->correctAnswer << "|" << sel << "\n";
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

bool loadClassesFromFile(Class *classes[], int &classCount,
                         const char *filepath) {
  if (filepath == nullptr)
    return false;

  std::ifstream in(filepath);
  if (!in.is_open())
    return false;

  std::string line;
  if (!std::getline(in, line))
    return false;
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
      if (!classId.empty())
        break;
    }
    if (classId.empty())
      break;

    if (!std::getline(in, className))
      break;
    trimLine(className);

    if (!std::getline(in, sCountStr))
      break;
    trimLine(sCountStr);

    Class *classroom = createClass(classId.c_str(), className.c_str());
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
        if (!sLine.empty())
          break;
      }
      if (sLine.empty())
        break;

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
        if (!std::getline(in, ho))
          break;
        if (!std::getline(in, ten))
          break;
        if (!std::getline(in, gender))
          break;
        if (!std::getline(in, pass))
          break;
        if (!std::getline(in, scoreCountStr))
          break;
      }

      trimLine(sId);
      trimLine(ho);
      trimLine(ten);
      trimLine(gender);
      trimLine(pass);
      trimLine(scoreCountStr);

      Student *student = nullptr;
      if (classroom != nullptr) {
        student = createStudent(sId.c_str(), ho.c_str(), ten.c_str(),
                                gender.c_str(), pass.c_str());
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
          if (!subId.empty())
            break;
        }
        if (subId.empty())
          break;

        if (!std::getline(in, scoreValStr))
          break;
        trimLine(scoreValStr);

        if (!std::getline(in, detailCountStr))
          break;
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

        ExamDetail *detailHead = nullptr;

        for (int m = 0; m < detailCount; ++m) {
          std::string dLine;
          while (std::getline(in, dLine)) {
            trimLine(dLine);
            if (!dLine.empty())
              break;
          }
          if (dLine.empty())
            break;

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
            if (!std::getline(in, content))
              break;
            if (!std::getline(in, a))
              break;
            if (!std::getline(in, b))
              break;
            if (!std::getline(in, c))
              break;
            if (!std::getline(in, d))
              break;
            if (!std::getline(in, correctStr))
              break;
            if (!std::getline(in, selectedStr))
              break;
          }

          trimLine(qIdStr);
          trimLine(content);
          trimLine(a);
          trimLine(b);
          trimLine(c);
          trimLine(d);
          trimLine(correctStr);
          trimLine(selectedStr);

          ExamDetail *detail = new ExamDetail;
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

          char sel = (selectedStr.empty() || selectedStr[0] == '-')
                         ? '\0'
                         : selectedStr[0];
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

static bool parsePositiveInt(const std::string& value, int& result) {
  try {
    size_t consumed = 0;
    result = std::stoi(value, &consumed);
    return consumed == value.size() && result > 0;
  } catch (...) {
    return false;
  }
}

static bool isExamConfigTimestamp(const std::string& value) {
  if (value.size() != 16 || value[4] != '-' || value[7] != '-' ||
      value[10] != ' ' || value[13] != ':') return false;
  for (size_t i = 0; i < value.size(); ++i) {
    if (i == 4 || i == 7 || i == 10 || i == 13) continue;
    if (value[i] < '0' || value[i] > '9') return false;
  }
  int month = std::stoi(value.substr(5, 2));
  int day = std::stoi(value.substr(8, 2));
  int hour = std::stoi(value.substr(11, 2));
  int minute = std::stoi(value.substr(14, 2));
  return month >= 1 && month <= 12 && day >= 1 && day <= 31 &&
         hour <= 23 && minute <= 59;
}

bool saveExamConfigsToFile(Class* classes[], int classCount,
                           const char* filepath) {
  if (filepath == nullptr || classes == nullptr || classCount < 0) return false;
  std::string dir = extractDirectory(filepath);
  if (!dir.empty()) CREATE_DIR(dir.c_str());
  std::ofstream out(filepath);
  if (!out.is_open()) return false;
  int total = 0;
  for (int i = 0; i < classCount; ++i) {
    if (classes[i] == nullptr) continue;
    for (ExamConfig* cfg = classes[i]->configList; cfg != nullptr;
         cfg = cfg->next) ++total;
  }
  out << total << '\n';
  for (int i = 0; i < classCount; ++i) {
    if (classes[i] == nullptr) continue;
    for (ExamConfig* cfg = classes[i]->configList; cfg != nullptr;
         cfg = cfg->next) {
      out << classes[i]->id << '|' << cfg->subjectId << '|'
          << cfg->questionCount << '|' << cfg->minutes << '|'
          << cfg->createdAt << '|';
      for (int j = 0; j < cfg->questionCount; ++j) {
        if (j != 0) out << ',';
        out << cfg->questionIds[j];
      }
      out << '\n';
    }
  }
  out.close();
  return !out.fail();
}

bool loadExamConfigsFromFile(Class* classes[], int classCount,
                             const char* filepath) {
  if (filepath == nullptr || classes == nullptr || classCount < 0) return false;
  for (int i = 0; i < classCount; ++i) {
    if (classes[i] != nullptr) deleteExamConfigList(classes[i]->configList);
  }
  std::ifstream in(filepath);
  if (!in.is_open()) return true;
  std::string line;
  if (!std::getline(in, line)) return false;
  trimLine(line);
  int declared = 0;
  try {
    size_t consumed = 0;
    declared = std::stoi(line, &consumed);
    if (consumed != line.size() || declared < 0) return false;
  } catch (...) {
    return false;
  }
  for (int row = 0; row < declared && std::getline(in, line); ++row) {
    trimLine(line);
    int pipes = 0;
    for (size_t i = 0; i < line.size(); ++i) if (line[i] == '|') ++pipes;
    if (pipes != 5) continue;
    std::string fields[6];
    if (splitPipe(line, fields, 6) != 6) continue;
    for (int i = 0; i < 6; ++i) trimLine(fields[i]);
    if (!isValidIdentifier(fields[0].c_str(), CLASS_ID_LEN) ||
        !isValidIdentifier(fields[1].c_str(), SUBJECT_ID_LEN) ||
        !isExamConfigTimestamp(fields[4])) continue;
    char classId[CLASS_ID_LEN];
    char subjectId[SUBJECT_ID_LEN];
    stringCopy(classId, fields[0].c_str(), CLASS_ID_LEN);
    stringCopy(subjectId, fields[1].c_str(), SUBJECT_ID_LEN);
    trimString(classId);
    trimString(subjectId);
    toUpperString(classId);
    toUpperString(subjectId);
    Class* classroom = findClass(classes, classCount, classId);
    if (classroom == nullptr || findExamConfig(classroom, subjectId) != nullptr)
      continue;
    int count = 0;
    int minutes = 0;
    if (!parsePositiveInt(fields[2], count) ||
        !parsePositiveInt(fields[3], minutes) || minutes > 180 ||
        static_cast<size_t>(count) > fields[5].size()) continue;
    int* ids = new int[count];
    bool valid = true;
    size_t start = 0;
    for (int i = 0; i < count; ++i) {
      size_t comma = fields[5].find(',', start);
      if ((i == count - 1 && comma != std::string::npos) ||
          (i < count - 1 && comma == std::string::npos)) {
        valid = false;
        break;
      }
      std::string token = fields[5].substr(start, comma - start);
      if (!parsePositiveInt(token, ids[i])) {
        valid = false;
        break;
      }
      for (int previous = 0; previous < i; ++previous) {
        if (ids[previous] == ids[i]) valid = false;
      }
      if (!valid) break;
      start = comma == std::string::npos ? fields[5].size() : comma + 1;
    }
    if (valid) {
      ExamConfig* cfg = createExamConfig(subjectId, minutes,
                                         fields[4].c_str(), ids, count);
      if (cfg != nullptr && !addExamConfig(classroom, cfg))
        deleteExamConfigList(cfg);
    }
    delete[] ids;
  }
  return true;
}

static void discardInvalidExamConfigs(Subject* root, Class* classes[],
                                      int classCount) {
  for (int i = 0; i < classCount; ++i) {
    if (classes[i] == nullptr) continue;
    ExamConfig* cfg = classes[i]->configList;
    while (cfg != nullptr) {
      ExamConfig* next = cfg->next;
      Subject* subject = findSubject(root, cfg->subjectId);
      bool valid = subject != nullptr && cfg->questionIds != nullptr;
      for (int j = 0; valid && j < cfg->questionCount; ++j) {
        valid = findQuestion(subject->questionList, cfg->questionIds[j]) != nullptr;
      }
      if (!valid) deleteExamConfig(classes[i], cfg->subjectId);
      cfg = next;
    }
  }
}

bool saveAllData(Subject *root, Class *classes[], int classCount,
                 const char *dir) {
  if (dir != nullptr && stringLength(dir) > 0) {
    CREATE_DIR(dir);
  }

  std::string baseDir =
      (dir != nullptr && stringLength(dir) > 0) ? (std::string(dir) + "/") : "";
  std::string subFile = baseDir + "subjects.txt";
  std::string classFile = baseDir + "classes.txt";
  std::string configFile = baseDir + "exam_configs.txt";

  bool s1 = saveSubjectsToFile(root, subFile.c_str());
  bool s2 = saveClassesToFile(classes, classCount, classFile.c_str());
  bool s3 = saveExamConfigsToFile(classes, classCount, configFile.c_str());

  return s1 && s2 && s3;
}

bool loadAllData(Subject *&root, Class *classes[], int &classCount,
                 const char *dir) {
  std::string baseDir =
      (dir != nullptr && stringLength(dir) > 0) ? (std::string(dir) + "/") : "";
  std::string subFile = baseDir + "subjects.txt";
  std::string classFile = baseDir + "classes.txt";
  std::string configFile = baseDir + "exam_configs.txt";

  bool s1 = loadSubjectsFromFile(root, subFile.c_str());
  bool s2 = loadClassesFromFile(classes, classCount, classFile.c_str());
  bool s3 = loadExamConfigsFromFile(classes, classCount, configFile.c_str());
  if (s3) discardInvalidExamConfigs(root, classes, classCount);
  if (s1) rebalanceSubjectBST(root);

  return (s1 || s2) && s3;
}
