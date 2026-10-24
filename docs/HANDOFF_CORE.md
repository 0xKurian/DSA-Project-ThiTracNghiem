# HANDOFF — UI → Core (Người 1)

> **Người nhận:** Người 1 — Core
> **Người giao:** Người 3 — UI
> Chỉ chứa việc đụng `core/**` hoặc API core. Việc business thuần → `docs/HANDOFF_BUSINESS.md`.
> UI **không tự sửa core**; các mục dưới đây là yêu cầu bàn giao.
> Trạng thái: `Chờ core` / `Đang làm` / `Xong` / `Từ chối`.

## Nguyên tắc

- Không sửa `ui/`; UI owner sẽ nối API sau.
- Không dùng `std::vector`, `std::unordered_map`, hashmap trong core. Dùng
  `new[]/delete[]`, danh sách liên kết, mảng động, `char[]`.
- `make` 0 warning, `make test` PASS; không đổi dữ liệu thật khi test.
- Giữ chữ ký API và quy tắc ownership hiện có; báo chữ ký thực tế để UI nối.

---

## Đang mở

### C01 — ExamConfig core-owned gắn Class (R2 core)

- **Trạng thái:** Xong
- **UI đang chờ:** `ui/GuiAdminExamConfigs.cpp`, `ui/GuiExamPrepare.cpp`, `ui/GuiState.h`
- **Mục tiêu:** mỗi cặp `(lớp, môn)` có cấu hình số câu, số phút, bộ câu đã bốc,
  ngày tạo; bốc N câu một lần khi lưu; freeze khi lớp đã thi; console dùng chung.

#### Core — dữ liệu

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
- Không thêm `Student.classId`; dùng `findStudentClass` do business cung cấp.

#### Core — API

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

#### Legacy cần xoá (core)

- `Subject.examQuestionCount/examMinutes` khỏi `Structures.h` và `createSubject`.
- Cập nhật `docs/CORE_API.md` theo hợp đồng mới.

#### Test core

- `create/add/find/delete`; trùng môn bị chặn.
- `deleteClass` giải phóng config + `questionIds`.
- Không rò rỉ khi `createExamConfig` lỗi hoặc `addExamConfig` từ chối.

#### Phần business

API workflow, freeze, persistence `data/exam_configs.txt`, xoá legacy
`setExamConfigWorkflow`, test tích hợp: xem `docs/HANDOFF_BUSINESS.md` B02.

### C02 — `deleteSubject` giữ config của successor

- **Trạng thái:** Xong
- **UI đang chờ:** gián tiếp qua `ui/GuiAdminSubjects.cpp` (hiển thị/sửa cấu hình môn)
- **Vấn đề:** `core/SubjectBST.cpp` nhánh node có 2 con chỉ chép `id`, `name`,
  `questionList` từ successor; không chép `examQuestionCount`, `examMinutes`.
  Node sống sót mang cấu hình của node bị xóa.
- **Hành vi mong muốn:** chép nguyên `examQuestionCount` và `examMinutes` từ
  successor sang node thay thế.
- **Acceptance/test:** cây `A <- B -> C`; B cấu hình `7/33`, C `9/44`; xóa B →
  node còn lại phải là `C` với `9/44`.
- **Ghi chú:** lỗi đã xác nhận bằng probe ngoài repo.

### C03 — `editSubject` giữ config khi đổi mã môn

- **Trạng thái:** Xong
- **UI đang chờ:** `ui/GuiModalDialogs.cpp` (modal sửa môn), `ui/ConsoleUI.cpp` (sửa mã)
- **Vấn đề:** `editSubject` tạo node mới qua `createSubject()` nên
  `examQuestionCount/examMinutes` về `0`.
- **Hành vi mong muốn:** copy 2 field cấu hình từ node cũ sang node mới trước khi
  xóa node cũ.
- **Acceptance/test:** đặt cấu hình → đổi mã môn → cấu hình giữ nguyên;
  `CORE_API.md` cập nhật lại (hiện đang ghi hành vi reset).
- **Ghi chú:** UI hiện không đổi được mã môn (lỗi UI riêng); mục này vẫn cần cho
  console/API.

---

## Hợp đồng UI sau khi API xong (phần core)

UI sẽ dùng:

- `findExamConfig(classroom, subjectId)` + `config->questionIds` để xem trước bộ đề.
- `config->questionCount`, `config->minutes`, `config->createdAt` để hiển thị.
- `Class.configList` (chỉ đọc).

UI sẽ xoá `UiExamConfig`, `drawDeck`, `GuiFindExamConfigIndex`, `GuiFreeExamConfigs`,
`GuiRemoveExamConfigs` sau khi B02 xong.

## Thứ tự thực hiện (core)

1. C01: `ExamConfig` + `Class.configList` + module CRUD + `createClass/deleteClass`.
2. C02/C03: độc lập, có thể làm song song.
3. Báo chữ ký thực tế để UI nối.

## Ngoài phạm vi

- Sửa `ui/`.
- AVL/cây tự cân bằng.
- Migration tự động cấu hình môn cũ sang lớp.
- Thêm `Student.classId`.

---

## Mẫu thêm item

```md
### Cxx — <tên ngắn>

- **Trạng thái:** Chờ core
- **UI đang chờ:** `ui/<file>:<line>`
- **Vấn đề:**
- **API/hành vi mong muốn:**
- **Acceptance/test:**
- **Spec chi tiết:** mục Rxx trong file này (nếu có)
```
