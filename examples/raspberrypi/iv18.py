import RPi.GPIO as GPIO
import spidev
import time

GPIO.setmode(GPIO.BCM)

DISP_LOAD = 23                     # driver board LOAD pin
DISP_BLANK = 18                    # Driver board BLANK pin

DISP_BRIGHTNESS = 6                # Brightness value from 1 - 10

# Grid positions  IV18 / MAX6921
GRID_8 = 0x800000 # OUT0 / D1 (right side of tube, last full segment)
GRID_7 = 0x400000 # OUT1 / D2
GRID_6 = 0x200000 # OUT2 / D3
GRID_5 = 0x100000 # OUT3 / D4
GRID_4 = 0x080000 # OUT4 / D5
GRID_3 = 0x040000 # OUT5 / D6
GRID_2 = 0x020000 # OUT6 / D7
GRID_1 = 0x010000 # OUT7 / D8 (left side of tube, first full segment)
GRID_0 = 0x008000 # OUT8 / D9 (dot / dash - very left of tube)

# Segments
SEG_DEGREE = 0x000080 # OUT 16
SEG_MINUS  = 0x000100 # OUT 15
SEG_DOT    = 0x000080 # OUT 16

SEG_A = 0x004000 # OUT 15
SEG_B = 0x002000 # OUT 14
SEG_C = 0x001000 # OUT 13
SEG_D = 0x000800 # OUT 12
SEG_E = 0x000400 # OUT 11
SEG_F = 0x000200 # OUT 10
SEG_G = 0x000100 # OUT 9

# Digits and letters
DIGIT_0 = SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F
DIGIT_1 = SEG_B | SEG_C
DIGIT_2 = SEG_A | SEG_B | SEG_D | SEG_E | SEG_G
DIGIT_3 = SEG_A | SEG_B | SEG_C | SEG_D | SEG_G
DIGIT_4 = SEG_B | SEG_C | SEG_F | SEG_G
DIGIT_5 = SEG_A | SEG_C | SEG_D | SEG_F | SEG_G
DIGIT_6 = SEG_A | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G
DIGIT_7 = SEG_A | SEG_B | SEG_C
DIGIT_8 = SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G
DIGIT_9 = SEG_A | SEG_B | SEG_C | SEG_D | SEG_F | SEG_G

CHAR_A = SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G
CHAR_B = SEG_C | SEG_D | SEG_E | SEG_F | SEG_G
CHAR_C = SEG_A | SEG_D | SEG_E | SEG_F
CHAR_D = SEG_B | SEG_C | SEG_D | SEG_E | SEG_G
CHAR_E = SEG_A | SEG_D | SEG_E | SEG_F | SEG_G
CHAR_F = SEG_A | SEG_E | SEG_F | SEG_G
CHAR_G = SEG_A | SEG_C | SEG_D | SEG_E | SEG_F
CHAR_H = SEG_B | SEG_C | SEG_E | SEG_F | SEG_G
CHAR_I = SEG_E | SEG_F
CHAR_J = SEG_B | SEG_C | SEG_D | SEG_E
CHAR_K = SEG_A | SEG_C | SEG_E | SEG_F | SEG_G
CHAR_L = SEG_D | SEG_E | SEG_F
CHAR_M = SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G
CHAR_N = SEG_A | SEG_B | SEG_C | SEG_E | SEG_F
CHAR_O = SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F
CHAR_P = SEG_A | SEG_B | SEG_E | SEG_F | SEG_G
CHAR_Q = SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G
CHAR_R = SEG_E | SEG_G
CHAR_S = SEG_A | SEG_C | SEG_D | SEG_F | SEG_G
CHAR_T = SEG_D | SEG_E | SEG_F | SEG_G
CHAR_U = SEG_B | SEG_C | SEG_D | SEG_E | SEG_F
CHAR_V = SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G
CHAR_W = SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G
CHAR_X = SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G
CHAR_Y = SEG_B | SEG_C | SEG_D | SEG_F | SEG_G
CHAR_Z = SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G

grids = [GRID_0, GRID_1, GRID_2, GRID_3, GRID_4, GRID_5, GRID_6, GRID_7, GRID_8]
letters = [CHAR_A, CHAR_B, CHAR_C, CHAR_D, CHAR_E, CHAR_F, CHAR_G, CHAR_H, CHAR_I, CHAR_J, CHAR_K, CHAR_L, CHAR_M, CHAR_N, CHAR_O, CHAR_P, CHAR_Q, CHAR_R, CHAR_S, CHAR_T, CHAR_U, CHAR_V, CHAR_W, CHAR_X, CHAR_Y, CHAR_Z]
digits = [DIGIT_0, DIGIT_1, DIGIT_2, DIGIT_3, DIGIT_4, DIGIT_5, DIGIT_6, DIGIT_7, DIGIT_8, DIGIT_9]

message = " HELLO"
grid_index = 0

spi = spidev.SpiDev(0, 0)
spi.max_speed_hz = 5000


# Convert ASCII to grid segment value
def char_to_code(char):
    if not char:
        return 0

    val = ord(char)

    # Uppercase letters
    if (val <= 90 and val >= 65):
        return letters[val - 65]

    # Lowercase letters
    if (val <= 122 and val >= 97):
        return letters[val - 97]

    # Digits
    if (val <= 57 and val >= 48):
        return digits[val - 48]

    if (val == "."):
        return SEG_DOT

    if (val == "*"):
        return SEG_DEGREE

    if (val == "-"):
        return SEG_MINUS

    return 0


# Sets up GPIO pins
def display_init():
    print("Display: Init")

    global pwm

    GPIO.setup(DISP_LOAD, GPIO.OUT)
    #GPIO.output(DISP_LOAD, 1)

    GPIO.setup(DISP_BLANK, GPIO.OUT)
    display_blank()

    pwm = GPIO.PWM(DISP_BLANK, 1000)
    pwm.start(0)


# Turns display off (Force MAX6921 outputs OUT0 to OUT19 low)
def display_blank():
    print("Display: Blank")
    GPIO.output(DISP_BLANK, 1)


# Turns the display on
def display_unblank():
    print("Display: Un-blank")
    GPIO.output(DISP_BLANK, 0)


# Convert the grid/segment to bytes and send to diplay via SPI
def display_write(val):
    spi.xfer2(val.to_bytes(3, 'little'))

    #spi.xfer2([0b00000000, 0b11101100, 0b10000000]) # H / Grid 2
    #spi.xfer2([0b00000000, 0b11110010, 0b01000000]) # E / Grid 3
    #spi.xfer2([0b00000000, 0b01110000, 0b00100000]) # L / Grid 4
    #spi.xfer2([0b00000000, 0b01110000, 0b00010000]) # L / Grid 5
    #spi.xfer2([0b00000000, 0b01111110, 0b00001000]) # O / Grid 6


# Loops over the grid and update the display
def update_display():
    global grid_index

    val = grids[grid_index] | char_to_code(message[grid_index:grid_index+1])

    display_write(val)

    grid_index += 1
    if (grid_index >= 9):
        grid_index = 0

try:
    display_init()
    display_unblank()

    while True:
        update_display()
        time.sleep(0.01)

except KeyboardInterrupt:
    print("GPIO: Cleanup")
    pwm.stop()
    GPIO.cleanup()
