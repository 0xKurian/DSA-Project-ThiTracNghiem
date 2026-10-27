# HANDOFF — UI → Business (Người 2)

> **Người nhận:** Người 2 — Business
> **Người giao:** Người 3 — UI
> Chỉ chứa việc đụng `business/**` (workflow, validate, file storage, exam engine).
> Việc core thuần → `docs/HANDOFF_CORE.md`.
> UI **không tự sửa business**; các mục dưới đây là yêu cầu bàn giao.
> Trạng thái: `Chờ business` / `Đang làm` / `Xong` / `Từ chối`.
> **UI đã nối B01–B08 ngày 25/10/2026** (`ui/GuiAdminExamConfigs.cpp`,
> `ui/GuiExamPrepare.cpp`, `ui/GuiExamRoom.cpp`, `ui/GuiModalDialogs.cpp`,
> `ui/GuiCommon.cpp`, `ui/ConsoleUI.cpp`). Phần UI tự làm (bốc đề RAM, đếm SV,
> ngưỡng đạt, lưu session mỗi giây, hủy phiên = xoá) đã xoá.

## Nguyên tắc

- Không sửa `ui/`; UI owner sẽ nối API sau.
- Không dùng `std::vector`, `std::unordered_map`, hashmap. Dùng `new[]/delete[]`,
  danh sách liên kết, mảng động.
- Business quyết định nghiệp vụ; UI chỉ gọi hàm và vẽ kết quả.
- `make` 0 warning, `make test` PASS; không đổi dữ liệu thật khi test.

---

## Đang mở (Đã hoàn tất toàn bộ B01 — B08)

### B01 — Rebalance BST môn học sau khi nạp file (R1)

- **Trạng thái:** Xong (24/10/2026)
- **File:** `business/SubjectService.h/.cpp`, tích hợp tại `business/FileStorage.cpp`
- **Chi tiết triển khai:**
  - Hàm `void rebalanceSubjectBST(Subject*& root);` đếm số node, duyệt in-order lấy mảng con trỏ `Subject**`, tái cấu trúc mid-first gán lại `left/right` (không tạo/xóa node, không cấp phát thừa).
  - Tự động gọi trong `loadAllData()` ngay sau khi nạp xong subjects.
- **Acceptance/test:** `tests/test_bst_rebalance.cpp` PASS 100% (n=0, 1, 2, 100 cây lệch dồn phải rebalance về chiều cao $\le 14$, idempotent khi gọi 2 lần).

### B02 — Workflow cấu hình thi theo lớp + persistence + freeze (R2 business)

- **Trạng thái:** Xong (24/10/2026)
- **File:** `business/ExamConfigService.h/.cpp`, `business/FileStorage.h/.cpp`
- **Chi tiết triển khai:**
  - Đầy đủ 6 API theo đặc tả: `drawQuestionIds`, `setClassExamConfigWorkflow`, `deleteClassExamConfigWorkflow`, `classHasTakenSubject`, `findStudentClass`, `prepareExamFromConfig`.
  - Quản lý Freeze chặt chẽ: `classHasTakenSubject` chặn sửa/xóa khi đã có SV thi môn đó (`ERR_CONSTRAINT_VIOLATION`).
  - Đổi số phút giữ nguyên bộ đề; đổi số câu hỏi tự động bốc bộ mới và giải phóng bộ cũ an toàn.
  - Sửa/xóa môn học tự động cập nhật hoặc dọn dẹp cấu hình liên quan trong mọi lớp (`SubjectService.cpp:143-176`).
  - Persistence `data/exam_configs.txt` qua `saveExamConfigsToFile` / `loadExamConfigsFromFile`, tích hợp sẵn trong `saveAllData` và `loadAllData`.
  - `FileStorage`: `subjects.txt` được ghi 3 cột chuẩn mới; loader tương thích ngược nạp được cả file 5 cột cũ và bỏ 2 cột cuối.
- **Acceptance/test:** `tests/test_exam_config.cpp` và `tests/test_subject_exam_config.cpp` PASS 100%.

### B03 — Timer + session runtime (R3)

