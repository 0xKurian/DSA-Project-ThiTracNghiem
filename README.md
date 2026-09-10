# BÁO CÁO TỔNG HỢP & BÀN GIAO CORE  
## Project Thi Trắc Nghiệm C++ — Data Structures & Algorithms

**Ngày tổng hợp:** 10/09/2026  
**Trạng thái:** `TASK 02 — CORE COMPLETE`  
**Mục đích tài liệu:** Dùng để ôn lại kiến thức, hiểu kiến trúc hệ thống, tiếp tục phát triển và bàn giao phần Core cho thành viên phụ trách Business Logic.

---

# 1. Tổng quan Project

Project xây dựng một chương trình **thi trắc nghiệm bằng C++**.

Theo yêu cầu gốc, chương trình quản lý:

- Môn học.
- Câu hỏi thi.
- Lớp học.
- Sinh viên.
- Điểm thi.
- Đăng nhập sinh viên / giảng viên.
- Thi trắc nghiệm theo môn.
- Chọn câu hỏi ngẫu nhiên.
- Xem lại chi tiết bài đã thi.
- In bảng điểm theo lớp và môn.
- Lưu / đọc dữ liệu từ file.
- Kiểm tra dữ liệu không hợp lệ.

Các cấu trúc dữ liệu chính theo đề:

```text
Subject        -> Binary Search Tree
Question       -> Singly Linked List
Class          -> Pointer Array, tối đa 10000 lớp
Student        -> Singly Linked List
Score          -> Singly Linked List
ExamHistory    -> Singly Linked List bổ sung
ExamDetail     -> Singly Linked List bổ sung
```

---

# 2. Phân công nhóm

```text
Người 1
└── UI / Giao diện

Người 2
└── Core C++ / Data Structures

Người 3
└── Business Logic / Chức năng
```

Phần được hoàn thành trong giai đoạn hiện tại là:

```text
Người 2 → Core C++ / Data Structures
```

Mục tiêu của Core là cung cấp các cấu trúc dữ liệu và thao tác cơ bản để UI và Business Logic có thể sử dụng mà không cần biết chi tiết cách quản lý con trỏ bên trong.

---

# 3. Các ràng buộc kỹ thuật đã thống nhất

Trong Core:

- Không dùng STL container:
  - `vector`
  - `list`
  - `map`
  - `set`
  - ...
- Không dùng các thuật toán có sẵn, ngoại trừ `swap`.
- Không dùng `std::string` cho dữ liệu chính.
- Dùng mảng ký tự `char[]`.
- Tự cài đặt:
  - xử lý chuỗi,
  - linked list,
  - BST,
  - tìm kiếm,
  - thêm,
  - xóa,
  - sửa.
- Dữ liệu động dùng:
  - `new`
  - `delete`
- Core không chứa luồng UI.
- Core không nên chứa validation nghiệp vụ phức tạp.
- Business Logic chịu trách nhiệm kiểm tra dữ liệu nhập và điều phối các module.

---

# 4. Cấu trúc thư mục Core hiện tại

```text
DSA/
├── main.cpp / test.cpp
└── core/
    ├── Constants.h
    ├── Structures.h
    ├── FunctionPrototype.h
    │
    ├── StringUtils.h
    ├── StringUtils.cpp
    │
    ├── QuestionList.h
    ├── QuestionList.cpp
    │
    ├── SubjectBST.h
    ├── SubjectBST.cpp
    │
    ├── StudentList.h
    ├── StudentList.cpp
    │
    ├── ScoreList.h
    ├── ScoreList.cpp
    │
    ├── ClassArray.h
    └── ClassArray.cpp
```

Các module Business Logic chưa nằm trong Core hiện tại.

---

# 5. Constants

Các giới hạn đang sử dụng:

```cpp
const int MAX_CLASS = 10000;

const int SUBJECT_ID_LEN = 16;
const int SUBJECT_NAME_LEN = 100;

const int CLASS_ID_LEN = 20;
const int CLASS_NAME_LEN = 100;

const int STUDENT_ID_LEN = 20;
const int HO_LEN = 50;
const int TEN_LEN = 30;
const int GENDER_LEN = 10;
const int PASSWORD_LEN = 50;

const int QUESTION_CONTENT_LEN = 500;
const int ANSWER_CONTENT_LEN = 200;
```

## 5.1 Vì sao `SUBJECT_ID_LEN = 16`?

Đề yêu cầu:

```text
MAMH (C15)
```

Nếu chuỗi có tối đa 15 ký tự thì cần thêm một ô cho ký tự kết thúc:

```cpp
'\0'
```

Do đó:

```text
15 ký tự dữ liệu + 1 ký tự '\0' = 16
```

---

# 6. Kiến trúc dữ liệu tổng thể

```text
Subject BST
│
├── Subject
│    └── Question Linked List
│         ├── Question
│         ├── Question
│         └── ...
│
└── ...

Class Pointer Array
│
├── Class
│    └── Student Linked List
│         ├── Student
│         │    ├── Score Linked List
│         │    └── ExamHistory Linked List
│         │          └── ExamDetail Linked List
│         └── ...
│
└── ...
```

Quan hệ ownership hiện tại:

```text
Subject
└── owns Question list

Class
└── owns Student list

Student
├── owns Score list
└── owns ExamHistory list

ExamHistory
└── owns ExamDetail list
```

Quy tắc quan trọng:

> Đối tượng sở hữu dữ liệu con phải chịu trách nhiệm giải phóng dữ liệu con trước khi bản thân nó bị `delete`.

---

# 7. Structures.h

## 7.1 Question

```cpp
struct Question {
    int id;

    char content[QUESTION_CONTENT_LEN];

    char answerA[ANSWER_CONTENT_LEN];
    char answerB[ANSWER_CONTENT_LEN];
    char answerC[ANSWER_CONTENT_LEN];
    char answerD[ANSWER_CONTENT_LEN];

    char correctAnswer;

    Question* next;
};
```

Dùng cho:

```text
Question Linked List
```

Ví dụ:

```text
Q1 -> Q2 -> Q3 -> nullptr
```

---

## 7.2 Subject

```cpp
struct Subject {
    char id[SUBJECT_ID_LEN];
    char name[SUBJECT_NAME_LEN];

    Question* questionList;

    Subject* left;
    Subject* right;
};
```

Mỗi môn học là một node trong BST.

Khóa BST:

```text
Subject.id / MAMH
```

Mỗi môn sở hữu một danh sách câu hỏi riêng:

```text
Subject
└── questionList
```

---

## 7.3 Score

```cpp
struct Score {
    char subjectId[SUBJECT_ID_LEN];
    float score;

    Score* next;
};
```

Ví dụ:

```text
CTDL: 8.5
   ↓
AI: 9.25
   ↓
TRR: 9.0
   ↓
nullptr
```

---

## 7.4 ExamDetail

