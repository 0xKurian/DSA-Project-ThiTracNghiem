# DSA core - ban sua an toan

Ban nay giu nguyen phong cach C/C++ char[], con tro va cac cau truc du lieu hien co.

## Da sua

1. `stringCopy` nhan them `destinationSize` de tranh ghi tran bo dem.
2. Cap nhat tat ca noi goi `stringCopy` bang cac hang kich thuoc trong `Constants.h`.
3. `insertSubject` giai phong node moi neu trung ma mon, tranh memory leak.
4. `editSubject` van cho phep hieu chinh ma/ten mon va giu lai danh sach cau hoi.
5. Them `addStudentGlobal` de chan trung ma sinh vien giua cac lop.
6. Them cac test assert cho safe string, duplicate subject, global student id, subject edit.
7. `test.cpp` duoc nang thanh integration test dung `assert`.

## Luu y kien truc can chot theo de

- Can doi chieu lai cau truc luu `Question` trong de chinh thuc. Ban hien tai van giu `Question` la linked list trong tung `Subject` de khong tu y thay doi kien truc khi chua chot yeu cau.
- Neu doi `subjectId` sau khi da co `Score`/`ExamHistory`, tang nghiep vu can cap nhat cac tham chieu cu sang ma moi. `editSubject` hien chi xu ly cay mon hoc va danh sach cau hoi cua mon.
