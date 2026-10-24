# BÁO CÁO CHI TIẾT CÁC THAY ĐỔI TRONG DỰ ÁN
**Đồ án Cấu trúc Dữ liệu & Giải thuật: Hệ thống Thi Trắc Nghiệm C++**  
*Ngày cập nhật: 19/10/2026*

---

## I. TỔNG QUAN PHẠM VI & NGUYÊN TẮC THỰC HIỆN

1. **Tuân thủ phân định trách nhiệm**: Không can thiệp vào thư mục `ui/` (đang thuộc quyền quản lý của nhóm UI tại nhánh `feat/uxui`, commit `b4b457d`).
2. **Bảo toàn dữ liệu**: Giữ nguyên vẹn cơ sở dữ liệu thật trong `data/subjects.txt` và `data/classes.txt`; toàn bộ kiểm thử sử dụng fixture và file tạm riêng.
3. **Bảo toàn giao diện lập trình**: Giữ nguyên chữ ký của tất cả các workflow hiện có.
4. **Thứ tự thực hiện chuẩn xác**: Core → Business → FileStorage → Test & Tài liệu.

---

## II. BẢNG TỔNG HỢP CÁC FILE ĐƯỢC THÊM, SỬA, XÓA

| STT | Tệp tin | Hành động | Nội dung thay đổi chính |
| :---: | :--- | :---: | :--- |
| 1 | `core/Structures.h` | **SỬA / THÊM** | Thêm 2 trường `examQuestionCount` và `examMinutes` vào `struct Subject`. |
| 2 | `core/SubjectBST.cpp` | **SỬA** | Khởi tạo 2 trường trên bằng `0` trong `createSubject()`. Giữ nguyên mọi hành vi khác của Core. |
| 3 | `business/SubjectService.h` | **THÊM** | Khai báo hàm workflow mới: `setExamConfigWorkflow()`. |
| 4 | `business/SubjectService.cpp` | **THÊM** | Thêm include `QuestionList.h` và cài đặt hàm `setExamConfigWorkflow()`. |
| 5 | `business/ClassService.cpp` | **SỬA / XÓA** | Xóa bỏ đoạn kiểm tra `classroom->studentList != nullptr` trong `deleteClassWorkflow()`. |
| 6 | `business/FileStorage.cpp` | **SỬA** | Cập nhật định dạng lưu file môn học 5 cột; đọc tương thích ngược cả file 3 cột và 5 cột. |
| 7 | `Makefile` | **SỬA** | Cập nhật rule test tự động liên kết cả Core và Business; hỗ trợ lệnh chạy test đa nền tảng. |
| 8 | `docs/CORE_API.md` | **SỬA / THÊM** | Bổ sung mục tài liệu hợp đồng "Cấu hình thi trên Subject", định nghĩa giá trị mặc định `0`. |
| 9 | `tests/test_subject_exam_config.cpp` | **THÊM MỚI** | Bộ kiểm thử toàn diện cấu hình thi, ràng buộc thời gian/câu hỏi và lưu/đọc file 5 cột & 3 cột. |
| 10 | `tests/test_class_delete_with_students.cpp` | **THÊM MỚI** | Bộ kiểm thử xóa lớp có sinh viên (kèm điểm thi), dồn mảng và giải phóng bộ nhớ an toàn. |

---

## III. CHI TIẾT TỪNG NHÓM YÊU CẦU

### 1. Lưu cấu hình thi trên môn học (`Subject`)

#### A. Tầng Core
* **Tệp `core/Structures.h`**:
  - **Đã thêm**:
    ```cpp
    struct Subject {
        char id[SUBJECT_ID_LEN];
        char name[SUBJECT_NAME_LEN];

        Question* questionList;

        int examQuestionCount; // 0 = chưa cấu hình
        int examMinutes;       // 0 = chưa cấu hình

        Subject* left;
        Subject* right;
    };
    ```
* **Tệp `core/SubjectBST.cpp`**:
  - **Đã thêm vào hàm `createSubject()`**:
    ```cpp
    subject->examQuestionCount = 0;
    subject->examMinutes = 0;
    ```
  - **Bảo lưu**: Không can thiệp bất kỳ hàm nào khác trong Core để giữ vững tính ổn định.

#### B. Tầng Business
* **Tệp `business/SubjectService.h`**:
  - **Đã thêm khai báo**:
    ```cpp
    BusinessResult setExamConfigWorkflow(
        Subject*& root,
        const char* subjectId,
        int questionCount,
        int minutes
    );
    ```
