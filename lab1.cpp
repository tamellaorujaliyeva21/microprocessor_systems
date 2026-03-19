// Method 1
void setup() { 
  pinMode(13, OUTPUT); 
}

void loop() {
  digitalWrite(13, HIGH); 
  delay(500);
  digitalWrite(13, LOW); 
  delay(500);
}

// Method 2

void setup() {
  DDRB |= (1 << DDB5);        // Set PB5 (pin 13) as output
}

void loop() {
  PORTB |= (1 << PORTB5);     // LED ON
  delay(500);
  PORTB &= ~(1 << PORTB5);    // LED OFF
  delay(500);
}
