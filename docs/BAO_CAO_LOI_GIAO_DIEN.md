# BÁO CÁO TỔNG HỢP CÁC LỖI GIAO DIỆN (UI BUGS REPORT)
**Dự án:** Hệ thống Quản lý & Thi Trắc Nghiệm C++ (DSA Project)  
**Ngày rà soát:** 28/10/2026  
**Phạm vi:** Toàn bộ tầng giao diện (`ui/`) bao gồm Raylib Graphic UI và các tương tác Modal, Form, Table, Canvas.

---

## 1. BẢNG TỔNG HỢP MỨC ĐỘ NGHIÊM TRỌNG

| STT | Mã lỗi | Vị trí (File & Dòng) | Phân loại | Mức độ | Trạng thái |
|:---:|:---:|:---|:---|:---:|:---:|
| 1 | **BUG-01** | `ui/GuiCommon.cpp:496-500` | Crash / Out of bounds khi cuộn chuột gợi ý | **Nghiêm trọng (P0)** | Đang mở |
| 2 | **BUG-02** | `ui/GuiModalDialogs.cpp:206` | Truy cập chỉ số âm `g.classes[-1]` khi lưu sinh viên | **Nghiêm trọng (P0)** | Đang mở |
| 3 | **BUG-03** | `ui/GuiAdminQuestions.cpp:61` | Sai công thức `maxWidth` làm câu hỏi tràn khỏi khung | **Cao (P1)** | Đang mở |
| 4 | **BUG-04** | `ui/GuiExamRoom.cpp:73-84` | Câu hỏi dài đè trực tiếp lên khung Đáp án A phòng thi | **Cao (P1)** | Đang mở |
| 5 | **BUG-05** | `ui/GuiAdminQuestions.cpp:54-68` | Câu hỏi dài đè lên Đáp án A ở danh sách câu hỏi GV | **Cao (P1)** | Đang mở |
| 6 | **BUG-06** | `ui/GuiAdminClasses.cpp:180-236` | Chồng lấn cột bảng SV và cắt cụt họ tên vô lý | **Cao (P1)** | Đang mở |
| 7 | **BUG-07** | `ui/GuiAdminSubjects.cpp:59` | Tên môn học dài tràn đè lên các cột kế bên | **Trung bình (P2)** | Đang mở |
| 8 | **BUG-08** | `ui/GuiExamRoom.cpp:125-148` | Bảng số câu hỏi phòng thi tràn đáy màn hình (đề >= 70 câu) | **Trung bình (P2)** | Đang mở |
| 9 | **BUG-09** | `ui/GuiScreenStudent.cpp:120-140` | Bảng điểm cá nhân SV thiếu phân trang, tràn đáy cửa sổ | **Trung bình (P2)** | Đang mở |
| 10 | **BUG-10** | `ui/GuiModalDialogs.cpp:392-402` | Phím `Tab` không hoạt động trong Modal Cấu hình thi | **Trung bình (P2)** | Đang mở |
| 11 | **BUG-11** | `ui/GuiAdminExamConfigs.cpp:27-30` | Lọc cấu hình thi phân biệt hoa thường (`strncmp`) | **Trung bình (P2)** | Đang mở |
| 12 | **BUG-12** | `ui/GuiModalDialogs.cpp:173-176` | Modal sửa sinh viên cho sửa MASV dẫn đến lỗi Lưu | **Trung bình (P2)** | Đang mở |
| 13 | **BUG-13** | `ui/GuiExamPrepare.cpp:53-103` | Ô tìm kiếm môn thi trống rỗng dù môn đầu đã được chọn | **Thấp (P3)** | Đang mở |
| 14 | **BUG-14** | `ui/GuiScreenAdmin.cpp`, `ui/GuiScreenStudent.cpp` | Đăng xuất không xóa mật khẩu / tên đăng nhập cũ | **Thấp (P3)** | Đang mở |

---

## 2. CHI TIẾT TỪNG LỖI & PHƯƠNG ÁN KHẮC PHỤC

