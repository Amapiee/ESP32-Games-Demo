#pragma once

// Plain data only: no hardware objects or shared mutable globals in this file.
struct InputData {
    int joystickY;
};

struct GameState {
    float paddleY;
    float aiPaddleY;
    unsigned int playerScore;
    unsigned int aiScore;
    float ballX;
    float ballY;
    float ballVX;
    float ballVY;
};
