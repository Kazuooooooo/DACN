// Khai bao thu vien
#include <LiquidCrystal_I2C.h>
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#define BLYNK_TEMPLATE_ID "TMPL6WwwAxEk7"
#define BLYNK_TEMPLATE_NAME "Smart Garden"
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>

// khai bao
LiquidCrystal_I2C lcd(0x27, 16, 2);
char auth[] = "vSfEUTAP3h9_lp0IaUcTiqMO8BY0esNZ";
char ssid[] = "k a z u o"; 
char pass[] = "0345102159";
DHT dht(D4, DHT11);
BlynkTimer timer;

// Khai bao phan cung
#define soil A0 
// Trang thai ban dau
int relay1State = LOW;
int manualRelay1State = LOW;
int pushButton1State = HIGH;
int relay2State = LOW;
int manualRelay2State = LOW;
int pushButton2State = HIGH;
int autoMode = 1; // mode auto
#define RELAY_PIN_1 D3   // D3 Relay Water
#define RELAY_PIN_2 D5   // D5 Relay Bulb
#define PUSH_BUTTON_1 D6  // D6 Button Water
#define PUSH_BUTTON_2 D7  // D7 Button Bulb
#define VPIN_BUTTON_1 V12 // Virtual Pin Water
#define VPIN_AUTO_MODE V5 // Virtual Pin cho chế độ Auto/Manual

void setup() {
    Serial.begin(9600);
    lcd.begin();
    lcd.backlight();
    pinMode(RELAY_PIN_1, OUTPUT);
    pinMode(RELAY_PIN_2, OUTPUT);
    digitalWrite(RELAY_PIN_1, LOW);
    digitalWrite(RELAY_PIN_2, LOW);
    pinMode(PUSH_BUTTON_1, INPUT_PULLUP);
    pinMode(PUSH_BUTTON_2, INPUT_PULLUP);

    Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);
    dht.begin();

    // Hien thi thong bao khoi tao "Initializing" trên LCD
    lcd.setCursor(0, 0);
    lcd.print("  Initializing  ");
    for (int a = 5; a <= 10; a++) {
        lcd.setCursor(a, 1);
        lcd.print(".");
        delay(500);
    }
    lcd.clear();
    delay(500);
    timer.setInterval(100L, soilMoistureSensor);
    timer.setInterval(100L, DHT11sensor);
    timer.setInterval(500L, checkPhysicalButton1);
    timer.setInterval(500L, checkPhysicalButton2);  
}
// Doc du lieu tu cam bien DHT11
void DHT11sensor() {
    float h = dht.readHumidity(); 
    float t = dht.readTemperature();

    if (isnan(h) || isnan(t)) {
        Serial.println("Failed to read from DHT sensor!");
        return;
    }
    // Gui gia tri len ung dung Blynk
    Blynk.virtualWrite(V0, t);
    Blynk.virtualWrite(V1, h);

    lcd.setCursor(0, 0);
    lcd.print("T:");
    lcd.print(t);
    lcd.print("    "); // Xóa ký tự cũ

    lcd.setCursor(9, 0);
    lcd.print("H:");
    lcd.print(h);
    lcd.print("    "); // Xóa ký tự cũ
    if (autoMode == 1) { // Nếu ở chế độ tự động
        if (t < 15) {
            relay2State = HIGH; // Bật relay để sưởi ấm
        } else {
            relay2State = LOW;  // Tắt relay khi độ ẩm >= 15
        }
        digitalWrite(RELAY_PIN_2, relay2State);
    }
}

void soilMoistureSensor() {
    int value = analogRead(soil);
    value = 100 - map(value, 0, 1024, 0, 100);

    Blynk.virtualWrite(V3, value);
    lcd.setCursor(11, 1);
    lcd.print("S:");
    lcd.print(value);
    lcd.print("  ");

    if (autoMode == 1) {
        if (value < 10) {
            relay1State = HIGH; // Bật relay để bơm nước
        } else {
            relay1State = LOW;
        }
        digitalWrite(RELAY_PIN_1, relay1State);
        Blynk.virtualWrite(VPIN_BUTTON_1, relay1State);
    }
}

BLYNK_CONNECTED() {
    Blynk.syncVirtual(VPIN_BUTTON_1);
    Blynk.syncVirtual(VPIN_AUTO_MODE);
}

BLYNK_WRITE(VPIN_BUTTON_1) {
    if (autoMode == 1) {
        autoMode = 0;
        Blynk.virtualWrite(VPIN_AUTO_MODE, autoMode);
    }
    manualRelay1State = param.asInt();
    relay1State = manualRelay1State;
    Blynk.virtualWrite(VPIN_BUTTON_1, relay1State);
    digitalWrite(RELAY_PIN_1, relay1State);
}

BLYNK_WRITE(VPIN_AUTO_MODE) {
    autoMode = param.asInt();
    if (autoMode == 1) {
        // Chế độ tự động
    } else {
        relay1State = manualRelay1State; // Giữ lại trạng thái relay1
        relay2State = manualRelay2State; // Giữ lại trạng thái relay2
        digitalWrite(RELAY_PIN_1, relay1State); // Cập nhật trạng thái
        digitalWrite(RELAY_PIN_2, relay2State); // Cập nhật trạng thái
        Blynk.virtualWrite(VPIN_BUTTON_1, relay1State); // Cập nhật lên Blynk
    }
}

// Nút nhấn vật lý cho Relay 1
void checkPhysicalButton1() {
    if (digitalRead(PUSH_BUTTON_1) == LOW) {
        if (pushButton1State != LOW) {
            if (autoMode == 1) {
                autoMode = 0;
                Blynk.virtualWrite(VPIN_AUTO_MODE, autoMode);
            }
            manualRelay1State = !manualRelay1State;
            relay1State = manualRelay1State;
            digitalWrite(RELAY_PIN_1, relay1State);
            Blynk.virtualWrite(VPIN_BUTTON_1, relay1State);
        }
        pushButton1State = LOW;
    } else {
        pushButton1State = HIGH;
    }
}

// Nút nhấn vật lý cho Relay 2
void checkPhysicalButton2() {
    if (digitalRead(PUSH_BUTTON_2) == LOW) {
        if (pushButton2State != LOW) {
            if (autoMode == 1) {
                autoMode = 0;
                Blynk.virtualWrite(VPIN_AUTO_MODE, autoMode);
            }
            manualRelay2State = !manualRelay2State;
            relay2State = manualRelay2State;
            digitalWrite(RELAY_PIN_2, relay2State);
        }
        pushButton2State = LOW;
    } else {
        pushButton2State = HIGH;
    }
}

void loop() {
  if (relay1State == HIGH) {
    lcd.setCursor(0, 1);
    lcd.print("W:ON ");
  } else {
    lcd.setCursor(0, 1);
    lcd.print("W:OFF");
  }
  if (relay2State == HIGH) {
    lcd.setCursor(5, 1);
    lcd.print("B:ON ");
  } else {
    lcd.setCursor(5, 1);
    lcd.print("B:OFF");
  }
  Blynk.run();
  timer.run();
}