- **Trạng thái:** Xong (24/10/2026)
- **File:** `business/ExamService.h/.cpp`
- **Chi tiết triển khai:**
  - Gộp timer vào `struct ExamTimer`: `getRemainingSeconds() const`, `isExpired() const` chuẩn theo giây, khử trôi thời gian bằng epoch clock.
  - Xóa bỏ hoàn toàn hàm cũ `isExamTimeExpired(time_t, int minutes)`.
  - Thêm `void formatExamClock(int seconds, char out[], int size);` xuất định dạng `"MM:SS"`.
  - Cung cấp `ExamSessionTracker` và `tickExamSession`: tự động lưu session `temp_<MASV>.dat` khi số giây thay đổi, tránh ghi đĩa trùng lặp mỗi tick.
  - Cung cấp 4 workflow kết thúc bài thi:
    - `finishExamWorkflow(...)`: nộp bài thủ công, tự chấm điểm, lưu `saveAllData`, xóa session.
    - `submitExpiredExamWorkflow(...)`: nộp bài khi hết giờ, xử lý tương tự `finishExamWorkflow`.
    - `cancelPendingExamWorkflow(...)`: hủy phiên thi dở dang (tự nạp session, nộp bài, chấm điểm và khóa thi lại).
    - `resumeExamWorkflow(...)`: khôi phục phiên thi dở dang cho sinh viên.
  - Hỗ trợ cờ `saveFailedOut` an toàn bộ nhớ nếu ghi file thất bại.
- **Acceptance/test:** `tests/test_exam_timer.cpp` PASS 100%.

### B04 — Pass/fail + điều kiện thi + query dùng chung (R4)

- **Trạng thái:** Xong (24/10/2026)
- **File:** `business/BusinessQueries.h/.cpp`, `business/ExamConfigService.h/.cpp`
- **Chi tiết triển khai:**
  - `bool isPassingScore(float score)` (ngưỡng 5.0).
  - `int countStudentsInClass(const Class* classroom)`.
  - `int collectSubjectsInOrder(Subject* root, Subject* out[], int capacity)`.
  - `void freeAllData(Subject*& root, Class* classes[], int& classCount)`.
  - `canStudentTakeSubject(...)` kiểm tra điều kiện thi của sinh viên và trả về chuỗi lý do chi tiết (chưa có lớp, chưa cấu hình, đã thi rồi, cấu hình không hợp lệ, câu hỏi trong ngân hàng bị xóa).
- **Acceptance/test:** Tích hợp kiểm tra trong `tests/test_exam_config.cpp` PASS 100%.

### B05 — Chính sách xóa câu hỏi khi môn đã có điểm

- **Trạng thái:** Xong (24/10/2026)
- **File:** `business/QuestionService.h/.cpp`
- **Quyết định của Business:** **Áp dụng Hướng 1 (Bảo toàn toàn vẹn dữ liệu)**:
  - Chặn xóa câu hỏi nếu môn học đã phát sinh điểm thi (`isSubjectUsedInScores`) HOẶC câu hỏi đang nằm trong cấu hình thi của bất kỳ lớp nào trong hệ thống (`ERR_CONSTRAINT_VIOLATION`).
  - Thêm overload 5 tham số: `deleteQuestionWorkflow(root, classes, classCount, subjectId, questionId)`.
  - Giữ lại overload 3 tham số cũ `deleteQuestionWorkflow(root, subjectId, questionId)` để tầng UI hiện tại không bị gãy khi biên dịch; UI sẽ chuyển sang overload 5 tham số khi nối API.
  - Modal UI giữ nguyên thông báo “Chỉ xóa được câu hỏi nếu chưa có sinh viên thi môn này” (hoặc cập nhật thêm "và câu hỏi chưa nằm trong cấu hình thi của lớp").
- **Acceptance/test:** `tests/test_business_edges.cpp` PASS 100%.

### B06 — Ký tự `|` trong dữ liệu người dùng (ưu tiên thấp)

