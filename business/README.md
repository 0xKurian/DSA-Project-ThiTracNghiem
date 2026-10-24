# 📘 TÀI LIỆU HƯỚNG DẪN TẦNG NGHIỆP VỤ (`business/`)

> **Cập nhật hợp đồng 24/10/2026:** Cấu hình thi mới nằm trong
> `Class.configList`, được quản lý bởi `ExamConfigService.h/.cpp` và lưu tại
> `data/exam_configs.txt`. Các đoạn bên dưới mô tả `setExamConfigWorkflow`,
> `Subject.examQuestionCount/examMinutes`, định dạng môn 5 cột và UI tự lưu
> session là tài liệu của luồng cũ; chỉ còn để tham khảo trong giai đoạn chuyển UI.

## API nghiệp vụ hiện hành cho cấu hình lớp và phiên thi

- `setClassExamConfigWorkflow` bốc và giữ cố định thứ tự ID câu hỏi cho cặp
  `(lớp, môn)`. Đổi phút giữ bộ đề; đổi số câu bốc bộ mới. Khi lớp đã có điểm
  môn này, `setClassExamConfigWorkflow` và `deleteClassExamConfigWorkflow` trả
  `ERR_CONSTRAINT_VIOLATION`.
- `findStudentClass`, `classHasTakenSubject`, `canStudentTakeSubject` và
  `prepareExamFromConfig` là các truy vấn/workflow dùng chung cho GUI và console.
  `prepareExamFromConfig` tạo snapshot câu hỏi theo thứ tự cấu hình và chuyển
  quyền sở hữu danh sách `ExamDetail` cho caller cho tới lúc nộp hoặc hủy.
- `saveAllData`/`loadAllData` đọc ghi thêm `exam_configs.txt`; môn được ghi
  3 cột, loader vẫn đọc được file môn 5 cột cũ nhưng bỏ cấu hình cũ. Sau khi
  nạp, BST môn được cân bằng lại. Không tự chuyển cấu hình môn cũ sang lớp.
- `tickExamSession` lưu khi số giây còn lại thay đổi. `finishExamWorkflow`,
  `submitExpiredExamWorkflow` và `cancelPendingExamWorkflow` chấm bài, lưu toàn
  bộ dữ liệu rồi xóa session khi lưu thành công. `saveFailedOut` báo trường hợp
  điểm đã được chấm trong RAM nhưng ghi file thất bại. Sau nộp, `head` là null.
- `deleteQuestionWorkflow(root, classes, classCount, subjectId, questionId)`
  chặn xóa nếu môn đã có điểm hoặc câu hỏi đang nằm trong cấu hình lớp. Chữ ký
  3 tham số cũ còn để UI hiện tại biên dịch, chưa có đủ dữ liệu để áp dụng
  quy tắc này; UI cần chuyển sang chữ ký mới.
- `isPassingScore`, `countStudentsInClass`, `collectSubjectsInOrder` và
  `freeAllData` nằm trong `BusinessQueries.h/.cpp`.

Các API cũ vẫn tồn tại để bản UI hiện tại biên dịch. UI cần chuyển sang API
theo lớp trước khi Core xóa hai trường cấu hình cũ trên `Subject`.

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
| `ERR_CONSTRAINT_VIOLATION` | 7 | Vi phạm ràng buộc toàn vẹn | Xóa/sửa môn đã có điểm thi, cấu hình thi số câu/thời gian không hợp lệ, sinh viên thi lại môn đã thi (Lưu ý: Xóa lớp có SV đã được bãi bỏ ràng buộc, tự cascade giải phóng) |
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
| `validateStudentId(id)` | Mã sinh viên (MASV) | `STUDENT_ID_LEN = 20` | Không rỗng/whitespace + độ dài < 20 + chỉ chứa `[A-Za-z0-9_-]` (chống path traversal cho session file) |
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
5. **Đồng bộ cấu hình lớp:** Tự động duyệt qua `classes[]` và cập nhật mã môn mới `cleanNewId` cho mọi `ExamConfig` của môn học đó.

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
3. **Dọn dẹp cấu hình lớp:** Tự động giải phóng `ExamConfig` của môn này khỏi toàn bộ danh sách lớp học (`classes[i]->configList`).
4. Gọi `deleteSubject(root, cleanId)` — tự động giải phóng toàn bộ `questionList` của môn trước khi xóa node BST.

---

#### 5.5. `rebalanceSubjectBST()` — Cân Bằng Lại Cây BST Môn Học (B01 / R1)

