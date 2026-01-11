#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

const uint64_t pipeOut = 0xE8E8F0F0E1LL;
RF24 radio(4, 3); // CE, CSN

unsigned long lastSendTime = 0; // Track last data send time
const unsigned long sendInterval = 20; // Send data every 20ms

struct MyData {
  byte throttle;
  byte pitch;
  byte roll; 
};

MyData data;

void resetData() {
  data.throttle = 0;
  data.pitch = 127;
  data.roll = 127;
}

void setup() {
  Serial.begin(9600); 
  radio.begin();
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);
  radio.openWritingPipe(pipeOut);
  radio.setPALevel(RF24_PA_MIN);    
  resetData();
}

int mapJoystickValues(int val, int lower, int middle, int upper, bool reverse) {
  val = constrain(val, lower, upper); // Constrain the input to safe bounds
  if (val < middle)
    val = map(val, lower, middle, 0, 127);   // Lower half → 0 to 127
  else
    val = map(val, middle, upper, 128, 255); // Upper half → 127 to 255
  return (reverse ? 255 - val : val); // Reverse axis if needed
}

void loop() {
  // Map joystick values  
  data.throttle = mapJoystickValues(analogRead(A1),  1,   514, 1023, false);  // Throttle
  data.pitch    = mapJoystickValues(analogRead(A2),  0,   510, 1023, true);  // Pitch
  data.roll     = mapJoystickValues(analogRead(A3),  3,   512, 1024, false);  // Roll

  // Send data at regular intervals
  if (millis() - lastSendTime >= sendInterval) {
    lastSendTime = millis();
    radio.write(&data, sizeof(MyData));
  }
}
