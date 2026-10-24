#include "SubjectService.h"
#include "../core/QuestionList.h"
#include "../core/ExamConfigList.h"
#include "../core/ScoreList.h"
#include "../core/StringUtils.h"
#include "../core/SubjectBST.h"
#include "Validation.h"

static int countSubjectNodes(Subject* root) {
  if (root == nullptr) return 0;
  return 1 + countSubjectNodes(root->left) + countSubjectNodes(root->right);
}

static void collectSubjectNodes(Subject* root, Subject* nodes[], int& index) {
  if (root == nullptr) return;
  collectSubjectNodes(root->left, nodes, index);
  nodes[index++] = root;
  collectSubjectNodes(root->right, nodes, index);
}

static Subject* buildBalancedSubjects(Subject* nodes[], int left, int right) {
  if (left > right) return nullptr;
  int mid = left + (right - left) / 2;
  Subject* root = nodes[mid];
  root->left = buildBalancedSubjects(nodes, left, mid - 1);
  root->right = buildBalancedSubjects(nodes, mid + 1, right);
  return root;
}

void rebalanceSubjectBST(Subject*& root) {
  int count = countSubjectNodes(root);
  if (count < 2) return;
  Subject** nodes = new Subject*[count];
  int index = 0;
  collectSubjectNodes(root, nodes, index);
  root = buildBalancedSubjects(nodes, 0, count - 1);
  delete[] nodes;
}

bool isSubjectUsedInScores(Class *classes[], int classCount,
                           const char subjectId[]) {
  if (classes == nullptr || subjectId == nullptr) {
    return false;
  }

  char cleanSubId[SUBJECT_ID_LEN];
  stringCopy(cleanSubId, subjectId, SUBJECT_ID_LEN);
  trimString(cleanSubId);
  toUpperString(cleanSubId);

  for (int i = 0; i < classCount; ++i) {
    if (classes[i] == nullptr)
      continue;

    Student *student = classes[i]->studentList;
    while (student != nullptr) {
      if (findScore(student, cleanSubId) != nullptr) {
        return true;
      }
      student = student->next;
    }
  }
  return false;
}

BusinessResult addSubjectWorkflow(Subject *&root, const char id[],
                                  const char name[]) {
  BusinessResult rId = validateSubjectId(id);
  if (rId != SUCCESS)
    return rId;

  BusinessResult rName = validateSubjectName(name);
  if (rName != SUCCESS)
    return rName;

  char cleanId[SUBJECT_ID_LEN];
  stringCopy(cleanId, id, SUBJECT_ID_LEN);
  trimString(cleanId);
  toUpperString(cleanId);

  char cleanName[SUBJECT_NAME_LEN];
  stringCopy(cleanName, name, SUBJECT_NAME_LEN);
  trimString(cleanName);

  if (findSubject(root, cleanId) != nullptr) {
    return ERR_DUPLICATE;
  }

  Subject *newSubject = createSubject(cleanId, cleanName);
  if (newSubject == nullptr) {
    return ERR_UNKNOWN;
  }

  root = insertSubject(root, newSubject);
  return SUCCESS;
}

BusinessResult editSubjectWorkflow(Subject *&root, Class *classes[],
                                   int classCount, const char oldId[],
                                   const char newId[], const char newName[]) {
  if (oldId == nullptr) {
    return ERR_NOT_FOUND;
  }

  char cleanOldId[SUBJECT_ID_LEN];
  stringCopy(cleanOldId, oldId, SUBJECT_ID_LEN);
  trimString(cleanOldId);
  toUpperString(cleanOldId);

  if (findSubject(root, cleanOldId) == nullptr) {
    return ERR_NOT_FOUND;
  }

  BusinessResult rId = validateSubjectId(newId);
  if (rId != SUCCESS)
    return rId;

  BusinessResult rName = validateSubjectName(newName);
  if (rName != SUCCESS)
    return rName;

  char cleanNewId[SUBJECT_ID_LEN];
  stringCopy(cleanNewId, newId, SUBJECT_ID_LEN);
  trimString(cleanNewId);
  toUpperString(cleanNewId);

  char cleanNewName[SUBJECT_NAME_LEN];
  stringCopy(cleanNewName, newName, SUBJECT_NAME_LEN);
  trimString(cleanNewName);

  if (!stringEqual(cleanOldId, cleanNewId)) {
    if (isSubjectUsedInScores(classes, classCount, cleanOldId)) {
      return ERR_CONSTRAINT_VIOLATION;
    }

    if (findSubject(root, cleanNewId) != nullptr) {
      return ERR_DUPLICATE;
    }
  }

  bool success = editSubject(root, cleanOldId, cleanNewId, cleanNewName);
  if (!success) return ERR_UNKNOWN;
  if (!stringEqual(cleanOldId, cleanNewId) && classes != nullptr) {
    for (int i = 0; i < classCount; ++i) {
      if (classes[i] == nullptr) continue;
      ExamConfig* cfg = findExamConfig(classes[i], cleanOldId);
      if (cfg != nullptr) stringCopy(cfg->subjectId, cleanNewId, SUBJECT_ID_LEN);
    }
  }
  return SUCCESS;
}

BusinessResult deleteSubjectWorkflow(Subject *&root, Class *classes[],
                                     int classCount, const char id[]) {
  if (id == nullptr) {
    return ERR_NOT_FOUND;
  }

  char cleanId[SUBJECT_ID_LEN];
  stringCopy(cleanId, id, SUBJECT_ID_LEN);
  trimString(cleanId);
  toUpperString(cleanId);

  if (findSubject(root, cleanId) == nullptr) {
    return ERR_NOT_FOUND;
  }

  if (isSubjectUsedInScores(classes, classCount, cleanId)) {
    return ERR_CONSTRAINT_VIOLATION;
  }

  if (classes != nullptr) {
    for (int i = 0; i < classCount; ++i) {
      if (classes[i] != nullptr) deleteExamConfig(classes[i], cleanId);
    }
  }
  root = deleteSubject(root, cleanId);
  return SUCCESS;
}

BusinessResult setExamConfigWorkflow(Subject *&root, const char *subjectId,
                                     int questionCount, int minutes) {
  if (root == nullptr || subjectId == nullptr) {
    return ERR_NOT_FOUND;
  }

  char cleanId[SUBJECT_ID_LEN];
  stringCopy(cleanId, subjectId, SUBJECT_ID_LEN);
  trimString(cleanId);
  toUpperString(cleanId);

  if (stringLength(cleanId) == 0) {
    return ERR_NOT_FOUND;
  }

  Subject *subject = findSubject(root, cleanId);
  if (subject == nullptr) {
    return ERR_NOT_FOUND;
  }

  int totalQuestions = countQuestions(subject->questionList);
  if (questionCount < 1 || questionCount > totalQuestions) {
    return ERR_CONSTRAINT_VIOLATION;
  }

  if (minutes < 1 || minutes > 180) {
    return ERR_CONSTRAINT_VIOLATION;
  }

  subject->examQuestionCount = questionCount;
  subject->examMinutes = minutes;
  return SUCCESS;
}