### BUG-01: Cuộn chuột trong dropdown gợi ý gây Out-of-bounds / Crash
- **File:** `ui/GuiCommon.cpp`
- **Dòng code:** `470-500` trong hàm `GuiSearchSuggest`
- **Hiện tượng:** Khi người dùng mở hộp thoại gợi ý (tìm môn, tìm lớp) và lăn chuột giữa, ứng dụng lập tức bị crash hoặc treo (Segmentation fault).
- **Nguyên nhân kỹ thuật:**
  ```cpp
  // Dòng 470-473: Clamping chỉ số suggestScroll
  int scrollMax = (int)matches.size() - maxRows;
  if (suggestScroll > scrollMax) suggestScroll = scrollMax;
  if (suggestScroll < 0) suggestScroll = 0;
  ...
  // Dòng 496: Thay đổi suggestScroll SAU KHI ĐÃ CLAMP
  if (shown > 0 && CheckCollisionPointRec(mouse, panel)) {
      suggestScroll += (int)GetMouseWheelMove() * -1;
  }
  // Dòng 500: Lấy phần tử mảng bằng suggestScroll chưa được kiểm tra lại
  for (int r = 0; r < shown; ++r) {
      const SuggestItem* it = matches[suggestScroll + r]; // TRÀN MẢNG NẾU LĂN CHUỘT
  ```
  Nếu số mục ít hơn hoặc bằng 6 mục (`scrollMax <= 0`) hoặc khi người dùng lăn chuột tiếp, `suggestScroll + r` sẽ vượt qua kích thước `matches.size()`, truy xuất vào vùng nhớ rác gây crash.
- **Phương án khắc phục:** Xử lý con lăn chuột trước khi tính toán `suggestScroll` và kẹp chặt giá trị:
  ```cpp
  if (shown > 0 && CheckCollisionPointRec(mouse, panel)) {
      suggestScroll += (int)GetMouseWheelMove() * -1;
  }
  int scrollMax = (int)matches.size() - maxRows;
  if (scrollMax < 0) scrollMax = 0;
  if (suggestScroll > scrollMax) suggestScroll = scrollMax;
  if (suggestScroll < 0) suggestScroll = 0;
  ```

---

### BUG-02: Truy cập chỉ số âm `g.classes[-1]` khi lưu sinh viên
- **File:** `ui/GuiModalDialogs.cpp`
- **Dòng code:** `206-209` trong hàm `drawStudentAddEditModal`
- **Hiện tượng:** Nếu trong hệ thống chưa có lớp học hoặc vừa xoá hết lớp (`g.selectedClassIdx = -1`), khi nhấn Enter hoặc bấm Lưu trong modal Sinh viên, chương trình truy cập vùng nhớ không hợp lệ.
- **Nguyên nhân kỹ thuật:**
  ```cpp
  if (g.selectedClassIdx < g.classCount && g.classes[g.selectedClassIdx] != nullptr)
  ```
  Biến `g.selectedClassIdx` là số nguyên có dấu (`int`). Khi mang giá trị `-1`, biểu thức `-1 < g.classCount` đúng nhưng `g.classes[-1]` là chỉ số âm ngoài phạm vi mảng con trỏ.
- **Phương án khắc phục:**
  ```cpp
  if (g.selectedClassIdx >= 0 && g.selectedClassIdx < g.classCount && g.classes[g.selectedClassIdx] != nullptr)
  ```

---

### BUG-03: Sai công thức `maxWidth` khiến câu hỏi tràn mép phải Card
- **File:** `ui/GuiAdminQuestions.cpp`
- **Dòng code:** `60-61` trong hàm `drawAdminTabQuestions`
- **Hiện tượng:** Chữ của câu hỏi dài trong bảng Quản lý câu hỏi của GV tràn hẳn ra ngoài viền phải của thẻ card thay vì tự xuống dòng.
- **Nguyên nhân kỹ thuật:**
  ```cpp
  float qx = std::max(qCard.x + 75.0f, qCard.x + 15 + MeasureTxt(idLabel, 16, true) + 10);
  DrawTxtWrapped(qList[i]->content, qx, qCard.y + 12, qCard.x + qCard.width - 85 - (qx - qCard.x), 16, COLOR_TEXT_DARK, true);
  ```
  Tham số thứ 4 của `DrawTxtWrapped` là **độ rộng tối đa** (tính bằng pixel). Việc cộng thêm `qCard.x` (~285px) vào độ rộng làm cho giới hạn dòng bị đẩy lên tới ~1150px, vượt xa kích thước thực tế của Card (~950px).
- **Phương án khắc phục:**
  ```cpp
  float maxW = qCard.width - (qx - qCard.x) - 25.0f;
  DrawTxtWrapped(qList[i]->content, qx, qCard.y + 12, maxW, 16, COLOR_TEXT_DARK, true);
  ```

---