```cpp
struct ExamDetail {
    int questionId;

    char content[QUESTION_CONTENT_LEN];

    char answerA[ANSWER_CONTENT_LEN];
    char answerB[ANSWER_CONTENT_LEN];
    char answerC[ANSWER_CONTENT_LEN];
    char answerD[ANSWER_CONTENT_LEN];

    char correctAnswer;
    char selectedAnswer;

    ExamDetail* next;
};
```

`ExamDetail` lưu **snapshot đầy đủ** của câu hỏi tại thời điểm sinh viên thi.

Điều này quan trọng vì câu hỏi gốc có thể:

- bị sửa,
- bị xóa,
- thay đổi đáp án.

Lịch sử thi vẫn cần hiển thị chính xác nội dung mà sinh viên đã làm lúc đó.

---

## 7.5 ExamHistory

```cpp
struct ExamHistory {
    char subjectId[SUBJECT_ID_LEN];

    int numberOfQuestions;
    float score;

    ExamDetail* detailList;

    ExamHistory* next;
};
```

Một sinh viên có thể có danh sách lịch sử thi:

```text
ExamHistory 1
     ↓
ExamHistory 2
     ↓
ExamHistory 3
     ↓
nullptr
```

Module thao tác ExamHistory chưa được xây dựng trong Task 02 hiện tại.

---

## 7.6 Student

```cpp
struct Student {
    char id[STUDENT_ID_LEN];

    char ho[HO_LEN];
    char ten[TEN_LEN];

    char gender[GENDER_LEN];
    char password[PASSWORD_LEN];

    Score* scoreList;
    ExamHistory* examHistoryList;

    Student* next;
};
```

Một Student vừa tạo:

```text
Student
├── id
├── ho
├── ten
├── gender
├── password
├── scoreList = nullptr
├── examHistoryList = nullptr
└── next = nullptr
```

---

## 7.7 Class

```cpp
struct Class {
    char id[CLASS_ID_LEN];
    char name[CLASS_NAME_LEN];

    Student* studentList;
};
```

Các lớp được lưu bằng:

```cpp
Class* classes[MAX_CLASS];
int classCount;
```

---

## 7.8 LoginRole

```cpp
enum LoginRole {
    LOGIN_FAILED,
    LOGIN_TEACHER,
    LOGIN_STUDENT
};
```

Dùng cho module đăng nhập sau này.

---

# 8. StringUtils

Files:

```text
core/StringUtils.h
core/StringUtils.cpp
```

Các hàm:

```cpp
int stringLength(const char str[]);

void stringCopy(
    char destination[],
    const char source[]
);

int stringCompare(
    const char a[],
    const char b[]
);

bool stringEqual(
    const char a[],
    const char b[]
);
```

---

## 8.1 `stringLength`

Tương đương ý tưởng của:

```cpp
strlen()
```

nhưng tự cài đặt.

Thuật toán:

```text
length = 0

while str[length] != '\0'
    length++

return length
```

Độ phức tạp:

```text
Time:  O(n)
Space: O(1)
```

---

## 8.2 `stringCopy`

Tương đương ý tưởng của:

```cpp
strcpy()
```

nhưng tự cài đặt.

Quan trọng:

```cpp
destination[i] = '\0';
```

phải được đặt sau khi copy xong.

### Cảnh báo

Hàm hiện tại **không biết kích thước của destination**.

Do đó:

```cpp
stringCopy(destination, source);
```

chỉ an toàn khi:

```text
length(source) < capacity(destination)
```

Validation độ dài phải được thực hiện ở Business Logic / Input layer.

---

## 8.3 `stringCompare`

Tương đương ý tưởng của:

```cpp
strcmp()
```

Quy ước:

```text
< 0  → a < b
= 0  → a == b
> 0  → a > b
```

Đây là hàm rất quan trọng đối với BST.

---

## 8.4 `stringEqual`

```cpp
return stringCompare(a, b) == 0;
```

Dùng nhiều trong:

- tìm sinh viên,
- tìm điểm,
- tìm lớp,
- kiểm tra duplicate.

---

# 9. Question Linked List

Files:

```text
core/QuestionList.h
core/QuestionList.cpp
```

API:

```cpp
Question* createQuestion(...);

void insertQuestion(
    Question*& head,
    Question* question
);

Question* findQuestion(
    Question* head,
    int id
);

bool deleteQuestion(
    Question*& head,
    int id
);

int countQuestions(
    Question* head
);

void printQuestions(
    Question* head
);

int generateQuestionId();

void updateQuestionIdCounter(
    int id
);
```

---

## 9.1 `createQuestion`

Cấp phát:

```cpp
Question* question = new Question;
```

Sau đó:

```text
gán ID
copy content
copy A/B/C/D
gán correctAnswer
next = nullptr
```

---

## 9.2 `insertQuestion`

Hiện tại insert vào **cuối linked list**.

```text
head
 ↓
Q1 -> Q2 -> Q3 -> nullptr
```

Thêm Q4:

```text
Q1 -> Q2 -> Q3 -> Q4 -> nullptr
```

Độ phức tạp hiện tại:

```text
Time: O(n)
Space: O(1)
```

vì không lưu tail.

---

## 9.3 `findQuestion`

Tìm tuyến tính theo:

```text
Question.id
```

Độ phức tạp:

```text
O(n)
```

---

## 9.4 `countQuestions`

Duyệt toàn bộ list:

```text
O(n)
```

---

## 9.5 `deleteQuestion`

Đã xử lý:

```text
list rỗng
xóa node đầu
xóa node giữa
xóa node cuối
ID không tồn tại
```

Ý tưởng xóa node giữa:

```text
current
   |
   v
  Q1 -> Q2 -> Q3
        ^
       temp
```

thành:

```text
Q1 ------> Q3
```

rồi:

```cpp
delete temp;
```

---

# 10. Global Question ID

Yêu cầu:

> ID câu hỏi phải tự động tăng và không trùng trên tất cả các môn học.

Core dùng:

```cpp
static int nextQuestionId = 1;
```

Tạo ID mới:

```cpp
int generateQuestionId() {
    return nextQuestionId++;
}
```

Khi người dùng tạo câu hỏi mới:

```cpp
createQuestion(
    generateQuestionId(),
    ...
);
```

Không được cho người dùng nhập ID thủ công.

---

## 10.1 Vì sao `createQuestion()` vẫn nhận `int id`?

Điều này có chủ ý.

Cần thiết cho:

```text
1. Unit test.
2. Load dữ liệu từ file.
```

Ví dụ file đã lưu:

```text
ID = 100
```

Ta phải khôi phục đúng ID 100 thay vì sinh ID mới.

---

## 10.2 `updateQuestionIdCounter`

Khi load câu hỏi:

```cpp
updateQuestionIdCounter(question->id);
```

Nếu ID lớn nhất đã đọc là:

