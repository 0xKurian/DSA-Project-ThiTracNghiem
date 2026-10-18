# Changelog

Dòng thời gian so với bản cũ trước khi cập nhật GitHub (mốc so sánh: commit `373b2a6 "Add files via upload"`). Kiểu viết theo Conventional Commits.

## [Unreleased] — 2026-10-18

### Changed

- **refactor(ui): split GraphicUI.cpp (1.910 dòng) thành 16 module nhỏ theo nhóm chức năng** — Không đổi hành vi, chỉ tách để dễ track và dễ trình bày khi bảo vệ:
  - `GuiCommon.h/.cpp` + `GuiState.h` — khung giao diện dùng chung: palette màu, font tiếng Việt, widget (nút/textbox/toast), trạng thái toàn cục `GuiState` (có "bản đồ file" ở đầu `GuiState.h`)
  - `GuiApp.cpp` — bộ điều phối: init, main loop, cleanup
  - `GuiScreenLogin.cpp` — nhóm xác thực: màn hình đăng nhập
  - `GuiScreenAdmin.cpp` + `GuiAdminSubjects.cpp` (BST) / `GuiAdminQuestions.cpp` (DSLK đơn) / `GuiAdminClasses.cpp` (mảng con trỏ) / `GuiAdminStudents.cpp` (DSLK đơn) / `GuiAdminScores.cpp` (bảng điểm + chi tiết bài thi) — nhóm quản trị giáo viên, mỗi tab = 1 cấu trúc dữ liệu
  - `GuiScreenStudent.cpp`, `GuiExamPrepare.cpp`, `GuiExamRoom.cpp` (kèm đồng hồ tự nộp bài + modal xác nhận nộp), `GuiExamResult.cpp` — nhóm trải nghiệm thi của sinh viên
  - `GuiModalDialogs.cpp` — nhóm hộp thoại: 8 modal CRUD + rule chặn click ngoài modal
  - Kiểm chứng: `make` 0 lỗi/0 warning, `make test` PASS, so sánh splash-screen từng màn hình với bản cũ (login, 6 tab GV, dashboard SV) — render đúng.

### Notes

- `Makefile` không cần sửa: wildcard `ui/*.cpp` tự nhận các file mới.
- Dữ liệu `data/subjects.txt` đang có môn trùng lặp (ATTT/CSDL xuất hiện lặp lại, tổng 162 môn) — do file dữ liệu cũ, chưa xử lý.
- Nút "Lưu File" trên dashboard GV hiện chữ trắng trên nền trắng (gần như vô hình khi chưa hover) — lỗi UX cũ, giữ nguyên hành vi, chờ quyết định sửa.
