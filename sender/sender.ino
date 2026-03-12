#include <esp_now.h>
#include <WiFi.h>
#include <TFT_eSPI.h>
#include "common.h"

TFT_eSPI tft = TFT_eSPI();

// MAC-адреса вашого ресивера (ПЕРЕВІРТЕ НА ЕКРАНІ РЕСИВЕРА!)
uint8_t broadcastAddress[] = {0xA0, 0xDD, 0x6C, 0x70, 0x8C, 0xD4}; 

// !!! ПІНИ 12, 15, 2 НЕ МОЖНА ВИКОРИСТОВУВАТИ (вони системні і перезавантажують плату) !!!
// Використовуємо тільки ці безпечні піни:
const int pinUp    = 27;
const int pinDown  = 26;
const int pinLeft  = 25;
const int pinRight = 33;

struct_message myData;
struct_message lastSentData; 
esp_now_peer_info_t peerInfo;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    if (status == ESP_NOW_SEND_SUCCESS) {
        tft.setCursor(10, 115);
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.println("Signal: OK     ");
        Serial.println("> Сигнал доставлено успішно");
    } else {
        tft.setCursor(10, 115);
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.println("Signal: ERROR  ");
        Serial.println("> Помилка доставки сигналу!");
    }
}

unsigned long lastSendTime = 0;
const unsigned long HEARTBEAT_MS = 300; // Відправляти дані кожні 300мс для підтримки зв'язку

void setup() {
    Serial.begin(115200);
    delay(1000); 
    Serial.println("\n--- СТАРТ СИСТЕМИ: SENDER ---");
    
    // Ініціалізація дисплея
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.setCursor(10, 50);
    tft.println("SYSTEM STARTING...");

    // Очищення структур
    memset(&myData, 0, sizeof(myData));
    memset(&lastSentData, 0, sizeof(lastSentData));
    memset(&peerInfo, 0, sizeof(peerInfo));

    // Ініціалізація кнопок
    pinMode(pinUp, INPUT_PULLUP);
    pinMode(pinDown, INPUT_PULLUP);
    pinMode(pinLeft, INPUT_PULLUP);
    pinMode(pinRight, INPUT_PULLUP);
    Serial.println("Кнопки налаштовано: 27, 26, 25, 33");

    delay(500); 
    WiFi.mode(WIFI_STA);
    
    // Обмеження потужності WiFi, щоб не "просідало" живлення
    WiFi.setTxPower(WIFI_POWER_11dBm); 
    Serial.println("WiFi потужність обмежена до 11dBm для стабільності");

    if (esp_now_init() != ESP_OK) {
        Serial.println("! Помилка ESP-NOW");
        return;
    }
    esp_now_register_send_cb(OnDataSent);
    
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 0;  
    peerInfo.encrypt = false;
    
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("! Не вдалося додати peer");
        return;
    }
    
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_YELLOW);
    tft.setCursor(10, 10);
    tft.println("RC SENDER ACTIVE");
    Serial.println("Система готова до роботи");
}

void updateUI(bool force = false) {
    if (!force && 
        myData.forward == lastSentData.forward &&
        myData.backward == lastSentData.backward &&
        myData.left == lastSentData.left &&
        myData.right == lastSentData.right &&
        myData.speed == lastSentData.speed) {
        return;
    }

    tft.fillRect(0, 60, 240, 55, TFT_BLACK);
    tft.setTextColor(TFT_CYAN);
    tft.setTextSize(2);
    
    tft.setCursor(10, 65);
    if (myData.forward)  tft.println("MOVE: FORWARD");
    else if (myData.backward) tft.println("MOVE: BACKWARD");
    else tft.println("MOVE: STOP");

    tft.setCursor(10, 90);
    if (myData.left)  tft.println("TURN: LEFT");
    else if (myData.right) tft.println("TURN: RIGHT");
    else tft.println("TURN: CENTER");
    
    tft.setCursor(160, 10);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.printf("SPD: %d  ", myData.speed);
}

void loop() {
    myData.forward  = (digitalRead(pinUp) == LOW);
    myData.backward = (digitalRead(pinDown) == LOW);
    myData.left     = (digitalRead(pinLeft) == LOW);
    myData.right    = (digitalRead(pinRight) == LOW);
    
    myData.speed = (myData.forward || myData.backward) ? 200 : 0;

    // Перевіряємо чи змінилися дані
    bool dataChanged = (myData.forward != lastSentData.forward ||
                        myData.backward != lastSentData.backward ||
                        myData.left != lastSentData.left ||
                        myData.right != lastSentData.right ||
                        myData.speed != lastSentData.speed);

    // Відправляємо дані ТІЛЬКИ якщо вони змінилися АБО пройшла 1 секунда (heartbeat)
    if (dataChanged || (millis() - lastSendTime > HEARTBEAT_MS)) {
        esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
        lastSendTime = millis();
        
        if (dataChanged) {
            Serial.printf("Дані змінено. Стан: F:%d B:%d L:%d R:%d SPD:%d\n", 
                          myData.forward, myData.backward, myData.left, myData.right, myData.speed);
        }
        
        // Зберігаємо для порівняння ПІСЛЯ відправки
        memcpy(&lastSentData, &myData, sizeof(myData));
    }
    
    updateUI();
    delay(50);
}