# SHSHchecker – Công cụ kiểm tra SHSH cho thiết bị Apple

# Giới thiệu
**SHSHchecker** là một công cụ dòng lệnh đơn giản được viết bằng C++, dùng để kiểm tra thông tin về SHSH blob của thiết bị Apple thông qua ECID và model thiết bị.

# Yêu cầu hệ thống

- Máy tính chạy **macOS** hoặc **Linux** ( ios armv7 )
- Trình biên dịch **g++**
- Theos && SDK6.1
- Đã cài đặt **Xcode Command Line Tools** (nếu dùng macOS):
  ```bash
  xcode-select --install
  ```
- Có sẵn công cụ `make` (thường đi kèm với Xcode Tools)

## Cách cài đặt và chạy

### Bước 1: Giải nén
Tải và giải nén thư mục `SHSHchecker-main.zip`.

### Bước 2: Mở Terminal và chuyển vào thư mục dự án
```bash
cd đường_dẫn_đến_thư_mục/SHSHchecker-main
```
Ví dụ:
```bash
cd ~/Downloads/SHSHchecker-main
```

### Bước 3: Tiến hành biên dịch
```bash
make
```

### Bước 4: Chạy chương trình
Cú pháp:
```bash
./SHSHchecker <ECID> <MODEL>
```

Ví dụ:
```bash
./SHSHchecker 2339416931829 iPhone4,1
```

## Tham số chương trình

| Tham số      | Mô tả                                                                 |
|--------------|----------------------------------------------------------------------|
| `<ECID>`     | ECID của thiết bị Apple (số định danh duy nhất của thiết bị)         |
| `<MODEL>`    | Mã định danh thiết bị, ví dụ: `iPhone4,1`, `iPhone6,1`, `iPad3,3`... |

## Lưu ý
- ECID có thể lấy thông qua **iTunes**, **3uTools** hoặc **Apple Configurator**.
- Model thiết bị cần nhập đúng định dạng (ví dụ: `iPhone4,1`, không phải chỉ `iPhone 4`).
- Nếu bạn nhập sai cú pháp, chương trình sẽ báo lỗi và yêu cầu nhập lại đúng định dạng.

## Cấu trúc thư mục

| Tệp tin         | Mô tả                                |
|------------------|----------------------------------------|
| `main.cpp`       | Mã nguồn chính của chương trình       |
| `json.hpp`       | Thư viện JSON header-only dùng để xử lý dữ liệu |
| `makefile`       | Tập lệnh để biên dịch bằng `make`     |
| `README.md`      | Tài liệu hướng dẫn (chính là file này)|

##  Tác giả & Giấy phép
- Dự án mã nguồn mở. Vui lòng kiểm tra trong mã nguồn để biết thông tin giấy phép cụ thể.
- Nếu bạn muốn đóng góp hoặc báo lỗi, hãy gửi pull request hoặc issue tại kho lưu trữ gốc.
- đối với ios hãy dùng theos và sdk6.1 và chạy **make package**
