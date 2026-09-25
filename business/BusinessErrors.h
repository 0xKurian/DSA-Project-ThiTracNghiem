#ifndef BUSINESS_ERRORS_H
#define BUSINESS_ERRORS_H

enum BusinessResult {
    SUCCESS = 0,
    ERR_EMPTY_FIELD,
    ERR_BUFFER_OVERFLOW,
    ERR_INVALID_FORMAT,
    ERR_NOT_FOUND,
    ERR_DUPLICATE,
    ERR_LIST_FULL,
    ERR_CONSTRAINT_VIOLATION,
    ERR_UNKNOWN
};

inline const char* getErrorMessage(BusinessResult result) {
    switch (result) {
        case SUCCESS:
            return "Thanh cong.";
        case ERR_EMPTY_FIELD:
            return "Truong du lieu khong duoc de trong.";
        case ERR_BUFFER_OVERFLOW:
            return "Do dai du lieu vuot qua gioi han cho phep.";
        case ERR_INVALID_FORMAT:
            return "Dinh dang du lieu khong hop le.";
        case ERR_NOT_FOUND:
            return "Khong tim thay doi tuong yeu cau.";
        case ERR_DUPLICATE:
            return "Du lieu da ton tai trong he thong (trung lap).";
        case ERR_LIST_FULL:
            return "Danh sach da day, khong the them moi.";
        case ERR_CONSTRAINT_VIOLATION:
            return "Khong the thuc hien do rang buoc toan ven du lieu.";
        default:
            return "Loi khong xac dinh.";
    }
}

#endif