```text
100
```

thì:

```text
nextQuestionId = 101
```

Câu hỏi mới tiếp theo không bị trùng.

---

# 11. Subject Binary Search Tree

Files:

```text
core/SubjectBST.h
core/SubjectBST.cpp
```

API:

```cpp
Subject* createSubject(...);

Subject* insertSubject(
    Subject* root,
    Subject* subject
);

Subject* findSubject(
    Subject* root,
    const char id[]
);

Subject* deleteSubject(
    Subject* root,
    const char id[]
);

bool editSubject(
    Subject*& root,
    const char oldId[],
    const char newId[],
    const char newName[]
);

void printSubjects(
    Subject* root
);
```

---

# 12. Kiến thức BST dùng trong Project

Khóa:

```text
MAMH
```

Quy tắc:

```text
id < root.id
    → left

id > root.id
    → right

id == root.id
    → duplicate
```

Ví dụ:

```text
        CTDL
       /    \
    CSDL     OOP
               \
                TRR
```

---

## 12.1 `insertSubject`

Dùng đệ quy.

Trung bình:

```text
O(h)
```

Trong đó:

```text
h = chiều cao cây
```

BST cân bằng tương đối:

```text
O(log n)
```

Trường hợp cây bị lệch:

```text
O(n)
```

---

## 12.2 Duplicate Subject

`insertSubject()` không chèn khi ID bằng nhau.

Ví dụ nguy hiểm:

```cpp
root = insertSubject(
    root,
    createSubject("CTDL", "...")
);
```

nếu `CTDL` đã tồn tại.

Node vừa tạo bằng `new` có thể không được đưa vào cây, dẫn tới leak nếu caller không `delete`.

Luồng Business Logic nên là:

```cpp
if (findSubject(root, id) != nullptr) {
    // Bao trung MAMH
}
else {
    root = insertSubject(
        root,
        createSubject(id, name)
    );
}
```

---

## 12.3 `findSubject`

Dùng tính chất BST.

```text
id == root → found
id < root  → left
id > root  → right
```

Không cần duyệt toàn bộ cây trong trường hợp thông thường.

---

## 12.4 `printSubjects`

Dùng:

```text
In-order traversal
Left → Root → Right
```

Do đó danh sách được in tăng dần theo `MAMH`.

---

# 13. Xóa node trong BST

Có 3 trường hợp.

## 13.1 Không có con trái

```text
root
  \
   right
```

Trả:

```text
right
```

lên thay root.

---

## 13.2 Không có con phải

```text
    root
    /
 left
```

Trả:

```text
left
```

lên thay.

---

## 13.3 Có hai con

Dùng:

```cpp
findMinSubject(root->right);
```

để lấy:

```text
in-order successor
```

tức node nhỏ nhất của cây con phải.

---

# 14. Ownership đặc biệt khi xóa Subject có hai con

Đây là một phần dễ gây bug.

Mỗi Subject sở hữu:

```cpp
Question* questionList;
```

Khi dùng successor để thay root, ta phải chuyển ownership:

```cpp
root->questionList =
    successor->questionList;

successor->questionList = nullptr;
```

Dòng:

```cpp
successor->questionList = nullptr;
```

rất quan trọng.

Nếu không:

```text
root nhận questionList của successor

sau đó successor bị delete
        ↓
questionList cũng bị delete
        ↓
root giữ pointer tới vùng nhớ đã giải phóng
```

Kết quả:

```text
dangling pointer
double delete
undefined behavior
```

---

# 15. `editSubject`

Có hai trường hợp.

## 15.1 Không đổi MAMH

Ví dụ:

```text
CTDL -> CTDL
```

Chỉ cần đổi tên.

---

## 15.2 Đổi MAMH

Không được:

```cpp
stringCopy(subject->id, newId);
```

trực tiếp.

Ví dụ cây:

```text
        CTDL
       /    \
    CSDL     OOP
```

Nếu sửa trực tiếp:

```text
OOP → AI
```

ta có:

```text
        CTDL
       /    \
    CSDL     AI
```

nhưng:

```text
AI < CTDL
```

nên BST invariant bị phá.

### Cách đúng

```text
1. Tìm Subject cũ.
2. Kiểm tra newId có trùng không.
3. Nếu oldId == newId → chỉ đổi tên.
4. Tạo Subject mới.
5. Chuyển questionList sang Subject mới.
6. subject cũ -> questionList = nullptr.
7. Xóa node cũ.
8. Insert node mới vào lại BST.
```

---

# 16. Student Linked List

Files:

```text
core/StudentList.h
core/StudentList.cpp
```

API:

```cpp
Student* createStudent(...);

Student* findStudent(
    Student* head,
    const char id[]
);

bool addStudent(
    Class* classroom,
    Student* student
);

bool deleteStudent(
    Class* classroom,
    const char studentId[]
);

void printStudents(
    Class* classroom
);

bool editStudent(
    Student* student,
    ...
);
```

---

## 16.1 `createStudent`

Khởi tạo:

```text
scoreList = nullptr
examHistoryList = nullptr
next = nullptr
```

---

## 16.2 `findStudent`

Chỉ tìm trong **một linked list sinh viên**.

```text
Class A
└── SV001 -> SV002 -> SV003
```

Độ phức tạp:

```text
O(number of students in class)
```

---

## 16.3 `addStudent`

Thêm cuối danh sách.

Kiểm tra duplicate hiện tại chỉ trong:

```text
classroom->studentList
```

Do đó Core function này **không đảm bảo MASV unique toàn hệ thống**.

Business Logic phải kiểm tra `findStudentGlobal()` trước.

---

## 16.4 `editStudent`

Hiện tại cho sửa:

```text
HO
TEN
PHAI / gender
password
```

Không sửa:

```text
MASV / id
```

Điều này giúp tránh phá các liên kết nghiệp vụ như:

- đăng nhập,
- lịch sử thi,
- điểm,
- uniqueness.

---

# 17. Cleanup khi xóa Student

Student sở hữu:

```text
Score list
ExamHistory list
```

ExamHistory lại sở hữu:

```text
ExamDetail list
```

Do đó trước khi:

```cpp
delete student;
```

phải cleanup theo thứ tự:

```text
Student
│
├── delete Score list
│
└── delete ExamHistory
        │
        └── delete ExamDetail
│
└── delete Student
```

`deleteStudent()` hiện đã thực hiện cleanup này ở cả:

```text
xóa node đầu
xóa node giữa / cuối
```

---

# 18. Score Linked List

Files:

```text
core/ScoreList.h
core/ScoreList.cpp
```

API:

```cpp
Score* createScore(
    const char subjectId[],
    float score
);

Score* findScore(
    Student* student,
    const char subjectId[]
);

bool addScore(
    Student* student,
    const char subjectId[],
    float score
);

bool updateScore(
    Student* student,
    const char subjectId[],
    float score
);

void printScores(
    Student* student
);

bool deleteScore(
    Student* student,
    const char subjectId[]
);
```

