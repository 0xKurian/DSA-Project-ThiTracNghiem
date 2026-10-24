# Hướng dẫn Cài đặt, Biên dịch và Khởi chạy Hệ thống Thi Trắc Nghiệm C++

Tài liệu này cung cấp hướng dẫn từng bước chi tiết để biên dịch và chạy dự án trên các hệ điều hành phổ biến: **Linux**, **Windows** và **macOS**.

---

## 1. Thông tin nhanh & Tài khoản thử nghiệm

Hệ thống hỗ trợ 2 chế độ hiển thị:
1. **Giao diện Đồ họa Desktop GUI (Raylib 6.2):** Màn hình trực quan, hỗ trợ chuột và phím.
2. **Giao diện Dòng lệnh Console CLI:** Chạy trực tiếp trong terminal, hỗ trợ môi trường không có màn hình đồ họa (server, SSH, máy ảo).

### Tài khoản đăng nhập thử nghiệm (dữ liệu trong `data/classes.txt`):
| Vai trò | Tài khoản | Mật khẩu | Quyền hạn |
|---|---|---|---|
| **Giảng viên** | `GV` | `GV` | Quản lý Môn học (BST), Câu hỏi, Lớp học, Sinh viên, Bảng điểm |
| **Sinh viên** | `SV001` | `pass123` | Thi trắc nghiệm, xem kết quả, xem lại bài thi và bảng điểm |
| **Sinh viên** | `SV002` | `pass123` | Thi trắc nghiệm |

> [!NOTE]
> **Quy tắc đường dẫn:** Luôn thực thi chương trình từ **thư mục gốc của repository**, vì các file dữ liệu (`data/*.txt`) và phông chữ (`assets/fonts/*.ttf`) được tham chiếu theo đường dẫn tương đối.

---

## 2. Hướng dẫn trên Linux

Dự án đã tích hợp sẵn thư viện tĩnh Raylib 6.2 cho kiến trúc Linux x86_64 (`third_party/raylib-6.2_linux_amd64`).

### Bước 2.1: Cài đặt công cụ biên dịch và thư viện hệ thống

#### Trên Ubuntu / Debian / Linux Mint:
```bash
sudo apt update
sudo apt install -y build-essential libgl-dev libx11-dev
```

#### Trên Fedora / Red Hat:
```bash
sudo dnf groupinstall "Development Tools"
sudo dnf install -y gcc-c++ make mesa-libGL-devel libX11-devel
```

#### Trên Arch Linux / Manjaro:
```bash
sudo pacman -Syu --needed base-devel mesa libx11
```

### Bước 2.2: Biên dịch và chạy

```bash
# 1. Biên dịch toàn bộ ứng dụng và các bộ test
make -j$(nproc)

# 2. Khởi chạy Giao diện Đồ họa (GUI):
make run

# 3. Khởi chạy Giao diện Dòng lệnh (Console CLI):
make run-cli

# 4. Chạy bộ kiểm thử tự động (9 bài test kiểm tra cấu trúc dữ liệu):
make test

# 5. Chạy demo cấu trúc dữ liệu Core:
make demo

# 6. Dọn dẹp các file đã biên dịch khi cần:
make clean
```

> [!TIP]
> **Xử lý sự cố trên Linux:**
> - Nếu gặp thông báo `cannot find -lGL` hoặc `cannot find -lX11`: Hãy kiểm tra và cài đặt gói `libgl-dev` và `libx11-dev` bằng lệnh `sudo apt install -y libgl-dev libx11-dev`.
> - Nếu chạy qua SSH hoặc môi trường không có màn hình (headless): Hãy sử dụng `make run-cli` hoặc `./build/app --console`.

---

## 3. Hướng dẫn trên Windows

Có 2 phương án để chạy trên Windows:

---

### Phương án 3A (Khuyên dùng): Chạy qua WSL2 (Windows Subsystem for Linux)

WSL2 với Ubuntu là giải pháp ổn định, chạy nhanh và tận dụng được ngay thư viện Raylib Linux kèm theo repository. Trên Windows 10/11 hiện nay, WSLg hỗ trợ mở cửa sổ GUI Raylib trực tiếp trên màn hình Windows mượt mà.

#### Các bước thực hiện:
1. Mở **PowerShell (Run as Administrator)** và cài đặt WSL nếu chưa có:
   ```powershell
   wsl --install -d Ubuntu
   ```
2. Khởi động lại máy (nếu được yêu cầu), sau đó mở ứng dụng **Ubuntu** từ Start Menu.
3. Trong terminal Ubuntu, cập nhật và cài đặt thư viện cần thiết:
   ```bash
   sudo apt update
   sudo apt install -y build-essential libgl-dev libx11-dev
   ```
