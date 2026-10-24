# Brief tổng hợp — Business backlog

- **Người nhận:** Người 3 (core + business)
- **Người giao:** Người 1 (UI)
- **Ngày:** 2026-10-24
- **Trạng thái:** chờ thực hiện. UI đã commit bản tạm cho cấu hình lớp (bốc đề/lưu RAM trong UI) — sẽ thay bằng API business dưới đây; xem R6.

Brief này gom toàn bộ việc business cần làm, thay thế `docs/EXAM_CONFIG_BRIEF.md` cũ:

| Ưu tiên | Hạng mục | Đụng core? |
|---|---|---|
| R1 | BST môn học không suy biến sau khi nạp file | Không |
| R2 | Cấu hình thi theo lớp + bốc đề cố định | Không |
| R3 | Timer + session runtime | Không |
| R4 | Pass/fail, điều kiện thi, query dùng chung | Không |
| R5 | Persistence/save policy | Không |

## 0. Nguyên tắc

- Không sửa `ui/` — UI owner sẽ nối API sau.
- Không đụng `core/` nếu không thật cần; R1–R3 làm được hoàn toàn ở business.
- Không dùng `std::vector`, `std::unordered_map`, hashmap (quy định của nhóm). Dùng `new[]/delete[]`, danh sách liên kết, mảng động.
- Business quyết định nghiệp vụ; UI chỉ gọi hàm và vẽ kết quả.
- Mọi thay đổi phải giữ `make` 0 warning và `make test` PASS.
- Không đổi dữ liệu thật trong `data/` khi test; dùng fixture/file tạm.

## R1. BST môn học không suy biến

### Vấn đề

`FileStorage::writeSubjectsRecursive` ghi môn theo **in-order** (tăng dần theo mã). Khi nạp, loader `insertSubject` lần lượt từng môn theo đúng thứ tự file → cây lệch hẳn sang phải, chiều cao `O(n)`. Với 6 môn hiện tại đã thành chuỗi; thêm môn càng tệ.

Không sửa core, không dùng AVL/cây tự cân bằng.

### Yêu cầu

Thêm vào `business/SubjectService.h/.cpp`:

```cpp
void rebalanceSubjectBST(Subject*& root);
```

Thuật toán:

1. Đếm số node (duyệt bất kỳ).
2. Cấp `Subject** nodes = new Subject*[n]`, duyệt in-order đổ vào mảng.
3. Dựng lại cây từ mảng: chọn phần tử giữa làm gốc, đệ quy nửa trái/phải; **chỉ gán `left/right`**, không tạo/xoá node, không đổi `id/name/questionList/examQuestionCount/examMinutes`.
4. `delete[] nodes`.

Gọi trong `loadAllData` (sau khi nạp subjects) để mọi entry point đều được lợi; cũng gọi được sau legacy per-subject load. Không cần gọi sau mỗi insert/delete.

### Test

Thêm `tests/test_bst_rebalance.cpp`:

- Insert 1..100 mã tăng dần → `rebalance` → chiều cao ≤ `2 * ceil(log2(n+1))`.
- Duyệt in-order sau rebalance trùng danh sách mã trước đó.
- `findSubject` tìm đủ 100 mã.
- n = 0, 1, 2 không crash.
- Gọi rebalance 2 lần liên tiếp → kết quả không đổi.

### Nghiệm thu

- Cây vẫn là BST, không đổi dữ liệu môn/câu hỏi.
- `make test` PASS; có test riêng cho rebalance.

## R2. Cấu hình thi theo lớp + bốc đề cố định

### Mục tiêu

- Mỗi cặp `(lớp, môn)` có **một** cấu hình: số câu, số phút, bộ câu đã bốc.
- Bốc N câu **một lần** khi lưu cấu hình; mọi SV trong lớp thi đúng bộ đó.
- Xem trước được bộ đã bốc.
- Console dùng chung nguồn, không bắt SV nhập số câu/phút.
- Không đụng core.

