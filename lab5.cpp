#include <avr/io.h>
#include <avr/interrupt.h>

// ================= GLOBAL VARIABLES =================

// Millisecond counter (updated by Timer1 interrupt)
volatile uint32_t ms_counter = 0;

// Flag set when button press is detected
volatile uint8_t button_event = 0;

// Stores previous button state (for edge detection)
volatile uint8_t last_button = 1;

// Index for action jump table (0–3)
volatile uint8_t action_index = 0;

// Current mode (A=1, B=2, C=3)
volatile uint8_t mode = 0;

// Declare mode entry functions (used with assembly jumps)
extern "C" {
  void modeA_entry() __attribute__((used));
  void modeB_entry() __attribute__((used));
  void modeC_entry() __attribute__((used));
}

/* =========================================================
   ACTION HANDLERS (Task 5C)
   Each action ends with a jump back to its mode loop
   ========================================================= */

extern "C" {
  void action0();
  void action1();
  void action2();
  void action3();
}

/* =========================================================
   TIMER 1 INITIALIZATION (1 ms tick @16 MHz)
   ========================================================= */
void timer1_init() {
  TCCR1A = 0;                     // Normal mode
  TCCR1B = (1 << WGM12);          // CTC mode (Clear Timer on Compare)
  OCR1A = 249;                    // Compare value → 1 ms interval
  TCCR1B |= (1 << CS11) | (1 << CS10);  // Prescaler = 64
  TIMSK1 |= (1 << OCIE1A);        // Enable compare interrupt
}

/* =========================================================
   TIMER INTERRUPT SERVICE ROUTINE
   Runs every 1 ms
   ========================================================= */
ISR(TIMER1_COMPA_vect) {
  ms_counter++; // Increment millisecond counter

  // Read current button state (PB0)
  uint8_t current = PINB & (1 << PINB0);

  // Detect falling edge (button press)
  if (last_button && !current) {
    button_event = 1;
  }

  // Save current state for next comparison
  last_button = current;
}

/* =========================================================
   ACTION HANDLERS
   ========================================================= */

// Jump back to the correct mode loop using assembly
void go_back_to_mode() {
  if (mode == 1)
    asm volatile("rjmp modeA_loop");
  else if (mode == 2)
    asm volatile("rjmp modeB_loop");
  else
    asm volatile("rjmp modeC_loop");
}

// Action 0: print 0 and move to next action
void action0() {
  Serial.println(0);
  action_index++;
  go_back_to_mode();
}

// Action 1: toggle LED and move to next action
void action1() {
  Serial.println(1);
  PORTB ^= (1 << PORTB5); // Toggle LED (pin 13)
  action_index++;
  go_back_to_mode();
}

// Action 2: turn LED ON briefly
void action2() {
  Serial.println(2);
  PORTB |= (1 << PORTB5);
  action_index++;
  go_back_to_mode();
}

// Action 3: reset action sequence
void action3() {
  Serial.println(3);
  action_index = 0;
  go_back_to_mode();
}

// Function pointer type for jump table
typedef void (*handler_t)();

// Jump table storing action handlers
handler_t jump_table[] = {
  action0,
  action1,
  action2,
  action3
};

/* =========================================================
   IJMP DISPATCH (Indirect Jump)
   ========================================================= */
void run_action() {
  handler_t target = jump_table[action_index];

  // Load address into Z register (r30:r31) and jump
  asm volatile(
    "mov r30, %A0    \n\t" // Low byte of address
    "mov r31, %B0    \n\t" // High byte of address
    "ijmp            \n\t" // Indirect jump
    :
    : "r" (target)
    : "r30", "r31"
  );
}

/* =========================================================
   WAIT LOOP (Busy wait using assembly)
   ========================================================= */
void wait_for_press()
{
  asm volatile(
    "wait:\n\t"
    "sbic %[pinreg], %[bit]\n\t" // Skip if button NOT pressed
    "rjmp wait\n\t"              // Loop until press detected
    :
    : [pinreg] "I" (_SFR_IO_ADDR(PINB)),
      [bit] "I" (PINB0)
  );
}

/* =========================================================
   MODE A — Slow Blink (500 ms)
   ========================================================= */
void modeA_entry()
{
  Serial.println("A");

  asm volatile ("modeA_loop:"); // Label for assembly jumps

  mode = 1;
  uint32_t last_toggle = 0;

  while (1)
  {
    // Run action if button pressed
    if (button_event) {
      button_event = 0;
      run_action();
    }

    // Toggle LED every 500 ms
    if (ms_counter - last_toggle >= 500) {
      PORTB ^= (1 << PORTB5);
      last_toggle = ms_counter;
    }
  }
}

/* =========================================================
   MODE B — Double Blink
   ========================================================= */
void modeB_entry()
{
  Serial.println("B");
  asm volatile ("modeB_loop:");

  mode = 2;
  uint32_t last_toggle = 0;
  uint8_t state = 0;

  while (1)
  {
    if (button_event) {
      button_event = 0;
      run_action();
    }

    // Change LED state every 1 second
    if (ms_counter - last_toggle >= 1000) {
      last_toggle = ms_counter;

      // Sequence: ON → OFF → ON → OFF
      if (state == 0) PORTB |= (1 << PORTB5);
      else if (state == 1) PORTB &= ~(1 << PORTB5);
      else if (state == 2) PORTB |= (1 << PORTB5);
      else PORTB &= ~(1 << PORTB5);

      state++;
      if (state > 3) state = 0;
    }
  }
}

/* =========================================================
   MODE C — Fast Strobe (100 ms)
   ========================================================= */
void modeC_entry()
{
  Serial.println("C");
  asm volatile ("modeC_loop:");

  mode = 3;
  uint32_t last_toggle = 0;

  while (1)
  {
    if (button_event) {
      button_event = 0;
      run_action();
    }

    // Fast LED toggle every 100 ms
    if (ms_counter - last_toggle >= 100) {
      PORTB ^= (1 << PORTB5);
      last_toggle = ms_counter;
    }
  }
}

/* =========================================================
   SETUP FUNCTION
   ========================================================= */
void setup()
{
  Serial.begin(9600);

  // Make assembly labels globally visible
  asm(
    ".global modeA_loop\n"
    ".global modeB_loop\n"
    ".global modeC_loop\n"
  );

  // Configure LED (PB5) as output
  DDRB |= (1 << DDB5);

  // Configure button (PB0) as input with pull-up
  DDRB &= ~(1 << DDB0);
  PORTB |= (1 << PORTB0);

  // Initialize timer and enable interrupts
  timer1_init();
  sei();

  // Wait for first button press
  wait_for_press();

  // LED blink as start signal
  PORTB |= (1 << PORTB5);
  _delay_ms(200);
  PORTB &= ~(1 << PORTB5);

  // Detect number of button presses within 2 seconds
  uint32_t start_time = ms_counter;
  uint8_t count = 0;
  button_event = 0;

  while (ms_counter - start_time < 2000) {
    if (button_event) {
      button_event = 0;
      count++;
    }
  }

  // Select mode based on number of presses
  if (count == 1)
  {
    asm volatile("jmp modeA_entry");
  }
  else if (count == 2)
  {
    asm volatile("jmp modeB_entry");
  }
  else
  {
    asm volatile("jmp modeC_entry");
  }
}

// Empty loop (not used since program jumps to modes)
void loop() {}
