# Hợp đồng Core

> **Trạng thái chuyển đổi:** `ExamConfig` trong `Class.configList` là mô hình
> cấu hình thi mới theo `HANDOFF_CORE.md` và `HANDOFF_BUSINESS.md`. Hai trường
> `Subject.examQuestionCount/examMinutes` bên dưới vẫn có trong Core để UI cũ
> biên dịch, nhưng business mới không còn lưu chúng vào `subjects.txt`. Việc
> xóa hai trường này cần thực hiện cùng lúc với chuyển đổi UI và test cũ.

## Module

`Structures.h` và `Constants.h` định nghĩa dữ liệu. Mỗi module có header riêng: `SubjectBST`, `QuestionList`, `ClassArray`, `StudentList`, `ScoreList`, `ExamConfigList`, `StringUtils`. `FunctionPrototype.h` tổng hợp header core và giữ các khai báo nghiệp vụ chưa được core cài đặt.

## Cấu hình thi trên Subject

`struct Subject` trong `Structures.h` chứa hai trường lưu cấu hình thi trắc nghiệm:
- `int examQuestionCount`: Số lượng câu hỏi lấy ra cho mỗi đề thi.
- `int examMinutes`: Thời gian làm bài thi (tính theo phút).

**Giá trị mặc định và ý nghĩa của giá trị `0`:**
- Khi khởi tạo node môn học mới thông qua `createSubject()`, cả hai trường đều được khởi tạo giá trị mặc định bằng `0`.
- Giá trị `0` biểu thị môn học **chưa được cấu hình thi**. Khi giá trị bằng `0`, hệ thống coi môn học chưa sẵn sàng cho sinh viên thi và cần được giáo viên/quản trị viên cấu hình trước.
- Khi cấu hình thông qua workflow nghiệp vụ `setExamConfigWorkflow()`, `examQuestionCount` phải nằm trong đoạn `1..countQuestions(subject->questionList)` và `examMinutes` phải nằm trong đoạn `1..180`.
- Lưu ý về Core: `editSubject()` khi đổi mã môn (`oldId != newId`) giữ nguyên cấu hình `examQuestionCount` và `examMinutes` từ node cũ sang node mới. `deleteSubject()` với node có 2 con sao chép cấu hình thi từ successor.

## Cấu hình thi theo lớp (ExamConfig)

`struct ExamConfig` gắn trực tiếp vào `Class.configList`. Module `ExamConfigList`:
- `createExamConfig(subjectId, minutes, createdAt, questionIds, questionCount)`: Tạo node cấu hình thi, cấp phát động mảng `questionIds` (`new int[count]`). Từ chối (`nullptr`) nếu ID không hợp lệ, `count <= 0`, `minutes <= 0` hoặc `questionIds == nullptr`.
- `addExamConfig(classroom, config)`: Nhận node mới gắn vào cuối danh sách của lớp. Từ chối (`false`) nếu lớp đã có cấu hình cho môn học này; khi thất bại bên gọi vẫn sở hữu node.
- `findExamConfig(classroom, subjectId)`: Tìm cấu hình thi của môn trong lớp.
- `deleteExamConfig(classroom, subjectId)`: Xóa và giải phóng node cấu hình cùng mảng `questionIds`.
- `deleteClass`: Tự động gọi `deleteExamConfigList` giải phóng toàn bộ cấu hình thi thuộc lớp (cascade).

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
