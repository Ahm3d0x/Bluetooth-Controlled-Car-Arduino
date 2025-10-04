#include <Servo.h>

// إعداد السيرفو
Servo steeringServo;

// Pins
int motorIn1 = 5;    // IN1 L298N
int motorIn2 = 6;    // IN2 L298N
int motorENA = 11;   // ENA L298N (PWM for speed)
int servoPin = 10;   // السيرفو
int baz = 2;         // البازر
int bled = 3;        // LED خلفي
int fled = 8;        // LED أمامي
int rightLed = 4;    // إشارة يمين
int leftLed  = 7;    // إشارة شمال

int bluetoothData;   // بيانات البلوتوث
int speed = 0;       // السرعة الافتراضية
int Left = 65; 
int right = 115; 
int steerDelay = 0;  // وقت بقاء العجلة لفة (ms)

char carState = 'S'; // الحالة الحالية للعربية (افتراضي واقف)
unsigned long lastBlink = 0;
bool blinkState = false;
bool rightState = false;
bool leftState = false;

char command;
bool lightMode = true; // في البداية الأنوار شغالة

void setup() {
  // موتور خلفي
  pinMode(motorIn1, OUTPUT);
  pinMode(motorIn2, OUTPUT);
  pinMode(motorENA, OUTPUT);

  // باز
  pinMode(baz, OUTPUT);
  digitalWrite(baz, LOW);

  // ليدات
  pinMode(bled, OUTPUT);
  pinMode(fled, OUTPUT);
  pinMode(rightLed, OUTPUT);
  pinMode(leftLed, OUTPUT);

  // سيرفو
  steeringServo.attach(servoPin);
  steeringServo.write(90); // العجلات مستقيمة

  // بلوتوث
  Serial.begin(9600); 
  Serial.println("Ready... Waiting for Bluetooth commands");
}

void loop() {
  // تشغيل نظام الإضاءات بناءً على حالة العربية
    if (Serial.available()) {
    command = Serial.read();

    if (command == 'U') {
      lightMode = false; // إغلاق الأنوار كلياً
    } else if (command == 'u') {
      lightMode = true; // تفعيل الأنوار
    }
  }

  handleLights();

  // استقبال أوامر البلوتوث
  if (Serial.available()) {
    bluetoothData = Serial.read();
    // Serial.print("Received: ");
    Serial.println((char)bluetoothData);

    switch (bluetoothData) {
      case '1': speed = 90; break;
      case '2': speed = 140; break; 
      case '3': speed = 200; break;
      case '4': speed = 255; break;

      case 'Y':  // باز
        digitalWrite(baz, HIGH);
        delay(200);
        digitalWrite(baz, LOW);
        break;

      case 'H':  // Forward Right
        digitalWrite(motorIn1, HIGH);
        digitalWrite(motorIn2, LOW);
        steeringServo.write(right);
        analogWrite(motorENA, speed);
        carState = 'F';
        break;

      case 'G':  // Forward Left
        digitalWrite(motorIn1, HIGH);
        digitalWrite(motorIn2, LOW);
        steeringServo.write(Left);
        analogWrite(motorENA, speed);
        carState = 'F';
        break;

      case 'I':  // Backward Left
        digitalWrite(motorIn1, LOW);
        digitalWrite(motorIn2, HIGH);
        steeringServo.write(Left);
        analogWrite(motorENA, speed);
        carState = 'B';
        break;

      case 'J':  // Backward Right
        digitalWrite(motorIn1, LOW);
        digitalWrite(motorIn2, HIGH);
        steeringServo.write(right);
        analogWrite(motorENA, speed);
        carState = 'B';
        break;

      case 'F':  // Forward
        steeringServo.write(90);
        digitalWrite(motorIn1, HIGH);
        digitalWrite(motorIn2, LOW);
        analogWrite(motorENA, speed);
        carState = 'F';
        break;

      case 'B':  // Backward
        steeringServo.write(90);
        digitalWrite(motorIn1, LOW);
        digitalWrite(motorIn2, HIGH);
        analogWrite(motorENA, speed);
        carState = 'B';
        break;

      case 'L':  // Left Only
        steeringServo.write(Left);
        break;

      case 'R':  // Right Only
        steeringServo.write(right);
        break;

      case 'S':  // Stop
        analogWrite(motorENA, 0);
        digitalWrite(motorIn1, LOW);
        digitalWrite(motorIn2, LOW);
        steeringServo.write(90);
        analogWrite(motorENA, 0);
        carState = 'S';
        break;

      default:
        Serial.println("Unknown command");
        break;
    }
  }
}

void handleLights() {
  if (lightMode) {
    if (carState == 'F') {
      digitalWrite(fled, HIGH);
      digitalWrite(bled, LOW);

      if (millis() - lastBlink > 400) {
        lastBlink = millis();
        int angle = steeringServo.read();

        if (angle > right - 5 && angle < right + 5) {
          rightState = !rightState;
          digitalWrite(rightLed, rightState);
          digitalWrite(leftLed, LOW);
        } 
        else if (angle > Left - 5 && angle < Left + 5) {
          leftState = !leftState;
          digitalWrite(leftLed, leftState);
          digitalWrite(rightLed, LOW);
        } 
        else {
          digitalWrite(rightLed, LOW);
          digitalWrite(leftLed, LOW);
        }
      }
    }
    else if (carState == 'B') {
      digitalWrite(fled, LOW);
      digitalWrite(bled, HIGH);

      if (millis() - lastBlink > 400) {
        lastBlink = millis();
        int angle = steeringServo.read();

        if (angle > right - 5 && angle < right + 5) {
          rightState = !rightState;
          digitalWrite(rightLed, rightState);
          digitalWrite(leftLed, LOW);
        } 
        else if (angle > Left - 5 && angle < Left + 5) {
          leftState = !leftState;
          digitalWrite(leftLed, leftState);
          digitalWrite(rightLed, LOW);
        } 
        else {
          digitalWrite(rightLed, LOW);
          digitalWrite(leftLed, LOW);
        }
      }
    }
    else if (carState == 'S') {
      if (millis() - lastBlink > 400) {
        blinkState = !blinkState;
        digitalWrite(fled, blinkState ? HIGH : LOW);
        digitalWrite(bled, blinkState ? LOW : HIGH);
        digitalWrite(rightLed, blinkState ? HIGH : LOW);
        digitalWrite(leftLed, blinkState ? HIGH : LOW);
        lastBlink = millis();
      }
    }
  } 
  else {
    // لو المود مطفي
    digitalWrite(fled, LOW);
    digitalWrite(bled, LOW);
    digitalWrite(rightLed, LOW);
    digitalWrite(leftLed, LOW);
  }
}