---

## 18.1 `createScore`

Khởi tạo:

```text
subjectId
score
next = nullptr
```

---

## 18.2 `findScore`

Tìm theo:

```text
subjectId
```

trong score list của một sinh viên.

---

## 18.3 `addScore`

Không cho một Student có hai Score node cùng `subjectId`.

Ví dụ:

```text
CTDL - 8.5
AI   - 9.0
```

Thêm tiếp:

```text
CTDL - 10
```

sẽ trả:

```cpp
false
```

### Ý nghĩa thiết kế hiện tại

Core đang giả định:

```text
mỗi sinh viên có tối đa một Score record cho mỗi môn
```

Nếu sau này project cho phép thi lại, Business Logic cần quyết định:

```text
ghi đè điểm cũ
hay
giữ tất cả lần thi trong ExamHistory
```

Core hiện có `updateScore()` để cập nhật Score tổng quát và `ExamHistory` để có thể giữ chi tiết từng lần thi.

---

## 18.4 `updateScore`

Tìm Score hiện có rồi thay:

```cpp
found->score = score;
```

Không tạo Score mới nếu môn chưa tồn tại.

---

## 18.5 `deleteScore`

Xử lý:

```text
student == nullptr
list rỗng
node đầu
node giữa
node cuối
subjectId không tồn tại
```

---

## 18.6 Validation điểm

Core hiện không ép:

```text
0 <= score <= 10
```

Đây nên là trách nhiệm của Business Logic.

Ví dụ:

```cpp
if (score < 0 || score > 10) {
    // Bao loi
}
```

---

# 19. Class Pointer Array

Files:

```text
core/ClassArray.h
core/ClassArray.cpp
```

Dữ liệu:

```cpp
Class* classes[MAX_CLASS];
int classCount;
```

với:

```cpp
MAX_CLASS = 10000;
```

API:

```cpp
Class* createClass(...);

Class* findClass(
    Class* classes[],
    int classCount,
    const char id[]
);

bool addClass(
    Class* classes[],
    int& classCount,
    Class* newClass
);

bool deleteClass(
    Class* classes[],
    int& classCount,
    const char id[]
);

void printClasses(
    Class* classes[],
    int classCount
);

Student* findStudentGlobal(
    Class* classes[],
    int classCount,
    const char id[]
);
```

---

# 20. `findClass`

Duyệt:

```text
classes[0]
classes[1]
...
classes[classCount - 1]
```

Độ phức tạp:

```text
O(classCount)
```

---

# 21. `addClass`

Kiểm tra:

```text
newClass != nullptr
classCount < MAX_CLASS
MALOP chưa tồn tại
```

Sau đó:

```cpp
classes[classCount] = newClass;
++classCount;
```

---

# 22. `deleteClass`

Khi xóa một lớp trong mảng, các phần tử phía sau phải được dịch trái.

Trước:

```text
[0] A
[1] B
[2] C
[3] D
```

Xóa B:

```text
[0] A
[1] C
[2] D
```

và:

```cpp
--classCount;
classes[classCount] = nullptr;
```

---

# 23. Deep cleanup khi xóa Class

Class sở hữu Student list.

Mỗi Student sở hữu Score và ExamHistory.

Do đó khi xóa Class:

```text
Class
  ↓
Student list
  ↓
mỗi Student
  ├── Score list
  └── ExamHistory list
        └── ExamDetail list
  ↓
delete Student
  ↓
delete Class
```

`ClassArray.cpp` hiện có helper nội bộ để thực hiện cleanup sâu.

---

# 24. `findStudentGlobal`

Đây là hàm quan trọng để bảo đảm:

```text
MASV unique toàn hệ thống
```

Thuật toán:

```text
for mỗi Class
    gọi findStudent(class.studentList, id)

    nếu found
        return Student*

return nullptr
```

Độ phức tạp tổng quát:

```text
O(tổng số sinh viên toàn hệ thống)
```

---

# 25. Quy tắc thêm Student đúng ở Business Logic

Không nên chỉ:

```cpp
addStudent(classroom, student);
```

vì hàm này chỉ kiểm tra trùng trong một lớp.

Luồng đúng:

```cpp
if (
    findStudentGlobal(
        classes,
        classCount,
        studentId
    ) != nullptr
) {
    // MASV da ton tai tren he thong
}
else {
    Student* student = createStudent(...);

    if (!addStudent(classroom, student)) {
        delete student;
    }
}
```

---

# 26. Ownership & Memory Management

Đây là phần quan trọng nhất khi tiếp tục project.

## 26.1 Ownership tree

```text
Subject
└── Question list

Class
└── Student list
    ├── Score list
    └── ExamHistory list
        └── ExamDetail list
```

---

## 26.2 Quy tắc `new/delete`

Mỗi:

```cpp
new T
```

cuối cùng phải có đúng một:

```cpp
delete
```

Không được:

```text
0 lần delete  → memory leak
2 lần delete  → double free
```

---

## 26.3 Dangling Pointer

Ví dụ sai:

```cpp
delete student;

printScores(student);
```

Sau `delete`, pointer không còn trỏ tới object hợp lệ.

Không được sử dụng lại.

---

## 26.4 Use-after-free

Sai:

```cpp
delete student;

findScore(student, "CTDL");
```

Đây là:

```text
undefined behavior
```

Cleanup luôn phải đặt **sau lần sử dụng cuối cùng**.

---

## 26.5 Ownership transfer

Khi chuyển pointer từ object A sang B:

```cpp
B->data = A->data;
```

nếu ownership cũng chuyển sang B thì nên:

```cpp
A->data = nullptr;
```

để tránh A giải phóng dữ liệu mà B đang sử dụng.

Ví dụ đã dùng trong `editSubject()` và `deleteSubject()`.

---

# 27. `static` helper trong `.cpp`

Ví dụ:

```cpp
static void deleteQuestionList(...);
```

Ý nghĩa trong trường hợp project này:

```text
helper chỉ dùng nội bộ file .cpp
không cần expose ra header
```

Lợi ích:

- giảm API public,
- tránh module khác phụ thuộc implementation detail,
- dễ kiểm soát ownership.

---

# 28. Các invariant cần luôn giữ

## 28.1 Subject BST invariant

Với mọi node:

```text
mọi id trong cây trái < node.id
mọi id trong cây phải > node.id
```

---

## 28.2 Class Array invariant

Các phần tử hợp lệ nằm liên tục trong:

```text
classes[0 .. classCount - 1]
```

Không để hole ở giữa.

---

## 28.3 Student uniqueness invariant

Nghiệp vụ cần bảo đảm:

```text
MASV unique toàn hệ thống
```

Dùng:

```cpp
findStudentGlobal()
```

