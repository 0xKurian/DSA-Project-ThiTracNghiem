# Brief tổng hợp — Business backlog (core-owned ExamConfig)

- **Người nhận:** Người 1 (Core) + Người 2 (Business)
- **Người giao:** Người 3 (UI)
- **Ngày:** 2026-10-24 (bản 2 — chốt core-owned)
- **Trạng thái:** chờ thực hiện. UI đã commit bản tạm cho cấu hình lớp (bốc đề/lưu RAM trong UI); sẽ thay bằng API dưới đây.

> **Handoff hiện hành đã tách theo người nhận:**
> - Core: [`docs/HANDOFF_CORE.md`](HANDOFF_CORE.md)
> - Business: [`docs/HANDOFF_BUSINESS.md`](HANDOFF_BUSINESS.md)
>
> File này giữ spec chi tiết R1–R8 để tra cứu; item mới ghi vào 2 file handoff trên.

Brief này gom toàn bộ việc core/business cần làm, thay thế `docs/EXAM_CONFIG_BRIEF.md` cũ.

| Ưu tiên | Hạng mục | Đụng core? |
|---|---|---|
| R1 | BST môn học không suy biến sau khi nạp file | Không |
| R2 | Cấu hình thi theo lớp (core-owned) + bốc đề cố định | **Có** |
| R3 | Timer + session runtime | Không |
| R4 | Pass/fail, điều kiện thi, query dùng chung | Không |
| R5 | Persistence/save policy | Không |

## 0. Nguyên tắc

- Không sửa `ui/` — UI owner sẽ nối API sau.
- Nhóm đã cho phép sửa core; R2 đụng core có kiểm soát. R1/R3/R4/R5 vẫn business-only.
- Không dùng `std::vector`, `std::unordered_map`, hashmap. Dùng `new[]/delete[]`, danh sách liên kết, mảng động.
- Business quyết định nghiệp vụ; UI chỉ gọi hàm và vẽ kết quả.
- `make` 0 warning, `make test` PASS; không đổi dữ liệu thật khi test.

## R1. BST môn học không suy biến

### Vấn đề

`FileStorage::writeSubjectsRecursive` ghi môn theo **in-order**. Loader `insertSubject` lần lượt theo thứ tự file → cây lệch hẳn sang phải, chiều cao `O(n)`.

Không dùng AVL/cây tự cân bằng; không sửa core.

### Yêu cầu

Thêm vào `business/SubjectService.h/.cpp`:

```cpp
void rebalanceSubjectBST(Subject*& root);
```

1. Đếm node; cấp `Subject** nodes = new Subject*[n]`; duyệt in-order.
2. Dựng lại mid-first; **chỉ gán `left/right`**, không tạo/xoá node, không đổi field.
3. `delete[] nodes`.

Gọi trong `loadAllData` sau khi nạp subjects (cả file mới và legacy). Không cần gọi sau mỗi insert/delete.

### Test

`tests/test_bst_rebalance.cpp`:

- Insert 1..100 tăng dần → chiều cao ≤ `2 * ceil(log2(n+1))`.
- In-order trước/sau giống nhau; `findSubject` đủ.
- n = 0/1/2; gọi 2 lần idempotent.

## R2. Cấu hình thi theo lớp (core-owned) + bốc đề cố định

### Mục tiêu

- Mỗi cặp `(lớp, môn)` có một cấu hình: số câu, số phút, bộ câu đã bốc, ngày tạo.
- Bốc N câu **một lần** khi lưu; mọi SV trong lớp thi đúng bộ đó (đảo đáp án vẫn ngẫu nhiên từng SV).
- Xem trước bộ đã bốc.
- **Freeze**: đã có SV thi môn đó trong lớp → cấm sửa/xoá cấu hình.
- Console dùng chung, không bắt SV nhập N/phút.

### Core — dữ liệu

`core/Structures.h`:

```cpp
struct ExamConfig {
    char subjectId[SUBJECT_ID_LEN];
    int questionCount;
    int minutes;
    char createdAt[20];      // "YYYY-MM-DD HH:MM"
    int* questionIds;        // new[]; node sở hữu
    ExamConfig* next;
};

struct Class {
    ...
    ExamConfig* configList;  // lớp sở hữu cấu hình
};
```

- Không cần `classId` trong node (đã nằm trong `Class`); file vẫn có cột `MALOP` để nạp.
- Không thêm `Student.classId`; dùng `findStudentClass`.

