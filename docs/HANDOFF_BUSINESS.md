# HANDOFF — UI → Business (Người 2)

> **Người nhận:** Người 2 — Business  
> **Người giao:** Người 3 — UI  
> Chỉ chứa việc đụng `business/**` (workflow, validate, file storage, exam engine). Việc core thuần → `docs/HANDOFF_CORE.md`.  
> UI **không tự sửa business**; các mục dưới đây là yêu cầu bàn giao.  
> Trạng thái: `Chờ business` / `Đang làm` / `Xong` / `Từ chối`.

---

## Đang mở

### B01 — Rebalance BST môn học sau khi nạp file (R1)

- **Trạng thái:** Chờ business
- **UI đang chờ:** `ui/GuiAdminSubjects.cpp` (hiển thị danh sách môn)
- **Vấn đề:** `writeSubjectsRecursive` ghi in-order; loader chèn lần lượt làm cây lệch phải, tìm kiếm xấu nhất O(n).
- **API/hành vi mong muốn:** `void rebalanceSubjectBST(Subject*& root)`; gọi trong `loadAllData` sau khi nạp subjects; chỉ gán `left/right`, không tạo/xóa node.
- **Acceptance/test:** `tests/test_bst_rebalance.cpp` theo `docs/BUSINESS_BRIEF.md` R1.
- **Spec chi tiết:** `docs/BUSINESS_BRIEF.md` R1.

### B02 — Workflow cấu hình thi theo lớp + persistence + freeze (R2 business)

- **Trạng thái:** Chờ business
- **UI đang chờ:** `ui/GuiAdminExamConfigs.cpp`, `ui/GuiExamPrepare.cpp`, `ui/GuiState.h`, `ui/GuiModalDialogs.cpp`
- **Vấn đề:** UI đang tự bốc đề/lưu config trong RAM; không freeze, không lưu file, console không dùng chung.
- **API/hành vi mong muốn:** `drawQuestionIds`, `setClassExamConfigWorkflow`, `deleteClassExamConfigWorkflow`, `classHasTakenSubject`, `findStudentClass`, `prepareExamFromConfig`; `saveExamConfigsToFile`/`loadExamConfigsFromFile`; xóa legacy `setExamConfigWorkflow`.
- **Acceptance/test:** theo `docs/BUSINESS_BRIEF.md` R2 (business) + R5.
- **Spec chi tiết:** `docs/BUSINESS_BRIEF.md` R2, R5.

### B03 — Timer + session runtime (R3)

- **Trạng thái:** Chờ business
- **UI đang chờ:** `ui/GuiExamRoom.cpp`, `ui/GuiExamPrepare.cpp`, `ui/GuiModalDialogs.cpp`, `ui/ConsoleUI.cpp`
- **Vấn đề:** UI tự lưu session mỗi giây; console hủy phiên chỉ xóa file, không nộp bài.
- **API/hành vi mong muốn:** `tickExamSession`, `formatExamClock`, `submitExpiredExamWorkflow`, `finishExamWorkflow`, `cancelPendingExamWorkflow`, `resumeExamWorkflow`; xóa `isExamTimeExpired(time_t,int)` nếu không còn caller.
- **Acceptance/test:** `tests/test_exam_timer.cpp` theo `docs/BUSINESS_BRIEF.md` R3.
- **Spec chi tiết:** `docs/BUSINESS_BRIEF.md` R3.

### B04 — Pass/fail + điều kiện thi + query dùng chung (R4)

- **Trạng thái:** Chờ business
- **UI đang chờ:** `ui/GuiExamResult.cpp:21` (hardcode 5.0), `ui/GuiAdminScores.cpp:13`, `ui/GuiAdminClasses.cpp:8`, `ui/ConsoleUI.cpp:56,145`
- **Vấn đề:** UI đang có bản trùng logic đếm SV, tìm lớp của SV, duyệt môn in-order, ngưỡng đạt.
- **API/hành vi mong muốn:** `isPassingScore`, `canStudentTakeSubject`, `countStudentsInClass`, `collectSubjectsInOrder` (và `freeAllData` nếu làm).
- **Acceptance/test:** theo `docs/BUSINESS_BRIEF.md` R4; UI xóa bản trùng sau khi API có.
- **Spec chi tiết:** `docs/BUSINESS_BRIEF.md` R4.

### B05 — Chính sách xóa câu hỏi khi môn đã có điểm

- **Trạng thái:** Chờ business quyết định
- **UI đang chờ:** `ui/GuiModalDialogs.cpp:332`
- **Vấn đề:** modal ghi “Chỉ xóa được câu hỏi nếu chưa có sinh viên thi môn này”, nhưng `deleteQuestionWorkflow` không kiểm tra và vẫn trả `SUCCESS`.
- **Hành vi mong muốn:** chọn một hướng:
  1. Chặn bằng `isSubjectUsedInScores` → trả `ERR_CONSTRAINT_VIOLATION`; hoặc
  2. Cho xóa và xác nhận lại text UI (bài thi cũ vẫn an toàn nhờ snapshot).
- **Acceptance/test:** có test cho cả trường hợp đã có điểm và chưa có điểm.
- **Ghi chú:** UI không tự quyết định policy; chỉ sửa text theo kết luận của business.

### B06 — Ký tự `|` trong dữ liệu người dùng (ưu tiên thấp)

- **Trạng thái:** Chờ business
- **UI đang chờ:** các form `ui/GuiModalDialogs.cpp`
- **Vấn đề:** `FileStorage` dùng `|` làm delimiter nhưng không escape; validation không chặn `|`, nên tên môn/câu hỏi/đáp án chứa `|` có thể làm lệch cột khi load lại.
- **Hành vi mong muốn:** escape/unescape khi ghi/đọc, hoặc từ chối `|` trong `Validation`; giữ đọc được file 3 cột/5 cột cũ.
- **Acceptance/test:** round-trip với `|` trong tên/câu hỏi/đáp án; dữ liệu hiện tại vẫn load/save byte-identical.

### B07 — Chuẩn hóa đường dẫn file session (ưu tiên thấp)

- **Trạng thái:** Chờ business
- **UI đang chờ:** `ui/GuiExamRoom.cpp` (save session), `ui/GuiExamPrepare.cpp` (load session)
- **Vấn đề:** `ExamService::getExamSessionFilePath` lấy `temp_<MASV>.dat` trực tiếp từ ID; validation chỉ kiểm tra rỗng/độ dài, không chặn `/`, `\`, `..`.
- **Hành vi mong muốn:** whitelist ký tự ID hoặc sanitize đường dẫn; không đổi tên file với ID hợp lệ hiện tại.
- **Acceptance/test:** ID chứa `../` không ghi ra ngoài thư mục làm việc; `SV001` vẫn là `temp_SV001.dat`.

---

## Mẫu thêm item

```md
### Bxx — <tên ngắn>

- **Trạng thái:** Chờ business
- **UI đang chờ:** `ui/<file>:<line>`
- **Vấn đề:**
- **API/hành vi mong muốn:**
- **Acceptance/test:**
- **Spec chi tiết:** `docs/BUSINESS_BRIEF.md` Rxx (nếu có)
```