---

## 28.4 Question ID invariant

```text
Question.id unique trên tất cả Subject
```

Câu mới dùng:

```cpp
generateQuestionId()
```

---

## 28.5 Score invariant hiện tại

Một Student có tối đa:

```text
1 Score / subjectId
```

---

# 29. Các độ phức tạp chính

| Thao tác | Độ phức tạp |
|---|---:|
| `stringLength` | `O(n)` |
| `stringCopy` | `O(n)` |
| `stringCompare` | `O(n)` |
| `insertQuestion` | `O(n)` |
| `findQuestion` | `O(n)` |
| `deleteQuestion` | `O(n)` |
| `countQuestions` | `O(n)` |
| `insertSubject` | `O(h)` |
| `findSubject` | `O(h)` |
| `deleteSubject` | `O(h)` về tìm vị trí, cộng cleanup question list nếu có |
| `findStudent` | `O(n)` |
| `addStudent` | `O(n)` |
| `deleteStudent` | `O(n)` cộng cleanup dữ liệu con |
| `findScore` | `O(n)` |
| `addScore` | `O(n)` |
| `updateScore` | `O(n)` |
| `deleteScore` | `O(n)` |
| `findClass` | `O(number of classes)` |
| `addClass` | `O(number of classes)` do kiểm tra duplicate |
| `deleteClass` | `O(number of classes + dữ liệu con)` |
| `findStudentGlobal` | `O(total students)` |

Trong BST:

```text
h = chiều cao cây
```

Nếu BST lệch hoàn toàn:

```text
h = n
```

Core hiện không tự cân bằng cây.

---

# 30. FunctionPrototype.h

File này hiện chứa prototype tổng hợp cho:

```text
QUESTION
SUBJECT
CLASS
STUDENT
SCORE
AUTH
EXAM
FILE
```

Phần Question / Subject / Class / Student / Score đã có implementation trong các module tương ứng.

Các prototype sau vẫn thuộc phần phát triển tiếp:

```cpp
LoginRole login(...);

bool canTakeExam(...);

float calculateScore(...);

void loadAllData();
void saveAllData();
```

Không nên coi các chức năng này là hoàn thành chỉ vì đã có prototype.

---

# 31. Lưu ý về tổ chức Header

Hiện project có cả:

```text
FunctionPrototype.h
```

và các module header:

```text
QuestionList.h
SubjectBST.h
StudentList.h
ScoreList.h
ClassArray.h
```

Điều này tạo ra declaration bị lặp.

Ví dụ một function có thể được khai báo ở:

```text
FunctionPrototype.h
và
ScoreList.h
```

C++ cho phép declaration giống nhau, nhưng về bảo trì có rủi ro:

```text
sửa một header
quên sửa header còn lại
→ API không đồng bộ
```

Khi refactor, nên chọn một hướng:

```text
A. Module headers là nguồn chính;
   FunctionPrototype.h chỉ include module headers.

hoặc

B. Bỏ FunctionPrototype.h nếu không thật sự cần.
```

Không bắt buộc sửa ngay trước khi Business Logic bắt đầu, nhưng người nhận bàn giao cần biết điểm này.

---

# 32. Một điểm tổ chức cần chú ý: `findStudentGlobal`

`findStudentGlobal()` có liên quan đến cả:

```text
Student
Class Array
```

Implementation hiện nằm trong:

```text
ClassArray.cpp
```

vì nó cần duyệt qua toàn bộ mảng Class.

Nếu declaration xuất hiện ở nhiều header, nên thống nhất trong lần refactor tiếp theo để API dễ hiểu hơn.

Về mặt logic, implementation hiện tại hoạt động đúng.

---

# 33. Core Integration Test đã đạt

Integration Test đã kiểm tra luồng:

```text
Subject BST
    ↓
Question List
    ↓
Global Question ID

Class Array
    ↓
Student List
    ↓
Global Student Search
    ↓
Score List

Edit
Delete
Deep Cleanup
```

Kết quả cuối:

```text
=== CORE INTEGRATION TEST PASSED ===
```

Các case đã được kiểm tra:

- Insert nhiều Subject.
- In-order BST.
- Tạo Question ở nhiều Subject.
- Global Question ID.
- Count questions.
- Đổi `OOP -> AI`.
- Question list vẫn được giữ sau edit Subject.
- Tạo nhiều Class.
- Thêm Student vào nhiều lớp.
- Global Student Search.
- Chống duplicate MASV ở Business-style flow.
- Add Score.
- Update Score.
- Find Score.
- Delete Question.
- Delete Student đang có Score.
- Tìm lại Student sau delete.
- Delete Class đang có Student và Score.
- Deep cleanup.
- Cleanup Class Array.
- Cleanup Subject BST.

---

# 34. Trạng thái Task 02

```text
✅ 02.1 String Utilities
✅ 02.2 Question Linked List
✅ 02.3 Subject BST
✅ 02.4 Student Linked List
✅ 02.5 Score Linked List
✅ 02.6 Class Pointer Array
✅ 02.7 Core Integration Test

TASK 02 — CORE COMPLETE
```

---

# 35. Cách compile project

Đứng ở:

```text
DSA/
```

Compile test:

```bash
g++ test.cpp core/*.cpp -o main
```

Chạy:

```bash
./main
```

Nên dùng compiler warning đầy đủ:

```bash
g++ -std=c++17 \
    -Wall \
    -Wextra \
    -pedantic \
    test.cpp \
    core/*.cpp \
    -o main
```

Mục tiêu:

```text
0 error
0 warning
```

---

# 36. Lỗi đã từng gặp & bài học

## 36.1 Linker error — `undefined reference`

Sai:

```bash
g++ main.cpp -o main
```

Nếu `main.cpp` gọi function nằm trong `core/*.cpp`, linker không có implementation.

Đúng:

```bash
g++ main.cpp core/*.cpp -o main
```

---

## 36.2 Compile sai thư mục

Nếu terminal đang ở:

```text
DSA/core/
```

mà chạy:

```bash
g++ main.cpp core/*.cpp -o main
```

thì path sai.

Nên compile từ:

```text
DSA/
```

---

## 36.3 Redeclare biến trong cùng scope

Sai:

```cpp
Student* current = ...;

// ...

Student* current = ...;
```

Có thể sửa bằng:

```cpp
current = ...;
```

hoặc tên biến khác.

---

## 36.4 Duplicate helper

Một helper `static` không được định nghĩa hai lần trong cùng `.cpp`.

Nếu không sẽ gặp:

```text
redefinition
```

---

## 36.5 Missing helper

Nếu function gọi:

```cpp
deleteQuestionList(...)
```

thì helper đó phải được khai báo / định nghĩa đúng trước khi dùng.

---

## 36.6 Cleanup quá sớm

Sai:

```cpp
delete student;

// tiếp tục dùng student
```