### Core — API

Module mới `core/ExamConfigList.h/.cpp`:

```cpp
ExamConfig* createExamConfig(
    const char subjectId[],
    int minutes,
    const char createdAt[],
    const int questionIds[],
    int questionCount
);

bool addExamConfig(Class* classroom, ExamConfig* config);
ExamConfig* findExamConfig(const Class* classroom, const char* subjectId);
bool deleteExamConfig(Class* classroom, const char* subjectId);
void deleteExamConfigList(ExamConfig*& head);
```

- `createExamConfig` copy `questionIds` sang `new int[count]`; reject ID/`count <= 0`.
- `addExamConfig` từ chối nếu môn đã có trong lớp; nhận node mới.
- `deleteExamConfig`/`deleteExamConfigList` giải phóng `questionIds`.
- `createClass` khởi tạo `configList = nullptr`.
- `deleteClass` gọi `deleteExamConfigList` (cascade).

### Business — API

```cpp
int drawQuestionIds(Subject* subject, int count, int outIds[]);

BusinessResult setClassExamConfigWorkflow(
    Subject* root,
    Class* classes[],
    int classCount,
    const char classId[],
    const char subjectId[],
    int numberOfQuestions,
    int minutes
);

BusinessResult deleteClassExamConfigWorkflow(
    Class* classes[],
    int classCount,
    const char classId[],
    const char subjectId[]
);

bool classHasTakenSubject(
    Class* classes[],
    int classCount,
    const char classId[],
    const char subjectId[]
);

Class* findStudentClass(
    Class* classes[],
    int classCount,
    const char studentId[]
);

BusinessResult prepareExamFromConfig(
    Subject* root,
    Class* classes[],
    int classCount,
    Student* student,
    const char subjectId[],
    ExamDetail*& examQuestionsHead
);
```

### Hành vi

`drawQuestionIds`: tách từ `prepareExamWorkflow` (Fisher-Yates + `initializeRngOnce`), dùng chung.

`setClassExamConfigWorkflow`:

1. Chuẩn hoá ID (trim + upper) bằng helper validation.
2. Tìm lớp/môn; sai → `ERR_NOT_FOUND`.
3. `1 <= numberOfQuestions <= countQuestions(subject->questionList)`; `1 <= minutes <= 180`; sai → `ERR_INVALID_FORMAT`.
4. **Freeze**: `classHasTakenSubject` → `ERR_CONSTRAINT_VIOLATION`.
5. Nếu cặp đã có (chưa freeze):
   - cùng `questionCount` → cập nhật `minutes`, giữ `questionIds`/`createdAt`;
   - khác `questionCount` → free bộ cũ, bốc mới, cập nhật `createdAt`.
6. Nếu chưa có → bốc bộ mới, `createExamConfig`, `addExamConfig`.
7. Bốc lỗi → `ERR_INVALID_FORMAT`, không đổi cấu hình cũ.

`deleteClassExamConfigWorkflow`: freeze → `ERR_CONSTRAINT_VIOLATION`; không có → `ERR_NOT_FOUND`; có → xoá node + `delete[]`.

`classHasTakenSubject`: duyệt `classroom->studentList`, `findScore(student, subjectId) != nullptr`.

`prepareExamFromConfig`:

1. Tìm lớp SV; không có → `ERR_NOT_FOUND`.
2. Tìm config `(lớp, môn)`; không có → `ERR_NOT_FOUND`.
3. Đã có điểm môn → `ERR_CONSTRAINT_VIOLATION`.
4. Mọi `questionId` còn trong ngân hàng; thiếu → `ERR_NOT_FOUND`.
5. Dựng `ExamDetail` đúng thứ tự ID bằng `createExamDetail` + `addExamDetail`.
6. Lỗi giữa chừng → `cancelExam(head)`, trả lỗi.

### Persistence

File `data/exam_configs.txt`:

```
<so_cau_hinh>
MALOP|MAMH|SO_CAU|PHUT|NGAY_TAO|id1,id2,...
```

Ví dụ:

```
2
D23CQCN01|CSDL|3|15|2026-10-24 08:00|4,5,6
D23CQCN01|CTDL|2|10|2026-10-24 09:30|1,2
```

```cpp
bool saveExamConfigsToFile(Class* classes[], int classCount, const char* filepath);
bool loadExamConfigsFromFile(Class* classes[], int classCount, const char* filepath);
```

