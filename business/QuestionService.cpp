#include "QuestionService.h"
#include "Validation.h"
#include "../core/SubjectBST.h"
#include "../core/QuestionList.h"
#include "../core/StringUtils.h"

BusinessResult addQuestionWorkflow(
    Subject* root,
    const char subjectId[],
    const char content[],
    const char answerA[],
    const char answerB[],
    const char answerC[],
    const char answerD[],
    char correctAnswer,
    int& generatedId
) {
    if (root == nullptr || subjectId == nullptr) {
        return ERR_NOT_FOUND;
    }

    char cleanSubId[SUBJECT_ID_LEN];
    stringCopy(cleanSubId, subjectId, SUBJECT_ID_LEN);
    trimString(cleanSubId);
    toUpperString(cleanSubId);

    Subject* subject = findSubject(root, cleanSubId);
    if (subject == nullptr) {
        return ERR_NOT_FOUND;
    }

    BusinessResult rContent = validateQuestionContent(content);
    if (rContent != SUCCESS) return rContent;

    BusinessResult rA = validateAnswerContent(answerA);
    if (rA != SUCCESS) return rA;

    BusinessResult rB = validateAnswerContent(answerB);
    if (rB != SUCCESS) return rB;

    BusinessResult rC = validateAnswerContent(answerC);
    if (rC != SUCCESS) return rC;

    BusinessResult rD = validateAnswerContent(answerD);
    if (rD != SUCCESS) return rD;

    BusinessResult rCorrect = validateCorrectAnswer(correctAnswer);
    if (rCorrect != SUCCESS) return rCorrect;

    char cleanContent[QUESTION_CONTENT_LEN];
    stringCopy(cleanContent, content, QUESTION_CONTENT_LEN);
    trimString(cleanContent);

    char cleanA[ANSWER_CONTENT_LEN];
    stringCopy(cleanA, answerA, ANSWER_CONTENT_LEN);
    trimString(cleanA);

    char cleanB[ANSWER_CONTENT_LEN];
    stringCopy(cleanB, answerB, ANSWER_CONTENT_LEN);
    trimString(cleanB);

    char cleanC[ANSWER_CONTENT_LEN];
    stringCopy(cleanC, answerC, ANSWER_CONTENT_LEN);
    trimString(cleanC);

    char cleanD[ANSWER_CONTENT_LEN];
    stringCopy(cleanD, answerD, ANSWER_CONTENT_LEN);
    trimString(cleanD);

    generatedId = generateQuestionId();

    Question* question = createQuestion(
        generatedId,
        cleanContent,
        cleanA,
        cleanB,
        cleanC,
        cleanD,
        correctAnswer
    );

    if (question == nullptr) {
        return ERR_UNKNOWN;
    }

    insertQuestion(subject->questionList, question);
    return SUCCESS;
}

BusinessResult editQuestionWorkflow(
    Subject* root,
    const char subjectId[],
    int questionId,
    const char content[],
    const char answerA[],
    const char answerB[],
    const char answerC[],
    const char answerD[],
    char correctAnswer
) {
    if (root == nullptr || subjectId == nullptr) {
        return ERR_NOT_FOUND;
    }

    char cleanSubId[SUBJECT_ID_LEN];
    stringCopy(cleanSubId, subjectId, SUBJECT_ID_LEN);
    trimString(cleanSubId);
    toUpperString(cleanSubId);

    Subject* subject = findSubject(root, cleanSubId);
    if (subject == nullptr) {
        return ERR_NOT_FOUND;
    }

    Question* question = findQuestion(subject->questionList, questionId);
    if (question == nullptr) {
        return ERR_NOT_FOUND;
    }

    BusinessResult rContent = validateQuestionContent(content);
    if (rContent != SUCCESS) return rContent;

    BusinessResult rA = validateAnswerContent(answerA);
    if (rA != SUCCESS) return rA;

    BusinessResult rB = validateAnswerContent(answerB);
    if (rB != SUCCESS) return rB;

    BusinessResult rC = validateAnswerContent(answerC);
    if (rC != SUCCESS) return rC;

    BusinessResult rD = validateAnswerContent(answerD);
    if (rD != SUCCESS) return rD;

    BusinessResult rCorrect = validateCorrectAnswer(correctAnswer);
    if (rCorrect != SUCCESS) return rCorrect;

    char cleanContent[QUESTION_CONTENT_LEN];
    stringCopy(cleanContent, content, QUESTION_CONTENT_LEN);
    trimString(cleanContent);

    char cleanA[ANSWER_CONTENT_LEN];
    stringCopy(cleanA, answerA, ANSWER_CONTENT_LEN);
    trimString(cleanA);

    char cleanB[ANSWER_CONTENT_LEN];
    stringCopy(cleanB, answerB, ANSWER_CONTENT_LEN);
    trimString(cleanB);

    char cleanC[ANSWER_CONTENT_LEN];
    stringCopy(cleanC, answerC, ANSWER_CONTENT_LEN);
    trimString(cleanC);

    char cleanD[ANSWER_CONTENT_LEN];
    stringCopy(cleanD, answerD, ANSWER_CONTENT_LEN);
    trimString(cleanD);

    stringCopy(question->content, cleanContent, QUESTION_CONTENT_LEN);
    stringCopy(question->answerA, cleanA, ANSWER_CONTENT_LEN);
    stringCopy(question->answerB, cleanB, ANSWER_CONTENT_LEN);
    stringCopy(question->answerC, cleanC, ANSWER_CONTENT_LEN);
    stringCopy(question->answerD, cleanD, ANSWER_CONTENT_LEN);
    question->correctAnswer = correctAnswer;

    return SUCCESS;
}

BusinessResult deleteQuestionWorkflow(
    Subject* root,
    const char subjectId[],
    int questionId
) {
    if (root == nullptr || subjectId == nullptr) {
        return ERR_NOT_FOUND;
    }

    char cleanSubId[SUBJECT_ID_LEN];
    stringCopy(cleanSubId, subjectId, SUBJECT_ID_LEN);
    trimString(cleanSubId);
    toUpperString(cleanSubId);

    Subject* subject = findSubject(root, cleanSubId);
    if (subject == nullptr) {
        return ERR_NOT_FOUND;
    }

    bool deleted = deleteQuestion(subject->questionList, questionId);
    return deleted ? SUCCESS : ERR_NOT_FOUND;
}
