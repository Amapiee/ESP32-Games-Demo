ESP32-Games-Demo
A retro gaming handheld project built on the ESP32 microcontroller, designed to revive classic arcade experiences on a compact OLED display. The system features a dedicated kernel that supports both single-player AI challenges and local multiplayer (PvP) modes, allowing two players to share the same device or connect wirelessly for competitive gameplay.

Key Features
Multiple Classic Games: Includes Pong (with AI and 2-player modes), Snake, and Breakout, structured within a scalable game registry.
OLED Display Support: Optimized for SSD1306 (128x64) matrices using I2C communication with hardware-accelerated rendering.
Local Multiplayer & AI: Built-in opponent logic for single-player sessions and split-controls or wireless connectivity (ESP-NOW) for face-to-face battles.
Input Handling: Supports both physical buttons and analog joystick controls with real-time debouncing for responsive gameplay.
System Console: A "System" menu allows users to select games, configure settings (e.g., switching between AI and PvP modes), and manage game progress.
Architecture
The project utilizes a FreeRTOS-based multi-tasking environment to separate concerns:

Input Task: Handles button/joy sampling and state debouncing.
Game Kernel: Executes game logic and updates the off-screen framebuffer.
Render Task: Blits the framebuffer to the OLED hardware, ensuring smooth frame delivery.
Hardware Requirements
MCU: ESP32 (ESP32-WROOM-32 or compatible)
Display: 0.96" OLED (SSD1306, 128x64, I2C interface)
Input: 8x push buttons or a 2-axis analog joystick
Roadmap
Multiplayer Networking: Enable ESP-NOW for wireless PvP between two consoles.
Game Expansion: Integration of more retro titles (Tetris, Arkanoid).
Sound Support: Addition of PWM-based audio for retro sound effects.
This project serves as a proof-of-concept for embedding a lightweight game engine on IoT hardware, focusing on performance, modularity, and the nostalgia of classic retro gaming.
