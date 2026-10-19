# Changelog

Dòng thời gian so với bản cũ trước khi cập nhật GitHub (mốc so sánh: commit `373b2a6 "Add files via upload"`). Kiểu viết theo Conventional Commits.

## [2.2.0] — 2026-10-19

### Added

- **feat(business): lưu cấu hình thi theo môn** — `Subject` thêm `examQuestionCount`/`examMinutes` (0 = chưa cấu hình, `createSubject` khởi tạo); `setExamConfigWorkflow` validate `1..số câu ngân hàng` và `1..180`; `FileStorage` ghi/đọc 2 cột cuối dòng môn trong `data/subjects.txt`, file cũ 3 cột vẫn đọc được (đọc 0); 2 test mới (`test_subject_exam_config`, `test_class_delete_with_students`); tài liệu tầng business (`business/README.md`, `readme_extension.md`).

### Changed

- **feat(ui): nối cấu hình thi vào data thật** — bỏ vector tạm `examConfigs` trong `ui/GuiState.h`; modal `Cau hinh thi` gọi `setExamConfigWorkflow`, tab chuẩn bị của SV đọc thẳng field của môn (không còn mất cấu hình khi thoát).
- **feat(business): xóa lớp còn sinh viên** — bỏ chặn `ERR_CONSTRAINT_VIOLATION` trong `deleteClassWorkflow`; `core/deleteClass` tự giải phóng danh sách SV của lớp; UI đã có sẵn cảnh báo sĩ số trong modal xác nhận.
- Makefile hỗ trợ build trên Windows (MinGW + raylib win64) song song Linux; thêm 2 target test mới.

### Kiểm chứng

- Merge `feat/business` (`62dd858`) vào `main`; `make app` 0 lỗi/0 warning; `make test` PASS (gồm 2 test mới + `test_core_contracts null`).

## [2.1.0] — 2026-10-18

### Added

- **widget tìm kiếm + gợi ý dùng chung** (`GuiSearchSuggest` trong `ui/GuiCommon`): gõ ký tự nào thì lọc theo mã hoặc tên, danh sách xếp tăng dần theo mã, chọn dòng để áp dụng; dropdown có cơ chế chặn click xuyên xuống bảng phía sau.
- **ô nhập liệu có con trỏ soạn thảo**: click đặt caret, phím trái/phải/Home/End/Delete/Backspace sửa được giữa chuỗi (UTF-8), ô cuộn ngang theo caret.
- **tab Môn học (GV)**: thanh tìm kiếm lọc bảng theo mã/tên (giữ thứ tự duyệt BST) và nút **Xem** trên từng dòng; tab "Ngan hang cau hoi" rời sidebar, mở qua nút Xem, có nút **"<- Quay lai"**.
- **tab Lop hoc & Sinh vien (GV)**: gộp "Danh muc lop hoc" + "Sinh vien theo lop" thành màn hình master-detail 2 cột — trái là danh mục lớp (lọc, sĩ số, `Xem`/`Xoa`, phân trang) kèm mục **"Tat ca cac lop (Toan truong)"**; phải là sinh viên của lớp (hoặc toàn trường) với ô tìm MASV/họ tên, bảng đầy đủ cột và phân trang; sau khi thêm/xóa sinh viên sĩ số cập nhật ngay.
- **tab Bang diem & Bai thi (GV)**: gộp "Bang diem mon hoc" + "Chi tiet bai thi", flow **môn → lớp → sinh viên**; bảng điểm theo môn/lớp, `[Xem]` mở chi tiết bài thi ngay trong tab (có nút quay lại).
- **tab Bảng điểm SV**: gộp "Bang diem ca nhan" + "Xem lai bai thi" thành 1 tab, cột **[Xem]** mở chi tiết bài thi ngay trong tab kèm nút quay lại.
- **cấu hình thi theo môn (tạm thời trong bộ nhớ UI)**: nút `Cau hinh thi` ở ngân hàng câu hỏi (số câu + số phút, validate theo ngân hàng); tab Vào thi của SV chỉ hiện cấu hình read-only và khóa nút bắt đầu khi môn chưa cấu hình.
- **đăng nhập**: bỏ hai nút điền tài khoản demo; không còn hiện `GV/GV` hay `SV001/pass123` trên UI, người dùng tự nhập.

### Changed

- Bảng môn học dịch xuống để chừa hàng tìm kiếm; chỉnh vị trí header/cột thao tác các bảng cho khỏi chồng chữ.
- **fix(ui): đăng xuất ở tab bảng điểm / xem lại làm tắt chương trình** — nguyên nhân: `loggedStudent` bị đặt về `nullptr` rồi vẫn vẽ tiếp trong cùng frame; nay thoát ngay sau khi chuyển về màn đăng nhập.
- **fix(ui): phím ESC làm tắt ứng dụng** — raylib mặc định gán ESC là exit key; đã `SetExitKey(KEY_NULL)` để ESC chỉ đóng modal.
- `ui/GuiState.h`: thêm buffer/cờ cho các ô tìm kiếm, `examConfigs`, `formClassId`, các ID chọn trong tab điểm; bỏ field điều hướng cũ không còn dùng.

### Kiểm chứng

- `make app`: 0 lỗi, 0 warning; `make test` PASS.
- Chụp tự động 18 màn hình trên bản copy tách riêng (login trống, ngân hàng câu hỏi + cấu hình thi, master-detail lớp/SV, xóa lớp có cảnh báo sĩ số, bảng điểm theo môn/lớp, chi tiết bài thi, prepare chưa cấu hình / đã cấu hình, phòng thi, kết quả) — bố cục đúng.
- Test tay bằng chuột/bàn phím thật: gõ "ditconrme" rồi ←← + Backspace sửa thành "ditconme"; click giữa chuỗi và Home/Delete hoạt động; lọc lớp "d23"; ESC đóng modal không tắt app; đăng nhập `GV/GV` bằng tay.

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