```cpp
void rebalanceSubjectBST(Subject*& root);
```

**Mục đích:** Khi nạp từ file `subjects.txt` (được ghi theo thứ tự in-order), cây BST sẽ bị suy biến lệch hoàn toàn sang phải với chiều cao $O(N)$. Hàm này tái cấu trúc cây về dạng cân bằng hoàn hảo (chiều cao $\le 2 \times \lceil\log_2(N+1)\rceil$) để đảm bảo tốc độ tìm kiếm $O(\log N)$.

**Thuật toán:**
1. Đếm tổng số node hiện tại trên cây. Nếu $N < 2$, kết thúc ngay.
2. Cấp phát mảng con trỏ tạm: `Subject** nodes = new Subject*[count];`.
3. Duyệt in-order đưa con trỏ các node vào mảng (đã tự động có thứ tự từ điển).
4. Đệ quy dựng lại cây theo nguyên lý mid-first (chọn phần tử giữa làm root, nửa trái làm cây con trái, nửa phải làm cây con phải). **Chỉ gán lại con trỏ `left/right`**, không tạo mới hay hủy node nào.
5. Giải phóng mảng con trỏ tạm: `delete[] nodes;`.

*Hàm được tự động gọi trong `loadAllData()` ngay sau khi nạp xong danh sách môn.*

---

#### 5.6. `setExamConfigWorkflow()` *(Legacy — Đang chờ UI loại bỏ)*

```cpp
BusinessResult setExamConfigWorkflow(
    Subject*& root,
    const char* subjectId,
    int questionCount,
    int minutes
);
```

> **Lưu ý:** Đây là hàm cấu hình thi trên thực thể `Subject` cũ. Hiện tại hệ thống đã chuyển sang cấu hình thi theo từng lớp (`ExamConfigService.h`). Hàm này được giữ lại tạm thời để UI hiện tại có thể biên dịch và sẽ được gỡ bỏ hoàn toàn sau khi UI chuyển đổi xong.

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

#### 6.3. `deleteQuestionWorkflow()` — Xóa Câu Hỏi & Bảo Toàn Ràng Buộc (B05)

Hệ thống hỗ trợ 2 chữ ký:

```cpp
// 1. Chữ ký chuẩn mới (Khuyến nghị cho UI):
BusinessResult deleteQuestionWorkflow(
    Subject* root,
    Class* classes[],
    int classCount,
    const char subjectId[],
    int questionId
);

// 2. Chữ ký cũ (Tương thích ngược để UI cũ biên dịch):
BusinessResult deleteQuestionWorkflow(
    Subject* root,
    const char subjectId[],
    int questionId
);
```

**Chính sách nghiệp vụ (B05):**
1. Tìm môn học và câu hỏi. Nếu không thấy → `ERR_NOT_FOUND`.
2. **Kiểm tra ràng buộc điểm số:** Nếu môn học đã có sinh viên thi (`isSubjectUsedInScores`) → trả về `ERR_CONSTRAINT_VIOLATION` (chặn xóa).
3. **Kiểm tra ràng buộc cấu hình thi:** Nếu câu hỏi này đang nằm trong bộ đề đã bốc của bất kỳ lớp nào (`ExamConfig.questionIds`) → trả về `ERR_CONSTRAINT_VIOLATION` (chặn xóa).
4. Nếu vượt qua các ràng buộc an toàn, gọi `deleteQuestion(subject->questionList, questionId)` và trả về `SUCCESS`.

---

## 6A. ExamConfigService.h / ExamConfigService.cpp — Cấu Hình Thi Theo Lớp & Đề Cố Định (B02 / R2)

**Đường dẫn:** `business/ExamConfigService.h` + `business/ExamConfigService.cpp`

Đây là phân hệ quản lý cấu hình thi mới: mỗi cặp `(lớp, môn)` sở hữu một cấu hình thi độc lập, lưu trực tiếp trong danh sách `Class.configList`. Bộ câu hỏi được bốc ngẫu nhiên **một lần** duy nhất khi tạo cấu hình; toàn bộ sinh viên trong lớp thi chung bộ câu hỏi này.

### Các Hàm Nghiệp Vụ Chính

#### 1. `drawQuestionIds()` — Bốc ngẫu nhiên N câu hỏi không lặp
```cpp
int drawQuestionIds(Subject* subject, int count, int outIds[]);
```
- Sử dụng giải thuật Fisher-Yates shuffle trên mảng con trỏ tạm, khởi tạo `srand` một lần duy nhất qua `initializeRngOnce()`.
- Trả về số câu hỏi thực tế bốc được (`= count` nếu thành công).

