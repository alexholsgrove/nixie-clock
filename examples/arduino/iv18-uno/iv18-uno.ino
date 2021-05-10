// IV-18 Test
//
// MAX6921 datasheet:
// https://datasheets.maximintegrated.com/en/ds/MAX6921-MAX6931.pdf
// Send bits from D19 first to D0 last
//
// Timer calculator:
// http://www.8bit-era.cz/arduino-timer-interrupts-calculator.html
//

#include "SPI.h"

// Arduino connections to driver board
#define DISP_CLK 13   // White - SPI SCK 
#define DISP_DATA 11  // Yellow - SPI MOSI
#define DISP_LOAD 2   // Blue
#define DISP_BLANK 3  // Green

#define DISP_BRIGHTNESS 6 // 0-10

// The number of grid positions
#define DISP_WIDTH 10

// Grid positions              IV18 / MAX6921
#define GRID_8     0x800000 // OUT0 / D1 (right side of tube, last full segment)
#define GRID_7     0x400000 // OUT1 / D2
#define GRID_6     0x200000 // OUT2 / D3
#define GRID_5     0x100000 // OUT3 / D4
#define GRID_4     0x080000 // OUT4 / D5
#define GRID_3     0x040000 // OUT5 / D6
#define GRID_2     0x020000 // OUT6 / D7
#define GRID_1     0x010000 // OUT7 / D8 (left side of tube, first full segment)
#define GRID_0     0x008000 // OUT8 / D9 (dot / dash - very left of tube)

// Segments
#define SEG_A      0x004000 // OUT 9
#define SEG_B      0x002000 // OUT 10
#define SEG_C      0x001000 // OUT 11
#define SEG_D      0x000800 // OUT 12
#define SEG_E      0x000400 // OUT 13
#define SEG_F      0x000200 // OUT 14
#define SEG_G      0x000100 // OUT 15
#define SEG_MINUS  0x000100 // OUT 15
#define SEG_DEGREE 0x000080 // OUT 16
#define SEG_DOT    0x000080 // OUT 16

// Digits and letters
#define DIGIT_0   SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F
#define DIGIT_1   SEG_B | SEG_C
#define DIGIT_2   SEG_A | SEG_B | SEG_D | SEG_E | SEG_G
#define DIGIT_3   SEG_A | SEG_B | SEG_C | SEG_D | SEG_G
#define DIGIT_4   SEG_B | SEG_C | SEG_F | SEG_G
#define DIGIT_5   SEG_A | SEG_C | SEG_D | SEG_F | SEG_G
#define DIGIT_6   SEG_A | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G
#define DIGIT_7   SEG_A | SEG_B | SEG_C
#define DIGIT_8   SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G
#define DIGIT_9   SEG_A | SEG_B | SEG_C | SEG_D | SEG_F | SEG_G

#define CHAR_A   SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G
#define CHAR_B   SEG_C | SEG_D | SEG_E | SEG_F | SEG_G
#define CHAR_C   SEG_A | SEG_D | SEG_E | SEG_F
#define CHAR_D   SEG_B | SEG_C | SEG_D | SEG_E | SEG_G
#define CHAR_E   SEG_A | SEG_D | SEG_E | SEG_F | SEG_G
#define CHAR_F   SEG_A | SEG_E | SEG_F | SEG_G
#define CHAR_G   SEG_A | SEG_C | SEG_D | SEG_E | SEG_F
#define CHAR_H   SEG_B | SEG_C | SEG_E | SEG_F | SEG_G
#define CHAR_I   SEG_E | SEG_F
#define CHAR_J   SEG_B | SEG_C | SEG_D | SEG_E
#define CHAR_K   SEG_A | SEG_C | SEG_E | SEG_F | SEG_G
#define CHAR_L   SEG_D | SEG_E | SEG_F
#define CHAR_M   SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G
#define CHAR_N   SEG_A | SEG_B | SEG_C | SEG_E | SEG_F
#define CHAR_O   SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F
#define CHAR_P   SEG_A | SEG_B | SEG_E | SEG_F | SEG_G
#define CHAR_Q   SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G
#define CHAR_R   SEG_E | SEG_G
#define CHAR_S   SEG_A | SEG_C | SEG_D | SEG_F | SEG_G
#define CHAR_T   SEG_D | SEG_E | SEG_F | SEG_G
#define CHAR_U   SEG_B | SEG_C | SEG_D | SEG_E | SEG_F
#define CHAR_V   SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G
#define CHAR_W   SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G
#define CHAR_X   SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G
#define CHAR_Y   SEG_B | SEG_C | SEG_D | SEG_F | SEG_G
#define CHAR_Z   SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G

long grids[] = {GRID_0, GRID_1, GRID_2, GRID_3, GRID_4, GRID_5, GRID_6, GRID_7, GRID_8};
long letters[] = {CHAR_A, CHAR_B, CHAR_C, CHAR_D, CHAR_E, CHAR_F, CHAR_G, CHAR_H, CHAR_I, CHAR_J, CHAR_K, CHAR_L, CHAR_M, CHAR_N, CHAR_O, CHAR_P, CHAR_Q, CHAR_R, CHAR_S, CHAR_T, CHAR_U, CHAR_V, CHAR_W, CHAR_X, CHAR_Y, CHAR_Z};
long digits[] = {DIGIT_0, DIGIT_1, DIGIT_2, DIGIT_3, DIGIT_4, DIGIT_5, DIGIT_6, DIGIT_7, DIGIT_8, DIGIT_9};

