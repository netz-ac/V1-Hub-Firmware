#include <Arduino.h>

// pins
const uint8_t RS485_ENABLE = 2;
const uint8_t OUTPUT_ENABLED = 7;
const uint8_t SHIFT_MODE = 9; // HIGH = Shift, LOW = Load
const uint8_t POWER_OUTPUT = 15;
const uint8_t POWER_FEEDBACK_PIN = 14;
const uint8_t DATA_OUT_PIN = 3;
const uint8_t DATA_IN_PIN = 4;
const uint8_t CLOCK_PIN = 5;
const uint8_t LATCH_PIN = 8;

uint8_t buffer[8];

bool inside_packet = false;

void RS485_begin() {
  Serial.begin(9600);
  pinMode(RS485_ENABLE, OUTPUT);
}

void RS485_tx_enable() {
  digitalWrite(RS485_ENABLE, HIGH);
}

void RS485_tx_disable() {
  Serial.flush();
  digitalWrite(RS485_ENABLE, LOW);
}

void RS485_print(char* str) {
  RS485_tx_enable();
  Serial.print(str);
  RS485_tx_disable();
}

void RS485_print(char str) {
  RS485_tx_enable();
  Serial.print(str);
  RS485_tx_disable();
}

void RS485_println(char* str) {
  RS485_tx_enable();
  Serial.println(str);
  RS485_tx_disable();
}

void RS485_println(char str) {
  RS485_tx_enable();
  Serial.println(str);
  RS485_tx_disable();
}

void setup() {
  RS485_begin();

  pinMode(OUTPUT_ENABLED, OUTPUT);
  digitalWrite(OUTPUT_ENABLED, LOW); // Output shift register always enabled
  pinMode(SHIFT_MODE, OUTPUT);
  digitalWrite(SHIFT_MODE, HIGH); // Shift mode
  pinMode(POWER_OUTPUT, OUTPUT);
  digitalWrite(POWER_OUTPUT, HIGH); // Power-Output for locks
  pinMode(POWER_FEEDBACK_PIN, INPUT);

  pinMode(DATA_OUT_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(LATCH_PIN, OUTPUT);
  pinMode(DATA_IN_PIN, INPUT);
  
  RS485_println("V1 Hub Control");
}

void loop() {
  if (Serial.available()) {
    // Wait for start byte
    if (Serial.read() == 0xFF) {
      inside_packet = true;
    }

    // Wait for 8 data bytes + 1 byte for power state, but not forever
    unsigned long start_time = millis();
    while (inside_packet && Serial.available() < 9) {
      if (millis() - start_time >= 50) {
        // Timeout occurred, clear Serial buffer
        while (Serial.available()) {
          Serial.read();
        }
        inside_packet = false; // Reset state
        return; // Exit the loop early
      }
      delay(1); // Small delay to allow data to arrive
    }

    if (inside_packet && Serial.available() >= 9) {
      inside_packet = false;
      for (size_t i = 0; i < 8; i++) {
        buffer[i] = Serial.read();
      }
      bool wished_power_state = Serial.read() >> 1 & 0x01;

      // Load mode briefly to read inputs
      digitalWrite(SHIFT_MODE, LOW);
      digitalWrite(CLOCK_PIN, HIGH); // Clock pulse must be present when reading inputs
      digitalWrite(SHIFT_MODE, HIGH);

      byte in_byte_buffer = 0;
      for (size_t i = 0; i < 64; i++) {
        // Write value
        digitalWrite(DATA_OUT_PIN, bitRead(buffer[i / 8], i % 8) ? HIGH : LOW);
        // Clock pulse
        digitalWrite(CLOCK_PIN, HIGH);
        digitalWrite(CLOCK_PIN, LOW);
        // Read input
        bool in_value = digitalRead(DATA_IN_PIN);

        in_byte_buffer = (in_byte_buffer << 1) | in_value;
        if (i % 8 == 7) {
          RS485_print(in_byte_buffer);
        }
      }

      // Set latch after 64 bits, now all bits should be in the correct register
      digitalWrite(LATCH_PIN, HIGH);
      digitalWrite(LATCH_PIN, LOW);

      // Set power state
      digitalWrite(POWER_OUTPUT, wished_power_state ? HIGH : LOW);

      // Additional values for power state
      RS485_print(wished_power_state << 1 | digitalRead(POWER_FEEDBACK_PIN));
    }
  }
  delay(1);
}
