# HANDOFF — UI → Core (Người 1)

> **Người nhận:** Người 1 — Core  
> **Người giao:** Người 3 — UI  
> Chỉ chứa việc đụng `core/**` hoặc API core. Việc business thuần → `docs/HANDOFF_BUSINESS.md`.  
> UI **không tự sửa core**; các mục dưới đây là yêu cầu bàn giao.  
> Trạng thái: `Chờ core` / `Đang làm` / `Xong` / `Từ chối`.

---

## Đang mở

### C01 — ExamConfig core-owned gắn Class (R2 core)

- **Trạng thái:** Chờ core
- **UI đang chờ:** `ui/GuiAdminExamConfigs.cpp`, `ui/GuiExamPrepare.cpp`, `ui/GuiState.h`
- **Vấn đề:** UI đang giữ `UiExamConfig` trong RAM; mất khi thoát, không freeze, không dùng chung với console.
- **API/hành vi mong muốn:** `ExamConfig` + `Class.configList`; module `ExamConfigList`; `createClass/deleteClass` cascade; bỏ `Subject.examQuestionCount/examMinutes`.
- **Acceptance/test:** theo `docs/BUSINESS_BRIEF.md` R2 phần core + test core.
- **Spec chi tiết:** `docs/BUSINESS_BRIEF.md` R2.

### C02 — `deleteSubject` giữ config của successor

- **Trạng thái:** Chờ core
- **UI đang chờ:** gián tiếp qua `ui/GuiAdminSubjects.cpp` (hiển thị/sửa cấu hình môn)
- **Vấn đề:** `core/SubjectBST.cpp` nhánh node có 2 con chỉ chép `id`, `name`, `questionList` từ successor; không chép `examQuestionCount`, `examMinutes`. Node sống sót mang cấu hình của node bị xóa.
- **Hành vi mong muốn:** chép nguyên `examQuestionCount` và `examMinutes` từ successor sang node thay thế.
- **Acceptance/test:** cây `A <- B -> C`; B cấu hình `7/33`, C `9/44`; xóa B → node còn lại phải là `C` với `9/44`.
- **Ghi chú:** lỗi đã xác nhận bằng probe ngoài repo.

### C03 — `editSubject` giữ config khi đổi mã môn

- **Trạng thái:** Chờ core
- **UI đang chờ:** `ui/GuiModalDialogs.cpp` (modal sửa môn), `ui/ConsoleUI.cpp` (sửa mã)
- **Vấn đề:** `editSubject` tạo node mới qua `createSubject()` nên `examQuestionCount/examMinutes` về `0`.
- **Hành vi mong muốn:** copy 2 field cấu hình từ node cũ sang node mới trước khi xóa node cũ.
- **Acceptance/test:** đặt cấu hình → đổi mã môn → cấu hình giữ nguyên; `CORE_API.md` cập nhật lại (hiện đang ghi hành vi reset).
- **Ghi chú:** UI hiện không đổi được mã môn (lỗi UI riêng); mục này vẫn cần cho console/API.

---

## Mẫu thêm item

```md
### Cxx — <tên ngắn>

- **Trạng thái:** Chờ core
- **UI đang chờ:** `ui/<file>:<line>`
- **Vấn đề:**
- **API/hành vi mong muốn:**
- **Acceptance/test:**
- **Spec chi tiết:** `docs/BUSINESS_BRIEF.md` Rxx (nếu có)
```
