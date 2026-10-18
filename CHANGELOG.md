# Changelog

Dòng thời gian so với bản cũ trước khi cập nhật GitHub (mốc so sánh: commit `373b2a6 "Add files via upload"`). Kiểu viết theo Conventional Commits.

## [2.0.1] — 2026-10-18

### Changed

- **refactor(ui): tách `GraphicUI.cpp` (1.910 dòng) thành 16 module nhỏ theo nhóm chức năng** — không đổi hành vi, chỉ tách để dễ track và dễ trình bày khi bảo vệ:
  - `GuiCommon.h/.cpp` + `GuiState.h` — khung giao diện dùng chung: palette màu, font tiếng Việt, widget (nút/textbox/toast), trạng thái toàn cục `GuiState` (có "bản đồ file" ở đầu `GuiState.h`)
  - `GuiApp.cpp` — bộ điều phối: init, main loop, cleanup
  - `GuiScreenLogin.cpp` — nhóm xác thực: màn hình đăng nhập
  - `GuiScreenAdmin.cpp` + `GuiAdminSubjects.cpp` (BST) / `GuiAdminQuestions.cpp` (DSLK đơn) / `GuiAdminClasses.cpp` (mảng con trỏ) / `GuiAdminStudents.cpp` (DSLK đơn) / `GuiAdminScores.cpp` (bảng điểm + chi tiết bài thi) — nhóm quản trị giáo viên, mỗi tab = 1 cấu trúc dữ liệu
  - `GuiScreenStudent.cpp`, `GuiExamPrepare.cpp`, `GuiExamRoom.cpp` (kèm đồng hồ tự nộp bài + modal xác nhận nộp), `GuiExamResult.cpp` — nhóm trải nghiệm thi của sinh viên
  - `GuiModalDialogs.cpp` — nhóm hộp thoại: 8 modal CRUD + rule chặn click ngoài modal
- **fix(ui): dọn `g.subjects` trước khi duyệt BST mỗi khung hình** trong `ui/GuiApp.cpp` — trước đó list chỉ được thêm vào, môn bị nhân đôi dần theo thời gian chạy (tự phát sinh "1 158 môn", phân trang 1/145). Sau fix: "Tổng: 6 môn / Trang 1/1" ổn định cả phiên.
- **fix(ui): thêm phân trang `< >` cho hàng "Mon:"** ở tab `Bang diem mon hoc` và `Chi tiet bai thi` (GV) — trước đó nút môn thứ 4 bị cắt lìa mép cửa sổ, vài môn không bấm tới được.
- **fix(ui): nút demo đăng nhập sinh viên dùng đúng mật khẩu thực tế** — `data/classes.txt` lưu `SV001/pass123`, nút cũ điền `123` nên bấm demo không vào được hệ thống; đồng thời nới nút cho chữ không tràn khung.
- **fix(ui): nút "Lưu File"** đổi chữ trắng (không đọc được trên nền trắng) sang viền + chữ xanh `COLOR_SUCCESS` — cùng pattern với các nút outline khác trong file.

### Kiểm chứng

- `make` / `make app`: 0 lỗi, 0 warning; `make test` PASS (kèm lượt chạy `test_core_contracts` với tham số `null`).
- Chụp ảnh tự động toàn bộ 24 màn hình (login, 6 tab GV, 4 modal thêm, 3 tab SV, phòng thi 3 câu, modal nộp bài, bảng kết quả, xem lại bài thi) trên bản build tách riêng; luồng thi CSDL → nộp → điểm → chi tiết chạy trọn vẹn.

### Notes

- `Makefile` không cần sửa: wildcard `ui/*.cpp` tự nhận các file mới.
- `AGENTS.md` và `PROGRESS.md` chỉ giữ bản local, không đưa vào repo (đã loại bằng `.gitignore`).
