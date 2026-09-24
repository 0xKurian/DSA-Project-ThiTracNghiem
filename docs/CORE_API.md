# Hợp đồng Core

## Module

`Structures.h` và `Constants.h` định nghĩa dữ liệu. Mỗi module có header riêng: `SubjectBST`, `QuestionList`, `ClassArray`, `StudentList`, `ScoreList`, `StringUtils`. `FunctionPrototype.h` tổng hợp header core và giữ các khai báo nghiệp vụ chưa được core cài đặt.

## ID và chuỗi

`createSubject`, `createClass`, `createStudent`, `createScore` trả `nullptr` nếu ID null, rỗng hoặc không vừa bộ đệm. `editSubject` và `addScore` trả `false` cho ID không hợp lệ. Kiểm tra kết quả trước khi dùng con trỏ.

Các chuỗi khác phải hợp lệ và kết thúc bằng ký tự null; nội dung quá dài có thể bị cắt theo kích thước bộ đệm. Validation tên, điểm, đáp án và dữ liệu nhập thuộc bên gọi theo chính sách nhóm. Điểm phải hữu hạn và nằm trong thang điểm đã thống nhất.

## Ownership

| Thao tác | Quyền sở hữu |
|---|---|
| `insertSubject` | Nhận node mới; tự giải phóng node bị từ chối do trùng mã |
| `addClass`, `addStudent`, `addStudentGlobal` | Chỉ nhận node khi thành công; bên gọi xử lý node khi thất bại |
| `createScore`/`addScore` có `detailList` | Thành công thì Score sở hữu danh sách; thất bại thì bên gọi vẫn sở hữu |
| Xóa lớp/sinh viên/điểm | Giải phóng cả dữ liệu con thuộc ownership |

Không chèn lại cùng node hoặc chia sẻ một danh sách chi tiết giữa nhiều Score. Sau khi sửa/xóa môn, tìm lại node theo ID; không mặc định con trỏ cũ còn hợp lệ hoặc còn biểu diễn cùng môn.

## ID câu hỏi

Mọi câu hỏi mới phải dùng `generateQuestionId()`. Khi nạp dữ liệu, tầng nạp kiểm tra tính duy nhất trên tất cả môn và gọi `updateQuestionIdCounter(id)` cho ID được phục hồi trước khi tạo câu mới. API nhận ID trực tiếp không có registry chống trùng toàn cục. ID phải dương và còn chỗ cho giá trị tiếp theo trong kiểu `int`.

## Lớp và sinh viên

Bên gọi cung cấp mảng đủ `MAX_CLASS` phần tử và số lớp hợp lệ. `addStudent` chỉ kiểm tra trùng trong lớp; `addStudentGlobal` kiểm tra toàn mảng và yêu cầu lớp đích được đăng ký trong đó.

## Chi tiết bài thi và trộn đáp án

`createExamDetail` sao chép câu hỏi và tự trộn đáp án trên bản sao; trả null nếu câu hỏi null hoặc đáp án đúng ngoài A–D. `selectedAnswer` ban đầu bằng ký tự null.

`shuffleExamDetailAnswers` bỏ qua node đã có lựa chọn. Giao diện phải dùng đáp án trong snapshot, chấm theo `correctAnswer` của snapshot, và lưu nguyên thứ tự này để xem lại. Bộ sinh ngẫu nhiên `rand` dùng trạng thái chung; tránh reseed trong từng câu hỏi.

`addScore` không nhận bản ghi trùng mã môn; quy tắc thi lại và cách chọn kết quả hiển thị thuộc tầng nghiệp vụ. Đổi/xóa môn không tự cập nhật các tham chiếu trong điểm. Core không điều phối đăng nhập, thời gian thi hoặc file.

## Kiểm tra

`make test` chạy 8 ca: tích hợp core, môn trùng, sinh viên toàn cục, độ dài ID sinh viên, độ dài mã điểm, sửa môn, các hợp đồng core và ID null. Bộ kiểm tra hợp đồng gồm 1.000 lần kiểm tra ánh xạ đáp án đúng sau trộn; đây không phải phép đo chất lượng phân phối ngẫu nhiên.
