# Thi trắc nghiệm C++ — môn học, lớp học, sinh viên, điểm thi

Đồ án Cấu trúc dữ liệu và Giải thuật: hệ thống quản lý và thi trắc nghiệm — môn học (BST), ngân hàng câu hỏi, lớp, sinh viên, điểm và bản chụp bài thi có trộn đáp án — với giao diện đồ họa raylib và giao diện console dùng chung tầng business.

## Cấu trúc repo

```text
core/          Cấu trúc dữ liệu cài tay: BST môn học, DSLK đơn câu hỏi/sinh viên/điểm/chi tiết, mảng lớp 10.000 phần tử
business/      Dịch vụ nghiệp vụ: đăng nhập, CRUD môn/lớp/SV/câu hỏi, đăng ký + chấm thi, lưu/đọc file
ui/            Giao diện raylib (các mô-đun Gui*) + giao diện console (ConsoleUI.cpp)
data/          subjects.txt (môn + câu hỏi), classes.txt (lớp, sinh viên) — dữ liệu chạy thử
assets/fonts/  Font dùng trong GUI
examples/      Demo sử dụng core
tests/         Kiểm tra hồi quy (mỗi file một main riêng)
docs/          Hợp đồng API và tài liệu lịch sử
third_party/   raylib 6.2 prebuilt (Linux amd64 & Win64 MinGW, link tĩnh)
chay_gui.bat   Script chạy nhanh GUI trên Windows
chay_console.bat Script chạy nhanh Console trên Windows
Makefile       Build app, core_demo và các test
```

## Cấu trúc dữ liệu (core)

| Dữ liệu | Cấu trúc |
|---|---|
| Môn học | Cây nhị phân tìm kiếm theo mã môn (lưu cấu hình thi: examQuestionCount, examMinutes) |
| Câu hỏi của môn | Danh sách liên kết đơn |
| Lớp | Mảng con trỏ, tối đa 10.000 lớp |
| Sinh viên của lớp | Danh sách liên kết đơn |
| Điểm của sinh viên | Danh sách liên kết đơn |
| Chi tiết bài thi | Danh sách `ExamDetail` thuộc từng `Score` |

Các cấu trúc chính dùng `char[]`, con trỏ và `new/delete`, không dùng STL container. `ExamDetail` sao chép câu hỏi, trộn đáp án và lưu vị trí đáp án đúng mới, giữ nguyên ngân hàng câu hỏi. STL được dùng tự do ở tầng `business/` và `ui/`.

## Build và chạy

Hỗ trợ cả môi trường **Linux** và **Windows** (GCC/Clang hỗ trợ C++17 và GNU Make).

### Chạy trên Windows (MinGW-w64)
- **Cách nhanh nhất:** Nhấp đúp hoặc chạy script batch có sẵn:
  ```cmd
  chay_gui.bat       # Tự động biên dịch và khởi chạy giao diện đồ họa Raylib
  chay_console.bat   # Tự động biên dịch và khởi chạy giao diện dòng lệnh Console
  ```
- **Hoặc qua Make:**
  ```cmd
  make app
  .\build\app.exe
  .\build\app.exe --console
  ```

### Chạy trên Linux (cần display X11 cho GUI)
```bash
make -j2
make app && ./build/app          # GUI raylib
./build/app --console            # giao diện console, không cần display
./build/core_demo                # demo core
```

- Chạy binary từ thư mục gốc repo: đường dẫn đọc/ghi đều tương đối (`data/subjects.txt`, `assets/fonts/*.ttf`).
- Tài khoản chạy thử (lấy từ `data/classes.txt`): giáo viên `GV` / `GV`, sinh viên `SV001` / `pass123`.
- Link GUI dùng raylib tĩnh:
  - Linux: `third_party/raylib-6.2_linux_amd64/lib/libraylib.a -lGL -lm -lpthread -ldl -lrt -lX11`
  - Windows: `third_party/raylib-6.2_win64_mingw-w64/lib/libraylib.a -lopengl32 -lgdi32 -lwinmm`

Kiểm tra bản tối ưu hóa trong thư mục riêng:

```bash
make BUILD_DIR=build-release CXXFLAGS="-std=c++17 -Wall -Wextra -Wpedantic -O2 -DNDEBUG" test
```

`make clean` xóa sản phẩm trong thư mục build mặc định. Mã nguồn không nằm trong thư mục này.

Mỗi file test có một `main` riêng, vì vậy không ghép tất cả `tests/*.cpp` vào cùng executable. Makefile đã tạo target riêng. Assert trong các test cũ được giữ hoạt động cả khi build Release.

## Tính năng chính (tính tới v2.2.2)

