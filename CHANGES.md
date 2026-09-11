# Core fixes

- `stringCopy` now receives destination size and always null-terminates when size > 0.
- Updated all core calls to `stringCopy` with constants from `Constants.h`.
- Duplicate subject insertion no longer leaks the rejected node.
- `editSubject` allows changing subject ID/name and preserves its question list.
  Updating `Score.subjectId` / `ExamHistory.subjectId` remains a Business Logic responsibility;
  this core API does not receive the student data needed to update those references.
- Added `addStudentGlobal` so student IDs can be enforced as unique across classes.
- Fixed `updateScore` to call `findScore` with the correct signature.
- Replaced the integration demo with assert-based regression checks.
- Added focused tests for safe strings, duplicate subjects, subject ID changes,
  and globally unique student IDs.

The required assignment data structures are unchanged: class pointer array, linked lists,
and subject BST remain intact.


## Current bug fixes

- Reject overlong subject IDs in `editSubject` before mutating the tree, preventing
  truncation collisions from deleting the original subject and its questions.
- Reject overlong subject IDs in `addScore` before its uniqueness check, preventing
  duplicate stored score keys caused by truncation.
- Release the remaining subject tree at the end of `main.cpp`.
- Correct stale README descriptions of bounded string copies and duplicate-node ownership.
- Add regression coverage for rejected overlong IDs while retaining valid renames.