#### 2. `setClassExamConfigWorkflow()` — Thiết lập / Cập nhật cấu hình thi của lớp
```cpp
BusinessResult setClassExamConfigWorkflow(
    Subject* root, Class* classes[], int classCount,
    const char classId[], const char subjectId[],
    int numberOfQuestions, int minutes
);
```
- **Ràng buộc:** $1 \le \text{numberOfQuestions} \le \text{tổng số câu của môn}$; $1 \le \text{minutes} \le 180$.
- **Freeze Policy:** Nếu trong lớp đã có bất kỳ sinh viên nào thi môn này (`classHasTakenSubject`) → lập tức trả về `ERR_CONSTRAINT_VIOLATION` (cấm sửa).
- **Hành vi linh hoạt:**
  - Nếu sửa số phút mà giữ nguyên số câu: cập nhật `minutes`, **giữ nguyên bộ đề cũ**.
  - Nếu sửa số câu: giải phóng bộ đề cũ, bốc ngẫu nhiên bộ đề mới.

#### 3. `deleteClassExamConfigWorkflow()` — Xóa cấu hình thi của lớp
```cpp
BusinessResult deleteClassExamConfigWorkflow(
    Class* classes[], int classCount,
    const char classId[], const char subjectId[]
);
```
- Nếu đã có sinh viên trong lớp thi môn này → trả về `ERR_CONSTRAINT_VIOLATION`.
- Nếu chưa thi → giải phóng cấu hình và thu hồi mảng `questionIds`.

#### 4. `findStudentClass()` & `classHasTakenSubject()` — Tra cứu & Kiểm tra Freeze
```cpp
Class* findStudentClass(Class* classes[], int classCount, const char studentId[]);
bool classHasTakenSubject(Class* classes[], int classCount, const char classId[], const char subjectId[]);
```

#### 5. `canStudentTakeSubject()` — Kiểm tra điều kiện làm bài thi
```cpp
BusinessResult canStudentTakeSubject(
    Subject* root, Class* classes[], int classCount,
    Student* student, const char subjectId[], const char** reasonOut
);
```
- Kiểm tra toàn diện: sinh viên có thuộc lớp không, lớp đã có cấu hình thi chưa, sinh viên đã thi môn này chưa, cấu hình có hợp lệ không, các câu hỏi trong cấu hình có còn tồn tại trong ngân hàng đề không.
- Nếu không thỏa mãn, trả về mã lỗi kèm con trỏ chuỗi lý do `reasonOut` (tiếng Việt có dấu).

#### 6. `prepareExamFromConfig()` — Tạo đề thi snapshot cho sinh viên
```cpp
BusinessResult prepareExamFromConfig(
    Subject* root, Class* classes[], int classCount,
    Student* student, const char subjectId[],
    ExamDetail*& examQuestionsHead
);
```
- Kiểm tra qua `canStudentTakeSubject`.
- Lần lượt tạo các node `ExamDetail` snapshot theo đúng thứ tự câu hỏi đã cấu hình cho lớp.
- Nếu gặp sự cố giữa chừng, tự động dọn dẹp qua `cancelExam()` chống rò rỉ bộ nhớ.

---

## 6B. BusinessQueries.h / BusinessQueries.cpp — Truy Vấn Dùng Chung (B04 / R4)

**Đường dẫn:** `business/BusinessQueries.h` + `business/BusinessQueries.cpp`

Gom các hàm tiện ích và quy chế thi dùng chung cho cả Console và GUI, loại bỏ triệt để các đoạn code trùng lặp:

```cpp
bool isPassingScore(float score);                                       // Trả về true nếu score >= 5.0f
int countStudentsInClass(const Class* classroom);                       // Đếm sĩ số lớp O(S)
int collectSubjectsInOrder(Subject* root, Subject* out[], int capacity); // Thu thập danh sách môn theo thứ tự bảng chữ cái
void freeAllData(Subject*& root, Class* classes[], int& classCount);    // Thu hồi toàn bộ dữ liệu khi thoát chương trình
```

---

## 6C. PipeCodec.h — Xử Lý Ký Tự Phân Cách Pipe Trong Dữ Liệu (B06)

**Đường dẫn:** `business/PipeCodec.h`

Để tránh hiện tượng lệch cột khi lưu trữ file với ký tự `|` (delimiter mặc định), module này cung cấp 2 hàm mã hóa / giải mã:

