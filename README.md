# Thi trắc nghiệm — Core C++

Phần core của đồ án Cấu trúc dữ liệu và Giải thuật: quản lý môn học, câu hỏi, lớp, sinh viên, điểm và bản chụp bài thi có trộn đáp án.

## Phạm vi

Core cung cấp cấu trúc dữ liệu và các thao tác trên dữ liệu. Demo và test chỉ dùng kiểm tra core. Giao diện, đăng nhập, điều phối thi, thời gian thi và lưu/đọc file thuộc tầng tích hợp; không phải ứng dụng hoàn chỉnh trong repo này.

| Dữ liệu | Cấu trúc |
|---|---|
| Môn học | Cây nhị phân tìm kiếm theo mã môn |
| Câu hỏi của môn | Danh sách liên kết đơn |
| Lớp | Mảng con trỏ, tối đa 10.000 lớp |
| Sinh viên của lớp | Danh sách liên kết đơn |
| Điểm của sinh viên | Danh sách liên kết đơn |
| Chi tiết bài thi | Danh sách `ExamDetail` thuộc từng `Score` |

Các cấu trúc chính dùng `char[]`, con trỏ và `new/delete`, không dùng STL container. `ExamDetail` sao chép câu hỏi, trộn đáp án và lưu vị trí đáp án đúng mới, giữ nguyên ngân hàng câu hỏi.

## Cấu trúc repo

```text
core/          Cấu trúc dữ liệu, header và phần cài đặt
examples/      Demo sử dụng core
tests/         Kiểm tra hồi quy
docs/          Hợp đồng API và tài liệu lịch sử
Makefile       Build demo và các test
```

## Build và kiểm tra

Cần GCC/Clang hỗ trợ C++17 và GNU Make (các lệnh dưới đây dùng trên Linux).

```bash
make -j2
make test
./build/core_demo
```

Kiểm tra bản tối ưu hóa trong thư mục riêng:

```bash
make BUILD_DIR=build-release CXXFLAGS="-std=c++17 -Wall -Wextra -Wpedantic -O2 -DNDEBUG" test
```

`make clean` xóa sản phẩm trong thư mục build mặc định. Mã nguồn không nằm trong thư mục này.

Nếu chỉ muốn chạy demo bằng GCC từ thư mục gốc:

```bash
g++ -std=c++17 -Wall -Wextra -Wpedantic examples/core_demo.cpp core/*.cpp -o core-test
./core-test
```

Mỗi file test có một `main` riêng, vì vậy không ghép tất cả `tests/*.cpp` vào cùng executable. Makefile đã tạo target riêng. Assert trong các test cũ được giữ hoạt động cả khi build Release.

## Những điều cần biết khi gọi API

- ID null, rỗng hoặc quá dài bị từ chối, không tự cắt ngắn: các hàm tạo node trả `nullptr`. Mã môn tối đa 15 ký tự một byte, mã lớp/sinh viên tối đa 19.
- Câu hỏi mới lấy ID từ `generateQuestionId`. Khi khôi phục dữ liệu phải kiểm tra ID trùng toàn bộ môn và đồng bộ bộ đếm.
- Dùng `addStudentGlobal` để kiểm tra mã sinh viên toàn hệ thống; lớp đích phải nằm trong mảng lớp.
- `addScore` chặn điểm trùng mã môn. Chính sách thi lại cần do nhóm thống nhất; đề bài không bắt buộc chỉ thi một lần.
- Không trộn lại snapshot sau khi đã trả lời. Chấm và xem bài theo thứ tự đáp án trong snapshot.
- Việc đổi mã môn không tự đổi `Score.subjectId`; tầng tích hợp phải bảo toàn liên kết này.

Xem [hợp đồng API](docs/CORE_API.md) trước khi tích hợp. [Tài liệu lịch sử](docs/history/README.md) chỉ giữ để tham khảo, có mô tả cũ không còn đúng với mã hiện tại.
