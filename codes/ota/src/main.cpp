#include <WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <stdio.h>
#include <stdint.h>

// Define memory size and registers
#define MEMORY_SIZE 65536
uint8_t memory[MEMORY_SIZE];
uint8_t A, B, C;
const char* ssid = "SRKS";  // Replace with your network credentials
const char* password = "sankara1";


void OTAsetup() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    delay(5000);
    ESP.restart();
  }
  ArduinoOTA.begin();
}

void OTAloop() {
  ArduinoOTA.handle();
}

void mov_r_imm(uint8_t* dest, uint8_t imm) {
  *dest = imm;
}

void add_r(uint8_t* dest, uint8_t src) {
  uint16_t sum = *dest + src;
  *dest = sum & 0xFF;
  C = (sum > 0xFF) ? 1 : 0;
}

void initialize() {
  A = B = C = 0;
}

void emulate() {
  // Load program into memory
  memory[0] = 0x3E; // MVI A, 0x10
  memory[1] = 0x10;
  memory[2] = 0x06; // MVI B, 0x05
  memory[3] = 0x05;
  memory[4] = 0x80; // ADD B
  memory[5] = 0xC3; // JMP $ (infinite loop)
  memory[6] = 0x05;
  memory[7] = 0x00;

  uint16_t PC = 0;

  while (1) {
    uint8_t opcode = memory[PC];

    // Decode and execute the instruction
    switch (opcode) {
      case 0x3E:
        mov_r_imm(&A, memory[PC + 1]);
        PC += 2;
        break;
      case 0x06:
        mov_r_imm(&B, memory[PC + 1]);
        PC += 2;
        break;
      case 0x80:
        add_r(&A, B);
        PC++;
        break;
      case 0xC3:
        PC = (memory[PC + 2] << 8) | memory[PC + 1];
        break;
      default:
        // Handle unrecognized opcode
        break;
    }

    // Check for termination condition
    if (opcode == 0xC3 && PC == 0x0005) {
      break;
    }
  }

  // If A is zero, turn on the LED
  if (A == 0) {
    digitalWrite(2, HIGH);
  } else {
    digitalWrite(2, LOW);
  }
}

void setup() {
  OTAsetup();
  pinMode(2, OUTPUT);
}

void loop() {
  OTAloop();
  delay(10);  // If no custom loop code, ensure to have a delay in loop
  
  // Initialize the emulator
  initialize();

  // Start emulation
  emulate();
}