### Dữ liệu

Đặt trong business (module mới `business/ExamConfigBook.h/.cpp`):

```cpp
struct ExamConfig {
    char classId[CLASS_ID_LEN];
    char subjectId[SUBJECT_ID_LEN];
    int questionCount;
    int minutes;
    int* questionIds;        // new[]; ExamConfig sở hữu
    ExamConfig* next;
};

struct ExamConfigBook {
    ExamConfig* head;
    int count;
};
```

Không gắn vào `Class` (tránh đụng core). UI giữ một `ExamConfigBook` như handle và truyền vào workflow.

### API

```cpp
bool initExamConfigBook(ExamConfigBook& book);
void freeExamConfigBook(ExamConfigBook& book);

int drawQuestionIds(Subject* subject, int count, int outIds[]);

BusinessResult setClassExamConfigWorkflow(
    Subject* root,
    Class* classes[],
    int classCount,
    const char classId[],
    const char subjectId[],
    int numberOfQuestions,
    int minutes,
    ExamConfigBook& book
);

BusinessResult deleteClassExamConfigWorkflow(
    Class* classes[],
    int classCount,
    const char classId[],
    const char subjectId[],
    ExamConfigBook& book
);

const ExamConfig* findExamConfig(
    const ExamConfigBook& book,
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
    const ExamConfigBook& book,
    ExamDetail*& examQuestionsHead
);
```

### Hành vi

`drawQuestionIds`:

- Tách từ `prepareExamWorkflow` (Fisher-Yates + `initializeRngOnce`), dùng chung, không viết bản thứ hai.
- Trả số ID bốc được; `0` nếu không đủ câu.

`setClassExamConfigWorkflow`:

1. Chuẩn hoá `classId/subjectId` bằng helper validation hiện có (trim + upper).
2. Tìm lớp/môn; sai → `ERR_NOT_FOUND`.
3. `1 <= numberOfQuestions <= countQuestions(subject->questionList)`; `1 <= minutes <= 180`; sai → `ERR_INVALID_FORMAT`.
4. Nếu cặp đã có:
   - cùng `questionCount` → chỉ cập nhật `minutes`, giữ `questionIds`;
   - khác `questionCount` → free bộ cũ, bốc bộ mới.
5. Nếu chưa có → bốc bộ mới, tạo node, append.
6. Bốc thất bại → `ERR_INVALID_FORMAT`, không đổi cấu hình cũ.

`deleteClassExamConfigWorkflow`: xoá node + `delete[] questionIds`; không có → `ERR_NOT_FOUND`.

`findStudentClass`: trả lớp chứa SV; không có → `nullptr`.

`prepareExamFromConfig`:

1. Tìm lớp của SV; không có → `ERR_NOT_FOUND`.
2. Tìm config `(lớp, môn)`; không có → `ERR_NOT_FOUND`.
3. `findScore(student, subjectId) != nullptr` → `ERR_CONSTRAINT_VIOLATION`.
4. Mọi `questionId` phải còn trong ngân hàng; thiếu → `ERR_NOT_FOUND` (giáo viên lưu lại cấu hình).
5. Dựng `ExamDetail` đúng thứ tự ID bằng `createExamDetail` + `addExamDetail`.
6. Lỗi giữa chừng → `cancelExam(head)`, trả lỗi.

`createExamDetail` vẫn đảo đáp án từng SV — cùng câu, khác thứ tự đáp án.

### Persistence

File `data/exam_configs.txt`:

```
<so_cau_hinh>
<MALOP>|<MAMH>|<SO_CAU>|<PHUT>|<id1>,<id2>,...
```

```cpp
bool saveExamConfigsToFile(Class* classes[], int classCount,
                           const ExamConfigBook& book, const char* filepath);
bool loadExamConfigsFromFile(Class* classes[], int classCount,
                             ExamConfigBook& book, const char* filepath);
```