### BUG-04: Nội dung câu hỏi dài đè lên Đáp án A trong phòng thi
- **File:** `ui/GuiExamRoom.cpp`
- **Dòng code:** `73-84` trong hàm `drawExamRoom`
- **Hiện tượng:** Khi sinh viên làm bài, nếu câu hỏi dài từ 3 đến 4 dòng, phần văn bản câu hỏi sẽ đè trực tiếp lên ô Đáp án A.
- **Nguyên nhân kỹ thuật:** Vị trí Y của Đáp án A bị gán cứng tại `qPane.y + 140`. Câu hỏi bắt đầu từ `qPane.y + 70`. Nếu câu hỏi cao hơn 70px (khoảng 3 dòng chữ trở lên), nó sẽ đè lên khung Đáp án A.
- **Phương án khắc phục:** Đo chiều cao câu hỏi bằng giá trị trả về của `DrawTxtWrapped` và bố trí các ô đáp án động:
  ```cpp
  float qH = DrawTxtWrapped(curQ->content, qPane.x + 30, qPane.y + 70, qPane.width - 60, 18, COLOR_TEXT_DARK, true);
  float startAnsY = std::max(qPane.y + 140.0f, qPane.y + 70.0f + qH + 15.0f);
  for (int i = 0; i < 4; ++i) {
      Rectangle ansRec = { qPane.x + 30, startAnsY + i * 82.0f, qPane.width - 60, 70 };
      ...
  }
  ```

---

### BUG-05: Nội dung câu hỏi dài đè lên Đáp án A ở Ngân hàng câu hỏi
- **File:** `ui/GuiAdminQuestions.cpp`
- **Dòng code:** `63-67`
- **Hiện tượng:** Tương tự BUG-04, đáp án A và B nằm cố định ở `qCard.y + 45` và `qCard.y + 70`. Câu hỏi bắt đầu ở `qCard.y + 12`. Nếu câu hỏi có từ 2 dòng trở lên sẽ chạm vào đáp án A.
- **Phương án khắc phục:** Nâng chiều cao card hoặc tính toán vị trí Y tương đối dựa trên số dòng của nội dung câu hỏi.

---

### BUG-06: Bảng Sinh viên chồng lấn cột và họ tên bị cắt cụt vô lý
- **File:** `ui/GuiAdminClasses.cpp`
- **Dòng code:** `180-236`
- **Hiện tượng:**
  1. Cột `MASV` cách `MA LOP` chỉ 67px (`xLop - xMasv = 112 - 45 = 67px`). Mã sinh viên dài (như `N21DCCN001`) bị dính sát hoặc đè sang chữ "MA LOP".
  2. Cột `HO VA TEN` bị ép hẹp chỉ còn 103px (`xGender - xName - 10`). Họ tên tiếng Việt bình thường (như `Nguyễn Văn Hoàng`, `Trần Thị Thu Hương`) đều bị cắt cụt thành `Nguyen Van...`.
  3. Cột `MAT KHAU` chỉ hiển thị cố định `"******"` nhưng chiếm tới ~100px chiều ngang.
- **Phương án khắc phục:**
  - Bỏ cột hiển thị "Mật khẩu" trên bảng danh sách (không cần thiết).
  - Tăng khoảng cách cột: `xMasv = 45`, `xLop = 135`, `xName = 225`, `xGender = 450`.
  - Độ rộng hiển thị họ tên nâng lên ~210px để hiển thị đầy đủ họ và tên tiếng Việt.

---

### BUG-07: Tên môn học dài đè lên cột Số câu & Thao tác
- **File:** `ui/GuiAdminSubjects.cpp`
- **Dòng code:** `59`
- **Hiện tượng:** Tên môn học dài (từ 35 ký tự trở lên) viết tràn qua vị trí `x + 660` (cột Số câu) và `x + 735` (các nút Xem/Sửa/Xóa).
- **Nguyên nhân kỹ thuật:** Đang gọi `DrawTxt(view[i]->name, ...)` mà không dùng `DrawTxtClipped(...)` có giới hạn `maxW`.
- **Phương án khắc phục:** Thay bằng:
  ```cpp
  DrawTxtClipped(view[i]->name, contentRec.x + 350, (float)rowY + 11, 15, COLOR_TEXT_DARK, false, 290.0f);
  ```

---

### BUG-08: Lưới số thứ tự câu hỏi phòng thi tràn đáy màn hình khi đề nhiều câu
- **File:** `ui/GuiExamRoom.cpp`
- **Dòng code:** `125-148`
- **Hiện tượng:** Danh sách câu hỏi bên phải fix cứng 6 cột (`cols = 6`). Với đề thi 60–100 câu (10–17 hàng nút), toạ độ Y của các câu hỏi cuối vượt quá chiều cao cửa sổ 768px, sinh viên không thể chọn các câu này.
- **Phương án khắc phục:**
  - Tăng số cột lên `cols = 8` hoặc `cols = 10` khi tổng số câu > 40.
  - Hoặc giảm kích thước nút / bổ sung phân trang (`Trang 1: 1-40`, `Trang 2: 41-80`...).

---

### BUG-09: Bảng điểm cá nhân của sinh viên thiếu phân trang
- **File:** `ui/GuiScreenStudent.cpp`
- **Dòng code:** `120-140`
- **Hiện tượng:** Dùng vòng lặp `while (sc != nullptr)` vẽ tất cả điểm thi mà sinh viên đã làm. Khi sinh viên có từ 12 bài thi trở lên, các dòng điểm sẽ vẽ tràn ra ngoài màn hình.
- **Phương án khắc phục:** Áp dụng biến hằng `SCORES_PER_PAGE = 8` và thanh điều hướng phân trang (tương tự như màn hình điểm của giáo viên).