Đây là use-after-free.

---

## 36.7 Tưởng output thứ tự là bug

Linked list giữ thứ tự insert.

Nếu insert:

```text
SV003
SV001
SV002
```

thì output đúng cũng là:

```text
SV003
SV001
SV002
```

Không được kỳ vọng list tự sort nếu code không có logic sort.

---

# 37. Những việc chưa thuộc Core hoặc chưa hoàn thành

Theo yêu cầu Project, các chức năng sau cần được thực hiện ở giai đoạn tiếp theo:

```text
1. Đăng nhập.
2. Nhập lớp.
3. In danh sách lớp ở luồng chương trình.
4. Nhập sinh viên theo mã lớp.
5. Quản lý môn học: thêm / xóa / hiệu chỉnh.
6. Nhập câu hỏi.
7. Thi trắc nghiệm.
8. Chọn câu hỏi ngẫu nhiên.
9. Giới hạn thời gian thi.
10. Tính điểm.
11. Ghi Score.
12. Ghi ExamHistory.
13. Lưu ExamDetail.
14. In chi tiết bài đã thi.
15. In bảng điểm lớp theo môn.
16. Hiển thị "Chưa thi".
17. Save file.
18. Load file.
19. Validation dữ liệu.
```

Đây là phạm vi chính cần bàn giao cho **Người 3 — Business Logic**.

---

# 38. Yêu cầu Login cần bàn giao

Đề yêu cầu:

```text
Sinh viên:
username = MASV
password = student.password
```

Tài khoản giáo viên đặc biệt:

```text
username = GV
password = GV
```

Nếu đúng:

```text
LOGIN_TEACHER
```

Sinh viên đúng tài khoản:

```text
LOGIN_STUDENT
```

Sai:

```text
LOGIN_FAILED
```

Prototype đã có:

```cpp
LoginRole login(
    Class* classes[],
    int classCount,
    const char username[],
    const char password[],
    Student*& loggedStudent
);
```

Business Logic / Auth module có thể dùng:

```cpp
findStudentGlobal()
```

để tìm tài khoản sinh viên.

---

# 39. Yêu cầu thêm lớp

Business Logic:

```text
Nhập MALOP
    ↓
validate length / empty
    ↓
findClass()
    ↓
nếu tồn tại → báo trùng
    ↓
createClass()
    ↓
addClass()
```

Nếu `addClass()` thất bại sau khi `new`, caller phải xử lý ownership phù hợp.

---

# 40. Yêu cầu thêm sinh viên

Luồng đề xuất:

```text
Nhập MALOP
    ↓
findClass()
    ↓
không tồn tại → báo lỗi
    ↓
Nhập MASV
    ↓
findStudentGlobal()
    ↓
đã tồn tại → báo lỗi
    ↓
validate HO/TEN/PHAI/password
    ↓
createStudent()
    ↓
addStudent()
```

Điểm quan trọng:

> Không chỉ kiểm tra duplicate trong một lớp.

---

# 41. Yêu cầu thêm môn học

Luồng:

```text
Nhập MAMH
    ↓
validate
    ↓
findSubject()
    ↓
đã tồn tại → báo lỗi
    ↓
createSubject()
    ↓
insertSubject()
```

Không tạo node trước rồi mới bỏ mặc node khi duplicate.

---

# 42. Yêu cầu nhập Question

Luồng:

```text
Nhập MAMH
    ↓
findSubject()
    ↓
nhập content
nhập A/B/C/D
nhập correctAnswer
    ↓
validate
    ↓
id = generateQuestionId()
    ↓
createQuestion(id, ...)
    ↓
insertQuestion(subject->questionList, question)
```

Người dùng không nhập `Question.id`.

`correctAnswer` cần kiểm tra:

```text
A / B / C / D
```

---

# 43. Thi trắc nghiệm — hướng bàn giao

Theo đề, trước khi thi phải hỏi:

```text
Môn thi
Số câu hỏi
Số phút thi
```

Sau đó lấy ngẫu nhiên câu hỏi trong danh sách câu hỏi của môn.

Prototype hiện có:

```cpp
bool canTakeExam(
    Subject* subject,
    int numberOfQuestions,
    int minutes
);

float calculateScore(
    int correctAnswers,
    int totalQuestions
);
```

Business Logic cần kiểm tra tối thiểu:

```text
subject tồn tại
numberOfQuestions > 0
minutes > 0
numberOfQuestions <= countQuestions(subject->questionList)
```

---

# 44. Chọn câu hỏi ngẫu nhiên khi không dùng `vector`

Vì project không dùng STL container, Business Logic cần tự thiết kế.

Một lựa chọn:

```text
1. countQuestions()
2. Tạo mảng động Question**
3. Duyệt linked list và đưa pointer vào mảng
4. Shuffle thủ công bằng swap
5. Lấy K phần tử đầu
6. delete[] mảng pointer sau khi dùng
```

Ví dụ ý tưởng:

```cpp
Question** questions =
    new Question*[count];
```

Sau khi thi:

```cpp
delete[] questions;
```

Lưu ý:

> Mảng này chỉ chứa pointer tham chiếu Question gốc, không sở hữu các Question đó.

Không được `delete questions[i]` chỉ vì đã tạo mảng pointer phục vụ random.

---

# 45. Tính điểm

Một công thức tự nhiên từ prototype hiện có:

```text
score = correctAnswers * 10.0 / totalQuestions
```

Cần xử lý:

```text
totalQuestions > 0
```

Điểm lưu trong:

```cpp
Score.score
```

---

# 46. Score và ExamHistory khác nhau

Nên phân biệt rõ.

## Score

Dùng cho:

```text
bảng điểm hiện tại theo môn
```

Ví dụ:

```text
CTDL - 8.5
AI   - 9.25
```

## ExamHistory

Dùng cho:

```text
lịch sử từng bài thi
chi tiết câu hỏi đã làm
đáp án đã chọn
đáp án đúng
điểm bài đó
```

Nếu cho thi lại, `ExamHistory` là nơi phù hợp để giữ nhiều lần thi.

---

# 47. In bảng điểm theo lớp / môn

Đề yêu cầu:

```text
Nếu sinh viên chưa thi → "Chưa thi"
```

Luồng:

```text
findClass(MALOP)

for mỗi Student trong class.studentList
    score = findScore(student, MAMH)

    nếu score == nullptr
        in "Chua thi"
    ngược lại
        in score->score
```

---

# 48. In chi tiết bài thi

Không nên đọc lại Question gốc để tái tạo bài đã thi.

Nên dùng:

```text
Student.examHistoryList
    ↓
ExamHistory.detailList
```

vì `ExamDetail` lưu snapshot câu hỏi tại thời điểm thi.

---

# 49. Save / Load File

Đề yêu cầu lưu các danh sách vào file.

Các dữ liệu cần cân nhắc lưu:

