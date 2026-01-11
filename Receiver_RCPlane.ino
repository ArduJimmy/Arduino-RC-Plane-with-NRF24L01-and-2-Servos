#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Servo.h>

RF24 radio(10, 9);
const uint64_t pipe = 0xE8E8F0F0E1LL;

Servo esc;
Servo servoRoll;
Servo servoPitch;

struct RF24Data {
  byte throttle;  // 0–255
  byte pitch;
  byte roll;
};

RF24Data data;
unsigned long lastRecvTime = 0;

void resetData() {
  data.throttle = 0;
  data.pitch = 128;
  data.roll = 128;

  esc.writeMicroseconds(1000);   // ⬅️ WAJIB
  servoRoll.write(90);
  servoPitch.write(90);
}

void setup() {
  Serial.begin(9600);

  esc.attach(3);
  servoRoll.attach(5);
  servoPitch.attach(6);

  delay(2000);                   // ⬅️ ESC BOOT
  esc.writeMicroseconds(1000);   // ⬅️ THROTTLE MIN
  delay(3000);                   // ⬅️ ARMING TIME

  radio.begin();
  radio.setDataRate(RF24_250KBPS);
  radio.setAutoAck(false);
  radio.openReadingPipe(1, pipe);
  radio.startListening();

  resetData();
}

void loop() {
  if (radio.available()) {
    radio.read(&data, sizeof(RF24Data));
    lastRecvTime = millis();
  }

  if (millis() - lastRecvTime > 500) {
    resetData();
    return;
  }
  int throttle_us = map(data.throttle, 0, 255, 1000, 2000);
  int roll_ang    = map(data.roll,     0, 255, 0, 180);
  int pitch_ang   = map(data.pitch,    0, 255, 0, 180);

  esc.writeMicroseconds(throttle_us);  // ✅ BENAR
  servoRoll.write(roll_ang);
  servoPitch.write(pitch_ang);
}