---

### BUG-10: Phím Tab không hoạt động trong Modal Cấu hình thi
- **File:** `ui/GuiModalDialogs.cpp`
- **Dòng code:** `392-402`
- **Hiện tượng:** Trong modal "Tạo/Sửa Cấu hình thi", bấm phím `Tab` không chuyển đổi giữa ô "Số câu" và "Thời gian thi".
- **Nguyên nhân:** Khối xử lý `KEY_TAB` chưa bổ sung nhánh cho `MODAL_CLASS_EXAM_CONFIG`.
- **Phương án khắc phục:** Bổ sung vào khối xử lý `KEY_TAB`:
  ```cpp
  else if (g.currentModal == MODAL_CLASS_EXAM_CONFIG) {
      g.activeFormBox = (g.activeFormBox == 3) ? 4 : 3;
  }
  ```

---

### BUG-11: Bộ lọc Cấu hình thi phân biệt chữ hoa / chữ thường
- **File:** `ui/GuiAdminExamConfigs.cpp`
- **Dòng code:** `27-30`
- **Hiện tượng:** Nếu gõ chữ thường vào ô lọc (vd: `d21cqcn01`), danh sách lọc sẽ trả về 0 kết quả dù cấu hình có tồn tại.
- **Nguyên nhân:** Dùng `strncmp` thay vì hàm tìm kiếm không phân biệt hoa thường `GuiMatchText`.
- **Phương án khắc phục:** Đổi sang dùng `GuiMatchText(g.classes[i]->id, g.configFilterClass)` và `GuiMatchText(cfg->subjectId, g.configFilterSubject)`.

---

### BUG-12: Modal Sửa sinh viên cho phép sửa Mã sinh viên
- **File:** `ui/GuiModalDialogs.cpp`
- **Dòng code:** `173-176`
- **Hiện tượng:** Khi chọn "Sửa sinh viên", ô `MASV` vẫn là ô nhập văn bản. Nếu người dùng sửa MASV rồi bấm Lưu, hệ thống sẽ báo lỗi `Khong tim thay sinh vien` vì MASV là khóa chính không được phép đổi.
- **Phương án khắc phục:** Kiểm tra nếu `g.currentModal == MODAL_EDIT_STUDENT` thì vẽ MASV dưới dạng Text cố định thay vì TextBox.

---

### BUG-13: Ô tìm kiếm môn thi trống rỗng dù môn đầu đã được chọn
- **File:** `ui/GuiExamPrepare.cpp`
- **Dòng code:** `53-103`
- **Hiện tượng:** Khi sinh viên vào tab "Chuẩn bị thi", môn đầu tiên (`g.studentSubChoice = 0`) đã được nạp thông tin bên dưới, nhưng ô tìm kiếm phía trên lại trống trơn và hiển thị placeholder.
- **Phương án khắc phục:** Nếu `g.searchPrepare[0] == '\0'` và danh sách môn không rỗng, tự động điền mã/tên của môn đang chọn vào `g.searchPrepare` hoặc hiển thị rõ nhãn môn đang chọn.

---

### BUG-14: Đăng xuất không xoá tài khoản / mật khẩu cũ
- **File:** `ui/GuiScreenAdmin.cpp:18-21`, `ui/GuiScreenStudent.cpp:20-24`
- **Hiện tượng:** Bấm "Đăng xuất" quay về màn hình Login nhưng tài khoản và mật khẩu của phiên làm việc trước vẫn lưu nguyên trong ô nhập.
- **Phương án khắc phục:** Gọi `memset(g.loginPass, 0, sizeof(g.loginPass));` và `memset(g.loginUser, 0, sizeof(g.loginUser));` khi nhấn Đăng xuất.

---

## 3. LỘ TRÌNH KHUYẾN NGHỊ KHẮC PHỤC

1. **Giai đoạn 1 (Khẩn cấp):** Sửa BUG-01, BUG-02, BUG-03 để đảm bảo ứng dụng chạy ổn định tuyệt đối, không crash bộ nhớ và không vẽ sai toạ độ.
2. **Giai đoạn 2 (Trực quan & Bố cục):** Sửa BUG-04, BUG-05, BUG-06, BUG-07 để các màn hình thi và danh sách quản trị không bị đè chữ, chữ rõ ràng và thẩm mỹ.
3. **Giai đoạn 3 (Hoàn thiện UX):** Sửa BUG-08 đến BUG-14 để hỗ trợ đề thi nhiều câu, phân trang bảng điểm và thao tác phím Tab mượt mà.
