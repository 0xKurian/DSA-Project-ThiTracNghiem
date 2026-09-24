# Báo cáo tiến độ nhóm — Thi trắc nghiệm C++

**Ngày:** 24/09/2026  
**Nhóm:** 3 thành viên  
**Repository:** [DSA-Project-ThiTracNghiem](https://github.com/0xKurian/DSA-Project-ThiTracNghiem)  
**Phiên bản đánh giá:** [`b92cbd5`](https://github.com/0xKurian/DSA-Project-ThiTracNghiem/commit/b92cbd5d65e14f53991b1c4085899472c607db13)

## 1. Kết luận đối chiếu đề bài

**Phần Core hiện tại đúng hướng và phù hợp cả 5 cấu trúc dữ liệu bắt buộc của đề bài.** Các thao tác quản lý dữ liệu cơ bản và phần mở rộng trộn đáp án đã có, đã được kiểm tra và có tài liệu hợp đồng API để bàn giao.

**Chưa thể kết luận toàn bộ chương trình đã đáp ứng đầy đủ đề bài.** Repo đang thể hiện phần Core, demo và kiểm tra. Chưa có bằng chứng triển khai đầy đủ UI, đăng nhập, luồng thi có giới hạn thời gian, bảng điểm theo lớp/môn và lưu/đọc file. Đây là công việc tích hợp của nhóm, không được tính là thiếu sót riêng của người phụ trách Core.

Core có thể bàn giao để người 1 và người 3 bắt đầu tích hợp, với các điều kiện rõ ràng về ID câu hỏi, quyền sở hữu con trỏ, giá trị điểm và liên kết mã môn. “Đã kiểm tra” không đồng nghĩa đã chứng minh mọi trường hợp biên đều đúng.

## 2. Phân công nhóm

| Thành viên | Vai trò | Trách nhiệm chính | Trạng thái có thể xác nhận từ repo |
|---|---|---|---|
| Người 1 | UI | Màn hình đăng nhập, menu theo vai trò, form nhập liệu, giao diện thi, hiển thị thời gian, bảng điểm và chi tiết bài thi | Chưa có phần triển khai UI trong snapshot được đánh giá |
| Người 2 — bạn | Core | Cấu trúc dữ liệu, thao tác thêm/tìm/xóa/sửa, quản lý bộ nhớ, ID câu hỏi, dữ liệu điểm và snapshot có trộn đáp án, hợp đồng API | Đã có các module chính, kiểm tra và tài liệu; sẵn sàng tích hợp với các điều kiện ở mục 6 |
| Người 3 | Business Logic | Xác thực và phân quyền; validation nghiệp vụ; điều phối quản lý dữ liệu và thi; chọn câu hỏi, chấm điểm, lưu/đọc file; truy vấn bảng điểm và bài đã thi | Chưa thấy luồng nghiệp vụ hoàn chỉnh trong snapshot; một số hàm mới có khai báo |

Trạng thái trên chỉ phản ánh mã đã có trong repo này. Không suy ra rằng người 1 hoặc người 3 chưa làm việc ở máy riêng, nhánh khác hoặc repo khác. Chưa đủ cơ sở để đưa ra phần trăm hoàn thành của từng người.

## 3. Đối chiếu cấu trúc dữ liệu bắt buộc

| Đề bài | Core hiện tại | Kết luận |
|---|---|---|
| Môn học: BST gồm MAMH C15, TENMH, con trỏ danh sách câu hỏi | `Subject` có ID, tên, `questionList`, `left`, `right`; bộ đệm mã môn 16 byte | Đúng mô hình; mã tối đa 15 ký tự một byte và ký tự kết thúc |
| Câu hỏi: danh sách liên kết đơn gồm ID, nội dung, A/B/C/D, đáp án | `Question` có các trường tương ứng và `next` | Đúng cấu trúc |
| ID câu hỏi tự tăng, không trùng giữa tất cả môn | Bộ đếm dùng chung qua `generateQuestionId`, có `updateQuestionIdCounter` | Có cơ chế hỗ trợ; tính duy nhất khi nhập/nạp dữ liệu phụ thuộc hợp đồng sử dụng, chưa có registry tự phát hiện mọi ID trùng |
| Lớp: mảng con trỏ tối đa 10.000 lớp | API dùng `Class* classes[]`, `MAX_CLASS = 10000`; mỗi lớp có `studentList` | Đúng; bên gọi phải cấp mảng đủ dung lượng và quản lý số lớp hợp lệ |
| Sinh viên: danh sách liên kết đơn với thông tin theo đề và con trỏ điểm | `Student` có ID, họ, tên, giới tính, mật khẩu, `scoreList`, `next` | Đúng cấu trúc |
| Điểm: danh sách liên kết đơn gồm mã môn và điểm | `Score` có `subjectId`, `score`, `next`; thêm `detailList` | Đúng cấu trúc nền; phần thêm phục vụ xem lại bài thi |

Nguồn: [Structures.h](https://github.com/0xKurian/DSA-Project-ThiTracNghiem/blob/b92cbd5d65e14f53991b1c4085899472c607db13/core/Structures.h), [Constants.h](https://github.com/0xKurian/DSA-Project-ThiTracNghiem/blob/b92cbd5d65e14f53991b1c4085899472c607db13/core/Constants.h), [QuestionList.cpp](https://github.com/0xKurian/DSA-Project-ThiTracNghiem/blob/b92cbd5d65e14f53991b1c4085899472c607db13/core/QuestionList.cpp).

Đề bài cho phép thiết kế thêm danh sách, nên việc thêm `ExamDetail` phù hợp. Không cần thay danh sách câu hỏi bằng cây. Không bắt buộc có cấu trúc tên `ExamHistory` nếu mô hình lưu kết quả hiện tại đáp ứng chính sách thi đã thống nhất.

## 4. Đối chiếu chức năng a–i

| Mục | Yêu cầu | Nền tảng Core đã có | Công việc tích hợp tiếp theo |
|---|---|---|---|
| a | Đăng nhập MASV/password; GV/GV toàn quyền | Thông tin sinh viên, tìm sinh viên toàn cục, enum vai trò; khai báo `login` | Người 3 xác thực và kiểm tra quyền; người 1 làm giao diện, menu phù hợp |
| b | Nhập lớp | `createClass`, `addClass`, chặn mã trùng và vượt số lớp | Người 1 làm form; người 3 kiểm tra đầu vào, gọi API và xử lý kết quả |
| c | In danh sách lớp | `printClasses` đã có | Người 1 đưa vào giao diện; người 3 cung cấp danh sách cần hiển thị |
| d | Nhập mã lớp trước, rồi nhập sinh viên vào lớp đó | `findClass`, `createStudent`, `addStudentGlobal` | Người 3 điều phối chọn lớp và thêm sinh viên; người 1 làm luồng nhập |
| e | Thêm/xóa/hiệu chỉnh môn học | Các thao tác BST, `editSubject` giữ danh sách câu hỏi khi đổi mã | Người 3 xử lý môn đã có điểm và cập nhật tham chiếu; người 1 làm giao diện |
| f | Nhập câu hỏi với ID tự sinh duy nhất toàn hệ thống | `generateQuestionId`, `createQuestion`, `insertQuestion` | Người 3 bảo đảm dùng đúng nguồn ID và kiểm tra dữ liệu; người 1 không cho người dùng tự gán ID |
| g | Chọn môn, số câu, số phút; lấy câu ngẫu nhiên và thi | Ngân hàng câu hỏi, đếm câu, tạo snapshot, lưu kết quả trong RAM | Người 3 chọn câu không lặp, kiểm tra điều kiện, quản lý hạn thi và chấm điểm; người 1 hiển thị đề, thời gian, nhận câu trả lời |
| h | In chi tiết các câu đã thi một môn của một sinh viên | `Score.detailList` giữ nội dung và thứ tự đáp án đã trộn, lựa chọn và đáp án đúng | Người 3 truy xuất đúng kết quả; người 1 hiển thị bản chụp đã lưu |
| i | In bảng điểm một môn của một lớp, ghi “Chưa thi” | Danh sách lớp/sinh viên, `findScore` | Người 3 duyệt đủ sinh viên, phân biệt chưa có điểm với điểm 0; người 1 hiển thị bảng |
| Bổ sung | Lưu danh sách vào file và kiểm tra dữ liệu sai | Dữ liệu có thể duyệt, API cập nhật bộ đếm và các thao tác core | Người 3 triển khai lưu/đọc, dựng lại con trỏ, kiểm tra dữ liệu và đồng bộ ID |

Các API nghiệp vụ/file trong [FunctionPrototype.h](https://github.com/0xKurian/DSA-Project-ThiTracNghiem/blob/b92cbd5d65e14f53991b1c4085899472c607db13/core/FunctionPrototype.h) mới là khai báo; không coi đó là chức năng hoàn chỉnh.

## 5. Tiến trình phần Core của người 2

### Đã thực hiện

- Xây dựng BST môn học, danh sách câu hỏi, mảng lớp, danh sách sinh viên và điểm.
- Cung cấp thao tác tạo, thêm, tìm, xóa; hiệu chỉnh môn học và thông tin sinh viên.
- Có kiểm tra sinh viên trùng mã toàn hệ thống qua `addStudentGlobal`.
- Có cơ chế ID câu hỏi tăng dùng chung và hàm đồng bộ bộ đếm khi phục hồi dữ liệu.
- Tạo snapshot bài thi và trộn đáp án trên bản sao; cập nhật đáp án đúng theo vị trí mới.
- Giải phóng dữ liệu con khi xóa lớp, sinh viên hoặc điểm.
- Sửa lỗi biên dịch; từ chối ID null/rỗng/quá dài thay vì cắt ID âm thầm.
- Chặn thêm sinh viên vào lớp ngoài mảng quản lý và chặn trộn lại snapshot đã có câu trả lời.
- Ghi rõ ownership, điều kiện đầu vào và trách nhiệm bên gọi trong header và tài liệu.
- Tổ chức repo thành `core/`, `examples/`, `tests/`, `docs/`; giữ tài liệu cũ trong `docs/history/`.
- Thêm `Makefile`, `.gitignore`, cập nhật các phép kiểm tra theo API mới.

### Phần mở rộng trộn đáp án

Luồng core: `Question → createExamDetail → sao chép → trộn A/B/C/D → cập nhật correctAnswer → ExamDetail`.

Cách làm phù hợp vì không thay đổi ngân hàng câu hỏi và có thể giữ lại đề đúng như sinh viên đã thấy. Khi hiển thị/chấm/xem lại, phải dùng dữ liệu của `ExamDetail`; không dùng thứ tự đáp án ở câu hỏi gốc. Chỉ trộn trước khi trả lời. Tính năng này không thay thế việc chọn ngẫu nhiên câu hỏi ở mục g.

### Kết quả kiểm tra hiện tại

Ngày 24/09/2026, đã chạy lại trên bản cục bộ tương ứng với commit được đánh giá:

| Cấu hình | Kết quả |
|---|---|
| `make test` bản Debug | 8 lượt kiểm tra đạt |
| Bản Release với `-O2 -DNDEBUG -Werror` | 8 lượt kiểm tra đạt |

Tám lượt gồm 7 executable kiểm tra và 1 lần gọi riêng cho ID null. Trong đó, bộ kiểm tra hợp đồng có 14 kiểm tra kết quả tổng cộng qua hai lần gọi, gồm kiểm tra ánh xạ đáp án đúng trong 1.000 lần trộn. Đây không phải số liệu coverage hay chứng minh chất lượng phân phối ngẫu nhiên.

Các lần kiểm tra trước đã chạy ASan/UBSan với kiểm tra leak tắt do giới hạn môi trường. Chưa xác nhận không rò rỉ bộ nhớ trên mọi đường đi, chưa benchmark tải lớn hoặc kiểm thử toàn ứng dụng.

## 6. Điều kiện bàn giao giữa các thành viên

1. **ID câu hỏi:** người 3 luôn dùng `generateQuestionId` cho câu mới; khi load phải phát hiện ID trùng trên toàn bộ môn và gọi `updateQuestionIdCounter`. Cần xử lý hoặc giới hạn dữ liệu để bộ đếm `int` không tràn.
2. **Node không hợp lệ:** các hàm tạo có thể trả `nullptr`; người 3 phải kiểm tra trước khi truy cập. Người 1 hiển thị lỗi nhập liệu tương ứng.
3. **Ownership:** bên gọi tự giải phóng node khi API thêm từ chối, trừ `insertSubject` có quy tắc nhận ownership riêng. Không chia sẻ một danh sách chi tiết giữa nhiều điểm.
4. **Đổi/xóa môn:** người 3 chọn chặn thao tác khi đã có điểm hoặc cập nhật tham chiếu thống nhất. Core không tự cập nhật `Score.subjectId`.
5. **Điểm và đáp án:** người 3 kiểm tra điểm hữu hạn, nằm trong thang điểm nhóm chọn, và đáp án hợp lệ. Đề bài chưa chỉ rõ thang điểm 10.
6. **Số lần thi:** hiện mỗi sinh viên có tối đa một bản ghi điểm cho một mã môn qua `addScore`. Đề bài không quy định chỉ thi một lần; nhóm phải chốt quy tắc trước khi hoàn thiện luồng thi và xem lại bài.
7. **Snapshot:** lưu đủ nội dung, đáp án theo thứ tự đã hiển thị, `correctAnswer`, `selectedAnswer`. Không tạo lại bài đã thi từ ngân hàng hiện tại.
8. **Phân quyền và thời gian:** tầng nghiệp vụ kiểm tra quyền khi thực hiện thao tác và quản lý thời điểm hết hạn; việc ẩn nút hoặc hiển thị đồng hồ ở UI không thay thế kiểm tra nghiệp vụ.
9. **File:** ghi dữ liệu có cấu trúc và dựng lại danh sách/cây khi đọc; không ghi địa chỉ con trỏ để dùng lại ở lần chạy sau.

Tham khảo hợp đồng chi tiết tại [CORE_API.md](https://github.com/0xKurian/DSA-Project-ThiTracNghiem/blob/b92cbd5d65e14f53991b1c4085899472c607db13/docs/CORE_API.md).

## 7. Kế hoạch tiếp theo của nhóm

| Thứ tự | Công việc | Phụ trách |
|---|---|---|
| 1 | Chốt thi lại, thang điểm, đổi/xóa môn đã có điểm, định dạng file và cách trả lỗi cho UI | Cả nhóm, người 3 chủ trì nghiệp vụ |
| 2 | Tích hợp đăng nhập GV/SV, quản lý lớp, sinh viên, môn và câu hỏi | Người 1 + người 3; người 2 hỗ trợ API |
| 3 | Hoàn thiện luồng thi: kiểm tra số câu/số phút, chọn câu không lặp, trộn đáp án, trả lời, hết giờ/nộp bài, chấm điểm | Người 3 điều phối; người 1 giao diện; người 2 hỗ trợ dữ liệu |
| 4 | Xem lại bài thi và bảng điểm lớp/môn, bao gồm “Chưa thi” | Người 1 + người 3 |
| 5 | Lưu/đọc toàn bộ dữ liệu liên quan, gồm snapshot và khôi phục bộ đếm ID | Người 3; người 2 hỗ trợ cấu trúc và kiểm tra |
| 6 | Kiểm thử toàn luồng, kiểm tra lỗi nhập liệu, dữ liệu file sai và quyền truy cập | Cả nhóm |

## 8. Tiêu chí xác nhận hoàn thành đồ án

- Đăng nhập sinh viên hoạt động; tài khoản GV/GV có toàn quyền theo đề.
- Các chức năng b–f dùng được qua UI và không tạo dữ liệu sai.
- Bài thi dùng đúng môn, số câu, thời gian; xử lý nộp bài và hết giờ đúng chính sách.
- Trộn đáp án vẫn chấm đúng và xem lại đúng đề đã hiển thị.
- Bảng điểm phân biệt điểm 0 với chưa thi.
- Khởi động lại ứng dụng vẫn đọc được các danh sách, điểm và chi tiết bài thi; ID mới không trùng dữ liệu cũ.
- Các chính sách chưa có trong đề được nhóm ghi rõ, thống nhất và kiểm tra.

**Trạng thái bàn giao hiện tại:** phần Core của người 2 đã có nền tảng và API phù hợp để tích hợp. Để tuyên bố toàn bộ đồ án đúng và hoàn thành theo đề, nhóm còn cần chứng minh các luồng UI + Business Logic + lưu trữ hoạt động cùng Core.

## 9. Hướng dẫn hai thành viên còn lại làm việc với repo

### 9.1. Quy ước phối hợp đề xuất

| Nội dung | Người 1 — UI | Người 3 — Business Logic |
|---|---|---|
| Thư mục phát triển mới đề xuất | `ui/` | `business/`; có thể tách phần file vào `persistence/` nếu cần |
| Nhánh cho từng công việc | `feat/ui-login`, `feat/ui-exam` | `feat/business-login`, `feat/business-exam` |
| Làm việc với Core | Nhận dữ liệu/kết quả qua tầng nghiệp vụ; thống nhất cách hiển thị lỗi | Gọi các API Core, kiểm tra giá trị trả về, thực hiện đúng ownership |
| Khi cần đổi API Core | Trao đổi với người 2 và người 3 trước khi phụ thuộc vào API mới | Trao đổi với người 2 về chữ ký hàm, điều kiện đầu vào và dữ liệu trả về |
| Khi sửa `Makefile` hoặc điểm khởi chạy ứng dụng | Thống nhất trước với người 3 để tránh hai bản điều phối khác nhau | Thống nhất với người 1 và giữ các target kiểm tra Core hoạt động |
| Kiểm tra trước khi gửi mã | Build, chạy thử màn hình/luồng đã sửa và `make test` | Build, kiểm tra nghiệp vụ/đầu vào sai và `make test` |
| Người đề xuất review PR | Người 3; thêm người 2 nếu liên quan Core | Người 1 nếu đổi giao tiếp UI; người 2 nếu liên quan Core |

Các thư mục `ui/`, `business/`, `persistence/` và nhánh trên là quy ước đề xuất, chưa được tạo trong snapshot hiện tại. Mỗi người nên tạo nhánh riêng cho từng chức năng và mở Pull Request vào `main`. Không làm việc trực tiếp trên cùng một nhánh tính năng.

`examples/core_demo.cpp` là demo Core, không mặc định là điểm khởi chạy ứng dụng hoàn chỉnh. Hai người cần thống nhất điểm khởi chạy và bổ sung target build riêng khi tích hợp.

### 9.2. Chuẩn bị và clone lần đầu

Repo cần quyền truy cập phù hợp. Nếu không clone/push được do quyền, chủ repo phải thêm thành viên làm collaborator. Khi dùng HTTPS, xác thực bằng công cụ quản lý thông tin đăng nhập hoặc token nếu GitHub yêu cầu; không chèn token vào URL hay commit token vào repo.

| Bước | Thao tác/lệnh | Ý nghĩa |
|---|---|---|
| 1 | Cài Git, compiler C++17 và GNU Make | Môi trường build hiện tại được kiểm tra trên Linux với GCC |
| 2 | `git clone https://github.com/0xKurian/DSA-Project-ThiTracNghiem.git` | Tải mã nguồn cùng lịch sử Git về máy |
| 3 | `cd DSA-Project-ThiTracNghiem` | Vào thư mục dự án |
| 4 | `git config user.name "Tên của bạn"` | Đặt tên tác giả commit riêng cho repo này |
| 5 | `git config user.email "email-cua-ban@example.com"` | Dùng email tài khoản GitHub hoặc địa chỉ noreply của bạn |
| 6 | `git status` | Kiểm tra đang ở nhánh nào và có file thay đổi không |
| 7 | `make -j2` rồi `make test` | Xác nhận bản gốc build và kiểm tra được trước khi bắt đầu |
| 8 | Đọc `README.md` và `docs/CORE_API.md` | Hiểu phạm vi Core và hợp đồng API |

Nếu dùng Windows, có thể làm trong WSL với Git, compiler và Make được cài trong WSL. Hướng dẫn lệnh dưới đây dùng shell Linux. Nếu trước đó chỉ tải ZIP từ GitHub, nên clone thành thư mục mới rồi chép phần mã mình đã sửa sang; thư mục ZIP không có lịch sử Git để làm việc theo quy trình này.

### 9.3. Mỗi lần bắt đầu một chức năng mới

Chỉ chuyển nhánh khi `git status` sạch. Nếu còn công việc đang làm, commit trên nhánh hiện tại hoặc lưu bằng `git stash push -u -m "WIP: mo ta cong viec"`; lấy lại bằng `git stash pop` khi quay về đúng nhánh. Stash chỉ lưu trên máy, không phải bản sao trên GitHub.

```bash
git switch main
git pull --ff-only origin main
```

Sau đó tạo nhánh mới từ `main` vừa cập nhật:

| Thành viên | Ví dụ |
|---|---|
| Người 1 | `git switch -c feat/ui-login` |
| Người 3 | `git switch -c feat/business-login` |

Nếu tiếp tục một nhánh đã có thì dùng `git switch ten-nhanh`, không dùng `-c` để tạo lại. `--ff-only` sẽ dừng nếu lịch sử nhánh local đã phân kỳ; khi đó kiểm tra nguyên nhân, không tự reset hoặc force push.

### 9.4. Sửa mã, kiểm tra, commit và push

| Bước | Lệnh | Lưu ý |
|---|---|---|
| Xem thay đổi | `git status` và `git diff` | Kiểm tra đúng file mình định sửa |
| Build | `make -j2` | Khi thêm module mới, phải cập nhật target build để mã mới thực sự được biên dịch |
| Chạy kiểm tra | `make test` | Đây chủ yếu là test Core; UI/nghiệp vụ cần kiểm tra riêng cho chức năng mới |
| Chọn file đưa vào commit | `git add duong-dan-file` | Thay bằng đường dẫn thật; chọn từng file để tránh đưa mã dở dang vào commit |
| Kiểm tra phần đã chọn | `git diff --cached` | Đây là nội dung sẽ được commit |
| Tạo commit | `git commit -m "feat(ui): add login screen"` | Mỗi commit nên tập trung vào một thay đổi dễ hiểu |
| Đẩy nhánh lần đầu | `git push -u origin feat/ui-login` | Dùng đúng tên nhánh của mình |
| Đẩy các commit tiếp theo | `git push` | Dùng sau khi nhánh đã có upstream |

Ví dụ thông điệp commit:

| Loại thay đổi | Ví dụ |
|---|---|
| UI mới | `feat(ui): add exam screen` |
| Nghiệp vụ mới | `feat(business): validate exam settings` |
| Sửa lỗi | `fix(business): reject duplicate student IDs` |
| Kiểm tra | `test(business): cover invalid exam duration` |
| Tài liệu | `docs: describe exam submission flow` |

`commit` lưu một mốc thay đổi trong repo trên máy. `push` đưa các commit lên GitHub. Chỉ sửa file hoặc chỉ commit chưa làm mã xuất hiện trên GitHub.

Không commit thư mục build, executable, mật khẩu/token hoặc dữ liệu sinh viên thật. `.gitignore` hiện có hỗ trợ bỏ qua sản phẩm build thông thường; vẫn cần xem `git status` và phần staged trước khi commit.

### 9.5. Gửi Pull Request và ghép mã

| Bước | Cách thực hiện |
|---|---|
| Mở PR | Sau khi push, vào repo trên GitHub, chọn tạo Pull Request; chọn base là `main`, compare là nhánh của mình |
| Viết mô tả | Nêu chức năng đã làm, file/API ảnh hưởng, cách kiểm tra và phần chưa xong. UI có thể kèm ảnh; nghiệp vụ kèm ca đầu vào/kết quả |
| Nếu chưa sẵn sàng | Mở Draft PR để chia sẻ tiến độ, chưa ghép vào `main` |
| Nhờ review | Chọn người review theo bảng phân công ở mục 9.1 |
| Sửa theo review | Sửa trên chính nhánh đó, kiểm tra, commit rồi `git push`; PR tự nhận commit mới |
| Ghép mã | Sau khi được review và kiểm tra đạt, người nhóm phân công thực hiện merge trên GitHub |
| Cập nhật máy sau merge | `git switch main` rồi `git pull --ff-only origin main` |

Không bắt buộc phải có CI mới kiểm tra được: ghi rõ các lệnh và kết quả đã chạy trong PR. Repo hiện có Makefile và test local; báo cáo này không khẳng định đã thiết lập GitHub Actions hoặc bảo vệ nhánh `main`.

### 9.6. Khi `main` có thay đổi trong lúc đang làm

Trước tiên commit phần công việc cần giữ trên nhánh tính năng, để working tree sạch. Sau đó cập nhật nhánh bằng merge:

```bash
git fetch origin
git merge origin/main
```

Các lệnh này chạy khi đang ở **nhánh tính năng**, không phải chuyển về `main`. Cách merge được dùng trong hướng dẫn để tránh phải viết lại lịch sử nhánh đã chia sẻ.

Nếu có xung đột:

1. Chạy `git status` để xem file bị xung đột.
2. Mở từng file, xử lý các vùng `<<<<<<<`, `=======`, `>>>>>>>` theo kết quả cuối cùng nhóm muốn giữ. Không chọn giữ toàn bộ một phía một cách máy móc.
3. Nếu xung đột liên quan `core/`, hỏi người 2 trước khi đổi hành vi API; nếu liên quan giao tiếp UI/nghiệp vụ, hai người thống nhất cùng nhau.
4. Chạy build/test, sau đó `git add duong-dan-file-da-sua` cho từng file và `git commit` để kết thúc merge.
5. Chạy `git push` để cập nhật PR.

Nếu chưa biết cách giải quyết và muốn hủy lần merge đang dở, dùng `git merge --abort`. Không dùng `git push --force` hoặc `git reset --hard` như cách xử lý mặc định vì có thể mất công việc hoặc ghi đè lịch sử của người khác.

### 9.7. Checklist ngắn trước khi bàn giao chức năng

- [ ] Đang làm trên nhánh riêng, đã cập nhật thay đổi cần thiết từ `main`.
- [ ] Mã mới được đưa vào target build và có thể biên dịch.
- [ ] `make test` đạt; chức năng UI/nghiệp vụ mới đã được kiểm tra riêng.
- [ ] Dùng đúng hợp đồng Core; xử lý `nullptr`/`false`, ownership và ID.
- [ ] Không đưa sản phẩm build, thông tin xác thực hoặc dữ liệu thật vào commit.
- [ ] Đã xem `git diff --cached`, commit với mô tả rõ và push nhánh.
- [ ] PR ghi cách kiểm tra, ảnh hưởng tới thành viên khác và người cần review.
