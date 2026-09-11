# Core fixes

- `stringCopy` now receives destination size and always null-terminates when size > 0.
- Updated all core calls to `stringCopy` with constants from `Constants.h`.
- Duplicate subject insertion no longer leaks the rejected node.
- `editSubject` keeps subject ID immutable; only the subject name may be changed.
  This protects references stored in `Score.subjectId` and `ExamHistory.subjectId`.
- Added `addStudentGlobal` so student IDs can be enforced as unique across classes.
- Fixed `updateScore` to call `findScore` with the correct signature.
- Replaced the integration demo with assert-based regression checks.
- Added focused tests for safe strings, duplicate subjects, immutable subject IDs,
  and globally unique student IDs.

The required assignment data structures are unchanged: class pointer array, linked lists,
and subject BST remain intact.