```cpp
inline std::string escapePipe(const char* value);
inline int splitEscapedPipe(const std::string& line, std::string fields[], int capacity);
```
- `escapePipe`: Chuyển đổi ký tự `|` thành `\|` và `\` thành `\\`.
- `splitEscapedPipe`: Tách chuỗi theo dấu `|`, bỏ qua các ký tự đã được escape (`\|`), và tự động unescape về chuỗi gốc ban đầu.
- Được sử dụng đồng bộ trong `FileStorage.cpp` (môn học, câu hỏi, điểm thi) và `ExamService.cpp` (phiên thi tạm).

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

**Hành vi nghiệp vụ & Giải phóng Cascade (Cập nhật v2.2.0):**
- **Cho phép xóa lớp kể cả khi lớp đang có sinh viên** (đã bãi bỏ kiểm tra `classroom->studentList != nullptr`).
- Tầng Core (`core/deleteClass`) tự động duyệt giải phóng dây chuyền (Cascade Cleanup):
  - Thu hồi toàn bộ danh sách liên kết sinh viên (`Student`) trong lớp.
  - Với mỗi sinh viên, thu hồi toàn bộ danh sách điểm (`Score`) và chi tiết bài thi snapshot (`ExamDetail`).
  - Giải phóng ô nhớ của `Class` và dồn mảng con trỏ `classes[]` về trước một nấc, giảm `classCount--`.
- **An toàn phía UI:** Hộp thoại xác nhận xóa của GUI hiển thị rõ sĩ số sinh viên của lớp để cảnh báo người dùng trước khi thực hiện.

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

#### 10.6. Các Tiện Ích Đếm Giờ Theo Giây & Định Dạng Đồng Hồ (B03 / R3)

Tầng Business cung cấp hệ thống hàm kiểm tra và định dạng thời gian chính xác theo giây:

```cpp
// 1. Tính số giây còn lại chính xác (chống trôi thời gian qua difftime)
int calculateExamRemainingSeconds(time_t startTime, int durationSeconds);

// 2. Kiểm tra hết giờ theo giây
bool isExamTimeExpiredSeconds(time_t startTime, int durationSeconds);

// 3. Định dạng chuỗi thời gian chi tiết: "MM:SS (M phut S giay)"
void formatExamTime(int remainingSeconds, char buffer[], int bufferSize);

// 4. Định dạng chuỗi đồng hồ điện tử chuẩn GUI/Console: "MM:SS" (vd: 65s -> "01:05")
void formatExamClock(int seconds, char out[], int size);
```

- **`calculateExamRemainingSeconds()`**: Tính `durationSeconds - difftime(time(nullptr), startTime)`. Nếu thời gian đã hết, hàm an toàn trả về `0`.
- **`formatExamClock()`**: Định dạng gọn dạng `MM:SS` dùng để render trực tiếp lên giao diện Raylib GUI hoặc Console UI.

---

#### 10.7. `struct ExamTimer` & Runtime Vòng Lặp Thi Trực Tuyến (`tickExamSession`)

Nhằm giúp tầng UI không phải tự quản lý biến thời gian rời rạc hay tự lưu file session mỗi giây (gây nghẽn I/O), Business cung cấp bộ công cụ runtime đồng bộ:

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

##### Quản lý nhịp thi & Tự động lưu phiên: `tickExamSession()`
```cpp
struct ExamSessionTracker {
    int lastSavedSecond = -1;
};

struct ExamTickResult {
    int remainingSeconds;
    bool expired;
    bool saved; // true nếu có đợt ghi đĩa thành công trong tick này
};

ExamTickResult tickExamSession(
    ExamTimer& timer,
    ExamSessionTracker& tracker,
    const char studentId[],
    const char subjectId[],
    ExamDetail* examHead,
    const char* filepath = nullptr
);
```
- UI chỉ cần gọi `tickExamSession()` trong mỗi frame vẽ màn hình.
- Hàm chỉ thực hiện ghi đĩa `saveExamSession()` khi **số giây thực tế thay đổi** (`remaining != tracker.lastSavedSecond`), loại bỏ hiện tượng spam I/O đĩa.
- Khi hết giờ (`isExpired() == true`), trả về `{0, true, false}` để UI chuyển sang bước nộp bài.

##### Các Workflow Kết Thúc Bài Thi & Chuyển Giao Quyền Sở Hữu Bộ Nhớ
```cpp
// 1. Nộp bài chủ động (khi sinh viên bấm nộp):
BusinessResult finishExamWorkflow(
    Subject* root, Class* classes[], int classCount, Student* student,
    const char subjectId[], ExamDetail*& head, float& score, int& correct,
    int& total, bool* saveFailedOut = nullptr, const char* dataDir = "data",
    const char* sessionFilepath = nullptr
);