// Digits and text to display
static char display_contents[DISP_WIDTH];

// Index for the grid position
static volatile uint8_t display_idx;

// The characters to display
String message;


// DEBUGGING
void print_binary(uint32_t number, byte Length) {
  static int Bits;
  if (number) { //The remaining bits have a value greater than zero continue
    Bits++; // Count the number of bits so we know how many leading zeros to print first
    print_binary(number >> 1, Length); // Remove a bit and do a recursive call this function.
    if (Bits) for (byte x = (Length - Bits); x; x--)Serial.write('0'); // Add the leading zeros
    Bits = 0; // clear no need for this any more
    Serial.write((number & 1) ? '1' : '0'); // print the bits in reverse order as we depart the recursive function
  }
}


// Convert ASCII to hex value
long char_to_code(char c) {
  if ((c <= 0x5A) && (c >= 0x41)) { // Uppercase letters
    return letters[c - 0x41];
  } else if ((c <= 0x7A) && (c >= 0x61)) { // Lowercase letters
    return letters[c - 0x61];
  } else if ((c <= 0x39) && (c >= 0x30)) { // Digits
    return digits[c - 0x30];
  } else if (c == '.') {
    return SEG_DOT;
  } else if (c == '*') {
    return SEG_DEGREE;
  } else if (c == '-') {
    return SEG_MINUS;
  } else if (c == ' ') {
    return 0x0;
  }

  return 0x0;
}


//
void display_init() {
  Serial.println("Display Init");

  pinMode(DISP_LOAD, OUTPUT);
  digitalWrite(DISP_LOAD, HIGH);

  pinMode(DISP_BLANK, OUTPUT);
  display_blank();
}


// Turns display off (Force MAX6921 outputs OUT0 to OUT19 low)
void display_blank() {
  digitalWrite(DISP_BLANK, HIGH);
}


// Turns display back on at the defined brightness
void display_unblank() {
  int brightness = map(DISP_BRIGHTNESS, 0, 10, 255, 0);
  analogWrite(DISP_BLANK, brightness);
}


// Assigns the message into the dsiplay contents array
void display_message(const char* message) {
  strncpy(display_contents, message, DISP_WIDTH);
}


// Sends data to the MAX6821 
void display_write(uint32_t data) {
  SPI.transfer(data & 0xff);
  SPI.transfer((data>> 8) & 0xff);
  SPI.transfer((data>> 16) & 0xff);
}


// Timer 1
ISR(TIMER1_COMPA_vect) {
  long j;

  if (display_idx == 0 && (display_contents[display_idx] == '-' || display_contents[display_idx] == '*')) {
    j = grids[display_idx] | char_to_code(display_contents[display_idx]);
  } else {
    j = grids[display_idx] | char_to_code(display_contents[display_idx]);
  }

  display_write(j);

  display_idx++;
  if (display_idx >= DISP_WIDTH-1)
    display_idx = 0;
}


/*
 * Sets up the timer
 * @link http://www.8bit-era.cz/arduino-timer-interrupts-calculator.html
 */
void display_timer_init() {
  Serial.println("Display Timer Init");

/*
// TIMER 1 for interrupt frequency 1 Hz:
cli(); // stop interrupts
TCCR1A = 0; // set entire TCCR1A register to 0
TCCR1B = 0; // same for TCCR1B
TCNT1  = 0; // initialize counter value to 0
// set compare match register for 1 Hz increments
OCR1A = 62499; // = 16000000 / (256 * 1) - 1 (must be <65536)
// turn on CTC mode
TCCR1B |= (1 << WGM12);
// Set CS12, CS11 and CS10 bits for 256 prescaler
TCCR1B |= (1 << CS12) | (0 << CS11) | (0 << CS10);
// enable timer compare interrupt
TIMSK1 |= (1 << OCIE1A);
sei(); // allow interrupts
*/

  // TIMER 1 for interrupt frequency 1000 Hz:
  cli(); // stop interrupts
  TCCR1A = 0; // set entire TCCR1A register to 0
  TCCR1B = 0; // same for TCCR1B
  TCNT1  = 0; // initialize counter value to 0
  // set compare match register for 1000 Hz increments
  OCR1A = 15999; // = 16000000 / (1 * 1000) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS12, CS11 and CS10 bits for 1 prescaler
  TCCR1B |= (0 << CS12) | (0 << CS11) | (1 << CS10);
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  sei(); // allow interrupts

  digitalWrite(DISP_LOAD, HIGH);
  display_unblank();
}


void setup() {
  Serial.begin(115200);
  Serial.println("IV18 Test");

  SPI.begin();
  SPI.setBitOrder(LSBFIRST);
  
  display_init();
  display_timer_init();

  display_message(" HELLO");
}


void loop() {
  message = "";

  // send data only when you receive data:
  if (Serial.available() > 0) {
    message = Serial.readString();
    Serial.println(message);
    char *p = const_cast<char*>(message.c_str());
    display_message(p);
  }
}
