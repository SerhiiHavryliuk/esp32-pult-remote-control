# ESP32 Remote Control Car Improvement Plan

This plan outlines the enhancements for the ESP32 T-Display based remote controller and car receiver using L298N and Lego motors.

## Proposed Changes

### Shared Components

#### [NEW] [common.h](file:///d:/ProjectsESP32/Pult-remoute-control/common.h)
Define a shared data structure for communication to ensure both Sender and Receiver are synced.

```cpp
typedef struct struct_message {
    bool forward;
    bool backward;
    bool left;
    bool right;
    uint8_t speed; // 0-255
} struct_message;
```

---

### Sender Component (Remote Control)

#### [MODIFY] [sender.ino](file:///d:/ProjectsESP32/Pult-remoute-control/sender/sender.ino)
- Implement 4-button input handling with debouncing.
- Add speed control logic (e.g., hold forward to increase speed or fixed speed steps).
- Enhance UI to show which direction is active and signal strength.

**Proposed Pins:**
- Forward: 12
- Backward: 13
- Left: 15
- Right: 17

---

### Receiver Component (Car)

#### [MODIFY] [resiver.ino](file:///d:/ProjectsESP32/Pult-remoute-control/resiver/resiver.ino)
- Configure L298N driver pins with PWM (LedC).
- Implement motor logic:
    - Motor 1 (Rear): Speed control and direction.
    - Motor 2 (Front): Turning control.
- Add a failsafe: if no data is received for > 500ms, stop all motors.
- Enhance UI to show motor status and connection state.

**Proposed Pins:**
- Motor A (Rear): PWM=25, IN1=26, IN2=27
- Motor B (Front): PWM=32, IN3=33, IN4=17 (or similar)

---

## User Review Required

> [!IMPORTANT]
> **Pin Configuration**: Please verify if the proposed pins (13, 15, 17 for buttons and 25, 26, 27, 32, 33 for L298N) are available and not used by your specific hardware layout. T-Display uses some pins for the LCD.
> **Lego Motors**: Lego Wedo motors are usually 5V or 9V. Ensure the L298N is powered with an appropriate voltage for them.

### Questions for you:
1.  **Speed Control**: Since you have buttons (ON/OFF), should the speed be fixed (e.g., 70%), or should it increase while you hold the button?
2.  **MAC Address**: Do you know the MAC address of your receiver board? I can provide a separate script to find it if needed.
3.  **Battery**: Would you like to see the battery level of the ESP32 on the screen? (LilyGo boards can monitor this).
4.  **Steering**: For Lego motors, "turning" usually means running the motor until it hits a limit. Do we need a "center" position or just "turn while held"?


## Verification Plan

### Manual Verification
1.  **Sender UI Test**: Press each button and verify the screen shows the correct direction "SENDING: [FWD/BWD/LEFT/RIGHT]".
2.  **Communication Test**: Verify "Delivery: SUCCESS" on the sender's screen.
3.  **Motor Test**:
    - Press UP: Rear wheels spin forward.
    - Press DOWN: Rear wheels spin backward.
    - Press LEFT/RIGHT: Front motor turns appropriately.
4.  **Failsafe Test**: Turn off the sender while the car is moving. The car should stop within 0.5s.
