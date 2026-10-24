#include "FileStorage.h"
#include "QuestionList.h"
#include "SubjectBST.h"

#include <cassert>
#include <fstream>
#include <string>

int main() {
    const char* oldFile = "build/test_legacy_subject_5col.txt";
    const char* newFile = "build/test_subject_3col.txt";
    {
        std::ofstream out(oldFile);
        out << "1\nMATH|Toan hoc|2|10|15\n";
        out << "101|Question 1|A|B|C|D|A\n";
        out << "102|Question 2|A|B|C|D|B\n";
    }
    Subject* root = nullptr;
    assert(loadSubjectsFromFile(root, oldFile));
    Subject* subject = findSubject(root, "MATH");
    assert(subject != nullptr);
    assert(countQuestions(subject->questionList) == 2);
    assert(saveSubjectsToFile(root, newFile));
    {
        std::ifstream in(newFile);
        std::string count;
        std::string header;
        assert(std::getline(in, count));
        assert(std::getline(in, header));
        assert(count == "1");
        assert(header == "MATH|Toan hoc|2");
    }
    while (root != nullptr) root = deleteSubject(root, root->id);
}
