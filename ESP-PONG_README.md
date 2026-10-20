# ESP-PONG: tài liệu context dự án

> **Cách dùng:** đính kèm file này ở tin nhắn đầu tiên của cuộc trò chuyện mới với Claude, kèm câu hỏi của bạn. Điền các mục có dấu `[ĐIỀN]` trước khi gửi để Claude hiểu đúng hiện trạng. Tài liệu được tổng hợp từ cuộc trò chuyện trước, không phải đặc tả cuối cùng.

---

## 1. Về người dùng

- Sinh viên **năm 3 ngành hệ thống nhúng và IoT**, giao tiếp bằng **tiếng Việt** (nên trả lời tiếng Việt).
- Đã làm: hệ thống nhà thông minh đo bụi mịn bằng ESP32, ứng dụng chat, và một số dự án tương tự.
- Mới học **một ít về Linux**, không muốn dự án quá nặng về Linux.
- Mục tiêu: **một dự án duy nhất** chạm được tới **FreeRTOS, embedded firmware, và (nhẹ) embedded Linux**, đủ chiều sâu để đưa vào CV/đồ án.

## 2. Ràng buộc thực tế

| Hạng mục | Thông tin |
|---|---|
| Phần cứng đã có | **2 board ESP32** |
| Có thể mua thêm | STM32 (Blue Pill / Black Pill), cảm biến, linh kiện nhỏ |
| Ngân sách | Khoảng **500.000đ** cho phần mua thêm |
| Không có | Raspberry Pi |
| Thiết bị | Điện thoại Android + laptop |
| Hệ điều hành laptop | Windows |

## 3. Quá trình chọn dự án

Các hướng đã cân nhắc: trạm giám sát môi trường có OTA, robot hai bánh tự cân bằng, máy vẽ CNC mini (pen plotter), mini logic analyzer (STM32 + DMA + USB), bộ điều khiển BLDC kiểu FOC, USB HID macro keyboard.

**Chốt:** làm **ESP-PONG** (game ping pong hai người chơi, mỗi người một ESP32) vì tận dụng ngay hai board đang có, chi phí thấp.

**Đánh đổi đã thống nhất:**
- FreeRTOS: **đạt tốt** (nhiều task, queue, mutex, chu kỳ cố định).
- Firmware sâu: **mức vừa** (driver SPI/I2C, DMA, giao thức), không có bootloader/bare-metal như STM32 vì ESP-IDF đã lo phần đó.
- Linux: **gần như không có** (chỉ toolchain/build, ví dụ WSL2).
- Nếu muốn đủ bộ kỹ năng: làm ESP-PONG trước, sau đó mua STM32 cho một dự án thứ hai về bootloader/driver bare-metal.

## 4. Mô tả dự án

