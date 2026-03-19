#include <Arduino.h>

static void printByteBin(uint8_t v) {
  for (int i = 7; i >= 0; i--) Serial.print((v >> i) & 1);
}

void setup() {
  Serial.begin(9600);
  Serial.println();
  Serial.println("=== Lab2: AVR SUB using CPU registers + SREG flags ===");
  Serial.println("Type two unsigned numbers (0..255) like: 10 5  (press Enter)");
  Serial.println("Test suggestions: 10 5 | 5 10 | 7 7");
  Serial.println();
}  

void loop() {
  if (Serial.available() <= 0) return;

  long a_in = Serial.parseInt();   
  long b_in = Serial.parseInt();   

  if (a_in < 0 || b_in < 0) return;

  uint8_t a = (uint8_t)constrain(a_in, 0, 255);
  uint8_t b = (uint8_t)constrain(b_in, 0, 255);

  uint8_t result = 0;
  uint8_t sreg_after = 0;

  asm volatile(
    "mov r16, %[A]              \n\t"
    "mov r17, %[B]              \n\t"
    "sub r16, r17               \n\t"
    "mov %[RES], r16            \n\t"
    "in  %[SREGOUT], __SREG__   \n\t"
    : [RES]     "=r" (result),
      [SREGOUT] "=r" (sreg_after)
    : [A] "r" (a),
      [B] "r" (b)
    : "r16", "r17"
  );

  // Extract flags:
  // Z flag = bit 1, C flag = bit 0
  uint8_t Z = (sreg_after >> 1) & 1;
  uint8_t C = (sreg_after >> 0) & 1;

  Serial.println("----------------------------------------------------");
  Serial.print("Input A = "); Serial.print(a); Serial.print(" (0b"); printByteBin(a); Serial.println(")");
  Serial.print("Input B = "); Serial.print(b); Serial.print(" (0b"); printByteBin(b); Serial.println(")");

  Serial.print("Result (A - B) = "); Serial.print(result);
  Serial.print(" (0b"); printByteBin(result); Serial.println(")");

  Serial.print("SREG after SUB = 0b"); printByteBin(sreg_after);
  Serial.print("  (Z="); Serial.print(Z);
  Serial.print(", C="); Serial.print(C); Serial.println(")");

  if (a == b) {
    Serial.println("Explanation: A==B => result=0 => Z=1. No borrow => C=0.");
  } else if (a > b) {
    Serial.println("Explanation: A>B => normal subtraction. Result !=0 => Z=0. No borrow => C=0.");
  } else { 
    Serial.println("Explanation: A<B => unsigned underflow (wrap-around). Borrow occurred => C=1. Result wraps (256 - (B-A)).");
  }

  Serial.println("Type next pair (0..255)...");
  Serial.println();

  // Flush remaining characters (optional cleanup)
  while (Serial.available()) Serial.read();
}