```text
Subject
Question
Class
Student
Score
ExamHistory
ExamDetail
```

Khi load Question:

```cpp
updateQuestionIdCounter(questionId);
```

để counter tiếp tục đúng.

Không lưu địa chỉ pointer.

Ví dụ sai:

```text
0x7ffe12345678
```

Pointer chỉ có ý nghĩa trong một lần chạy chương trình.

File phải lưu **giá trị dữ liệu**, sau đó khi load phải tái tạo node và liên kết.

---

# 50. Validation phải thuộc Business Logic

Core hiện ưu tiên thao tác cấu trúc dữ liệu.

Business Logic cần validate:

```text
MAMH không rỗng
MAMH <= 15 ký tự

MALOP không rỗng
MALOP vừa buffer

MASV không rỗng
MASV unique toàn hệ thống

HO/TEN không vượt buffer
password không vượt buffer

Question content không vượt buffer
A/B/C/D không vượt buffer

correctAnswer thuộc A/B/C/D

score thuộc [0, 10]

numberOfQuestions > 0
minutes > 0

số câu yêu cầu <= số câu hiện có
```

Điều này đặc biệt quan trọng vì `stringCopy()` không kiểm tra capacity.

---

# 51. Referential Integrity — vấn đề cần Người 3 quyết định

Có một số quan hệ bằng ID thay vì pointer:

```text
Score.subjectId
ExamHistory.subjectId
```

Nếu Business Logic cho phép đổi:

```text
Subject.id
```

ví dụ:

```text
OOP → AI
```

thì Subject BST đã đổi đúng, nhưng các Score / ExamHistory cũ có thể vẫn chứa:

```text
OOP
```

Do đó Business Logic cần chọn một policy rõ ràng.

### Policy A — Cascade update

Khi đổi MAMH:

```text
duyệt tất cả Student
    cập nhật Score.subjectId
    cập nhật ExamHistory.subjectId
```

### Policy B — Không cho đổi MAMH sau khi môn đã được sử dụng

Chỉ cho đổi:

```text
TENMH
```

khi đã có điểm / lịch sử thi.

### Policy C — Dùng immutable internal ID

Phức tạp hơn, không cần thiết nếu bài tập không yêu cầu.

Điểm này **chưa được Core hiện tại tự giải quyết** và phải được quyết định trước khi hoàn thiện chức năng sửa môn.

---

# 52. Xóa Subject và dữ liệu liên quan

Tương tự, nếu xóa Subject:

```text
CTDL
```

nhưng Student vẫn có:

```text
Score.subjectId = "CTDL"
```

thì tồn tại record tham chiếu tới môn không còn trong BST.

Business Logic cần xác định:

```text
A. Không cho xóa môn nếu đã phát sinh điểm/lịch sử.

hoặc

B. Xóa cascade Score liên quan.

hoặc

C. Giữ lịch sử nhưng có cơ chế hiển thị môn đã xóa.
```

Đề không mô tả chi tiết policy này, nên không nên tự giả định im lặng.

---

# 53. Duplicate Question ID khi Load

`generateQuestionId()` đảm bảo câu mới trong runtime không trùng.

Nhưng file dữ liệu có thể bị lỗi.

Khi load nên kiểm tra:

```text
Question ID trong file không bị duplicate
```

`updateQuestionIdCounter()` chỉ cập nhật counter, không tự phát hiện mọi duplicate trong file.

---

# 54. Handoff Contract — Người 2 → Người 3

Người 3 có thể coi các module sau là API Core đã hoàn thành:

```text
StringUtils
QuestionList
SubjectBST
StudentList
ScoreList
ClassArray
```

Người 3 **không nên viết lại linked list hoặc BST** trong Business Logic.

Nên sử dụng API:

```text
create...
find...
add / insert...
delete...
edit / update...
print...
```

Business Logic chịu trách nhiệm:

```text
validation
workflow
permissions
coordination
exam rules
persistence
error messages
```

---

# 55. Nguyên tắc bàn giao quan trọng

1. **Core không đồng nghĩa Business Logic.**  
   Core cung cấp thao tác; Business quyết định khi nào được phép gọi.

2. **Ownership phải rõ trước mọi `delete`.**

3. **Không sửa trực tiếp khóa BST.**

4. **Question ID mới luôn đi qua `generateQuestionId()`.**

5. **MASV phải kiểm tra bằng `findStudentGlobal()` trước khi thêm.**

6. **Không dùng `stringCopy()` với input chưa validate độ dài.**

7. **Node tạo ra nhưng insert thất bại vẫn cần được giải phóng bởi caller nếu Core không nhận ownership.**

8. **Khi load file phải rebuild pointer relationships.**

9. **Không serialize địa chỉ pointer.**

10. **Compile toàn bộ `.cpp` khi integration test.**

---

# 56. Checklist trước khi bắt đầu Business Logic

```text
[x] Structures ổn định
[x] Constants ổn định
[x] StringUtils hoạt động
[x] Question Linked List hoạt động
[x] Global Question ID hoạt động
[x] Subject BST hoạt động
[x] Student Linked List hoạt động
[x] Score Linked List hoạt động
[x] Class Pointer Array hoạt động
[x] Global Student Search hoạt động
[x] Deep cleanup khi delete Student
[x] Deep cleanup khi delete Class
[x] Subject cleanup Question list
[x] Integration Test passed

[ ] Auth
[ ] ExamHistory operations
[ ] ExamDetail operations
[ ] Exam workflow
[ ] Random question selection
[ ] Timer / time handling
[ ] Business validation
[ ] Scoreboard
[ ] Review exam detail
[ ] File save
[ ] File load
[ ] Final UI integration
```

---

# 57. Roadmap đề xuất sau bàn giao

```text
Phase 1 — Business Foundation
├── Validation helpers
├── Class management workflow
├── Student management workflow
├── Subject management workflow
└── Question management workflow

Phase 2 — Authentication
└── login()

Phase 3 — Exam Engine
├── canTakeExam()
├── random selection
├── answer collection
├── calculateScore()
├── add/update Score
├── create ExamHistory
└── create ExamDetail snapshots

Phase 4 — Reports
├── review exam detail
└── class score table

Phase 5 — Persistence
├── saveAllData()
└── loadAllData()

Phase 6 — UI Integration
└── kết nối toàn bộ chức năng với giao diện

Phase 7 — Final Testing
├── normal cases
├── invalid input
├── duplicate data
├── empty structures
├── max limits
├── memory cleanup
└── save/load round trip
```

---

# 58. Kiến thức C++ đã sử dụng trong Core

Qua Task 02, các kiến thức C++ chính gồm:

```text
struct
enum
pointer
pointer to pointer-like behavior thông qua reference to pointer
reference
dynamic memory
new / delete
array of pointers
char arrays
header guards
multi-file compilation
function prototype
static file-scope helper
recursion
linked list
binary search tree
in-order traversal
ownership
dangling pointer
memory leak
deep cleanup
API/module separation
```