4. Di chuyển vào thư mục dự án trên Windows (ví dụ ổ `D:`):
   ```bash
   cd /mnt/d/DSA-Project-ThiTracNghiem-main
   ```
5. Biên dịch và chạy:
   ```bash
   make run        # Mở giao diện đồ họa GUI
   make run-cli    # Chạy giao diện console
   ```

---

### Phương án 3B: Chạy Native trên Windows bằng MinGW-w64

Dự án **đã tích hợp sẵn thư viện Raylib Windows 64-bit** tại `third_party/raylib-6.2_win64_mingw-w64`. Bạn không cần phải tải hay cài đặt thêm Raylib thủ công.

#### Bước 1: Cài đặt MinGW-w64 và Make
- Tải bộ công cụ [w64devkit](https://github.com/skeeto/w64devkit/releases) (gói khuyên dùng, gọn nhẹ, đã có sẵn `gcc`, `g++`, `make`, `gdb`) hoặc cài đặt qua [MSYS2](https://www.msys2.org/).
- Giải nén `w64devkit` vào thư mục (ví dụ `C:\w64devkit`) và thêm `C:\w64devkit\bin` vào biến môi trường **PATH** của Windows.
- Mở **Command Prompt** (cmd) hoặc **PowerShell** kiểm tra:
  ```cmd
  g++ --version
  make --version
  ```
  *(Nếu dùng MSYS2, lệnh make có thể là `mingw32-make`).*

#### Bước 2: Biên dịch và chạy
Mở **Command Prompt** (cmd) hoặc **PowerShell** tại thư mục gốc của dự án:
```cmd
:: Biên dịch ứng dụng (Makefile sẽ tự động copy raylib.dll vào thư mục build)
make app

:: Khởi chạy giao diện đồ họa GUI
make run

:: Hoặc khởi chạy giao diện Console
make run-cli
```
*(Nếu Makefile dùng `mingw32-make`, thay `make` bằng `mingw32-make`).*

> [!NOTE]
> **Hiển thị tiếng Việt trên Windows CMD/PowerShell:**
> Trước khi chạy giao diện console, hãy đổi bảng mã sang UTF-8 để không bị lỗi ký tự tiếng Việt:
> ```cmd
> chcp 65001
> ```

---

## 4. Hướng dẫn trên macOS (Apple Silicon M-Series & Intel)

Trên macOS, các thư viện nhị phân Linux ELF không thể liên kết được, do đó cần sử dụng Raylib được quản lý qua Homebrew.

### Bước 4.1: Cài đặt công cụ & Homebrew
1. Cài đặt Xcode Command Line Tools:
   ```bash
   xcode-select --install
   ```
2. Cài đặt [Homebrew](https://brew.sh/) (nếu chưa có).
3. Cài đặt các gói phụ thuộc:
   ```bash
   brew install make gcc raylib pkg-config
   ```

### Bước 4.2: Biên dịch và chạy
Makefile trong repo đã được cấu hình tự động nhận diện macOS (`Darwin`) và đường dẫn Homebrew (`/opt/homebrew` hoặc `/usr/local`):

```bash
# Biên dịch và chạy GUI:
make run

# Hoặc chạy Console CLI:
make run-cli

# Chạy kiểm thử:
make test
```

---

## 5. Bảng tổng hợp các lệnh Make hữu ích

| Lệnh | Mô tả tác vụ |
|---|---|
| `make app` | Chỉ biên dịch ứng dụng chính (`build/app` hoặc `build/app.exe`) |
| `make run` | Biên dịch và chạy ứng dụng ở chế độ Đồ họa (Raylib GUI) |
| `make run-cli` | Biên dịch và chạy ứng dụng ở chế độ Dòng lệnh (Console Terminal) |
| `make run-gui` | Tương đương `make run` |
| `make demo` | Biên dịch và chạy chương trình demo các thao tác cấu trúc dữ liệu Core |
| `make test` | Biên dịch và chạy tất cả 9 bộ kiểm thử logic tự động |
| `make clean` | Xóa sạch thư mục `build/` |
| `make -j4` | Biên dịch song song với 4 luồng CPU giúp tăng tốc độ build |

---

## 6. Cấu trúc thư mục dữ liệu

Khi ứng dụng chạy, hệ thống đọc và ghi trực tiếp vào thư mục `data/`:
- `data/subjects.txt`: Lưu trữ danh sách môn học (Cây nhị phân tìm kiếm BST) cùng toàn bộ ngân hàng câu hỏi của từng môn học.
- `data/classes.txt`: Lưu trữ danh sách lớp học (Mảng con trỏ), sinh viên (DSLK đơn), điểm thi và chi tiết các bài thi đã làm.

Nếu muốn đặt lại dữ liệu về trạng thái ban đầu, bạn chỉ cần giữ nguyên hoặc khôi phục 2 file văn bản mẫu này.
