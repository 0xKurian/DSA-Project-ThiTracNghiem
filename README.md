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
third_party/   raylib 6.2 prebuilt (Linux amd64, link tĩnh)
Makefile       Build app, core_demo và các test
```

## Cấu trúc dữ liệu (core)

| Dữ liệu | Cấu trúc |
|---|---|
| Môn học | Cây nhị phân tìm kiếm theo mã môn |
| Câu hỏi của môn | Danh sách liên kết đơn |
| Lớp | Mảng con trỏ, tối đa 10.000 lớp |
| Sinh viên của lớp | Danh sách liên kết đơn |
| Điểm của sinh viên | Danh sách liên kết đơn |
| Chi tiết bài thi | Danh sách `ExamDetail` thuộc từng `Score` |

Các cấu trúc chính dùng `char[]`, con trỏ và `new/delete`, không dùng STL container. `ExamDetail` sao chép câu hỏi, trộn đáp án và lưu vị trí đáp án đúng mới, giữ nguyên ngân hàng câu hỏi. STL được dùng tự do ở tầng `business/` và `ui/`.

## Build và chạy

Cần GCC/Clang hỗ trợ C++17 và GNU Make (các lệnh dưới đây dùng trên Linux; GUI cần display X11).

```bash
make -j2
make app && ./build/app          # GUI raylib
./build/app --console            # giao diện console, không cần display
./build/core_demo                # demo core
```

- Chạy binary từ thư mục gốc repo: đường dẫn đọc/ghi đều tương đối (`data/subjects.txt`, `assets/fonts/*.ttf`).
- Tài khoản chạy thử (lấy từ `data/classes.txt`): giáo viên `GV` / `GV`, sinh viên `SV001` / `pass123`.
- Link GUI dùng raylib tĩnh: `third_party/raylib-6.2_linux_amd64/lib/libraylib.a -lGL -lm -lpthread -ldl -lrt -lX11`.

Kiểm tra bản tối ưu hóa trong thư mục riêng:

```bash
make BUILD_DIR=build-release CXXFLAGS="-std=c++17 -Wall -Wextra -Wpedantic -O2 -DNDEBUG" test
```

`make clean` xóa sản phẩm trong thư mục build mặc định. Mã nguồn không nằm trong thư mục này.

Mỗi file test có một `main` riêng, vì vậy không ghép tất cả `tests/*.cpp` vào cùng executable. Makefile đã tạo target riêng. Assert trong các test cũ được giữ hoạt động cả khi build Release.

## Tính năng chính (tính tới v2.1.0)

- **Giáo viên**: tab môn học (BST) có tìm kiếm lọc theo mã/tên; ngân hàng câu hỏi mở qua nút `Xem` của từng môn; master-detail "Lop hoc & Sinh vien" (lọc lớp, sĩ số, xem toàn trường, thêm/sửa/xóa SV); "Bang diem & Bai thi" theo flow môn → lớp → sinh viên với chi tiết bài thi ngay trong tab.
- **Sinh viên**: chọn môn thi (tìm kiếm), phòng thi có đồng hồ + nộp bài, kết quả và xem lại từng câu; bảng điểm cá nhân có cột `[Xem]` mở chi tiết.
- **Cấu hình thi theo môn**: GV đặt số câu + số phút; SV chỉ xem và không tự nhập. *Hiện lưu tạm trong bộ nhớ UI, đang chờ đội Core/Business thêm field + cột FileStorage.*
- **Xóa lớp còn sinh viên**: UI có modal cảnh báo số SV; việc mở chặn cascade đang ở team Business (hiện `deleteClassWorkflow` vẫn trả lỗi ràng buộc).
- Chi tiết từng phiên bản xem [CHANGELOG.md](CHANGELOG.md).

## Những điều cần biết khi gọi API

- ID null, rỗng hoặc quá dài bị từ chối, không tự cắt ngắn: các hàm tạo node trả `nullptr`. Mã môn tối đa 15 ký tự một byte, mã lớp/sinh viên tối đa 19.
- Câu hỏi mới lấy ID từ `generateQuestionId`. Khi khôi phục dữ liệu phải kiểm tra ID trùng toàn bộ môn và đồng bộ bộ đếm.
- Dùng `addStudentGlobal` để kiểm tra mã sinh viên toàn hệ thống; lớp đích phải nằm trong mảng lớp.
- `addScore` chặn điểm trùng mã môn. Chính sách thi lại cần do nhóm thống nhất; đề bài không bắt buộc chỉ thi một lần.
- Không trộn lại snapshot sau khi đã trả lời. Chấm và xem bài theo thứ tự đáp án trong snapshot.
- Việc đổi mã môn không tự đổi `Score.subjectId`; tầng tích hợp phải bảo toàn liên kết này.

Xem [hợp đồng API](docs/CORE_API.md) trước khi tích hợp. [Tài liệu lịch sử](docs/history/README.md) chỉ giữ để tham khảo, có mô tả cũ không còn đúng với mã hiện tại.