- **Trạng thái:** Xong (24/10/2026)
- **File:** `business/PipeCodec.h`, `business/FileStorage.cpp`, `business/ExamService.cpp`
- **Chi tiết triển khai:**
  - Cung cấp `escapePipe()` (chuyển `|` → `\|` và `\` → `\\`) và `splitEscapedPipe()`.
  - Áp dụng vào việc lưu/đọc tên môn học, nội dung câu hỏi, 4 phương án, họ tên sinh viên và file phiên thi `temp_<MASV>.dat`.
- **Acceptance/test:** `tests/test_business_edges.cpp` kiểm tra round-trip dữ liệu chứa `|` và `\` PASS 100%.

### B07 — Chuẩn hóa đường dẫn file session (ưu tiên thấp)

- **Trạng thái:** Xong (24/10/2026)
- **File:** `business/Validation.cpp`, `business/ExamService.cpp`
- **Chi tiết triển khai:**
  - `validateStudentId()` bổ sung whitelist ký tự `[A-Za-z0-9_-]`, chặn đứng nguy cơ Path Traversal (`../`, `/`, `\`).
  - `getExamSessionFilePath()` kiểm tra hợp lệ của MASV trước khi tạo file session; trả về chuỗi rỗng nếu vi phạm.
- **Acceptance/test:** `tests/test_exam_timer.cpp` kiểm tra từ chối ID dạng `../SV1` PASS 100%.

### B08 — Persistence/save policy (R5)

- **Trạng thái:** Xong (24/10/2026)
- **File:** `business/ExamService.cpp`, `business/FileStorage.cpp`
- **Chi tiết triển khai:**
  - `finishExamWorkflow`, `submitExpiredExamWorkflow` và `cancelPendingExamWorkflow` tự động gọi `saveAllData()`. UI không cần tự lưu sau khi nộp hoặc hủy thi.
  - Thứ tự nạp dữ liệu chuẩn trong `loadAllData()`: subjects → classes → exam configs → dọn dẹp configs không hợp lệ (`discardInvalidExamConfigs`) → rebalance BST (`rebalanceSubjectBST`).
- **Acceptance/test:** Kiểm tra trong `tests/test_exam_config.cpp` và `tests/test_exam_timer.cpp` PASS 100%.

---

## Hợp đồng UI sau khi API xong (R6)

Tầng UI chỉ cần include các header tương ứng và gọi các API chuẩn:

| Nghiệp vụ | Hàm API | Header File |
|---|---|---|
| Tạo/sửa cấu hình theo lớp | `setClassExamConfigWorkflow` | `business/ExamConfigService.h` |
| Xoá cấu hình theo lớp | `deleteClassExamConfigWorkflow` | `business/ExamConfigService.h` |
| Tìm cấu hình / xem trước bộ đề | `findExamConfig(classroom, subjectId)` | `core/ExamConfigList.h` |
| Chuẩn bị thi từ cấu hình lớp | `prepareExamFromConfig` | `business/ExamConfigService.h` |
| Tra cứu lớp của sinh viên | `findStudentClass` | `business/ExamConfigService.h` |
| Kiểm tra trạng thái đã thi (Freeze) | `classHasTakenSubject` | `business/ExamConfigService.h` |
| Kiểm tra điều kiện thi & lý do | `canStudentTakeSubject` | `business/ExamConfigService.h` |
| Tick đếm giờ & tự động lưu session | `tickExamSession` | `business/ExamService.h` |
| Định dạng chuỗi đồng hồ (`MM:SS`) | `formatExamClock` | `business/ExamService.h` |
| Nộp bài thi thủ công | `finishExamWorkflow` | `business/ExamService.h` |
| Tự động nộp khi hết giờ | `submitExpiredExamWorkflow` | `business/ExamService.h` |
| Hủy phiên thi dở dang (tự nộp) | `cancelPendingExamWorkflow` | `business/ExamService.h` |
| Khôi phục phiên thi dở dang | `resumeExamWorkflow` | `business/ExamService.h` |
| Đánh giá Đạt / Không đạt (ngưỡng 5.0) | `isPassingScore` | `business/BusinessQueries.h` |
| Đếm số sinh viên trong lớp | `countStudentsInClass` | `business/BusinessQueries.h` |
| Thu thập danh sách môn in-order | `collectSubjectsInOrder` | `business/BusinessQueries.h` |
| Giải phóng toàn bộ dữ liệu | `freeAllData` | `business/BusinessQueries.h` |
| Xóa câu hỏi (kiểm tra điểm & config) | `deleteQuestionWorkflow(root, classes, count, subId, qId)` | `business/QuestionService.h` |

UI sẽ xoá:

- `UiExamConfig`, `Arr<UiExamConfig> examConfigs`, `drawDeck`,
  `GuiFindExamConfigIndex`, `GuiFreeExamConfigs`, `GuiRemoveExamConfigs`,
  `normalizeCode`.
- Vòng lặp `createExamDetail/addExamDetail` trong prepare.
- `GuiFindStudentClass`.
- `examLastSavedSec`, auto-submit, `saveExamSession/removeExamSession` trong UI.
- Pass threshold, `countStudentsInClass`, `collectSubjectsInOrder` trùng.

- Modal "Cau hinh console" + validation cấu hình môn cũ.

UI giữ: vẽ, tìm kiếm/sắp xếp/phân trang, toast/caret, chuyển màn, đọc dữ liệu để
hiển thị, set `examHead = nullptr` sau chuyển quyền.

## Thứ tự thực hiện & bàn giao (R7)

1. **R2 core:** `ExamConfig` + `Class` + CRUD + `createClass/deleteClass`
   (`docs/HANDOFF_CORE.md` C01).
2. **R2 FileStorage:** `exam_configs.txt`, bỏ 5 cột môn, loader legacy.
3. **R2 business:** workflow config/freeze/prepare; xoá legacy subject config.
4. **R1 BST** (độc lập, có thể làm song song).
5. **R3 timer/session.**
6. **R4/R5 query + save policy.**
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

## Ngoài phạm vi (R8)

- Sửa `ui/`.
- AVL/cây tự cân bằng.
- Migration tự động cấu hình môn cũ sang lớp (không map được; giáo viên tạo lại).
- Thêm `Student.classId` (giữ `findStudentClass`).
- Menu console quản lý cấu hình (GUI đã có).

---

## Đang mở

### B09 — `cancelPendingExamWorkflow` dọn phiên thi hỏng

- **Trạng thái:** Chờ business
- **UI đang chờ:** `ui/GuiExamPrepare.cpp` (banner phiên dở), `ui/GuiModalDialogs.cpp`
  (`MODAL_CANCEL_EXAM`), `ui/ConsoleUI.cpp` (`takeExamSession`)
- **Vấn đề:** `business/ExamService.cpp` — nếu file `temp_<MASV>.dat` không đọc được,
  `cancelPendingExamWorkflow` trả `ERR_NOT_FOUND` và **không xoá file**. UI không còn
  được gọi `removeExamSession` trực tiếp (R6), nên sinh viên kẹt banner "phiên dở"
  vĩnh viễn cho tới khi có người xoá file tay.
- **Hành vi mong muốn:** khi `loadExamSession` thất bại trong
  `cancelPendingExamWorkflow`, gọi `removeExamSession` rồi trả `ERR_NOT_FOUND`
  (hoặc `SUCCESS` kèm cờ) để UI thoát trạng thái kẹt.
- **Acceptance/test:** tạo `temp_SV001.dat` rác → gọi `cancelPendingExamWorkflow`
  → file bị xoá, lần gọi sau không còn cảnh báo phiên dở.

---

### B10 — API ghi nhận đáp án đang chọn của sinh viên

- **Trạng thái:** Chờ business
- **UI đang chờ:** `ui/GuiExamRoom.cpp:87`
- **Vấn đề:** UI gán trực tiếp `curQ->selectedAnswer = choices[i].letter` vào node
  `ExamDetail` của core khi sinh viên click đáp án trong phòng thi. Việc ghi nhận
  lựa chọn là thao tác trên dữ liệu phiên thi nhưng chưa có API business nào;
  UI không được tự quyết định rule (vd: chữ hợp lệ, chạm để chấm điểm).
- **API/hành vi mong muốn:** `BusinessResult recordSelectedAnswer(ExamDetail* detail,
  char answer)` (hoặc hàm tương đương nhận `ExamDetail*` đang mở): chỉ nhận
  'A'..'D', trả SUCCESS khi gán được; chữ khác/trả ERR_INVALID... để UI disable.
- **Acceptance/test:** gọi với 'A'..'D' → `selectedAnswer` đổi đúng; gọi với 'E'
  → lỗi BusinessResult, node không đổi; tick/nộp bài vẫn đọc được lựa chọn như cũ.

---

## Mẫu thêm item

```md
### Bxx — <tên ngắn>

- **Trạng thái:** Chờ business
- **UI đang chờ:** `ui/<file>:<line>`
- **Vấn đề:**
- **API/hành vi mong muốn:**
- **Acceptance/test:**
- **Spec chi tiết:** mục Rxx trong file này (nếu có)
```