// 2. Nộp bài tự động khi hết giờ:
BusinessResult submitExpiredExamWorkflow(
    Subject* root, Class* classes[], int classCount, Student* student,
    const char subjectId[], ExamDetail*& head, float& score, int& correct,
    int& total, bool* saveFailedOut = nullptr, const char* dataDir = "data",
    const char* sessionFilepath = nullptr
);

// 3. Hủy phiên thi dở dang (tự nạp session, chấm điểm dở dang và khóa thi lại):
BusinessResult cancelPendingExamWorkflow(
    Subject* root, Class* classes[], int classCount, Student* student,
    bool* saveFailedOut = nullptr, const char* dataDir = "data",
    const char* sessionFilepath = nullptr
);

// 4. Tiếp tục bài thi từ session:
BusinessResult resumeExamWorkflow(
    Subject* root, Class* classes[], int classCount, Student* student,
    ExamDetail*& head, char subjectOut[], int& remainingOut,
    const char* sessionFilepath = nullptr
);
```
- Các hàm trên tự động thực hiện trọn gói: **Chấm điểm → Ghi điểm vào ScoreList → Gọi saveAllData() → Tự xóa file session `temp_<MASV>.dat` → Đặt con trỏ `head = nullptr`**.
- UI không cần phải gọi các hàm lưu đĩa hay quản lý con trỏ riêng rẽ. Nếu việc ghi đĩa gặp sự cố, biến cờ `saveFailedOut = true` được bật để cảnh báo nhưng điểm số trong RAM vẫn được bảo toàn.

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

- **Save:** Duyệt BST theo **in-order** (trái → gốc → phải) → ghi thông tin danh mục môn học theo định dạng chuẩn 3 cột vào `subjects.txt`:
  ```text
  MAMH|TENMH|SO_CAU_HOI
  ```
  đồng thời ghi trực tiếp các câu hỏi của môn đó ngay sau dòng môn học (hoặc xuất ra file `<dir>/<MAMH>.txt`). Ký tự `|` và `\` trong tên và nội dung được tự động escape bằng `escapePipe()`.

- **Load (Tương thích ngược):** Đọc môn học từ `subjects.txt`. Parser hỗ trợ cả file 3 cột chuẩn mới và file 5 cột cũ (tự động bỏ qua 2 cột cấu hình cũ để chuyển giao quyền quản lý cho `ExamConfig`). Tự động kiểm tra tính duy nhất ID câu hỏi (`isQuestionIdUsedAnywhere`) và đồng bộ counter `updateQuestionIdCounter()`.

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

#### 12.4. `saveExamConfigsToFile()` / `loadExamConfigsFromFile()` (B02)

```cpp
bool saveExamConfigsToFile(Class* classes[], int classCount, const char* filepath);
bool loadExamConfigsFromFile(Class* classes[], int classCount, const char* filepath);
```

- **Định dạng file `data/exam_configs.txt`:**
  ```text
  <tổng số cấu hình>
  MALOP|MAMH|SO_CAU|PHUT|NGAY_TAO|id1,id2,id3,...
  ```
- **Load:** Nạp cấu hình vào `Class.configList`. Tự động bỏ qua dòng sai cú pháp hoặc ngày giờ không hợp lệ. Nếu file không tồn tại, hàm trả về `true` (không có cấu hình).

#### 12.5. `saveAllData()` / `loadAllData()` — Hàm Tổng Hợp

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

- **`saveAllData()`:** Tự tạo thư mục `data/` nếu chưa có. Ghi `subjects.txt`, các file câu hỏi `<MAMH>.txt`, `classes.txt` và `exam_configs.txt`.
- **`loadAllData()`:** Nạp toàn bộ dữ liệu từ thư mục chỉ định theo quy trình chuẩn:
  1. `loadSubjectsFromFile()`
  2. `loadClassesFromFile()`
  3. `loadExamConfigsFromFile()`
  4. `discardInvalidExamConfigs()` (dọn sạch các cấu hình chứa câu hỏi mồ côi đã bị xóa)
  5. `rebalanceSubjectBST(root)` (cân bằng lại cây nhị phân môn học)
  Trả `true` nếu dữ liệu cơ bản đọc thành công.

---

## 13. Bảng Tổng Hợp Tất Cả Hàm Nghiệp Vụ

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
| 9 | `Validation.cpp` | `validateStudentId()` | `BusinessResult` | Validate mã SV (whitelist ký tự chống path traversal) |
| 10 | `Validation.cpp` | `validateStudentName()` | `BusinessResult` | Validate họ tên |
| 11 | `Validation.cpp` | `validateGender()` | `BusinessResult` | Validate & chuẩn hóa phái |
| 12 | `Validation.cpp` | `validatePassword()` | `BusinessResult` | Validate mật khẩu |
| 13 | `Validation.cpp` | `validateQuestionContent()` | `BusinessResult` | Validate nội dung câu hỏi |
| 14 | `Validation.cpp` | `validateAnswerContent()` | `BusinessResult` | Validate nội dung đáp án |
| 15 | `Validation.cpp` | `validateCorrectAnswer()` | `BusinessResult` | Validate & chuẩn hóa đáp án đúng |
| 16 | `SubjectService.cpp` | `isSubjectUsedInScores()` | `bool` | Kiểm tra môn có được tham chiếu |
| 17 | `SubjectService.cpp` | `addSubjectWorkflow()` | `BusinessResult` | Thêm môn học |
| 18 | `SubjectService.cpp` | `editSubjectWorkflow()` | `BusinessResult` | Sửa môn học (đồng bộ sang ExamConfig) |
| 19 | `SubjectService.cpp` | `deleteSubjectWorkflow()` | `BusinessResult` | Xóa môn học (dọn dẹp ExamConfig) |
| 20 | `SubjectService.cpp` | `rebalanceSubjectBST()` | `void` | Tái cân bằng BST môn học sau nạp file |
| 21 | `QuestionService.cpp` | `addQuestionWorkflow()` | `BusinessResult` | Thêm câu hỏi (auto sinh ID) |
| 22 | `QuestionService.cpp` | `editQuestionWorkflow()` | `BusinessResult` | Sửa câu hỏi |
| 23 | `QuestionService.cpp` | `deleteQuestionWorkflow()` | `BusinessResult` | Xóa câu hỏi (kiểm tra điểm & config lớp) |
| 24 | `ClassService.cpp` | `addClassWorkflow()` | `BusinessResult` | Thêm lớp học |
| 25 | `ClassService.cpp` | `deleteClassWorkflow()` | `BusinessResult` | Xóa lớp học (cascade giải phóng SV) |
| 26 | `StudentService.cpp` | `addStudentWorkflow()` | `BusinessResult` | Thêm SV (kiểm tra toàn cục) |
| 27 | `StudentService.cpp` | `editStudentWorkflow()` | `BusinessResult` | Sửa SV |
| 28 | `StudentService.cpp` | `deleteStudentWorkflow()` | `BusinessResult` | Xóa SV |
| 29 | `AuthService.cpp` | `login()` | `LoginRole` | Đăng nhập (đơn giản) |
| 30 | `AuthService.cpp` | `loginWorkflow()` | `BusinessResult` | Đăng nhập (chi tiết lỗi) |
| 31 | `ExamConfigService.cpp` | `drawQuestionIds()` | `int` | Bốc ngẫu nhiên N câu hỏi không lặp |
| 32 | `ExamConfigService.cpp` | `setClassExamConfigWorkflow()` | `BusinessResult` | Tạo/sửa cấu hình thi theo lớp (Freeze guard) |
| 33 | `ExamConfigService.cpp` | `deleteClassExamConfigWorkflow()` | `BusinessResult` | Xóa cấu hình thi của lớp |
| 34 | `ExamConfigService.cpp` | `findStudentClass()` | `Class*` | Tra cứu con trỏ lớp của SV |
| 35 | `ExamConfigService.cpp` | `classHasTakenSubject()` | `bool` | Kiểm tra lớp đã có SV thi môn này chưa |
| 36 | `ExamConfigService.cpp` | `canStudentTakeSubject()` | `BusinessResult` | Kiểm tra điều kiện thi kèm chuỗi lý do |
| 37 | `ExamConfigService.cpp` | `prepareExamFromConfig()` | `BusinessResult` | Tạo đề thi snapshot cho SV từ config lớp |
| 38 | `BusinessQueries.cpp` | `isPassingScore()` | `bool` | Đánh giá Đạt/Không đạt (ngưỡng 5.0f) |
| 39 | `BusinessQueries.cpp` | `countStudentsInClass()` | `int` | Đếm sĩ số sinh viên của một lớp |
| 40 | `BusinessQueries.cpp` | `collectSubjectsInOrder()` | `int` | Thu thập mảng môn học theo thứ tự in-order |
| 41 | `BusinessQueries.cpp` | `freeAllData()` | `void` | Giải phóng toàn bộ bộ nhớ ứng dụng |
| 42 | `ExamService.cpp` | `calculateScore()` | `float` | Tính điểm trên thang 10 |
| 43 | `ExamService.cpp` | `calculateExamRemainingSeconds()` | `int` | Tính số giây còn lại chính xác |
| 44 | `ExamService.cpp` | `isExamTimeExpiredSeconds()` | `bool` | Kiểm tra hết giờ thi (theo giây) |
| 45 | `ExamService.cpp` | `formatExamClock()` | `void` | Định dạng chuỗi đồng hồ "MM:SS" |
| 46 | `ExamService.cpp` | `formatExamTime()` | `void` | Định dạng chuỗi "MM:SS (M phut S giay)" |
| 47 | `ExamService.h` | `ExamTimer` | `struct` | Bộ đếm thời gian hướng đối tượng theo giây |
| 48 | `ExamService.cpp` | `tickExamSession()` | `ExamTickResult` | Nhịp thi và tự động lưu session khi giây đổi |
| 49 | `ExamService.cpp` | `finishExamWorkflow()` | `BusinessResult` | Nộp bài thủ công, lưu file và xóa session |
| 50 | `ExamService.cpp` | `submitExpiredExamWorkflow()` | `BusinessResult` | Nộp bài khi hết giờ, lưu file và xóa session |
| 51 | `ExamService.cpp` | `cancelPendingExamWorkflow()` | `BusinessResult` | Hủy bài thi dở dang (tự nộp và khóa môn) |
| 52 | `ExamService.cpp` | `resumeExamWorkflow()` | `BusinessResult` | Khôi phục bài thi dở dang từ session |
| 53 | `ExamService.cpp` | `saveExamSession()` | `bool` | Lưu file session tạm (`temp_<MASV>.dat`) |
| 54 | `ExamService.cpp` | `checkExamSession()` | `bool` | Kiểm tra và trích xuất thông tin session |
| 55 | `ExamService.cpp` | `removeExamSession()` | `bool` | Xóa file session |
| 56 | `ExamService.cpp` | `loadExamSession()` | `bool` | Khôi phục danh sách `ExamDetail*` |
| 57 | `ReportService.cpp` | `hasStudentTakenExam()` | `bool` | Kiểm tra SV đã thi và lấy điểm |
| 58 | `ReportService.cpp` | `getStudentExamScore()` | `Score*` | Lấy con trỏ điểm của SV |
| 59 | `ReportService.cpp` | `getStudentExamDetails()` | `ExamDetail*` | Lấy snapshot bài thi đã làm |
| 60 | `ReportService.cpp` | `printClassScoreTable()` | `BusinessResult` | In bảng điểm lớp |
| 61 | `ReportService.cpp` | `printExamDetailReview()` | `BusinessResult` | In chi tiết bài thi |
| 62 | `ReportService.cpp` | `printStudentScores()` | `BusinessResult` | In bảng điểm cá nhân SV |
| 63 | `FileStorage.cpp` | `saveSubjectsToFile()` | `bool` | Ghi BST môn học & câu hỏi ra file (3 cột) |
| 64 | `FileStorage.cpp` | `loadSubjectsFromFile()` | `bool` | Nạp BST môn học & câu hỏi từ file |
| 65 | `FileStorage.cpp` | `saveClassesToFile()` | `bool` | Ghi mảng lớp học ra file |
| 66 | `FileStorage.cpp` | `loadClassesFromFile()` | `bool` | Nạp mảng lớp học từ file |
| 67 | `FileStorage.cpp` | `saveExamConfigsToFile()` | `bool` | Ghi cấu hình thi theo lớp (`exam_configs.txt`) |
| 68 | `FileStorage.cpp` | `loadExamConfigsFromFile()` | `bool` | Nạp cấu hình thi theo lớp |
| 69 | `FileStorage.cpp` | `saveAllData()` | `bool` | Lưu toàn bộ dữ liệu hệ thống |
| 70 | `FileStorage.cpp` | `loadAllData()` | `bool` | Nạp toàn bộ dữ liệu & tự động cân bằng cây |

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
| Xóa lớp có SV (Cascade Cleanup) | `core/deleteClass` | Giải phóng an toàn toàn bộ SV, điểm, bài thi |
| MASV duy nhất toàn hệ thống | `findStudentGlobal()` | Trả `ERR_DUPLICATE` |
| Mỗi SV chỉ thi mỗi môn 1 lần | `findScore(student, subjectId)` | Trả `ERR_CONSTRAINT_VIOLATION` |
| Cấu hình thi trong phạm vi hợp lệ | `setExamConfigWorkflow()` | Trả `ERR_CONSTRAINT_VIOLATION` nếu sai số câu hoặc số phút |

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

### 15.3. Xử Lý Khi Hết Giờ Hoặc Nộp Bài Thi (B03)

Khi đồng hồ hết giờ (`timer.isExpired() == true`) hoặc sinh viên bấm nộp bài:
- **Khuyến nghị chuẩn:** Gọi trực tiếp `finishExamWorkflow(...)` (khi nộp chủ động) hoặc `submitExpiredExamWorkflow(...)` (khi hết giờ).
- Hai workflow này tự động:
  1. Chấm điểm bài thi và lưu vào `ScoreList`.
  2. Ghi toàn bộ dữ liệu ra file thông qua `saveAllData()`.
  3. Xóa file session tạm `temp_<MASV>.dat`.
  4. Gán `examQuestionsHead = nullptr` để UI không cần quản lý con trỏ đã chuyển giao quyền sở hữu.
- **Nếu dùng hàm cấp thấp cũ (`submitExamWorkflow`):** UI bắt buộc phải gọi xóa session và lưu file thủ công, hoặc gọi `cancelExam(examQuestionsHead)` nếu không muốn ghi nhận điểm để tránh rò rỉ bộ nhớ.

### 15.4. Gọi `cancelExam()` Khi Sinh Viên Hủy Trước Khi Nộp
Nếu sinh viên thoát khỏi màn hình làm bài thi mà chưa từng lưu session và không muốn nộp bài dở dang, UI **bắt buộc** phải gọi:
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

### 15.6. Quy Trình Khôi Phục & Quản Lý Phiên Thi Chuẩn Mới (Runtime Loop)

Khi triển khai màn hình sinh viên đăng nhập và làm bài thi, UI nên sử dụng bộ API runtime hiện đại:

1. **Khi sinh viên đăng nhập:**
   Kiểm tra bài thi dở dang qua `checkExamSession()`:
   ```cpp
   char subId[SUBJECT_ID_LEN] = {};
   int remainingSeconds = 0;
   if (checkExamSession(loggedStudent->id, subId, remainingSeconds)) {
       // Hỏi sinh viên muốn tiếp tục hay hủy:
       // - Nếu tiếp tục:
       ExamDetail* examHead = nullptr;
       if (resumeExamWorkflow(root, classes, classCount, loggedStudent, examHead, subId, remainingSeconds) == SUCCESS) {
           ExamTimer timer;
           timer.resume(remainingSeconds);
           ExamSessionTracker tracker;
           // Vào vòng lặp thi...
       }
       // - Nếu hủy phiên dở dang (tự nộp điểm dở dang và khóa thi lại):
       //   cancelPendingExamWorkflow(root, classes, classCount, loggedStudent);
   }
   ```
2. **Trong vòng lặp thi (Frame loop / Tick):**
   Chỉ cần gọi `tickExamSession()` mỗi frame vẽ:
   ```cpp
   ExamTickResult tick = tickExamSession(timer, tracker, loggedStudent->id, subId, examHead);
   if (tick.expired) {
       // Hết giờ -> tự động nộp:
       float score; int correct, total;
       submitExpiredExamWorkflow(root, classes, classCount, loggedStudent, subId, examHead, score, correct, total);
   }
   ```
3. **Cấu hình thi theo lớp & Khóa thi lại (Retake Guard):**
   - Trước khi thi, kiểm tra điều kiện thi qua:
     ```cpp
     const char* reason = nullptr;
     BusinessResult res = canStudentTakeSubject(root, classes, classCount, student, subjectId, &reason);
     if (res != SUCCESS) {
         // Hiển thị reason ra modal hoặc toast thông báo
     }
     ```
   - Sinh viên đã thi môn nào sẽ bị khóa thi lại môn đó (`ERR_CONSTRAINT_VIOLATION`).

---

*Tài liệu được cập nhật đầy đủ và đồng bộ với toàn bộ tính năng của tầng Business Logic Layer (ExamConfigService, ExamService, BusinessQueries, FileStorage).*

