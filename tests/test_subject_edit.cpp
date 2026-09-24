#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include "../core/SubjectBST.h"
#include "../core/QuestionList.h"

int main() {
    Subject* root = nullptr;
    root = insertSubject(
        root,
        createSubject("OOP", "Lap trinh huong doi tuong")
    );

    Subject* oop = findSubject(root, "OOP");
    assert(oop != nullptr);

    insertQuestion(
        oop->questionList,
        createQuestion(1, "OOP?", "A", "B", "C", "D", 'A')
    );

    assert(editSubject(
        root,
        "OOP",
        "AI",
        "Artificial Intelligence"
    ));

    assert(findSubject(root, "OOP") == nullptr);
    Subject* ai = findSubject(root, "AI");
    assert(ai != nullptr);
    assert(countQuestions(ai->questionList) == 1);

    root = deleteSubject(root, "AI");
    assert(root == nullptr);
    return 0;
}