- **Giáo viên**:
  - **Quản lý Môn học (BST)**: Tìm kiếm lọc theo mã/tên giữ thứ tự duyệt BST; ngân hàng câu hỏi tích hợp trực tiếp qua nút `Xem` kèm nút `Quay lại`.
  - **Cấu hình thi theo môn học bền vững (v2.2.0)**: Giáo viên cấu hình số câu hỏi và thời gian thi (phút) gắn liền với môn học (`Subject.examQuestionCount`/`examMinutes`). Lưu file 5 cột trong `data/subjects.txt`, tương thích ngược file cũ 3 cột (giá trị 0 = chưa cấu hình).
  - **Master-detail Lớp học & Sinh viên (v2.1.0 - v2.2.1)**: Cột trái quản lý danh mục lớp (lọc mã, sĩ số, chọn xem toàn trường); cột phải quản lý danh sách sinh viên theo lớp với tìm kiếm MASV/họ tên, phân trang và CRUD tức thì.
  - **Xóa lớp có sinh viên (v2.2.0)**: Modal xác nhận hiển thị cảnh báo sĩ số; cho phép xóa lớp kèm cascade giải phóng an toàn toàn bộ sinh viên, điểm số và chi tiết bài thi trong Core.
  - **Bảng điểm & Bài thi (v2.1.0)**: Flow chuẩn môn → lớp → sinh viên, tra cứu điểm và bấm `[Xem]` mở chi tiết bài thi trực tiếp trong tab.
- **Sinh viên**:
  - **Chuẩn bị thi & Khóa thi lại (Retake Guard - v2.2.1)**: Đọc cấu hình thi read-only từ môn học; nếu môn chưa được GV cấu hình hoặc sinh viên đã có điểm thi thì nút `Bắt đầu thi ngay` bị khóa kèm hiển thị điểm số đã đạt (quy chế: mỗi SV chỉ thi 1 lần/môn).
  - **Phòng thi & Đồng hồ đếm ngược**: Hiển thị thời gian thực theo giây, tự động nộp bài khi hết giờ, xác nhận nộp bài.
  - **Xác nhận hủy phiên dở dang = Tự động nộp bài (v2.2.2)**: Bấm "Hủy phiên thi dở dang" mở modal `MODAL_CANCEL_EXAM` cảnh báo rõ: Hủy phiên sẽ tự động nạp đề và nộp bài qua `submitExamWorkflow` (câu chưa làm chấm SAI), ghi điểm vào hệ thống và khóa quyền thi lại môn học.
  - **Bảng điểm & Xem lại bài thi**: Bảng điểm cá nhân tích hợp nút `[Xem]` mở chi tiết từng câu hỏi, phương án đã chọn và đáp án đúng.
- **Tối ưu hóa UI/UX & Nền tảng**:
  - Widget tìm kiếm gợi ý (`GuiSearchSuggest`) và ô nhập văn bản có con trỏ UTF-8 (click đặt caret, phím Home/End/Arrows).
  - Chống click xuyên modal (`GuiModalAbsorbsInput`), click chọn hàng lớp thông minh, xử lý chống tràn chữ.
  - Hỗ trợ biên dịch và chạy test trên cả Windows (MinGW w64) và Linux.
- Chi tiết từng phiên bản xem [CHANGELOG.md](CHANGELOG.md) và [BAO_CAO_CAP_NHAT.md](BAO_CAO_CAP_NHAT.md).

## Những điều cần biết khi gọi API

- ID null, rỗng hoặc quá dài bị từ chối, không tự cắt ngắn: các hàm tạo node trả `nullptr`. Mã môn tối đa 15 ký tự một byte, mã lớp/sinh viên tối đa 19.
- Câu hỏi mới lấy ID từ `generateQuestionId`. Khi khôi phục dữ liệu phải kiểm tra ID trùng toàn bộ môn và đồng bộ bộ đếm.
- Dùng `addStudentGlobal` để kiểm tra mã sinh viên toàn hệ thống; lớp đích phải nằm trong mảng lớp.
- `addScore` chặn điểm trùng mã môn. Chính sách thi lại cần do nhóm thống nhất; đề bài không bắt buộc chỉ thi một lần.
- Không trộn lại snapshot sau khi đã trả lời. Chấm và xem bài theo thứ tự đáp án trong snapshot.
- Việc đổi mã môn không tự đổi `Score.subjectId`; tầng tích hợp phải bảo toàn liên kết này.

Xem [hợp đồng API](docs/CORE_API.md) trước khi tích hợp. [Tài liệu lịch sử](docs/history/README.md) chỉ giữ để tham khảo, có mô tả cũ không còn đúng với mã hiện tại.
