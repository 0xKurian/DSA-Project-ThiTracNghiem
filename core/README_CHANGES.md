# Changes made

1. Removed `ExamHistory` completely.
2. `Student` now owns only `Score* scoreList` for exam results.
3. Each `Score` owns `ExamDetail* detailList`, matching the rule that each subject can be taken only once.
4. Added `createExamDetail(const Question*)`.
   - Copies the original question.
   - Shuffles A/B/C/D only on the copied `ExamDetail`.
   - Updates `correctAnswer` to the new position after shuffling.
   - Does not modify the original `Question` in the question bank.
5. Added `addExamDetail`, `shuffleExamDetailAnswers`, and `deleteExamDetailList`.
6. Added overloads of `createScore` and `addScore` that accept an `ExamDetail*` list.
7. `addScore` rejects a second score for the same subject, enforcing one attempt per subject.
8. Updated all delete paths so deleting a score/student/class also frees its exam-detail list.
9. Updated `FunctionPrototype.h` to match the new API.

## Typical exam flow

```cpp
ExamDetail* detailList = nullptr;

// For each selected random Question* q:
ExamDetail* detail = createExamDetail(q);
addExamDetail(detailList, detail);

// During the exam:
detail->selectedAnswer = 'A'; // example

// After grading:
addScore(student, subject->id, finalScore, detailList);
```

After `addScore(..., detailList)` succeeds, the corresponding `Score` owns `detailList`.