Tham khảo: một video YouTube Shorts tên "ESP-PONG" (link: https://youtube.com/shorts/nvi1VAK4e_o).

**Phần cứng nhìn thấy trong ảnh (mỗi người chơi một bộ):**
- 1 board ESP32 hàn trên bo đục lỗ.
- 1 màn hình OLED 0.96" giao tiếp I2C (nhãn chân GND, VCC, SCL, SDA; nhiều khả năng driver SSD1306, `[ĐIỀN]` để xác nhận).
- 1 module joystick analog (có vẻ 2 trục + nút nhấn).
- Mỗi người có **màn hình riêng**, nên bắt buộc phải đồng bộ trạng thái qua mạng.

**Kiến trúc đề xuất:**

```
[ESP32 #1: Host]  <--- ESP-NOW (hoặc UDP qua Wi-Fi) --->  [ESP32 #2: Client]
 OLED + joystick                                           OLED + joystick
 Chạy vật lý (nguồn sự thật)                               Gửi input, nhận trạng thái để vẽ
```

Host chạy vật lý bóng/vợt và quyết định ghi điểm. Client gửi input và nhận trạng thái. ESP-NOW có độ trễ thấp và không cần router. Có thể thêm dự đoán phía client (client prediction) nếu muốn điều khiển mượt hơn.

**Phân chia task FreeRTOS (đề xuất):**

| Task | Việc làm | Công cụ |
|---|---|---|
| Game loop | Cập nhật vật lý ở chu kỳ cố định (ví dụ 60 Hz) | `vTaskDelayUntil` |
| Input | Đọc joystick (ADC), chống nhiễu, vùng chết | Queue |
| Network | Gửi/nhận trạng thái | Queue từ callback ESP-NOW sang game loop |
| Render | Vẽ lên OLED | Mutex bảo vệ frame buffer |
| Audio (tùy chọn) | Buzzer khi bóng chạm vợt | Task riêng hoặc timer |
| Giám sát | Theo dõi kết nối, watchdog, reset khi mất sóng | Event group |

## 5. Lưu ý kỹ thuật đã nêu

- **ADC2 không dùng được khi Wi-Fi (kể cả ESP-NOW) đang chạy** trên ESP32. Nối trục joystick vào chân **ADC1** (GPIO32 đến 39), nếu không giá trị đọc sẽ lỗi.
- **Chống nhiễu joystick:** cần deadzone và lọc nhẹ, tránh vợt rung khi buông tay.
- **Băng thông OLED I2C:** một khung 128x64 là 1024 byte, ở 400 kHz mất khoảng 23 ms mỗi khung (ước tính), tức trần khoảng 40 khung/giây. Có thể thử I2C nhanh hơn hoặc màn SPI, và nên đo thật.
- **ESP32 có hai lõi:** tự quyết định ghim task nào vào lõi nào (`xTaskCreatePinnedToCore`) và giải thích lý do.
- Truyền dữ liệu từ callback mạng sang game loop bằng **queue**, không xử lý nặng trong callback.

## 6. Để dự án có chiều sâu (không chỉ "làm theo video")

1. Dùng **ESP-IDF** thay vì Arduino, để làm việc trực tiếp với FreeRTOS, SPI master và DMA.
2. **Tự viết driver màn hình** (có double buffering, truyền bằng DMA nếu dùng SPI) thay vì dùng thư viện sẵn.
3. **Tự thiết kế giao thức gói tin:** số thứ tự, CRC, xử lý gói trễ hoặc mất.
4. **OTA có rollback** (ESP-IDF hỗ trợ sẵn), kiểm thử các tình huống lỗi.
5. **Có số liệu đo** để đưa vào báo cáo: độ trễ trung bình và tối đa, tỉ lệ mất gói, jitter của game loop, FPS, mức dùng CPU từng lõi, stack usage từng task.
6. Đưa lên GitHub kèm sơ đồ kiến trúc, sơ đồ task (độ ưu tiên, chu kỳ), ảnh logic analyzer nếu có.

## 7. Hiện trạng và việc cần quyết định

**Cần người dùng điền:**
- Tiến độ hiện tại: Mới bắt đầu
- Mã board ESP32 cụ thể (ESP32-DevKitC, WROOM, WROVER...): ESP-32D
- Chân GPIO đang nối OLED và joystick: `[ĐIỀN]`
- Framework: Arduino
- Mạng: chưa quyết định

**Câu hỏi mở:**
- Có thể thêm âm thanh, điểm số, nhiều chế độ 

**Lộ trình gợi ý (chưa chốt, chỉ để tham khảo):**
1. Chạy được OLED + joystick trên một board, hiển thị vợt di chuyển.
2. Game loop chu kỳ cố định + vật lý bóng trên một board (chơi với máy).
3. Giao tiếp hai board, host/client, đo độ trễ.
4. Tách task FreeRTOS, đo CPU load/jitter, thêm watchdog.
5. Giao thức gói tin có CRC, xử lý mất gói.
6. OTA có rollback, hoàn thiện tài liệu và số liệu.

## 8. Cách người dùng thích làm việc với Claude

- Câu trả lời thực tế, có cấu trúc, **kiểm tra và đo thật** khi có thể (biên dịch với `-Wall -Wextra -Wpedantic`, chạy test, đo cấp phát heap).
- Với **dự án**, người dùng chấp nhận code mẫu và lộ trình chi tiết theo từng tuần.
- Giá linh kiện chỉ là ước lượng; nên so sánh trước khi mua.

## Mẫu tin nhắn mở đầu cuộc trò chuyện mới

> Mình đính kèm file context về dự án ESP-PONG. Hãy đọc kỹ rồi giúp mình [việc cần làm, ví dụ: vạch kiến trúc task FreeRTOS chi tiết / viết code đọc joystick bằng ADC1 / thiết kế giao thức gói tin ESP-NOW]. Nếu cần thêm thông tin, hãy hỏi mình tối đa một câu.