- Gọi trong `saveAllData/loadAllData`; thêm tham số cuối mặc định `nullptr` để test/call cũ không phải sửa:
```cpp
bool saveAllData(..., const char* dir, const ExamConfigBook* configs = nullptr);
bool loadAllData(..., const char* dir, ExamConfigBook* configs = nullptr);
```
- File không tồn tại → `true`, book rỗng.
- Dòng lỗi (lớp/môn không tồn tại, ID mất, số câu sai) → bỏ qua dòng đó, tiếp tục.
- `load` phải cấp lại `questionIds` qua `createExamConfig`.

### Legacy

- `Subject.examQuestionCount/examMinutes`: giữ nguyên (không đụng core), chỉ dùng làm **gợi ý prefill** cho modal cấu hình lớp; không dùng để thi.
- UI sẽ bỏ modal "Cau hinh console"; `setExamConfigWorkflow` đánh dấu deprecated, có thể xoá sau bảo vệ.
- `prepareExamWorkflow` cũ: giữ nếu còn test/console legacy; GUI/console mới dùng `prepareExamFromConfig`.
- Xoá lớp: `deleteClassWorkflow` gọi `deleteClass` core; config trong book phải được dọn bởi business (duyệt book theo `classId`).
- Xoá môn: `deleteSubjectWorkflow` dọn mọi config có `subjectId` tương ứng trước khi xoá môn.

### Test

`tests/test_exam_config.cpp`:

- Tạo 2 lớp, 1 môn 10 câu; tạo config 3 câu/15 phút.
- 2 SV cùng lớp → `prepareExamFromConfig` ra cùng danh sách `questionId`, cùng thứ tự.
- SV lớp khác có config riêng → bộ khác (nếu ngân hàng đủ).
- Trùng cặp không tạo node thứ hai.
- N > ngân hàng, N <= 0, phút <= 0, phút > 180 bị từ chối.
- Sửa phút giữ bộ; sửa N đổi bộ.
- Xoá câu trong ngân hàng → prepare trả lỗi, không crash.
- Save/load round-trip; file thiếu → book rỗng.
- Xoá lớp/môn dọn config.

## R3. Timer + session runtime

### Gộp timer

Trong `business/ExamService.cpp`:

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

- Xoá `isExamTimeExpired(time_t, int minutes)` (hiện không có caller ngoài README).
- Cập nhật `business/README.md`.
- Thêm:
```cpp
void formatExamClock(int seconds, char out[], int size);   // "MM:SS"
```

### Session runtime

Thêm:

```cpp
struct ExamSessionTracker {
    int lastSavedSecond;
};

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

- `tick` gọi `timer.getRemainingSeconds()`; nếu expired → `{0, true, false}`.
- Nếu giây đổi so với `lastSavedSecond` → `saveExamSession(...)`, cập nhật tracker, `saved = true`.
- UI không còn `examLastSavedSec`, không tự gọi `saveExamSession` mỗi giây.

Các workflow kết thúc:

```cpp
BusinessResult submitExpiredExamWorkflow(
    Subject* root, Class* classes[], int classCount,
    Student* student, const char subjectId[],
    const ExamConfigBook& book,
    ExamDetail*& examHead,
    float& score, int& correct, int& total
);

