#include <esp_now.h>
#include <WiFi.h>
#include <TFT_eSPI.h> 
#include "common.h"

TFT_eSPI tft = TFT_eSPI(); 

// Піни L298N для Мотора A (Привід - задні колеса)
const int motorA_PWM = 25;
const int motorA_IN1 = 26;
const int motorA_IN2 = 27;

// Піни L298N для Мотора B (Повороти - передні колеса)
const int motorB_PWM = 32;
const int motorB_IN3 = 33;
const int motorB_IN4 = 21; 

// Пін для світлодіода-індикатора сигналу
const int ledPin = 12;

// Налаштування PWM
const int freq = 5000;
const int res  = 8;
// У версії 3.x ESP32 Core канали керуються автоматично через пін

struct_message myData;
unsigned long lastRecvTime = 0;
const unsigned long TIMEOUT_MS = 2000; // Збільшено для стабільності

void OnDataRecv(const esp_now_recv_info *info, const uint8_t *incomingData, int len) {
    memcpy(&myData, incomingData, sizeof(myData));
    lastRecvTime = millis();
}

void stopMotors() {
    ledcWrite(motorA_PWM, 0);
    ledcWrite(motorB_PWM, 0);
    digitalWrite(motorA_IN1, LOW);
    digitalWrite(motorA_IN2, LOW);
    digitalWrite(motorB_IN3, LOW);
    digitalWrite(motorB_IN4, LOW);
}

void setup() {
    Serial.begin(115200);
    
    // Очищення даних
    memset(&myData, 0, sizeof(myData));

    // Налаштування пінів
    pinMode(motorA_IN1, OUTPUT);
    pinMode(motorA_IN2, OUTPUT);
    pinMode(motorB_IN3, OUTPUT);
    pinMode(motorB_IN4, OUTPUT);
    pinMode(ledPin, OUTPUT); // Світлодіод

    // Ініціалізація екрана
    tft.init();
    tft.setRotation(1); 
    tft.fillScreen(TFT_BLACK);
    
    WiFi.mode(WIFI_STA);
    
    tft.setTextColor(TFT_YELLOW);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.println("RC RECEIVER");
    
    // Показуємо на екрані MAC ресивера
    tft.setCursor(10, 40);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE);
    tft.printf("MY MAC: %s", WiFi.macAddress().c_str());
    
    tft.drawFastHLine(0, 55, 240, TFT_WHITE);

    // Налаштування PWM (ledc) для ESP32 Core 3.x
    ledcAttach(motorA_PWM, freq, res);
    ledcAttach(motorB_PWM, freq, res);

    stopMotors();

    if (esp_now_init() != ESP_OK) {
        tft.setTextColor(TFT_RED);
        tft.println("ESP-NOW Error");
        return;
    }
    
    esp_now_register_recv_cb(OnDataRecv);
}

void updateMotors() {
    // Управління приводом (Мотор A)
    if (myData.forward) {
        digitalWrite(motorA_IN1, HIGH);
        digitalWrite(motorA_IN2, LOW);
        ledcWrite(motorA_PWM, myData.speed);
    } else if (myData.backward) {
        digitalWrite(motorA_IN1, LOW);
        digitalWrite(motorA_IN2, HIGH);
        ledcWrite(motorA_PWM, myData.speed);
    } else {
        digitalWrite(motorA_IN1, LOW);
        digitalWrite(motorA_IN2, LOW);
        ledcWrite(motorA_PWM, 0);
    }

    // Управління поворотами (Мотор B) - зазвичай швидкість тут фіксована
    if (myData.left) {
        digitalWrite(motorB_IN3, HIGH);
        digitalWrite(motorB_IN4, LOW);
        ledcWrite(motorB_PWM, 100); 
    } else if (myData.right) {
        digitalWrite(motorB_IN3, LOW);
        digitalWrite(motorB_IN4, HIGH);
        ledcWrite(motorB_PWM, 100);
    } else {
        digitalWrite(motorB_IN3, LOW);
        digitalWrite(motorB_IN4, LOW);
        ledcWrite(motorB_PWM, 0);
    }
}

void updateUI() {
    tft.fillRect(0, 65, 240, 70, TFT_BLACK);
    tft.setTextSize(2);
    tft.setCursor(10, 70);

    if (lastRecvTime == 0) {
        tft.setTextColor(TFT_YELLOW);
        tft.println("STATUS: WAITING...");
    } else if (millis() - lastRecvTime > TIMEOUT_MS) {
        tft.setTextColor(TFT_RED);
        tft.println("STATUS: LOST SIGNAL");
        stopMotors();
    } else {
        tft.setTextColor(TFT_GREEN);
        tft.println("STATUS: CONNECTED");
        
        tft.setTextColor(TFT_CYAN);
        tft.setCursor(10, 95);
        if (myData.forward) tft.printf("DRIVE: FWD (%d)", myData.speed);
        else if (myData.backward) tft.printf("DRIVE: BWD (%d)", myData.speed);
        else tft.print("DRIVE: STOP");

        tft.setCursor(10, 120);
        if (myData.left) tft.print("STEER: LEFT");
        else if (myData.right) tft.print("STEER: RIGHT");
        else tft.print("STEER: CENTER");
    }
}

void loop() {
    if (millis() - lastRecvTime > TIMEOUT_MS) {
        stopMotors();
        digitalWrite(ledPin, LOW); // Вимикаємо індикатор, якщо зв'язок втрачено
    } else {
        updateMotors();
        
        // Індикація сигналу: якщо натиснута будь-яка кнопка - вмикаємо LED
        bool isAnyButtonPressed = (myData.forward || myData.backward || myData.left || myData.right);
        digitalWrite(ledPin, isAnyButtonPressed ? HIGH : LOW);
    }
    
    updateUI();
    delay(50);
}