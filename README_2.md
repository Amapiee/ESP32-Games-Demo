# ESP-PONG — bản chia module

Project PlatformIO dành cho ESP32-WROOM hai lõi, OLED SSD1306 I2C 128×64
và một joystick. Dựa trên bản main.cpp chơi với máy vừa tạo.

## Mở và nạp

1. Giải nén, mở thư mục `ESP-PONG-modular` (chứa `platformio.ini`) bằng VS Code.
2. Dùng PlatformIO **Build**, sau đó **Upload** và Serial Monitor 115200 baud.
3. Nếu chép vào project cũ, thay nội dung `src/` và chép các header vào `include/`.
   Di chuyển các bản main.cpp cũ ra ngoài `src/` để tránh trùng `setup()`/`loop()`.
4. Không include file `.cpp`; PlatformIO tự biên dịch các file trong `src/`.

## Phân chia file

| File | Trách nhiệm |
|---|---|
| `include/config.h` | GPIO, thông số hình học, độ khó AI, chu kỳ và cấu hình task |
| `include/game_types.h` | `InputData` và `GameState` dùng để trao đổi bản sao |
| `include/input.h`, `src/input.cpp` | Khởi tạo ADC/nút dự phòng và task đọc joystick |
| `include/game.h`, `src/game.cpp` | Trạng thái riêng của game, AI, bóng, va chạm, ghi điểm |
| `include/display.h`, `src/display.cpp` | OLED, vẽ vợt/bóng/điểm và render task |
| `include/network.h`, `src/network.cpp` | Khung để bổ sung ESP-NOW; hiện chưa bật Wi-Fi/BLE |
| `src/main.cpp` | Khởi tạo, tạo queue và task, kiểm tra lỗi khởi tạo |

## Luồng dữ liệu và quyền truy cập

`inputTask` → `inputQueue` → `gameTask` → `gameQueue` → `renderTask`.

- Hai queue có độ dài một và dùng `xQueueOverwrite()` để giữ dữ liệu mới nhất.
- Sau khởi tạo, chỉ `gameTask` sửa trạng thái game. Biến `game` nằm riêng trong
  `game.cpp`, không dùng `extern` để task khác sửa trực tiếp.
- Chỉ render task truy cập OLED sau khi khởi tạo. Vì thế chưa cần mutex cho OLED.
- Các context truyền vào task được giữ ở vùng nhớ tĩnh trong `main.cpp`.
- `networkBegin()` hiện không làm gì; `networkIsConnected()` trả về `false`.
  Khi bổ sung mạng, callback nên đưa input vào queue.

## Nối dây

Ngắt USB khi thay dây. OLED phải là module hỗ trợ nguồn 3,3 V.

| Thiết bị | Chân | ESP32 |
|---|---|---|
| OLED | VDD/VCC | 3V3 |
| OLED | GND | GND |
| OLED | SDA | GPIO21 |
| OLED | SCL | GPIO22 |
| Joystick | VCC | 3V3 |
| Joystick | GND | GND |
| Joystick | VRy | GPIO35 |

VRx và SW chưa cần nối. GPIO27 được cấu hình `INPUT_PULLUP` để dành cho nút;
chưa có xử lý đổi chế độ nên có thể để trống. GPIO4 giữ chức năng LED của code cũ;
LED ngoài nếu dùng cần điện trở hạn dòng. Không cần LED để chơi game.

## Hành vi hiện tại

- Người chơi bên trái, máy bên phải, điểm hiển thị `người : máy`.
- Máy đi tối đa `AI_SPEED = 38` pixel/giây; tăng giá trị để khó hơn.
- Giao bóng từ giữa về phía người chơi, tốc độ 40–69 pixel/giây, góc 20–50 độ.
- Chưa có nút đổi chế độ, chơi mạng, màn hình kết thúc hoặc reset điểm bằng nút.
- LED GPIO4 được đặt HIGH sau lần ghi điểm đầu tiên, giữ như bản trước.
- Chu kỳ game yêu cầu 16 ms (~62,5 Hz nếu tick là 1 ms). Task hiển thị chờ 33 ms
  sau khi vẽ, nên FPS thực tế còn phụ thuộc thời gian truyền OLED.
- `display.begin()` thành công không đảm bảo OLED có mặt trên bus; nếu màn đen,
  kiểm tra dây, địa chỉ I2C (mặc định 0x3C) và đúng driver SSD1306.

## Kiểm tra sau khi nạp

Kiểm tra joystick điều khiển vợt trái, AI điều khiển vợt phải, bóng bật từ hai vợt,
và điểm mỗi bên tăng khi đối thủ bỏ lỡ.

Đã biên dịch và liên kết toàn bộ file C++ bằng g++ C++11 với API Arduino/FreeRTOS
giả lập. Đã đối chiếu 24.000 lần cập nhật game với bản một file, dùng cùng input
và chuỗi số ngẫu nhiên: toàn bộ bản chụp GameState giống nhau, cả hai nhánh ghi
điểm đều được chạy. Phép kiểm tra này không xác nhận driver OLED, ADC, lịch task
hay khả năng biên dịch bằng Arduino-ESP32 thực tế. Môi trường tạo gói không có
PlatformIO/toolchain ESP32 và không có board để thử; cần Build và nạp trên máy bạn.
