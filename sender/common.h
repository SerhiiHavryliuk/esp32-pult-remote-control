#ifndef COMMON_H
#define COMMON_H

#include <Arduino.h>

// Структура даних для обміну між пультом та машинкою
typedef struct struct_message {
    bool forward;
    bool backward;
    bool left;
    bool right;
    uint8_t speed; // 0-255
} struct_message;

#endif
