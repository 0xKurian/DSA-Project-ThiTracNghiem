# 📘 TÀI LIỆU HƯỚNG DẪN TẦNG NGHIỆP VỤ (`business/`)

## Hệ Thống Thi Trắc Nghiệm C++ — Data Structures & Algorithms

---

## MỤC LỤC

1. [Tổng Quan Kiến Trúc](#1-tổng-quan-kiến-trúc)
2. [Sơ Đồ Phụ Thuộc Giữa Các File](#2-sơ-đồ-phụ-thuộc-giữa-các-file)
3. [BusinessErrors.h — Mã Lỗi Chuẩn](#3-businesserrorsh--mã-lỗi-chuẩn)
4. [Validation.h / Validation.cpp — Kiểm Tra & Chuẩn Hóa Dữ Liệu](#4-validationh--validationcpp--kiểm-tra--chuẩn-hóa-dữ-liệu)
5. [SubjectService.h / SubjectService.cpp — Quản Lý Môn Học](#5-subjectserviceh--subjectservicecpp--quản-lý-môn-học)
6. [QuestionService.h / QuestionService.cpp — Quản Lý Câu Hỏi](#6-questionserviceh--questionservicecpp--quản-lý-câu-hỏi)
7. [ClassService.h / ClassService.cpp — Quản Lý Lớp Học](#7-classserviceh--classservicecpp--quản-lý-lớp-học)
8. [StudentService.h / StudentService.cpp — Quản Lý Sinh Viên](#8-studentserviceh--studentservicecpp--quản-lý-sinh-viên)
9. [AuthService.h / AuthService.cpp — Đăng Nhập & Phân Quyền](#9-authserviceh--authservicecpp--đăng-nhập--phân-quyền)
10. [ExamService.h / ExamService.cpp — Động Cơ Thi Trắc Nghiệm](#10-examserviceh--examservicecpp--động-cơ-thi-trắc-nghiệm)
11. [ReportService.h / ReportService.cpp — Báo Cáo & In Bảng Điểm](#11-reportserviceh--reportservicecpp--báo-cáo--in-bảng-điểm)
12. [FileStorage.h / FileStorage.cpp — Lưu Trữ & Nạp Dữ Liệu File](#12-filestorageh--filestoragecpp--lưu-trữ--nạp-dữ-liệu-file)
13. [Bảng Tổng Hợp Tất Cả Hàm](#13-bảng-tổng-hợp-tất-cả-hàm)
14. [Quy Tắc Thiết Kế & Ràng Buộc](#14-quy-tắc-thiết-kế--ràng-buộc)
15. [Lưu Ý Quan Trọng Cho Tầng UI](#15-lưu-ý-quan-trọng-cho-tầng-ui)

---

## 1. Tổng Quan Kiến Trúc

Tầng `business/` là **lớp nghiệp vụ (Business Logic Layer)** của hệ thống, đóng vai trò trung gian giữa **tầng giao diện (`ui/`)** và **tầng cấu trúc dữ liệu (`core/`)**. Mọi thao tác CRUD (Thêm, Sửa, Xóa, Tìm kiếm), đăng nhập, thi trắc nghiệm, báo cáo và lưu trữ file đều đi qua tầng này.

```
┌──────────────────────────────────────────────────────┐
│              UI Layer  (ui/ConsoleUI.cpp)             │
│          Giao diện người dùng tương tác               │
└───────────────────────┬──────────────────────────────┘
                        │ gọi hàm Workflow
                        ▼
┌──────────────────────────────────────────────────────┐
│           BUSINESS Layer  (business/*.cpp)            │
│  ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌────────┐  │
│  │Validation│ │ Subject  │ │ Question │ │ Class  │  │
│  │          │ │ Service  │ │ Service  │ │Service │  │
│  └──────────┘ └──────────┘ └──────────┘ └────────┘  │
│  ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌────────┐  │
│  │ Student  │ │   Auth   │ │   Exam   │ │ Report │  │
│  │ Service  │ │ Service  │ │ Service  │ │Service │  │
│  └──────────┘ └──────────┘ └──────────┘ └────────┘  │
│  ┌──────────┐ ┌──────────────────────────────────┐   │
│  │  File    │ │      BusinessErrors.h             │   │
│  │ Storage  │ │   Mã lỗi chuẩn cho toàn tầng     │   │
│  └──────────┘ └──────────────────────────────────┘   │
└───────────────────────┬──────────────────────────────┘
                        │ gọi hàm CRUD trên cấu trúc
                        ▼
┌──────────────────────────────────────────────────────┐
│            CORE Layer  (core/*.cpp)                   │
│  SubjectBST, QuestionList, StudentList, ClassArray,  │
│  ScoreList, StringUtils, Structures, Constants       │
└──────────────────────────────────────────────────────┘
```

**Nguyên tắc chung:** Mọi hàm trong tầng `business/` đều:
1. **Validate** dữ liệu đầu vào trước khi xử lý.
2. **Chuẩn hóa** chuỗi (trim khoảng trắng, chuyển hoa mã ID).
3. **Kiểm tra ràng buộc nghiệp vụ** (trùng lặp, toàn vẹn tham chiếu).
4. **Trả về mã lỗi `BusinessResult`** thay vì crash chương trình.

---

## 2. Sơ Đồ Phụ Thuộc Giữa Các File

```
BusinessErrors.h  ◄─── Tất cả Service đều include
       │
Validation.h/cpp  ◄─── SubjectService, QuestionService, ClassService,
       │               StudentService, AuthService, ExamService,
       │               ReportService, FileStorage
       │
       ▼
┌─────────────────────────────────────────────────┐
│  SubjectService ──► core/SubjectBST             │
│  QuestionService ──► core/QuestionList           │
│  ClassService ──► core/ClassArray                │
│  StudentService ──► core/StudentList             │
│  AuthService ──► core/ClassArray (findStudentGlobal) │
│  ExamService ──► core/SubjectBST, QuestionList, ScoreList │
│  ReportService ──► core/ClassArray, SubjectBST, ScoreList │
│  FileStorage ──► Tất cả core modules            │
└─────────────────────────────────────────────────┘
```

---

## 3. BusinessErrors.h — Mã Lỗi Chuẩn

**Đường dẫn:** `business/BusinessErrors.h`

File này định nghĩa enum `BusinessResult` — hệ thống mã lỗi chuẩn được sử dụng **xuyên suốt toàn bộ tầng nghiệp vụ**.

### Bảng Mã Lỗi

| Mã Lỗi | Giá Trị | Ý Nghĩa Tiếng Việt | Khi Nào Xảy Ra |
|---|:---:|---|---|
| `SUCCESS` | 0 | Thành công | Thao tác hoàn tất không lỗi |
| `ERR_EMPTY_FIELD` | 1 | Trường dữ liệu trống | Chuỗi rỗng `""` hoặc chỉ toàn khoảng trắng `"   "` |
| `ERR_BUFFER_OVERFLOW` | 2 | Vượt quá giới hạn độ dài | Chuỗi dài hơn hằng số cho phép (vd: MAMH > 15 ký tự) |
| `ERR_INVALID_FORMAT` | 3 | Định dạng không hợp lệ | Đáp án không phải A/B/C/D, phái không phải NAM/NU, sai mật khẩu |
| `ERR_NOT_FOUND` | 4 | Không tìm thấy đối tượng | Tìm môn học, lớp, sinh viên nhưng không tồn tại |
| `ERR_DUPLICATE` | 5 | Dữ liệu trùng lặp | Thêm môn học/sinh viên có mã đã tồn tại |
| `ERR_LIST_FULL` | 6 | Danh sách đầy | Mảng lớp học đạt `MAX_CLASS = 10000` |
| `ERR_CONSTRAINT_VIOLATION` | 7 | Vi phạm ràng buộc toàn vẹn | Xóa môn đã có điểm thi, xóa lớp đang có sinh viên, sinh viên thi lại môn đã thi |
| `ERR_UNKNOWN` | 8 | Lỗi không xác định | Lỗi cấp phát bộ nhớ thất bại hoặc lỗi bất ngờ |

### Hàm Tiện Ích

```cpp
const char* getErrorMessage(BusinessResult result);
```

Trả về chuỗi thông báo lỗi tiếng Việt (không dấu) tương ứng với mã lỗi. Dùng để hiển thị trực tiếp lên giao diện Console.

**Ví dụ sử dụng:**

```cpp
BusinessResult result = addSubjectWorkflow(root, "CTDL", "Cau truc du lieu");
if (result != SUCCESS) {
    cout << "Loi: " << getErrorMessage(result) << endl;
}
```

---

## 4. Validation.h / Validation.cpp — Kiểm Tra & Chuẩn Hóa Dữ Liệu

**Đường dẫn:** `business/Validation.h` + `business/Validation.cpp`

Module này cung cấp hai nhóm chức năng:

### 4.1. Nhóm Chuẩn Hóa Chuỗi (String Sanitization)

| Hàm | Chữ Ký | Mô Tả |
|---|---|---|
| `trimString` | `void trimString(char str[])` | Cắt bỏ khoảng trắng đầu và cuối chuỗi. Vd: `"  CTDL  "` → `"CTDL"` |
| `toUpperString` | `void toUpperString(char str[])` | Chuyển toàn bộ ký tự thường thành in hoa. Vd: `"ctdl"` → `"CTDL"` |
| `containsWhitespace` | `bool containsWhitespace(const char str[])` | Kiểm tra chuỗi có chứa khoảng trắng/tab/xuống dòng hay không |

**Hàm nội bộ (static):**
- `isSpace(char c)` — Kiểm tra ký tự có phải khoảng trắng (` `, `\t`, `\n`, `\r`).
- `isAllWhitespace(const char str[])` — Kiểm tra chuỗi chỉ gồm toàn khoảng trắng. **Đây là hàm quan trọng** để chặn trường hợp `"   "` (chuỗi 3 dấu cách) vượt qua kiểm tra `length > 0` thông thường.

### 4.2. Nhóm Validate Từng Trường Dữ Liệu

Mỗi hàm kiểm tra một trường cụ thể, trả về `BusinessResult`:

| Hàm | Trường Kiểm Tra | Hằng Số Giới Hạn | Logic Kiểm Tra |
|---|---|---|---|
| `validateSubjectId(id)` | Mã môn học (MAMH) | `SUBJECT_ID_LEN = 16` | Không rỗng/whitespace + độ dài < 16 |
| `validateSubjectName(name)` | Tên môn học (TENMH) | `SUBJECT_NAME_LEN = 100` | Không rỗng/whitespace + độ dài < 100 |
| `validateClassId(id)` | Mã lớp (MALOP) | `CLASS_ID_LEN = 20` | Không rỗng/whitespace + độ dài < 20 |
| `validateClassName(name)` | Tên lớp (TENLOP) | `CLASS_NAME_LEN = 100` | Không rỗng/whitespace + độ dài < 100 |
| `validateStudentId(id)` | Mã sinh viên (MASV) | `STUDENT_ID_LEN = 20` | Không rỗng/whitespace + độ dài < 20 |
| `validateStudentName(ho, ten)` | Họ và Tên | `HO_LEN=50`, `TEN_LEN=30` | Cả hai không rỗng + đúng giới hạn |
| `validateGender(gender)` | Phái (Giới tính) | `GENDER_LEN = 10` | Phải là `"NAM"` hoặc `"NU"` (tự trim & upper) |
| `validatePassword(pass)` | Mật khẩu | `PASSWORD_LEN = 50` | Không rỗng + không chứa khoảng trắng + < 50 ký tự |
| `validateQuestionContent(content)` | Nội dung câu hỏi | `QUESTION_CONTENT_LEN = 500` | Không rỗng/whitespace + < 500 ký tự |
| `validateAnswerContent(ans)` | Nội dung đáp án A/B/C/D | `ANSWER_CONTENT_LEN = 200` | Không rỗng/whitespace + < 200 ký tự |
| `validateCorrectAnswer(correct)` | Đáp án đúng | 1 ký tự | Phải là `'A'`, `'B'`, `'C'` hoặc `'D'` (tự chuyển hoa nếu `a-d`) |

**Lưu ý đặc biệt:** `validateGender()` nhận tham số `char gender[]` (không phải `const`) vì nó sẽ **tự động chuẩn hóa** giá trị thành `"Nam"` hoặc `"Nu"` trước khi trả về. Tương tự, `validateCorrectAnswer()` nhận `char&` để tự chuyển hoa.

---

## 5. SubjectService.h / SubjectService.cpp — Quản Lý Môn Học

**Đường dẫn:** `business/SubjectService.h` + `business/SubjectService.cpp`

Cung cấp các workflow CRUD cho thực thể **Môn Học (Subject)**, được lưu trữ trên cây **BST (Binary Search Tree)**.

### Danh Sách Hàm

#### 5.1. `isSubjectUsedInScores()`

```cpp
bool isSubjectUsedInScores(
    Class* classes[],   // Mảng con trỏ tất cả lớp học
    int classCount,     // Số lượng lớp hiện tại
    const char subjectId[] // Mã môn cần kiểm tra
);
```

**Mục đích:** Kiểm tra xem môn học có đang được tham chiếu bởi bất kỳ bản ghi điểm (`Score`) nào của bất kỳ sinh viên nào trong toàn hệ thống hay không.

**Luồng xử lý:**
1. Chuẩn hóa `subjectId` (trim + upper).
2. Duyệt tất cả lớp → tất cả sinh viên → tìm `findScore(student, subjectId)`.
3. Nếu tìm thấy bất kỳ → trả `true` (môn đang được sử dụng).

**Dùng ở đâu:** Được gọi bởi `editSubjectWorkflow()` và `deleteSubjectWorkflow()` để bảo vệ ràng buộc toàn vẹn dữ liệu.

---

#### 5.2. `addSubjectWorkflow()`

```cpp
BusinessResult addSubjectWorkflow(
    Subject*& root,     // Con trỏ gốc cây BST (tham chiếu vì có thể thay đổi gốc)
    const char id[],    // Mã môn học mới
    const char name[]   // Tên môn học mới
);
```

**Luồng xử lý:**
1. Validate `id` → `validateSubjectId()`.
2. Validate `name` → `validateSubjectName()`.
3. Chuẩn hóa: `trimString()` + `toUpperString()` cho ID.
4. Kiểm tra trùng: `findSubject(root, cleanId)`. Nếu đã có → `ERR_DUPLICATE`.
5. Tạo node: `createSubject(cleanId, cleanName)`.
6. Chèn vào BST: `root = insertSubject(root, newSubject)`.

**Trả về:** `SUCCESS` hoặc `ERR_EMPTY_FIELD` / `ERR_BUFFER_OVERFLOW` / `ERR_DUPLICATE` / `ERR_UNKNOWN`.

---

#### 5.3. `editSubjectWorkflow()`

```cpp
BusinessResult editSubjectWorkflow(
    Subject*& root,
    Class* classes[],
    int classCount,
    const char oldId[],   // Mã môn cũ
    const char newId[],   // Mã môn mới (có thể giống cũ)
    const char newName[]  // Tên mới
);
```

**Luồng xử lý:**
1. Tìm môn cũ trên BST. Không tìm thấy → `ERR_NOT_FOUND`.
2. Validate `newId` và `newName`.
3. Nếu đổi mã (`oldId ≠ newId`):
   - Kiểm tra ràng buộc: nếu môn cũ đã có điểm thi → `ERR_CONSTRAINT_VIOLATION`.
   - Kiểm tra trùng: nếu `newId` đã tồn tại → `ERR_DUPLICATE`.
4. Gọi `editSubject()` (xóa node cũ, chèn node mới trên BST để giữ tính chất BST).

**Quan trọng:** Khi đổi mã `MAMH`, hàm `editSubject()` ở tầng `core` sẽ **xóa node cũ và chèn node mới** vì vị trí trên cây BST phụ thuộc vào giá trị khóa.

---

#### 5.4. `deleteSubjectWorkflow()`

```cpp
BusinessResult deleteSubjectWorkflow(
    Subject*& root,
    Class* classes[],
    int classCount,
    const char id[]
);
```

**Luồng xử lý:**
1. Chuẩn hóa và tìm môn học trên BST.
2. **Kiểm tra ràng buộc toàn vẹn:** Nếu bất kỳ sinh viên nào đã thi môn này → `ERR_CONSTRAINT_VIOLATION` (chặn xóa).
3. Gọi `deleteSubject(root, cleanId)` — tự động giải phóng toàn bộ `questionList` của môn trước khi xóa node BST.

---

## 6. QuestionService.h / QuestionService.cpp — Quản Lý Câu Hỏi

**Đường dẫn:** `business/QuestionService.h` + `business/QuestionService.cpp`

Quản lý câu hỏi trắc nghiệm bên trong **danh sách liên kết đơn (Singly Linked List)** của mỗi môn học.

### Danh Sách Hàm

#### 6.1. `addQuestionWorkflow()`

```cpp
BusinessResult addQuestionWorkflow(
    Subject* root,            // Gốc cây BST môn học
    const char subjectId[],   // Mã môn để thêm câu hỏi vào
    const char content[],     // Nội dung câu hỏi
    const char answerA[],     // Đáp án A
    const char answerB[],     // Đáp án B
    const char answerC[],     // Đáp án C
    const char answerD[],     // Đáp án D
    char correctAnswer,       // Đáp án đúng ('A'/'B'/'C'/'D')
    int& generatedId          // [OUT] ID tự động được sinh ra
);
```

**Luồng xử lý:**
1. Tìm môn học theo `subjectId` trên BST.
2. Validate tất cả 7 trường (content, 4 đáp án, đáp án đúng).
3. Sinh ID tự động: `generatedId = generateQuestionId()` (counter toàn cục tăng dần).
4. Tạo node `Question` → chèn vào cuối `subject->questionList`.

**Đặc biệt:** ID câu hỏi **không bao giờ nhập tay**, luôn được sinh tự động bởi hàm `generateQuestionId()` trong `core/QuestionList.cpp`. Counter này được đồng bộ lại khi nạp dữ liệu từ file bằng `updateQuestionIdCounter()`.

---

#### 6.2. `editQuestionWorkflow()`

```cpp
BusinessResult editQuestionWorkflow(
    Subject* root,
    const char subjectId[],
    int questionId,           // ID câu hỏi cần sửa
    const char content[],
    const char answerA[],
    const char answerB[],
    const char answerC[],
    const char answerD[],
    char correctAnswer
);
```

**Luồng xử lý:**
1. Tìm môn học → tìm câu hỏi theo ID trong linked list.
2. Validate toàn bộ nội dung mới.
3. Ghi đè trực tiếp nội dung vào node `Question` hiện tại (không cần xóa + tạo mới).

**Lưu ý quan trọng:** Sửa câu hỏi gốc **KHÔNG ảnh hưởng** đến bài thi đã được nộp, vì bài thi lưu `ExamDetail` snapshot độc lập.

---

#### 6.3. `deleteQuestionWorkflow()`

```cpp
BusinessResult deleteQuestionWorkflow(
    Subject* root,
    const char subjectId[],
    int questionId
);
```

**Luồng xử lý:**
1. Tìm môn học → gọi `deleteQuestion(subject->questionList, questionId)`.
2. Hàm `deleteQuestion()` ở tầng `core` sẽ nối lại linked list và `delete` node.

---

## 7. ClassService.h / ClassService.cpp — Quản Lý Lớp Học

**Đường dẫn:** `business/ClassService.h` + `business/ClassService.cpp`

Quản lý lớp học trên **mảng con trỏ (Pointer Array)** với giới hạn `MAX_CLASS = 10000`.

### Danh Sách Hàm

#### 7.1. `addClassWorkflow()`

```cpp
BusinessResult addClassWorkflow(
    Class* classes[],   // Mảng con trỏ lớp học
    int& classCount,    // Số lượng lớp hiện tại (tham chiếu, sẽ tăng +1)
    const char id[],    // Mã lớp
    const char name[]   // Tên lớp
);
```

**Luồng xử lý:**
1. Kiểm tra `classCount >= MAX_CLASS` → `ERR_LIST_FULL`.
2. Validate ID và tên lớp.
3. Chuẩn hóa (trim + upper cho ID).
4. Kiểm tra trùng: `findClass()`. Nếu đã có → `ERR_DUPLICATE`.
5. Tạo `Class` mới → thêm vào mảng: `addClass(classes, classCount, newClass)`.

---

#### 7.2. `deleteClassWorkflow()`

```cpp
BusinessResult deleteClassWorkflow(
    Class* classes[],
    int& classCount,
    const char id[]
);
```

**Ràng buộc nghiệp vụ quan trọng:**
- **Không được xóa lớp nếu lớp đang có sinh viên** (`classroom->studentList != nullptr` → `ERR_CONSTRAINT_VIOLATION`).
- Phải xóa hết sinh viên trong lớp trước khi xóa lớp.

> **📌 Ghi Chú Thiết Kế — Không Có `editClassWorkflow()`:**  
> Tầng `business` **không cung cấp** hàm sửa lớp học vì:
> 1. `MALOP` (khóa chính) **không được phép thay đổi** sau khi tạo (tương tự `MASV`).
> 2. Nếu chỉ cần sửa `TENLOP`, tầng UI có thể gọi trực tiếp `findClass()` từ `core/ClassArray.h` và ghi đè trường `name` — không cần workflow nghiệp vụ phức tạp vì không có ràng buộc toàn vẹn nào liên quan đến tên lớp.

---

## 8. StudentService.h / StudentService.cpp — Quản Lý Sinh Viên

**Đường dẫn:** `business/StudentService.h` + `business/StudentService.cpp`

Quản lý sinh viên trong **danh sách liên kết đơn** (`studentList`) của mỗi lớp.

### Danh Sách Hàm

#### 8.1. `addStudentWorkflow()`

```cpp
BusinessResult addStudentWorkflow(
    Class* classes[],
    int classCount,
    const char classId[],    // Mã lớp để thêm SV vào
    const char id[],         // Mã sinh viên (MASV)
    const char ho[],         // Họ
    const char ten[],        // Tên
    const char gender[],     // Phái ("Nam" hoặc "Nu")
    const char password[]    // Mật khẩu đăng nhập
);
```

**Luồng xử lý:**
1. Tìm lớp theo `classId`.
2. Validate tất cả trường: ID, họ tên, phái, mật khẩu.
3. **Kiểm tra MASV duy nhất toàn hệ thống:** `findStudentGlobal(classes, classCount, cleanId)`. Nếu đã tồn tại ở bất kỳ lớp nào → `ERR_DUPLICATE`.
4. Tạo `Student` → thêm vào linked list của lớp.

**Ràng buộc đặc biệt:** Mã sinh viên `MASV` phải là **duy nhất trên toàn bộ hệ thống**, không chỉ trong 1 lớp. Ví dụ: SV001 ở lớp D23CQ01 thì không lớp nào khác được có SV001.

---

#### 8.1.1. 💡 Hướng Dẫn Tích Hợp Cho Tầng UI: Nhập Nhiều Sinh Viên & Hủy Bỏ Giữa Chừng

> **Tình huống thực tế:** Giáo viên dự định nhập 4 sinh viên vào một lớp, nhưng nhập xong người thứ 3 thì muốn dừng lại (không nhập người thứ 4 nữa).

**1. Nguyên tắc thiết kế của tầng Business:**
* Hàm `addStudentWorkflow()` được thiết kế theo tính chất **nguyên tử (Atomic)**: Mỗi lần gọi hàm là thêm độc lập **đúng 1 sinh viên** vào lớp.
* Sinh viên nào đã thêm thành công sẽ **lưu ngay lập tức vào RAM** (danh sách liên kết của lớp).
* Do đó, việc dừng lại bất ngờ ở người thứ 3 **không ảnh hưởng gì đến dữ liệu**: 3 người trước vẫn được bảo toàn nguyên vẹn, và người thứ 4 chưa được gọi hàm sẽ không sinh ra bất kỳ dữ liệu rác hay rò rỉ bộ nhớ nào.

**2. Trách nhiệm của người làm UI:**
Tầng UI cần bọc hàm `addStudentWorkflow()` trong một vòng lặp nhập liệu có **phím thoát / giá trị dừng (Sentinel Value)** thay vì bắt giáo viên phải nhập cứng nhắc.

**3. Code mẫu gợi ý cho Người 1 (UI) tham khảo cài đặt:**

```cpp
string classId = readLine("Nhap Ma lop can them sinh vien: ");
Class* classroom = findClass(classes, classCount, classId.c_str());
if (classroom == nullptr) {
    cout << "-> Khong tim thay lop hoc!\n";
    return;
}

int addedCount = 0;
while (true) {
    cout << "\n--- Nhap thong tin sinh vien thu " << (addedCount + 1) << " ---\n";
    string sId = readLine("Nhap MASV (Nhap '0' hoac de trong de DUNG): ");
    
    // KIỂM TRA ĐIỀU KIỆN DỪNG: Nếu giáo viên nhập 0 hoặc để trống -> Thoát ngay!
    if (sId.empty() || sId == "0") {
        cout << "-> Da dung nhap theo yeu cau cua ban.\n";
        break;
    }

    string ho = readLine("Nhap Ho dem: ");
    string ten = readLine("Nhap Ten: ");
    string gender = readLine("Nhap Phai (Nam/Nu): ");
    string pass = readLine("Nhap Mat khau: ");

    BusinessResult res = addStudentWorkflow(
        classes, classCount, classId.c_str(),
        sId.c_str(), ho.c_str(), ten.c_str(),
        gender.c_str(), pass.c_str()
    );

    if (res == SUCCESS) {
        addedCount++;
        cout << "-> Da them thanh cong SV: " << ho << " " << ten << " (" << sId << ")!\n";
    } else {
        cout << "-> Loi: " << getErrorMessage(res) << ". Vui long thu lai.\n";
    }
}

cout << "-> Tong ket: Da them thanh cong " << addedCount << " sinh vien vao lop " << classId << ".\n";
```

---

#### 8.2. `editStudentWorkflow()`

```cpp
BusinessResult editStudentWorkflow(
    Class* classes[],
    int classCount,
    const char studentId[],  // MASV cần sửa
    const char ho[],         // Họ mới
    const char ten[],        // Tên mới
    const char gender[],     // Phái mới
    const char password[]    // Mật khẩu mới
);
```

**Luồng xử lý:**
1. Tìm sinh viên toàn hệ thống bằng `findStudentGlobal()`.
2. Validate các trường mới.
3. Ghi đè thông tin: họ, tên, phái, mật khẩu.

**Lưu ý:** Không cho phép đổi `MASV` vì nó là khóa nhận diện.

---

#### 8.3. `deleteStudentWorkflow()`

```cpp
BusinessResult deleteStudentWorkflow(
    Class* classes[],
    int classCount,
    const char classId[],     // Mã lớp chứa SV
    const char studentId[]    // MASV cần xóa
);
```

**Luồng xử lý:**
1. Tìm lớp → tìm sinh viên bằng `findStudent()` để xác nhận SV tồn tại trong lớp đó.
2. Nếu không tìm thấy → `ERR_NOT_FOUND`.
3. Gọi `deleteStudent(classroom, cleanStudentId)` — hàm `core` **tự động giải phóng** toàn bộ `scoreList` và `examDetailList` trước khi `delete` node sinh viên.

---

## 9. AuthService.h / AuthService.cpp — Đăng Nhập & Phân Quyền

**Đường dẫn:** `business/AuthService.h` + `business/AuthService.cpp`

Cung cấp **2 phiên bản** hàm đăng nhập:

### 9.1. `login()` — Phiên Bản Đơn Giản (Theo FunctionPrototype)

```cpp
LoginRole login(
    Class* classes[],
    int classCount,
    const char username[],    // "GV" hoặc MASV
    const char password[],    // "GV" hoặc mật khẩu sinh viên
    Student*& loggedStudent   // [OUT] con trỏ đến SV đã đăng nhập
);
```

**Trả về `LoginRole`:**
- `LOGIN_TEACHER` — Đăng nhập giảng viên thành công (username=`"GV"`, password=`"GV"`).
- `LOGIN_STUDENT` — Đăng nhập sinh viên thành công. `loggedStudent` trỏ đến đối tượng `Student*`.
- `LOGIN_FAILED` — Sai thông tin.

**Cơ chế:**
1. Username luôn được chuẩn hóa hoa → so sánh `"GV"` trước.
2. Nếu không phải GV → tìm sinh viên bằng `findStudentGlobal()`.
3. So sánh mật khẩu **case-sensitive** (chính xác từng ký tự).

---

### 9.2. `loginWorkflow()` — Phiên Bản Mở Rộng Cho UI

```cpp
BusinessResult loginWorkflow(
    Class* classes[],
    int classCount,
    const char username[],
    const char password[],
    LoginRole& role,          // [OUT] vai trò
    Student*& loggedStudent   // [OUT] con trỏ SV
);
```

**Khác biệt với `login()`:** Trả về `BusinessResult` chi tiết hơn, giúp UI hiển thị thông báo lỗi cụ thể:
- `ERR_EMPTY_FIELD` — username/password rỗng.
- `ERR_BUFFER_OVERFLOW` — chuỗi quá dài.
- `ERR_NOT_FOUND` — MASV không tồn tại.
- `ERR_INVALID_FORMAT` — Sai mật khẩu.

---

## 10. ExamService.h / ExamService.cpp — Động Cơ Thi Trắc Nghiệm

**Đường dẫn:** `business/ExamService.h` + `business/ExamService.cpp`

Đây là **module phức tạp nhất** trong tầng nghiệp vụ, xử lý toàn bộ quy trình thi.

### Danh Sách Hàm

#### 10.1. `canTakeExam()` — Kiểm Tra Điều Kiện Thi

```cpp
bool canTakeExam(
    Subject* subject,         // Con trỏ đến môn học
    int numberOfQuestions,    // Số câu hỏi muốn thi (N)
    int minutes               // Thời gian thi (phút)
);
```

**Trả `false` khi:**
- `subject == nullptr` (môn không tồn tại)
- `N <= 0` hoặc `T <= 0`
- `N > tổng số câu hỏi hiện có` của môn

---

#### 10.2. `calculateScore()` — Tính Điểm

```cpp
float calculateScore(
    int correctAnswers,      // Số câu trả lời đúng
    int totalQuestions       // Tổng số câu thi
);
```

**Công thức:** `Điểm = (correctAnswers × 10.0) / totalQuestions`

Ví dụ: 7 đúng / 10 câu = **7.00 điểm**.

---

#### 10.3. `prepareExamWorkflow()` — Chuẩn Bị Đề Thi Ngẫu Nhiên

```cpp
BusinessResult prepareExamWorkflow(
    Subject* root,
    Student* student,
    const char subjectId[],
    int numberOfQuestions,     // Số câu muốn rút
    int minutes,               // Số phút (dùng cho kiểm tra hợp lệ)
    ExamDetail*& examQuestionsHead  // [OUT] danh sách câu hỏi snapshot
);
```

**Đây là hàm cốt lõi của quy trình thi.** Luồng xử lý chi tiết:

1. **Kiểm tra ràng buộc:** Sinh viên đã thi môn này chưa? (`findScore()` → `ERR_CONSTRAINT_VIOLATION`).
2. **Tìm môn học** trên BST.
3. **Kiểm tra điều kiện thi** bằng `canTakeExam()`.
4. **Thuật toán bốc đề ngẫu nhiên Fisher-Yates:**
   ```
   a. Đếm tổng số câu hỏi M trong ngân hàng đề.
   b. Cấp phát mảng con trỏ const tạm: const Question** pool = new const Question*[M].
   c. Duyệt linked list, nạp con trỏ vào mảng.
   d. Shuffle mảng bằng Fisher-Yates (hoán đổi thủ công, không dùng std::swap).
   e. Lấy N con trỏ đầu tiên làm đề thi.
   f. Giải phóng mảng phụ: delete[] pool (KHÔNG delete câu hỏi gốc).
   ```
   > **Lưu ý:** Pool dùng kiểu `const Question**` để đảm bảo không vô tình sửa câu hỏi gốc qua con trỏ trong quá trình shuffle.
5. **Tạo snapshot `ExamDetail`:** Với mỗi câu hỏi được chọn, tạo một bản sao độc lập (`createExamDetail()`), chứa nội dung + 4 đáp án + đáp án đúng tại thời điểm thi. Trường `selectedAnswer` ban đầu = `'\0'`.

**Tại sao dùng mảng con trỏ thay vì `std::vector`?** Vì ràng buộc của đồ án: **Không được sử dụng STL containers** (`vector`, `list`, `map`,...).

---

#### 10.4. `submitExamWorkflow()` — Nộp Bài & Chấm Điểm

```cpp
BusinessResult submitExamWorkflow(
    Student* student,
    const char subjectId[],
    ExamDetail* examQuestionsHead,  // Danh sách câu hỏi đã trả lời
    float& finalScore,              // [OUT] điểm cuối cùng
    int& correctCount,              // [OUT] số câu đúng
    int& totalCount                 // [OUT] tổng số câu
);
```

**Luồng xử lý:**
1. Kiểm tra lại ràng buộc (sinh viên đã có điểm môn này chưa).
2. Duyệt từng `ExamDetail`, so sánh `selectedAnswer` với `correctAnswer`.
3. Tính điểm bằng `calculateScore()`.
4. **Ghi nhận kết quả:** Gọi `addScore(student, subjectId, finalScore, examQuestionsHead)`. Hàm này sẽ:
   - Tạo node `Score` mới.
   - Gán `examQuestionsHead` vào `score->detailList` → **chuyển quyền sở hữu bộ nhớ** cho `Score`.
5. Sau khi nộp thành công, `examQuestionsHead` thuộc sở hữu của sinh viên và sẽ được giải phóng khi sinh viên bị xóa.

---

#### 10.5. `cancelExam()` — Hủy Bài Thi

```cpp
void cancelExam(ExamDetail*& examQuestionsHead);
```

Giải phóng toàn bộ danh sách `ExamDetail` khi sinh viên hủy thi giữa chừng. Gọi `deleteExamDetailList()` từ tầng `core`.

---

#### 10.6. `isExamTimeExpired()` & Các Tiện Ích Đếm Giờ Theo Giây

Tầng Business cung cấp hệ thống hàm kiểm tra và định dạng thời gian chính xác theo giây:

```cpp
// 1. Kiểm tra hết giờ theo phút
bool isExamTimeExpired(time_t startTime, int minutes);

// 2. Tính số giây còn lại chính xác
int calculateExamRemainingSeconds(time_t startTime, int durationSeconds);

// 3. Kiểm tra hết giờ theo giây
bool isExamTimeExpiredSeconds(time_t startTime, int durationSeconds);

// 4. Định dạng chuỗi thời gian hiển thị: "MM:SS (M phut S giay)"
void formatExamTime(int remainingSeconds, char buffer[], int bufferSize);
```

- **`calculateExamRemainingSeconds()`**: Tính `durationSeconds - difftime(time(nullptr), startTime)`. Nếu thời gian đã hết, hàm an toàn trả về `0`.
- **`formatExamTime()`**: Tự động chuyển đổi số giây còn lại sang chuỗi hiển thị đẹp mắt (ví dụ: `150` giây → `"02:30 (2 phut 30 giay)"`), đảm bảo UI hiển thị đồng nhất.

---

#### 10.7. `struct ExamTimer` — Bộ Đếm Thời Gian Hướng Đối Tượng

Nhằm giúp tầng UI không phải tự quản lý các biến thời gian rời rạc, Business cung cấp struct đóng gói `ExamTimer`:

```cpp
struct ExamTimer {
    time_t startTime;               // Thời điểm bắt đầu (epoch seconds)
    int totalDurationSeconds;       // Tổng thời gian thi tính theo giây
    int initialRemainingSeconds;     // Thời gian còn lại tại thời điểm bắt đầu/resume
    bool isRunning;                 // Trạng thái đang chạy

    ExamTimer();
    explicit ExamTimer(int durationSeconds);

    void start(int durationSeconds);                // Khởi động đếm ngược theo giây
    void startMinutes(int minutes);                  // Khởi động đếm ngược theo phút
    void resume(int remainingSeconds);               // Tiếp tục đếm ngược từ số giây còn lại (khôi phục session)
    int getRemainingSeconds() const;                // Lấy số giây còn lại thực tế
    int getElapsedSeconds() const;                  // Lấy số giây đã trôi qua
    bool isExpired() const;                         // Kiểm tra đã hết giờ chưa
    void stop();                                    // Dừng đồng hồ
    void formatTime(char buffer[], int bufferSize) const; // Định dạng thời gian còn lại vào buffer
};
```

**Ví dụ ứng dụng `ExamTimer` trong UI:**
```cpp
ExamTimer timer;
timer.startMinutes(15); // Bắt đầu làm bài thi 15 phút

while (!timer.isExpired()) {
    char timeStr[32];
    timer.formatTime(timeStr, sizeof(timeStr));
    // UI in đồng hồ: "Thoi gian con lai: 14:59 (14 phut 59 giay)"
    
    // Xử lý phím chọn đáp án...
    if (userSubmitted) break;
}

if (timer.isExpired()) {
    cout << "[THONG BAO]: Da het thoi gian lam bai! Tu dong nop bai...\n";
    submitExamWorkflow(...);
}
```

---

#### 10.8. Cơ Chế Lưu & Phục Hồi Phiên Thi Dở Dang (Exam Session Persistence)

Để chống lại các sự cố bất khả kháng trong phòng thi (mất điện, vô tình đóng cửa sổ Console, sập nguồn máy tính), `ExamService` tích hợp cơ chế **lưu trữ bền vững phiên thi (Fault-Tolerant Session Persistence)**:

##### 1. Định dạng file phiên thi tạm: `temp_<MASV>.dat`
Mỗi sinh viên đang làm bài sẽ có một file lưu trữ phiên thi riêng (mặc định là `temp_<MASV>.dat` hoặc đường dẫn tùy biến `filepath`):
- **Dòng tiêu đề (Header)**: `MASV|MAMON|THOI_GIAN_CON_LAI|SO_CAU`
- **Các dòng câu hỏi (Snapshot hoặc Compact)**:
  - Chuẩn Snapshot đầy đủ: `ID|Selected|Correct|Content|A|B|C|D`
  - Chuẩn Compact: `ID|Selected`

##### 2. Các hàm lưu phiên thi: `saveExamSession()`
```cpp
// 1. Lưu từ danh sách snapshot ExamDetail*
bool saveExamSession(const char studentId[], const char subjectId[], int remainingSeconds, ExamDetail* examQuestionsHead, const char* filepath = nullptr);
bool saveExamSession(const char studentId[], int remainingSeconds, ExamDetail* examQuestionsHead, const char* filepath = nullptr);
bool saveExamSession(Student* student, const char subjectId[], int remainingSeconds, ExamDetail* examQuestionsHead, const char* filepath = nullptr);

// 2. Lưu từ mảng ID câu hỏi và mảng đáp án đã chọn (phù hợp UI lưu mảng phẳng)
bool saveExamSession(const char studentId[], const char subjectId[], int remainingSeconds, const int questionIds[], const char selectedAnswers[], int count, const char* filepath = nullptr);
bool saveExamSession(const char studentId[], int remainingSeconds, const int questionIds[], const char selectedAnswers[], int count, const char* filepath = nullptr);
```

##### 3. Các hàm kiểm tra phiên thi: `checkExamSession()`
```cpp
// 1. Kiểm tra nhanh xem sinh viên có bài thi dở dang không
bool checkExamSession(const char studentId[], const char* filepath = nullptr);
bool checkExamSession(Student* student, const char* filepath = nullptr);

// 2. Trích xuất môn thi và số giây còn lại để UI hỏi xác nhận
bool checkExamSession(const char studentId[], char subjectIdOut[], int& remainingSecondsOut, const char* filepath = nullptr);

// 3. Đọc danh sách mảng ID câu hỏi và đáp án đã chọn từ file
bool checkExamSession(const char studentId[], char subjectIdOut[], int& remainingSecondsOut, int questionIdsOut[], char selectedAnswersOut[], int& countOut, int maxQuestions = 100, const char* filepath = nullptr);
```

##### 4. Hàm khôi phục đề thi: `loadExamSession()`
```cpp
bool loadExamSession(
    const char studentId[],
    char subjectIdOut[],
    int& remainingSecondsOut,
    ExamDetail*& examQuestionsHead,
    Subject* root = nullptr,
    const char* filepath = nullptr
);
```
- Khôi phục trọn vẹn danh sách liên kết `ExamDetail*` (bao gồm câu hỏi, 4 đáp án và đáp án `selectedAnswer` sinh viên đã chọn trước khi gặp sự cố).
- Hỗ trợ nạp trực tiếp từ file snapshot hoặc tự động tra cứu lại từ cây BST `root` nếu file session ở dạng compact.

##### 5. Hàm xóa phiên thi: `removeExamSession()`
```cpp
bool removeExamSession(const char studentId[], const char* filepath = nullptr);
bool removeExamSession(Student* student, const char* filepath = nullptr);
```
- Tự động xóa file `temp_<MASV>.dat` khi sinh viên **nộp bài thành công** hoặc **chủ động hủy bài thi**.

##### 6. Overloads an toàn kiểu `nullptr_t` (C++11)
Nhằm tránh lỗi biên dịch ambiguity khi lập trình viên truyền trực tiếp con trỏ `nullptr`, các overloads nội tuyến được cung cấp:
```cpp
inline bool saveExamSession(std::nullptr_t, const char*, int, ExamDetail*, const char* = nullptr);
inline bool checkExamSession(std::nullptr_t, const char* = nullptr);
inline bool removeExamSession(std::nullptr_t, const char* = nullptr);
```

---


## 11. ReportService.h / ReportService.cpp — Báo Cáo & Xem Bài Thi (UI Integration)

**Đường dẫn:** `business/ReportService.h` + `business/ReportService.cpp`  
**Mục đích:** Cung cấp dữ liệu báo cáo bảng điểm lớp và chi tiết bài thi trắc nghiệm để **người phụ trách UI (Người 1)** dễ dàng tương tác và hiển thị lên giao diện.

Tầng Business cung cấp **2 cách tiếp cận** cho tầng UI:

---

### Cách 1: Tầng UI tự thiết kế giao diện (Khuyến nghị cho UI)

Người 3 cung cấp các **Data Query APIs** trả về dữ liệu thô, giúp Người 1 toàn quyền quyết định cách trình bày (vẽ bảng, đổi màu sắc, phân trang, hoặc làm GUI):

#### 11.1. In bảng điểm một môn của một lớp (Mục i)
Người 1 chỉ cần duyệt danh sách sinh viên của lớp và gọi `hasStudentTakenExam`:

```cpp
Class* c = findClass(classes, classCount, classId);
Student* s = c->studentList;
while (s != nullptr) {
    float score = 0.0f;
    if (hasStudentTakenExam(s, subjectId, score)) {
        // UI tự hiển thị: s->id, s->ho, s->ten và điểm: score
    } else {
        // UI hiển thị: "Chua thi"
    }
    s = s->next;
}
```
* **Quy ước:** Phân biệt rõ điểm `0.0f` (đã thi và bị 0 điểm) với trường hợp `hasStudentTakenExam == false` (chưa thi).

#### 11.2. Xem lại chi tiết bài đã thi của sinh viên (Mục h)
Người 1 gọi `getStudentExamDetails` để nhận danh sách snapshot câu hỏi:

```cpp
ExamDetail* cur = getStudentExamDetails(student, subjectId);
if (cur == nullptr) {
    // Sinh viên chưa thi môn này
} else {
    while (cur != nullptr) {
        // UI tự do hiển thị:
        // cur->content: nội dung câu hỏi
        // cur->answerA, B, C, D: 4 đáp án (đã giữ nguyên thứ tự xáo trộn khi thi)
        // cur->selectedAnswer: đáp án sinh viên đã chọn (hoặc '\0' nếu bỏ trống)
        // cur->correctAnswer: đáp án đúng
        cur = cur->next;
    }
}
```

---

### Cách 2: Sử dụng các hàm in sẵn (Quick Print Helpers)

Nếu tầng UI muốn hiển thị nhanh ra console theo mẫu mặc định mà không cần tự định dạng bảng, có thể gọi trực tiếp:

```cpp
// 1. In toàn bộ bảng điểm lớp (tự động kẻ bảng và điền "Chua thi")
printClassScoreTable(classes, classCount, root, classId, subjectId, std::cout);

// 2. In toàn bộ chi tiết bài thi snapshot của sinh viên kèm kết quả Đúng/Sai
printExamDetailReview(student, subjectId, std::cout);

// 3. In toàn bộ điểm các môn đã thi của một sinh viên
printStudentScores(student, root, std::cout);
```

---

## 12. FileStorage.h / FileStorage.cpp — Lưu Trữ & Nạp Dữ Liệu File

**Đường dẫn:** `business/FileStorage.h` + `business/FileStorage.cpp`

Module này chịu trách nhiệm **serialize (ghi)** và **deserialize (đọc)** toàn bộ dữ liệu RAM ra/vào file text.

### Định Dạng File

Hệ thống sử dụng cơ chế **tách file môn học - câu hỏi độc lập** và chuẩn hóa dữ liệu dòng bằng dấu gạch đứng (`|`):

#### 1. File `data/subjects.txt` (Danh mục môn học độc lập)
Chỉ lưu thông tin danh mục môn học (không chứa câu hỏi):
```
<tổng số môn>
MAMH|TENMH
... (mỗi môn 1 dòng)
```

#### 2. Các file câu hỏi riêng biệt: `data/<MAMH>.txt` (Ví dụ: `data/CTDL.txt`, `data/OOP.txt`)
Mỗi môn học lưu toàn bộ câu hỏi của môn đó vào một file riêng mang tên mã môn. Mỗi câu hỏi được lưu trên **đúng 1 dòng duy nhất**, các trường ngăn cách bởi dấu `|`:
```
ID|Nội dung|Đáp án A|Đáp án B|Đáp án C|Đáp án D|Đáp án đúng
```

#### 3. File `data/classes.txt` (Lớp học, Sinh viên & Lịch sử thi gom dòng)
Thông tin sinh viên và câu hỏi bài thi snapshot được gom gọn gàng trên 1 dòng duy nhất:
```
<tổng số lớp>
<MALOP>
<TENLOP>
<số sinh viên>
  MASV|Họ|Tên|Phái|Mật khẩu|Số môn đã thi
  <subjectId>
  <điểm>
  <số ExamDetail>
    questionId|content|answerA|answerB|answerC|answerD|correctAnswer|selectedAnswer
```

> **Tính năng Backward-Compatible:** Bộ giải mã (Parser) tự động nhận diện dữ liệu ngăn cách bởi dấu `|` (chuẩn mới) hoặc đọc theo các dòng rời rạc (chuẩn cũ), đảm bảo an toàn tuyệt đối khi nạp dữ liệu.

### Danh Sách Hàm

#### 12.1. `saveSubjectsToFile()` / `loadSubjectsFromFile()`

```cpp
bool saveSubjectsToFile(Subject* root, const char* filepath);
bool loadSubjectsFromFile(Subject*& root, const char* filepath);
```

- **Save:** Duyệt BST theo **in-order** (trái → gốc → phải) → ghi danh mục `MAMH|TENMH` vào `subjects.txt`, đồng thời tự động xuất toàn bộ câu hỏi của từng môn ra file `<dir>/<MAMH>.txt`.

  > **📌 Cách suy ra thư mục câu hỏi từ tham số `filepath`:**  
  > Hàm nhận `filepath` là đường dẫn đầy đủ đến `subjects.txt` (ví dụ: `"data/subjects.txt"`).  
  > Bên trong, hàm cắt phần tên file để lấy thư mục cha: `"data/subjects.txt"` → thư mục `"data"`.  
  > Mọi file câu hỏi sau đó được ghi vào `data/<MAMH>.txt`. Nếu truyền vào `"C:/project/data/subjects.txt"`, file câu hỏi sẽ xuất hiện ở `C:/project/data/<MAMH>.txt`.  
  > **Hệ quả:** Phải đảm bảo thư mục `data/` đã tồn tại trước khi gọi (hoặc dùng `saveAllData()` để tự tạo thư mục).

- **Load:** Đọc danh mục môn học từ `subjects.txt`, sau đó nạp câu hỏi từ file `<dir>/<MAMH>.txt` tương ứng. Tự động kiểm tra tính duy nhất ID câu hỏi toàn hệ thống (`isQuestionIdUsedAnywhere`), đồng bộ counter bằng `updateQuestionIdCounter(qId)` hoặc tự động sinh mã mới bằng `generateQuestionId()` nếu phát hiện trùng lặp.

#### 12.2. Tiện Ích Quản Lý File Câu Hỏi Tự Động

```cpp
bool createEmptyQuestionFile(const char* dir, const char* subjectId);
bool renameSubjectQuestionFile(const char* dir, const char* oldId, const char* newId);
bool deleteSubjectQuestionFile(const char* dir, const char* subjectId);
```

- **`createEmptyQuestionFile()`:** Tự động tạo file `data/<MAMH>.txt` rỗng khi thêm môn học mới thành công.
- **`renameSubjectQuestionFile()`:** Tự động đổi tên file câu hỏi khi giảng viên đổi mã môn học.
- **`deleteSubjectQuestionFile()`:** Tự động xóa file `data/<MAMH>.txt` tương ứng khi môn học bị xóa khỏi hệ thống.

#### 12.3. `saveClassesToFile()` / `loadClassesFromFile()`

```cpp
bool saveClassesToFile(Class* classes[], int classCount, const char* filepath);
bool loadClassesFromFile(Class* classes[], int& classCount, const char* filepath);
```

- **Save:** Ghi mảng lớp học ra file, gom thông tin sinh viên và câu hỏi chi tiết bài thi snapshot trên từng dòng bằng dấu `|`.
- **Load:** Đọc và tái tạo cấu trúc (Class → Student → Score → ExamDetail), hỗ trợ linh hoạt cả định dạng mới lẫn định dạng cũ.

#### 12.4. `saveAllData()` / `loadAllData()` — Hàm Tổng Hợp

```cpp
bool saveAllData(
    Subject* root,
    Class* classes[],
    int classCount,
    const char* dir = "data"     // Thư mục lưu trữ
);

bool loadAllData(
    Subject*& root,
    Class* classes[],
    int& classCount,
    const char* dir = "data"
);
```

- **`saveAllData()`:** Tự tạo thư mục `data/` nếu chưa có. Ghi `subjects.txt`, các file câu hỏi `<MAMH>.txt` và `classes.txt`.
- **`loadAllData()`:** Đọc toàn bộ dữ liệu từ thư mục chỉ định. Trả `true` nếu **ít nhất một** file đọc thành công (cho phép chạy với dữ liệu một phần).

---

## 13. Bảng Tổng Hợp Tất Cả Hàm

| # | File | Hàm | Kiểu Trả Về | Vai Trò |
|:---:|---|---|---|---|
| 1 | `BusinessErrors.h` | `getErrorMessage()` | `const char*` | Chuyển mã lỗi → chuỗi tiếng Việt |
| 2 | `Validation.cpp` | `trimString()` | `void` | Cắt khoảng trắng đầu/cuối |
| 3 | `Validation.cpp` | `toUpperString()` | `void` | Chuyển hoa toàn bộ |
| 4 | `Validation.cpp` | `containsWhitespace()` | `bool` | Kiểm tra có khoảng trắng |
| 5 | `Validation.cpp` | `validateSubjectId()` | `BusinessResult` | Validate mã môn |
| 6 | `Validation.cpp` | `validateSubjectName()` | `BusinessResult` | Validate tên môn |
| 7 | `Validation.cpp` | `validateClassId()` | `BusinessResult` | Validate mã lớp |
| 8 | `Validation.cpp` | `validateClassName()` | `BusinessResult` | Validate tên lớp |
| 9 | `Validation.cpp` | `validateStudentId()` | `BusinessResult` | Validate mã SV |
| 10 | `Validation.cpp` | `validateStudentName()` | `BusinessResult` | Validate họ tên |
| 11 | `Validation.cpp` | `validateGender()` | `BusinessResult` | Validate & chuẩn hóa phái |
| 12 | `Validation.cpp` | `validatePassword()` | `BusinessResult` | Validate mật khẩu |
| 13 | `Validation.cpp` | `validateQuestionContent()` | `BusinessResult` | Validate nội dung câu hỏi |
| 14 | `Validation.cpp` | `validateAnswerContent()` | `BusinessResult` | Validate nội dung đáp án |
| 15 | `Validation.cpp` | `validateCorrectAnswer()` | `BusinessResult` | Validate & chuẩn hóa đáp án đúng |
| 16 | `SubjectService.cpp` | `isSubjectUsedInScores()` | `bool` | Kiểm tra môn có được tham chiếu |
| 17 | `SubjectService.cpp` | `addSubjectWorkflow()` | `BusinessResult` | Thêm môn học |
| 18 | `SubjectService.cpp` | `editSubjectWorkflow()` | `BusinessResult` | Sửa môn học |
| 19 | `SubjectService.cpp` | `deleteSubjectWorkflow()` | `BusinessResult` | Xóa môn học |
| 20 | `QuestionService.cpp` | `addQuestionWorkflow()` | `BusinessResult` | Thêm câu hỏi |
| 21 | `QuestionService.cpp` | `editQuestionWorkflow()` | `BusinessResult` | Sửa câu hỏi |
| 22 | `QuestionService.cpp` | `deleteQuestionWorkflow()` | `BusinessResult` | Xóa câu hỏi |
| 23 | `ClassService.cpp` | `addClassWorkflow()` | `BusinessResult` | Thêm lớp học |
| 24 | `ClassService.cpp` | `deleteClassWorkflow()` | `BusinessResult` | Xóa lớp học |
| 25 | `StudentService.cpp` | `addStudentWorkflow()` | `BusinessResult` | Thêm sinh viên |
| 26 | `StudentService.cpp` | `editStudentWorkflow()` | `BusinessResult` | Sửa sinh viên |
| 27 | `StudentService.cpp` | `deleteStudentWorkflow()` | `BusinessResult` | Xóa sinh viên |
| 28 | `AuthService.cpp` | `login()` | `LoginRole` | Đăng nhập (đơn giản) |
| 29 | `AuthService.cpp` | `loginWorkflow()` | `BusinessResult` | Đăng nhập (chi tiết lỗi) |
| 30 | `ExamService.cpp` | `canTakeExam()` | `bool` | Kiểm tra điều kiện thi |
| 31 | `ExamService.cpp` | `calculateScore()` | `float` | Tính điểm |
| 32 | `ExamService.cpp` | `prepareExamWorkflow()` | `BusinessResult` | Bốc đề ngẫu nhiên |
| 33 | `ExamService.cpp` | `submitExamWorkflow()` | `BusinessResult` | Nộp bài & chấm điểm |
| 34 | `ExamService.cpp` | `cancelExam()` | `void` | Hủy bài thi, giải phóng bộ nhớ |
| 35 | `ExamService.cpp` | `isExamTimeExpired()` | `bool` | Kiểm tra bài thi đã hết giờ chưa (theo phút) |
| 36 | `ExamService.cpp` | `calculateExamRemainingSeconds()` | `int` | Tính số giây còn lại chính xác |
| 37 | `ExamService.cpp` | `isExamTimeExpiredSeconds()` | `bool` | Kiểm tra hết giờ thi (theo giây) |
| 38 | `ExamService.cpp` | `formatExamTime()` | `void` | Định dạng số giây sang "MM:SS (M phut S giay)" |
| 39 | `ExamService.h` | `ExamTimer` | `struct` | Bộ đếm thời gian hướng đối tượng (start, resume, isExpired, format) |
| 40 | `ExamService.cpp` | `saveExamSession()` | `bool` | Lưu phiên thi tạm chống mất điện/sập nguồn (`temp_<MASV>.dat`) |
| 41 | `ExamService.cpp` | `checkExamSession()` | `bool` | Kiểm tra và trích xuất thông tin phiên thi dở dang |
| 42 | `ExamService.cpp` | `removeExamSession()` | `bool` | Xóa file phiên thi sau khi nộp bài hoặc hủy |
| 43 | `ExamService.cpp` | `loadExamSession()` | `bool` | Khôi phục trọn vẹn danh sách `ExamDetail*` từ file phiên thi |
| 44 | `ReportService.cpp` | `hasStudentTakenExam()` | `bool` | Kiểm tra SV đã thi và lấy điểm |
| 45 | `ReportService.cpp` | `getStudentExamScore()` | `Score*` | Lấy con trỏ điểm của SV |
| 46 | `ReportService.cpp` | `getStudentExamDetails()` | `ExamDetail*` | Lấy snapshot bài thi đã làm |
| 47 | `ReportService.cpp` | `printClassScoreTable()` | `BusinessResult` | Helper in bảng điểm lớp |
| 48 | `ReportService.cpp` | `printExamDetailReview()` | `BusinessResult` | Helper in chi tiết bài thi |
| 49 | `ReportService.cpp` | `printStudentScores()` | `BusinessResult` | Helper in bảng điểm cá nhân SV |
| 50 | `FileStorage.cpp` | `saveSubjectsToFile()` | `bool` | Ghi BST môn học & câu hỏi ra file |
| 51 | `FileStorage.cpp` | `loadSubjectsFromFile()` | `bool` | Nạp BST môn học & câu hỏi từ file |
| 52 | `FileStorage.cpp` | `createEmptyQuestionFile()` | `bool` | Tự động tạo file câu hỏi rỗng cho môn |
| 53 | `FileStorage.cpp` | `renameSubjectQuestionFile()` | `bool` | Đổi tên file câu hỏi khi đổi MAMH |
| 54 | `FileStorage.cpp` | `deleteSubjectQuestionFile()` | `bool` | Xóa file câu hỏi khi xóa môn |
| 55 | `FileStorage.cpp` | `saveClassesToFile()` | `bool` | Ghi mảng lớp học ra file (gom dòng pipe) |
| 56 | `FileStorage.cpp` | `loadClassesFromFile()` | `bool` | Nạp mảng lớp học từ file (hỗ trợ cả 2 định dạng) |
| 57 | `FileStorage.cpp` | `saveAllData()` | `bool` | Lưu toàn bộ dữ liệu |
| 58 | `FileStorage.cpp` | `loadAllData()` | `bool` | Nạp toàn bộ dữ liệu |

---

## 14. Quy Tắc Thiết Kế & Ràng Buộc

### 14.1. Nguyên Tắc "Không STL Containers"

Toàn bộ tầng `core/` và `business/` **tuyệt đối không sử dụng** `std::vector`, `std::list`, `std::map`, `std::set`, `std::string` cho cấu trúc dữ liệu chính. Tất cả đều dùng:
- **Binary Search Tree** (BST) tự cài cho Subject.
- **Singly Linked List** tự cài cho Question, Student, Score, ExamDetail.
- **Mảng con trỏ** (`Class* classes[MAX_CLASS]`) cho Class.

> **Ngoại lệ duy nhất:** `FileStorage.cpp` được phép dùng `std::string`, `std::ifstream`, `std::ofstream` vì đây là tầng I/O, không phải cấu trúc dữ liệu lõi.

### 14.2. Ràng Buộc Toàn Vẹn Dữ Liệu (Referential Integrity)

| Ràng Buộc | Hàm Kiểm Tra | Hành Động |
|---|---|---|
| Không xóa/đổi mã môn nếu có SV đã thi | `isSubjectUsedInScores()` | Trả `ERR_CONSTRAINT_VIOLATION` |
| Không xóa lớp nếu có SV | `classroom->studentList != nullptr` | Trả `ERR_CONSTRAINT_VIOLATION` |
| MASV duy nhất toàn hệ thống | `findStudentGlobal()` | Trả `ERR_DUPLICATE` |
| Mỗi SV chỉ thi mỗi môn 1 lần | `findScore(student, subjectId)` | Trả `ERR_CONSTRAINT_VIOLATION` |

### 14.3. Quản Lý Bộ Nhớ (Memory Safety)

- Mọi `new` đều có `delete` tương ứng.
- Khi xóa sinh viên: tự động giải phóng `scoreList` → mỗi Score giải phóng `detailList`.
- Khi xóa môn học: tự động giải phóng `questionList`.
- Khi thoát chương trình: **tầng UI** chịu trách nhiệm gọi deep cleanup toàn bộ BST + mảng lớp (không phải tầng business).
- `cancelExam()` giải phóng đề thi khi sinh viên hủy thi giữa chừng — **phải được UI gọi** khi người dùng bấm hủy hoặc hết giờ mà không nộp bài.
- `loadClassesFromFile()` giải phóng `detailHead` nếu `addScore` thất bại để đảm bảo không rò rỉ bộ nhớ khi nạp file.

### 14.4. Chuẩn Hóa Dữ Liệu

Mọi hàm `Workflow` đều thực hiện **chuẩn hóa trước khi xử lý**:
1. `trimString()` — Cắt khoảng trắng thừa.
2. `toUpperString()` — Chuyển hoa cho các trường mã (`MAMH`, `MALOP`, `MASV`).
3. Điều này đảm bảo `"  ctdl  "` và `"CTDL"` được nhận diện là **cùng một mã**.

---

## 15. Lưu Ý Quan Trọng Cho Tầng UI

Các hành vi sau **không thuộc tầng business** mà phải được **tầng UI xử lý**:

### 15.1. Early Exit Khi Nhập Danh Sách Sinh Viên

Xem mục **8.1.1** để biết chi tiết. Tóm tắt: `addStudentWorkflow()` xử lý **từng sinh viên một** (atomic), UI kiểm soát vòng lặp và cho phép dừng sớm.

### 15.2. Deep Cleanup Khi Thoát Chương Trình

Khi thoát, UI phải tự giải phóng bộ nhớ:
```cpp
// Giải phóng mảng lớp
while (classCount > 0) {
    deleteClass(classes, classCount, classes[0]->id);
}
// Giải phóng BST môn học
while (root != nullptr) {
    root = deleteSubject(root, root->id);
}
```

### 15.3. Gọi `cancelExam()` Hoặc `submitExamWorkflow()` Khi Hết Giờ

Khi `isExamTimeExpired()` trả `true`, UI **bắt buộc** phải:
- Gọi `submitExamWorkflow()` nếu muốn tự động nộp bài.
- Hoặc gọi `cancelExam(examQuestionsHead)` nếu muốn hủy bài. 
Nếu không làm gì, sẽ xảy ra **memory leak**.

### 15.4. Gọi `cancelExam()` Khi Sinh Viên Bấm Hủy

Nếu sinh viên thoát khỏi màn hình làm bài trước khi nộp, UI **bắt buộc** phải gọi:
```cpp
cancelExam(examQuestionsHead);
```

### 15.5. Xử Lý Khi File Câu Hỏi `<MAMH>.txt` Bị Thiếu

Trường hợp cạnh: `subjects.txt` tồn tại và ghi nhận môn `CTDL`, nhưng file `data/CTDL.txt` bị **xóa thủ công** hoặc không tồn tại.

**Hành vi của `loadSubjectsFromFile()`:**
- Môn học `CTDL` **vẫn được tạo** trên BST với thông tin tên môn đầy đủ.
- `loadSubjectsFromFile()` **bỏ qua** bước nạp câu hỏi và **không báo lỗi** — môn học tải lên với `questionList = nullptr` (0 câu hỏi).
- Giảng viên vẫn có thể thêm câu hỏi mới bình thường qua `addQuestionWorkflow()`, và file câu hỏi sẽ được tái tạo khi `saveAllData()` được gọi tiếp theo.

**Khuyến nghị cho UI:** Sau khi `loadAllData()`, nếu cần, có thể kiểm tra và cảnh báo người dùng:
```cpp
Subject* s = root;
while (s != nullptr) {
    if (countQuestions(s->questionList) == 0) {
        cout << "[CANH BAO] Mon " << s->id << " chua co cau hoi nao.\n";
    }
    // duyệt BST theo cách phù hợp
}
```

### 15.6. Quy Trình Khôi Phục Phiên Thi Dở Dang (Crash / Fault Tolerance)

Khi triển khai màn hình sinh viên đăng nhập và làm bài thi, UI nên áp dụng quy trình kiểm tra bài thi dở dang:

1. **Khi sinh viên đăng nhập:**
   ```cpp
   char subId[SUBJECT_ID_LEN] = {};
   int remainingSeconds = 0;
   if (checkExamSession(loggedStudent->id, subId, remainingSeconds)) {
       // Phát hiện bài thi chưa hoàn thành do tắt máy/mất điện
       cout << "Phat hien bai thi mon " << subId << " dang lam do voi " 
            << remainingSeconds << " giay con lai. Ban co muon tiep tuc?\n";
       // Nếu tiếp tục:
       ExamDetail* examHead = nullptr;
       if (loadExamSession(loggedStudent->id, subId, remainingSeconds, examHead, rootSubject)) {
           ExamTimer timer;
           timer.resume(remainingSeconds);
           // Tiếp tục vòng lặp làm bài thi...
       }
   }
   ```
2. **Trong lúc làm bài thi:**
   Sau mỗi lần sinh viên chọn một đáp án, UI nên gọi `saveExamSession()` để ghi nhận trạng thái mới nhất:
   ```cpp
   saveExamSession(loggedStudent->id, subId, timer.getRemainingSeconds(), examHead);
   ```
3. **Khi nộp bài hoặc hủy bài thi:**
   UI **bắt buộc** phải gọi xóa session để không còn tàn dư file rác:
   ```cpp
   removeExamSession(loggedStudent->id);
   ```

---

*Tài liệu được cập nhật đầy đủ và đồng bộ với toàn bộ tính năng của tầng Business Logic Layer (`ExamService`, `ReportService`, `FileStorage`,...).*