* **Tệp `business/SubjectService.cpp`**:
  - **Đã thêm logic xử lý**:
    1. Kiểm tra nếu `root == nullptr`, `subjectId == nullptr`, hoặc chuỗi sau khi `trim` rỗng/chỉ chứa khoảng trắng, hoặc không tìm thấy môn qua `findSubject`: trả về `ERR_NOT_FOUND` (đảm bảo tuyệt đối không crash).
    2. Kiểm tra `questionCount` trước: phải nằm trong đoạn `1..countQuestions(subject->questionList)`. Nếu sai, trả về `ERR_CONSTRAINT_VIOLATION`.
    3. Kiểm tra `minutes` sau: phải nằm trong đoạn `1..180`. Nếu sai, trả về `ERR_CONSTRAINT_VIOLATION`.
    4. Khi cả hai giá trị hợp lệ mới gán:
       ```cpp
       subject->examQuestionCount = questionCount;
       subject->examMinutes = minutes;
       return SUCCESS;
       ```
    5. Nếu có lỗi ở bất kỳ bước nào, hai giá trị cấu hình cũ của môn học được giữ nguyên 100%. Workflow chỉ thao tác trên bộ nhớ RAM, không tự ý ghi file.

#### C. Tầng FileStorage
* **Tệp `business/FileStorage.cpp`**:
  - **Ghi file (`writeSubjectsRecursive`)**:
    - *Trước đây*: `MAMH|TENMH|SO_CAU_HOI`
    - *Hiện tại*: `MAMH|TENMH|SO_CAU_HOI|EXAM_QUESTION_COUNT|EXAM_MINUTES` (luôn ghi đủ 5 cột).
    - Giữ nguyên dòng tổng số môn ở đầu file và thứ tự các câu hỏi ngay sau tiêu đề môn.
  - **Đọc file (`loadSubjectsFromFile`)**:
    - Khi gặp dòng có 5 cột: Parse nạp `examQuestionCount` và `examMinutes` vào node `Subject`.
    - Khi gặp file định dạng cũ (3 cột hoặc 2 cột): Tự động đặt `examQuestionCount = 0` và `examMinutes = 0`, không bắt buộc người dùng chuyển đổi file.
    - Không làm lệch vị trí đọc các dòng câu hỏi bên dưới.

---

### 2. Cho phép xóa lớp đang có sinh viên

* **Tệp `business/ClassService.cpp`**:
  - **Đã xóa bỏ**:
    ```cpp
    // ĐÃ XÓA: Khong duoc xoa lop neu dang co sinh vien
    // if (classroom->studentList != nullptr) {
    //     return ERR_CONSTRAINT_VIOLATION;
    // }
    ```
  - **Hành vi mới**:
    - Giữ nguyên chữ ký `deleteClassWorkflow(Class* classes[], int& classCount, const char id[])`.
    - Kiểm tra `id == nullptr` hoặc không tìm thấy lớp: trả về `ERR_NOT_FOUND`.
    - Khi tìm thấy lớp, chuyển giao trực tiếp cho hàm `deleteClass()` của Core.
    - Hàm `deleteClass()` của Core sẽ giải phóng chuỗi danh sách liên kết sinh viên (`Student`), điểm thi (`Score`), chi tiết bài thi (`ExamDetail`), thu hồi ô nhớ của lớp và dồn mảng con trỏ `classes[]` về trước.

---

### 3. Cập nhật hệ thống Makefile & Test Runner

* **Tệp `Makefile`**:
  - **Đã cập nhật**:
    - Rule biên dịch test: `$(BUILD_DIR)/tests/%$(BIN_EXT): tests/%.cpp $(CORE_OBJ) $(BUSINESS_OBJ)` giúp các file test có thể gọi trực tiếp các hàm nghiệp vụ (Workflow) và lưu trữ (FileStorage) mà không cần kéo `main.cpp`.
    - Thiết lập biến `RUN_TESTS` linh hoạt để chạy vòng lặp test trơn tru trên cả môi trường Windows CMD (`for %%B in (...)`) và Linux (`set -e; for ...`).

---

### 4. Bổ sung các bộ kiểm thử mới (`tests/`)

1. **`tests/test_subject_exam_config.cpp`**:
   - `createSubject`: Khởi tạo cả 2 trường bằng 0.
   - Thiết lập hợp lệ: Môn có 20 câu, đặt 10 câu / 15 phút trả `SUCCESS` và lưu đúng giá trị.
   - Ràng buộc câu hỏi: Đặt `0`, `-5`, `21` câu trả `ERR_CONSTRAINT_VIOLATION`, giữ nguyên cấu hình cũ (10 câu, 15 phút).
   - Ràng buộc thời gian: Đặt `0`, `181` phút trả `ERR_CONSTRAINT_VIOLATION`, giữ nguyên cấu hình cũ.
   - Tìm kiếm: ID không tồn tại, `nullptr`, chuỗi rỗng `""`, chuỗi khoảng trắng `"   "`, `root == nullptr` đều trả `ERR_NOT_FOUND` an toàn.
   - Ghi và đọc lại file 5 cột: Cấu hình và toàn bộ câu hỏi được bảo toàn trọn vẹn.
   - Tương thích ngược: Đọc fixture 3 cột tự gán cấu hình = 0, lưu lại thành 5 cột và đọc lại bình thường.