- Gọi trong `saveAllData/loadAllData`; **giữ nguyên chữ ký** (duyệt classes, không thêm tham số book).
- Nạp subjects → classes → configs; đối chiếu lớp/môn/ID.
- File không tồn tại → `true`, không có config.
- Dòng lỗi → bỏ qua, tiếp tục.
- Save: duyệt classes/configList; ghi `classId` từ `Class`.

### Legacy cần xoá

- `Subject.examQuestionCount/examMinutes` khỏi `Structures.h` và `createSubject`.
- `setExamConfigWorkflow` khỏi `SubjectService`.
- `FileStorage`: ghi `subjects.txt` 3 cột; loader vẫn nhận file 5 cột cũ và bỏ 2 cột cuối.
- `prepareExamWorkflow` cũ: xoá nếu không còn caller; giữ `drawQuestionIds`.
- `tests/test_subject_exam_config.cpp`: xoá/viết lại theo config lớp.
- `docs/CORE_API.md`, `business/README.md`: cập nhật.
- **Dữ liệu cũ không map được sang lớp** → giáo viên tạo lại cấu hình; điểm thi không mất.

### Test

`tests/test_exam_config.cpp` + core test:

- Core: create/add/find/delete; trùng môn bị chặn; `deleteClass` giải phóng config + `questionIds`.
- Tạo 2 lớp, môn 10 câu; config 3 câu/15 phút.
- 2 SV cùng lớp → cùng danh sách ID, cùng thứ tự.
- Lớp khác có config riêng → bộ khác.
- Freeze: SV nộp bài → sửa/xoá trả `ERR_CONSTRAINT_VIOLATION`; trước khi thi thì được.
- Sửa phút giữ bộ; sửa N đổi bộ.
- N > bank / <= 0 / phút ngoài 1..180 bị chặn.
- Xoá câu trong ngân hàng → prepare lỗi, không crash.
- Save/load round-trip; file thiếu → rỗng; xoá lớp → file sạch sau save.
- Xoá môn → business dọn config khỏi mọi lớp trước khi xoá môn.

## R3. Timer + session runtime

### Gộp timer

```cpp
int ExamTimer::getRemainingSeconds() const {
    if (!isRunning) return 0;
    return calculateExamRemainingSeconds(startTime, initialRemainingSeconds);
}

bool ExamTimer::isExpired() const {
    if (!isRunning) return true;
    return isExamTimeExpiredSeconds(startTime, initialRemainingSeconds);
}
```

- Xoá `isExamTimeExpired(time_t, int minutes)` chết; cập nhật `business/README.md`.
- Thêm `void formatExamClock(int seconds, char out[], int size);` → `"MM:SS"`.

### Session runtime

```cpp
struct ExamSessionTracker { int lastSavedSecond; };

struct ExamTickResult {
    int remainingSeconds;
    bool expired;
    bool saved;
};

ExamTickResult tickExamSession(
    ExamTimer& timer,
    ExamSessionTracker& tracker,
    const char studentId[],
    const char subjectId[],
    ExamDetail* examHead
);
```

- Expired → `{0, true, false}`.
- Giây đổi → `saveExamSession`, cập nhật tracker, `saved = true`.
- UI bỏ `examLastSavedSec` và lời gọi `saveExamSession` mỗi giây.

Workflow kết thúc:

```cpp
BusinessResult submitExpiredExamWorkflow(root, classes, classCount,
    Student*, subjectId, ExamDetail*& head, float& score, int& correct, int& total);
BusinessResult finishExamWorkflow(...);          // nộp tay
BusinessResult cancelPendingExamWorkflow(root, classes, classCount, Student*);
BusinessResult resumeExamWorkflow(root, classes, classCount, Student*,
    ExamDetail*& head, char subjectOut[], int& remainingOut);
```

- Submit/finish: `submitExamWorkflow` + `removeExamSession` + `saveAllData`; trả điểm.
- Cancel: check/load + submit + xoá phiên + save.
- Resume: check/load + trả head/môn/giây.
- Save lỗi → vẫn trả kết quả nộp, kèm cảnh báo.

### Test

`tests/test_exam_timer.cpp`:

- start/resume/stop; start(0)/resume(0) expired ngay.
- `formatExamClock(0) == "00:00"`, `formatExamClock(65) == "01:05"`.
- Tick lưu đúng khi giây đổi, không lặp; expired đúng.