---

# 59. Reference to Pointer — kiến thức quan trọng

Ví dụ:

```cpp
Question*& head
```

hoặc:

```cpp
Student*& head
```

Cho phép function sửa trực tiếp pointer head của caller.

Ví dụ:

```cpp
head = head->next;
```

sau khi function kết thúc, head bên ngoài cũng đã thay đổi.

Nếu chỉ truyền:

```cpp
Question* head
```

thì việc gán lại `head` chỉ thay đổi bản copy cục bộ của pointer.

---

# 60. Pointer Array

```cpp
Class* classes[MAX_CLASS];
```

Đây là:

```text
mảng gồm MAX_CLASS phần tử
mỗi phần tử là Class*
```

Không phải:

```text
mảng Class object
```

Do đó:

```cpp
classes[i]->id
```

dùng toán tử `->`.

---

# 61. Header Guards

Ví dụ:

```cpp
#ifndef SCORE_LIST_H
#define SCORE_LIST_H

// declarations

#endif
```

Mục đích:

```text
tránh một header bị include nhiều lần trong cùng translation unit
```

---

# 62. `.h` và `.cpp`

Quy ước project:

```text
.h
└── declarations / public API

.cpp
└── implementation
```

Ví dụ:

```text
ScoreList.h
└── createScore(...)
    findScore(...)
    ...

ScoreList.cpp
└── thân hàm tương ứng
```

---

# 63. Tách Core / Business / UI

Kiến trúc mong muốn:

```text
UI
 ↓
Business Logic
 ↓
Core Data Structures
```

Ví dụ thêm sinh viên:

```text
UI
│
│ nhận dữ liệu người dùng
↓
Business
│
│ validate
│ kiểm tra class
│ kiểm tra MASV global
↓
Core
│
├── createStudent()
└── addStudent()
```

Core không nên tự hỏi:

```text
"Nhập mã sinh viên:"
```

UI / Business chịu trách nhiệm đó.

---

# 64. Những điều KHÔNG nên làm ở bước tiếp theo

Không nên:

```text
- viết lại linked list trong Business Logic
- dùng vector để né cấu trúc đã làm
- sửa Subject.id trực tiếp
- cho user nhập Question.id
- chỉ check MASV trong một lớp
- gọi delete trên pointer không sở hữu
- lưu raw pointer vào file
- gọi stringCopy với input dài chưa kiểm tra
- bỏ qua nullptr
- compile chỉ main.cpp
```

---

# 65. Test strategy cho giai đoạn tiếp theo

Mỗi chức năng Business nên test tối thiểu 3 nhóm:

```text
1. Success case
2. Invalid case
3. Boundary / empty case
```

Ví dụ add student:

```text
Success:
MALOP tồn tại, MASV mới

Invalid:
MALOP không tồn tại
MASV đã tồn tại

Boundary:
chuỗi rỗng
chuỗi quá dài
class student list rỗng
```

---

# 66. Tiêu chuẩn hoàn thành một module

Một module nên chỉ được đánh dấu DONE khi:

```text
[ ] Compile được
[ ] Không warning quan trọng
[ ] Test success case
[ ] Test fail case
[ ] Test nullptr / empty nếu phù hợp
[ ] Không leak do ownership rõ ràng
[ ] Header và implementation đồng bộ
[ ] Không phá invariant của cấu trúc
[ ] Integration với module liên quan chạy đúng
```

---

# 67. Tóm tắt kiến trúc trong một hình

```text
                         ┌───────────────────┐
                         │    Subject BST    │
                         └─────────┬─────────┘
                                   │
                                   ▼
                         ┌───────────────────┐
                         │      Subject      │
                         │ id, name          │
                         └─────────┬─────────┘
                                   │ questionList
                                   ▼
                         Question -> Question -> ...



┌────────────────────────────────────────────────────────────┐
│                 Class* classes[MAX_CLASS]                  │
└────────────────────────────┬───────────────────────────────┘
                             │
                             ▼
                         ┌─────────┐
                         │  Class  │
                         └────┬────┘
                              │ studentList
                              ▼
                     Student -> Student -> ...
                        │          │
                        │          ├── Score -> Score -> ...
                        │          │
                        │          └── ExamHistory -> ...
                        │                            │
                        │                            └── ExamDetail -> ...
                        │
                        ├── Score -> Score -> ...
                        │
                        └── ExamHistory -> ExamDetail -> ...
```

---

# 68. Tóm tắt cho người nhận bàn giao

Nếu chỉ có vài phút để đọc, hãy nhớ các điểm sau:

```text
1. Subject là BST theo MAMH.
2. Mỗi Subject sở hữu Question Linked List.
3. Question ID phải global unique.
4. Class được quản lý bằng pointer array tối đa 10000.
5. Mỗi Class sở hữu Student Linked List.
6. MASV cần unique toàn hệ thống.
7. Dùng findStudentGlobal trước khi addStudent.
8. Mỗi Student sở hữu Score + ExamHistory.
9. ExamHistory sở hữu ExamDetail snapshots.
10. Deep cleanup đã được xử lý cho Student/Class.
11. Không dùng STL container và std::string cho dữ liệu chính.
12. stringCopy không tự chống buffer overflow.
13. Không sửa trực tiếp key của BST.
14. Business Logic chưa hoàn thành.
15. Auth / Exam / Save-Load mới có prototype, cần implement tiếp.
16. Cần quyết định policy khi đổi/xóa MAMH đã có Score/History.
17. Core Integration Test đã PASS.
```

---

# 69. Trạng thái bàn giao cuối cùng

```text
NGƯỜI 2 — CORE C++ / DATA STRUCTURES
STATUS: READY FOR HANDOFF
```

Đã hoàn thành:

```text
String Utilities
Question Linked List
Global Question ID
Subject BST
Student Linked List
Score Linked List
Class Pointer Array
Global Student Search
Memory cleanup
Core Integration Test
```

Chưa hoàn thành và thuộc bước tiếp:

```text
Business workflows
Authentication
Exam engine
Exam history functions
Reports
Persistence
Validation
UI integration
```

---

# 70. Kết luận

Core hiện cung cấp nền tảng dữ liệu chính của Project:

```text
Subject ↔ Question
Class ↔ Student ↔ Score
                  ↘ ExamHistory ↔ ExamDetail
```

Các cấu trúc cơ bản đã được unit test và integration test chung.

Điểm quan trọng nhất khi phát triển tiếp là:

> Không phá các invariant của Core, không làm mất ownership, và đặt toàn bộ kiểm tra nghiệp vụ ở Business Logic trước khi gọi các thao tác Core.

Mốc hiện tại:

```text
TASK 02 — CORE COMPLETE
NEXT     — BUSINESS LOGIC
```