2. **`tests/test_class_delete_with_students.cpp`**:
   - Tạo lớp có 2 sinh viên (kèm điểm thi chi tiết) và lớp thứ hai có 1 sinh viên.
   - Xóa lớp có sinh viên: trả về `SUCCESS`, giảm `classCount`.
   - Lớp bị xóa không còn tìm thấy trong mảng.
   - Lớp còn lại được dồn vị trí chính xác, sĩ số và danh sách sinh viên của lớp còn lại nguyên vẹn.
   - Xóa lớp không tồn tại hoặc danh sách rỗng trả về `ERR_NOT_FOUND`.

---

### 5. Cập nhật tài liệu hợp đồng (`docs/CORE_API.md`)

* **Bổ sung mục `## Cấu hình thi trên Subject`**:
  - Giải thích mục đích của hai trường `examQuestionCount` và `examMinutes`.
  - Quy định rõ giá trị mặc định là `0` và mang ý nghĩa: **Môn học chưa được cấu hình thi** (chưa sẵn sàng để sinh viên thi).
  - Cảnh báo về hành vi của Core khi đổi mã môn học.

---

## IV. ĐÁNH GIÁ RỦI RO: THAO TÁC ĐỔI MÃ MÔN HỌC (`editSubject`)

* **Hiện tượng phát hiện**: Trong `core/SubjectBST.cpp:L240-L246`, khi đổi mã môn (`oldId != newId`), Core tạo một node mới bằng `createSubject(newId, newName)`. Hàm này sẽ khởi tạo `examQuestionCount = 0` và `examMinutes = 0`. Nó chỉ bàn giao `questionList` sang node mới mà **chưa sao chép lại hai trường cấu hình thi**.
* **Hậu quả**: Khi đổi mã môn, cấu hình số câu và thời gian thi của môn học cũ bị đặt lại về `0`.
* **Đề xuất hướng xử lý cho nhóm**:
  - *Giải pháp 1 (ở Core)*: Bổ sung 2 dòng sao chép trong `editSubject()` ở `core/SubjectBST.cpp`:
    ```cpp
    newSubject->examQuestionCount = subject->examQuestionCount;
    newSubject->examMinutes = subject->examMinutes;
    ```
  - *Giải pháp 2 (ở Business)*: Trong `editSubjectWorkflow()`, lưu tạm 2 giá trị này của `oldId`, sau khi `editSubject()` thành công thì gán lại vào node `newId`.
  *(Hiện tại mã nguồn tuân thủ đúng yêu cầu: không tự ý mở rộng phạm vi sửa Core/workflow cũ, hành vi được ghi nhận rõ ràng vào tài liệu).*

---

## V. KẾT QUẢ KIỂM THỬ THỰC TẾ

Toàn bộ các test (cũ và mới) cùng bản build ứng dụng đều vượt qua 100%:

```text
Lệnh chạy: make clean; make test; make app

Running build\tests\test_class_delete_with_students.exe 
=== Running test_class_delete_with_students ===
PASS test_class_delete_with_students

Running build\tests\test_core_contracts.exe 
PASS subject rejects overlong ID
PASS class rejects overlong ID
PASS student rejects overlong ID
PASS score rejects overlong ID
PASS empty IDs rejected
PASS maximum valid ID lengths accepted
PASS rename rejects empty ID
PASS addScore rejects empty ID
PASS add class
PASS global add rejects unregistered class without mutation
PASS global add accepts registered class
PASS answered snapshot cannot be reshuffled
PASS shuffle preserves correct answer and original question

Running build\tests\test_core_integration.exe 
ALL CORE TESTS PASSED

Running build\tests\test_duplicate_subject.exe 
Running build\tests\test_global_student_id.exe 
Running build\tests\test_score_id_length.exe 
Running build\tests\test_student_id_length.exe 
Running build\tests\test_subject_edit.exe 

Running build\tests\test_subject_exam_config.exe 
=== Running test_subject_exam_config ===
PASS test_subject_exam_config

build/tests/test_core_contracts.exe null
PASS null identifiers rejected

-> Build app.exe: HOÀN TẤT (0 lỗi, 0 cảnh báo)
```

---

## VI. THÔNG ĐIỆP BÀN GIAO CHO NHÓM UI (`feat/uxui`)

> **Kính gửi nhóm UI:**
> 
> Đối tượng `Subject` hiện đã có hai trường chính thức:
> - `int examQuestionCount;` (0 = chưa cấu hình thi)
> - `int examMinutes;`       (0 = chưa cấu hình thi)
> 
> Nhóm UI có thể đọc trực tiếp hai trường này từ con trỏ `Subject*` trên các màn hình thi (`GuiExamPrepare`, phòng thi) thay vì dùng vector lưu tạm. Để lưu cấu hình từ hộp thoại quản trị, hãy gọi workflow `setExamConfigWorkflow(root, subjectId, questionCount, minutes)` rồi lưu ra file qua `saveAllData()`.