BusinessResult finishExamWorkflow(...);        // nộp tay, cùng tham số
BusinessResult cancelPendingExamWorkflow(..., Student*, const ExamConfigBook& book);
BusinessResult resumeExamWorkflow(
    Subject* root, Class* classes[], int classCount,
    Student* student,
    ExamDetail*& examHead,
    char subjectOut[], int& remainingOut
);
```

- `submitExpired`/`finish`: `submitExamWorkflow` + `removeExamSession` + `saveAllData(..., book)`; trả điểm; UI chỉ đổi màn.
- `cancelPending`: check/load session + submit + xoá phiên + save; không cần UI giữ head.
- `resume`: check/load + trả head, mã môn, giây còn lại; UI chỉ đổ head vào danh sách hiển thị.
- Save lỗi → vẫn trả kết quả nộp, kèm cảnh báo (out `bool persisted` hoặc log).

### Test

`tests/test_exam_timer.cpp`:

- `start(60)`, `resume(120)` → remaining trong `(0, 60]/[0,120]`.
- `stop` → remaining 0, expired true.
- `start(0)`/`resume(0)` → expired ngay.
- `formatExamClock(0) == "00:00"`, `formatExamClock(65) == "01:05"`.
- `tick` lưu đúng khi giây đổi, không lưu lặp cùng giây; expired trả đúng.

## R4. Pass/fail + điều kiện thi + query dùng chung

```cpp
bool isPassingScore(float score);                    // ngưỡng 5.0
BusinessResult canStudentTakeSubject(
    Subject* root, Class* classes[], int classCount,
    const ExamConfigBook& book, Student* student,
    const char subjectId[], const char** reasonOut);
int countStudentsInClass(const Class* classroom);
int collectSubjectsInOrder(Subject* root, Subject* out[], int capacity);
void freeAllData(Subject*& root, Class* classes[], int& classCount);  // tuỳ chọn
```

- `isPassingScore` thay `GuiExamResult.cpp:21`.
- `canStudentTakeSubject` trả lý do để UI disable nút: chưa có lớp, chưa có config, đã thi.
- `countStudentsInClass` xoá 3 bản trùng trong UI.
- `collectSubjectsInOrder` xoá bản console trùng.
- `freeAllData` tuỳ chọn, thay vòng lặp cleanup ở entry point.

## R5. Persistence/save policy

- `submitExpired/finish/cancelPending` tự gọi `saveAllData` — UI không tự lưu sau nộp/huỷ.
- Nút `Luu File` và lưu lúc thoát vẫn do UI gọi (app lifecycle).
- `loadAllData` nhận `ExamConfigBook*`; nạp subjects → classes → configs → rebalance BST.

## R6. Hợp đồng UI sau khi API xong

UI sẽ gọi:

| Việc | Hàm |
|---|---|
| Tạo/sửa cấu hình | `setClassExamConfigWorkflow` |
| Xoá cấu hình | `deleteClassExamConfigWorkflow` |
| Tìm cấu hình để hiển thị/preview | `findExamConfig` |
| Chuẩn bị thi | `prepareExamFromConfig` |
| Lớp của SV | `findStudentClass` |
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

1. **R1 BST** (độc lập, nhanh).
2. **R2 config + FileStorage** (UI hết tạm).
3. **R3 timer/session**.
4. **R4/R5 query + save policy**.
5. Báo lại chữ ký thực tế để UI nối.

Nhánh riêng, commit conventional:

- `feat(business): rebalance subject BST after load`
- `feat(business): class exam config book + fixed deck`
- `feat(business): exam session runtime + timer helpers`
- `feat(business): shared queries and pass policy`

Nghiệm thu chung:

- `make clean && make`: 0 warning; `make test` PASS.
- BST: chiều cao logarit, in-order không đổi.
- Config: 2 SV cùng lớp cùng bộ ID; restart còn config; preview khớp; console không hỏi N/phút.
- Timer: auto-save 1 lần/giây; hết giờ nộp đúng; resume đúng giây.
- Không sửa `ui/`, không dùng STL, không đổi dữ liệu thật khi test.

## R8. Ngoài phạm vi

- Sửa `ui/`.
- Đổi core (R1–R3 không cần).
- AVL/cây tự cân bằng.
- Migration tự động cấu hình môn cũ sang lớp (không map được; giáo viên tạo lại).
- Chuyển console sang flow mới (UI owner làm sau khi API có).
- Xoá `Subject.examQuestionCount/examMinutes` (giữ làm legacy/prefill; xoá khi được phép đụng core).
