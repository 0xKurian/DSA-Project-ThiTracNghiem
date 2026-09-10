#ifndef FUNCTION_PROTOTYPE_H
#define FUNCTION_PROTOTYPE_H

#include "Structures.h"

// ==================== QUESTION ====================

Question* createQuestion(
    int id,
    const char content[],
    const char answerA[],
    const char answerB[],
    const char answerC[],
    const char answerD[],
    char correctAnswer
);

void insertQuestion(Question*& head, Question* question);
Question* findQuestion(Question* head, int id);
bool deleteQuestion(Question*& head, int id);
int countQuestions(Question* head);

void printQuestions(
    Question* head
);

int generateQuestionId();

void updateQuestionIdCounter(
    int id
);


// ==================== SUBJECT ====================

Subject* createSubject(
    const char id[],
    const char name[]
);

Subject* insertSubject(
    Subject* root,
    Subject* subject
);

Subject* findSubject(
    Subject* root,
    const char id[]
);

Subject* deleteSubject(
    Subject* root,
    const char id[]
);

bool editSubject(
    Subject*& root,
    const char oldId[],
    const char newId[],
    const char newName[]
);

void printSubjects(Subject* root);


// ==================== CLASS ====================

Class* createClass(
    const char id[],
    const char name[]
);

Class* findClass(
    Class* classes[],
    int classCount,
    const char id[]
);

bool addClass(
    Class* classes[],
    int& classCount,
    Class* newClass
);

bool deleteClass(
    Class* classes[],
    int& classCount,
    const char id[]
);

void printClasses(
    Class* classes[],
    int classCount
);


// ==================== STUDENT ====================

Student* createStudent(
    const char id[],
    const char ho[],
    const char ten[],
    const char gender[],
    const char password[]
);

Student* findStudent(
    Student* head,
    const char id[]
);

Student* findStudentGlobal(
    Class* classes[],
    int classCount,
    const char id[]
);

bool addStudent(
    Class* classroom,
    Student* student
);

bool deleteStudent(
    Class* classroom,
    const char studentId[]
);

void printStudents(
    Class* classroom
);

bool editStudent(
    Student* student,
    const char ho[],
    const char ten[],
    const char gender[],
    const char password[]
);

// ==================== SCORE ====================

Score* createScore(
    const char subjectId[],
    float score
);

Score* findScore(
    Student* student,
    const char subjectId[]
);

bool addScore(
    Student* student,
    const char subjectId[],
    float score
);

bool updateScore(
    Student* student,
    const char subjectId[],
    float score
);

void printScores(
    Student* student
);

bool deleteScore(
    Student* student,
    const char subjectId[]
);

// ==================== AUTH ====================

LoginRole login(
    Class* classes[],
    int classCount,
    const char username[],
    const char password[],
    Student*& loggedStudent
);


// ==================== EXAM ====================

bool canTakeExam(
    Subject* subject,
    int numberOfQuestions,
    int minutes
);

float calculateScore(
    int correctAnswers,
    int totalQuestions
);


// ==================== FILE ====================

void loadAllData();
void saveAllData();

#endif