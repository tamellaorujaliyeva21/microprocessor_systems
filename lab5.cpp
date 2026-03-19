#include <avr/io.h>
#include <avr/interrupt.h>

volatile uint32_t ms_counter = 0;
volatile uint8_t button_event = 0;
volatile uint8_t last_button = 1;
volatile uint8_t action_index = 0;
volatile uint8_t mode = 0;

extern "C" {
  void modeA_entry() __attribute__((used));
}

extern "C" {
  void modeB_entry() __attribute__((used));
}

extern "C" {
  void modeC_entry() __attribute__((used));
}

/* =========================================================
   ACTION HANDLERS (Task 5C)
   Each one jumps back to mode_loop (NOT return)
   ========================================================= */

extern "C" {
  void action0();
  void action1();
  void action2();
  void action3();
}

/* =========================================================
   TIMER 1 INIT (1ms tick @16MHz)
   ========================================================= */
void timer1_init() {
  TCCR1A = 0;
  TCCR1B = (1 << WGM12);          // CTC
  OCR1A = 249;                    // 1ms
  TCCR1B |= (1 << CS11) | (1 << CS10);  // prescaler 64
  TIMSK1 |= (1 << OCIE1A);
}

/* =========================================================
   TIMER ISR
   ========================================================= */
ISR(TIMER1_COMPA_vect) {
  ms_counter++;

  uint8_t current = PINB & (1 << PINB0);

  if (last_button && !current) {
    button_event = 1;
  }

  last_button = current;
}

/* =========================================================
   ACTION HANDLERS
   ========================================================= */

void go_back_to_mode() {
  if (mode == 1)
    asm volatile("rjmp modeA_loop");
  else if (mode == 2)
    asm volatile("rjmp modeB_loop");
  else
    asm volatile("rjmp modeC_loop");
}

void action0() {
  // Normal
  Serial.println(0);
  action_index++;

  go_back_to_mode();
}

void action1() {
  Serial.println(1);
  PORTB ^= (1 << PORTB5);
  action_index++;

  go_back_to_mode();
}

void action2() {
  Serial.println(2);
  // quick flash
  PORTB |= (1 << PORTB5);
  action_index++;

  go_back_to_mode();
}

void action3() {
  Serial.println(3);
  action_index = 0;

  go_back_to_mode();
}

typedef void (*handler_t)();

handler_t jump_table[] = {
  action0,
  action1,
  action2,
  action3
};

/* =========================================================
   IJMP DISPATCH
   ========================================================= */
void run_action() {
  handler_t target = jump_table[action_index];

  asm volatile(
    "mov r30, %A0    \n\t"
    "mov r31, %B0    \n\t"
    "ijmp            \n\t"
    :
    : "r" (target)
    : "r30", "r31"
  );
}

/* =========================================================
   WAIT LOOP (RJMP REQUIRED)
   ========================================================= */
void wait_for_press()
{
  asm volatile(
    "wait:\n\t"
    "sbic %[pinreg], %[bit]\n\t"
    "rjmp wait\n\t"
    :
    : [pinreg] "I" (_SFR_IO_ADDR(PINB)),
      [bit] "I" (PINB0)
  );
}

/* =========================================================
   MODE A — Slow Blink
   ========================================================= */
void modeA_entry()
{
  Serial.println("A");

  asm volatile ("modeA_loop:");
  mode = 1;
  uint32_t last_toggle = 0;

  while (1)
  {
    if (button_event) {
      button_event = 0;
      run_action();
    }

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

    if (ms_counter - last_toggle >= 1000) {
      last_toggle = ms_counter;

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
   MODE C — Fast Strobe
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

    if (ms_counter - last_toggle >= 100) {
      PORTB ^= (1 << PORTB5);
      last_toggle = ms_counter;
    }
  }
}

/* =========================================================
   SETUP
   ========================================================= */
void setup()
{
  Serial.begin(9600);

  asm(
    ".global modeA_loop\n"
    ".global modeB_loop\n"
    ".global modeC_loop\n"
  );

  DDRB |= (1 << DDB5);
  DDRB &= ~(1 << DDB0);
  PORTB |= (1 << PORTB0);

  timer1_init();
  sei();

  wait_for_press();

  PORTB |= (1 << PORTB5);
  _delay_ms(200);
  PORTB &= ~(1 << PORTB5);

  uint32_t start_time = ms_counter;
  uint8_t count = 0;
  button_event = 0;

  while (ms_counter - start_time < 2000) {
    if (button_event) {
      button_event = 0;
      count++;
    }
  }

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

void loop() {}
