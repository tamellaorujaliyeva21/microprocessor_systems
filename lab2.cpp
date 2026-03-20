#include <Arduino.h>

// Function to print an 8-bit value in binary format
static void printByteBin(uint8_t v) {
  for (int i = 7; i >= 0; i--) 
    Serial.print((v >> i) & 1); // Shift and mask each bit
}

void setup() {
  Serial.begin(9600); // Start serial communication

  // Print program title and instructions
  Serial.println();
  Serial.println("=== Lab2: AVR SUB using CPU registers + SREG flags ===");
  Serial.println("Type two unsigned numbers (0..255) like: 10 5  (press Enter)");
  Serial.println("Test suggestions: 10 5 | 5 10 | 7 7");
  Serial.println();
}  

void loop() {

  // Wait until user inputs data
  if (Serial.available() <= 0) return;

  // Read two integers from Serial input
  long a_in = Serial.parseInt();   
  long b_in = Serial.parseInt();   

  // Ignore negative inputs
  if (a_in < 0 || b_in < 0) return;

  // Limit values to 0–255 and convert to 8-bit unsigned
  uint8_t a = (uint8_t)constrain(a_in, 0, 255);
  uint8_t b = (uint8_t)constrain(b_in, 0, 255);

  // Variables to store result and status register
  uint8_t result = 0;
  uint8_t sreg_after = 0;

  // Inline assembly block (AVR instructions)
  asm volatile(
    "mov r16, %[A]              \n\t" // Move A into register r16
    "mov r17, %[B]              \n\t" // Move B into register r17
    "sub r16, r17               \n\t" // Perform subtraction r16 = r16 - r17
    "mov %[RES], r16            \n\t" // Store result back into variable
    "in  %[SREGOUT], __SREG__   \n\t" // Read Status Register (SREG)
    : [RES]     "=r" (result),       // Output: result
      [SREGOUT] "=r" (sreg_after)   // Output: status register value
    : [A] "r" (a),                  // Input: a
      [B] "r" (b)                   // Input: b
    : "r16", "r17"                  // Clobbered registers
  );

  // Extract flags from SREG
  // Z (Zero flag) = bit 1 → result is zero
  // C (Carry flag) = bit 0 → borrow occurred in subtraction
  uint8_t Z = (sreg_after >> 1) & 1;
  uint8_t C = (sreg_after >> 0) & 1;

  // Print separator
  Serial.println("----------------------------------------------------");

  // Display input values in decimal and binary
  Serial.print("Input A = "); 
  Serial.print(a); 
  Serial.print(" (0b"); 
  printByteBin(a); 
  Serial.println(")");

  Serial.print("Input B = "); 
  Serial.print(b); 
  Serial.print(" (0b"); 
  printByteBin(b); 
  Serial.println(")");

  // Display subtraction result
  Serial.print("Result (A - B) = "); 
  Serial.print(result);
  Serial.print(" (0b"); 
  printByteBin(result); 
  Serial.println(")");

  // Display SREG value and extracted flags
  Serial.print("SREG after SUB = 0b"); 
  printByteBin(sreg_after);
  Serial.print("  (Z="); 
  Serial.print(Z);
  Serial.print(", C="); 
  Serial.print(C); 
  Serial.println(")");

  // Explain the result based on comparison
  if (a == b) {
    Serial.println("Explanation: A==B => result=0 => Z=1. No borrow => C=0.");
  } 
  else if (a > b) {
    Serial.println("Explanation: A>B => normal subtraction. Result !=0 => Z=0. No borrow => C=0.");
  } 
  else { 
    Serial.println("Explanation: A<B => unsigned underflow (wrap-around). Borrow occurred => C=1. Result wraps (256 - (B-A)).");
  }

  // Prompt for next input
  Serial.println("Type next pair (0..255)...");
  Serial.println();

  // Clear any remaining serial data (optional cleanup)
  while (Serial.available()) 
    Serial.read();
}
