#include <iostream>
#include "core/SubjectBST.h"

using namespace std;

int main() {
    Subject* root = nullptr;

    // ==================== INSERT ====================

    root = insertSubject(
        root,
        createSubject("CTDL", "Cau truc du lieu")
    );

    root = insertSubject(
        root,
        createSubject("OOP", "Lap trinh huong doi tuong")
    );

    root = insertSubject(
        root,
        createSubject("CSDL", "Co so du lieu")
    );

    root = insertSubject(
        root,
        createSubject("TRR", "Toan roi rac")
    );

    cout << "=== SUBJECT LIST ===\n";
    printSubjects(root);

    // ==================== FIND ====================

    cout << "\n=== FIND ===\n";

    Subject* found = findSubject(root, "OOP");

    if (found != nullptr) {
        cout << "Found: "
             << found->id << " - "
             << found->name << '\n';
    }

    found = findSubject(root, "JAVA");

    if (found == nullptr) {
        cout << "JAVA not found\n";
    }

    // ==================== EDIT NAME ====================

    cout << "\n=== EDIT NAME ===\n";

    editSubject(
        root,
        "CTDL",
        "CTDL",
        "Data Structures"
    );

    printSubjects(root);

    // ==================== EDIT ID ====================

    cout << "\n=== EDIT ID ===\n";

    editSubject(
        root,
        "OOP",
        "AI",
        "Artificial Intelligence"
    );

    printSubjects(root);

    // ==================== DUPLICATE ID ====================

    cout << "\n=== EDIT DUPLICATE ID ===\n";

    if (!editSubject(
        root,
        "TRR",
        "CSDL",
        "Test"
    )) {
        cout << "Edit failed\n";
    }

    // ==================== DELETE ====================

    cout << "\n=== DELETE AI ===\n";

    root = deleteSubject(
        root,
        "AI"
    );

    printSubjects(root);

    cout << "\n=== DELETE CTDL ===\n";

    root = deleteSubject(
        root,
        "CTDL"
    );

    printSubjects(root);

    cout << "\n=== DELETE UNKNOWN ===\n";

    root = deleteSubject(
        root,
        "JAVA"
    );

    printSubjects(root);

    // Release the subjects still present after the demo.
    while (root != nullptr) {
        root = deleteSubject(root, root->id);
    }

    return 0;
}