## R4. Pass/fail + điều kiện thi + query

```cpp
bool isPassingScore(float score);                    // ngưỡng 5.0
BusinessResult canStudentTakeSubject(
    Subject* root, Class* classes[], int classCount,
    Student* student, const char subjectId[], const char** reasonOut);
int countStudentsInClass(const Class* classroom);
int collectSubjectsInOrder(Subject* root, Subject* out[], int capacity);
void freeAllData(Subject*& root, Class* classes[], int& classCount);  // tuỳ chọn
```

- `isPassingScore` thay `GuiExamResult.cpp:21`.
- `canStudentTakeSubject` trả lý do (chưa lớp, chưa config, đã thi, config lỗi).
- `countStudentsInClass` xoá 3 bản trùng UI.
- `collectSubjectsInOrder` xoá bản console trùng.
- `freeAllData` tuỳ chọn.

## R5. Persistence/save policy

- `submitExpired/finish/cancelPending` tự gọi `saveAllData`; UI không tự lưu sau nộp/huỷ.
- Nút `Luu File` và lưu lúc thoát vẫn UI gọi.
- `loadAllData`: subjects → classes → configs → rebalance BST.
- `saveAllData`: subjects → classes (kèm config) → file.

## R6. Hợp đồng UI sau khi API xong

UI sẽ gọi:

| Việc | Hàm |
|---|---|
| Tạo/sửa cấu hình | `setClassExamConfigWorkflow` |
| Xoá cấu hình | `deleteClassExamConfigWorkflow` |
| Tìm cấu hình/preview | `findExamConfig(class, subjectId)` + `questionIds` |
| Chuẩn bị thi | `prepareExamFromConfig` |
| Lớp của SV | `findStudentClass` |
| Trạng thái/freeze | `classHasTakenSubject` |
| Tick/lưu phiên | `tickExamSession` |
| Nộp/huỷ/tiếp tục | `submitExpired`, `finish`, `cancelPending`, `resume` |
| Pass/fail | `isPassingScore` |
| Điều kiện thi | `canStudentTakeSubject` |

UI sẽ xoá:

- `UiExamConfig`, `Arr<UiExamConfig> examConfigs`, `drawDeck`, `GuiFindExamConfigIndex`, `GuiFreeExamConfigs`, `GuiRemoveExamConfigs`, `normalizeCode`.
- Vòng lặp `createExamDetail/addExamDetail` trong prepare.
- `GuiFindStudentClass`.
- `examLastSavedSec`, auto-submit, `saveExamSession/removeExamSession` trong UI.
- Pass threshold, `countStudentsInClass`, `collectSubjectsInOrder` trùng.
- Modal "Cau hinh console" + validation cấu hình môn cũ.

UI giữ: vẽ, tìm kiếm/sắp xếp/phân trang, toast/caret, chuyển màn, đọc dữ liệu để hiển thị, set `examHead = nullptr` sau chuyển quyền.

## R7. Thứ tự thực hiện & bàn giao

1. **R2 core**: `ExamConfig` + `Class` + CRUD + `createClass/deleteClass`.
2. **R2 FileStorage**: `exam_configs.txt`, bỏ 5 cột môn, loader legacy.
3. **R2 business**: workflow config/freeze/prepare; xoá legacy subject config.
4. **R1 BST** (độc lập, có thể làm song song).
5. **R3 timer/session**.
6. **R4/R5 query + save policy**.
7. Báo chữ ký thực tế để UI nối.

Commit gợi ý:

- `feat(core): ExamConfig list attached to Class`
- `feat(business): class exam config workflows and fixed deck`
- `refactor(core): remove legacy subject exam config`
- `feat(business): rebalance subject BST after load`
- `feat(business): exam session runtime + timer helpers`
- `feat(business): shared queries and pass policy`

Nghiệm thu chung:

- `make clean && make`: 0 warning; `make test` PASS.
- BST logarit; config cùng bộ ID; freeze đúng; persistence; console không hỏi N/phút.
- Không sửa `ui/`, không STL, không đổi dữ liệu thật khi test.

## R8. Ngoài phạm vi

- Sửa `ui/`.
- AVL/cây tự cân bằng.
- Migration tự động cấu hình môn cũ sang lớp (không map được; giáo viên tạo lại).
- Thêm `Student.classId` (giữ `findStudentClass`).
- Menu console quản lý cấu hình (GUI đã có).
