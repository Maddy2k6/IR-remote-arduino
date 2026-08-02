#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <IRremote.hpp>
#include <ctype.h>
#include <string.h>

#define IR_PIN 2

LiquidCrystal_I2C lcd(0x27, 16, 2);

// =====================================================
// REMOTE CODES
// (Same codes are used over Serial: send ONE raw byte
//  equal to the code below and it's treated exactly like
//  that button being pressed on the IR remote.)
// =====================================================

#define KEY_0     0x19
#define KEY_1     0x45
#define KEY_2     0x46
#define KEY_3     0x47
#define KEY_4     0x44
#define KEY_5     0x40
#define KEY_6     0x43
#define KEY_7     0x07
#define KEY_8     0x15
#define KEY_9     0x09

#define KEY_OK    0x1C
#define KEY_BACK  0x0D
#define KEY_LEFT  0x08
#define KEY_RIGHT 0x5A

#define KEY_MUTE  0x16   // ABC <-> 123
#define KEY_UP    0x18   // CAPITAL
#define KEY_DOWN  0x52   // small

// =====================================================
// NOKIA KEYPAD
// =====================================================

const char* keypad[] = {
  " ",
  "1",
  "ABC",
  "DEF",
  "GHI",
  "JKL",
  "MNO",
  "PQRS",
  "TUV",
  "WXYZ"
};

// =====================================================
// EMOJI LIBRARY
// =====================================================

#define EMOJI_COUNT 16

const byte emojis[EMOJI_COUNT][8] PROGMEM = {
  { B00000, B01010, B01010, B00000, B10001, B01110, B00000, B00000 }, // 0 - HAPPY
  { B00000, B01010, B01010, B00000, B01110, B10001, B00000, B00000 }, // 1 - SAD
  { B00000, B01010, B11111, B11111, B11111, B01110, B00100, B00000 }, // 2 - HEART
  { B10001, B01010, B00000, B01010, B00000, B10001, B01110, B00000 }, // 3 - ANGRY
  { B00000, B01010, B01010, B00000, B00100, B01010, B00100, B00000 }, // 4 - SURPRISED
  { B00000, B01000, B00010, B00000, B10001, B01110, B00000, B00000 }, // 5 - WINK
  { B00000, B01010, B01010, B00000, B11111, B10101, B01110, B00000 }, // 6 - LAUGH
  { B00000, B11111, B10101, B00000, B10001, B01110, B00000, B00000 }, // 7 - COOL
  { B00100, B10101, B01110, B11111, B01110, B10101, B00100, B00000 }, // 8 - STAR
  { B00000, B00001, B00010, B10100, B01000, B00000, B00000, B00000 }, // 9 - CHECK
  { B00010, B00011, B00010, B00010, B01110, B11110, B01100, B00000 }, // 10 - MUSIC
  { B00100, B01110, B01110, B01110, B11111, B00100, B00000, B00000 }, // 11 - BELL
  { B00100, B01110, B10101, B00100, B00100, B00100, B00000, B00000 }, // 12 - UP ARROW
  { B00100, B00100, B00100, B10101, B01110, B00100, B00000, B00000 }, // 13 - DOWN ARROW
  { B01110, B11011, B10001, B10101, B10101, B10001, B11111, B00000 }, // 14 - BATTERY
  { B01110, B11111, B10101, B11111, B01110, B01010, B10001, B00000 }  // 15 - ALIEN
};

// =====================================================
// CGRAM EMOJI MANAGEMENT (MAX 8 CGRAM SLOTS)
// =====================================================

int activeCgramEmojis[8] = {-1, -1, -1, -1, -1, -1, -1, -1};
int cgramCount = 0;

int getOrAllocCgramSlot(int emojiIdx) {
  // Check if emoji is already in CGRAM
  for (int i = 0; i < cgramCount; i++) {
    if (activeCgramEmojis[i] == emojiIdx) {
      return i;
    }
  }

  // Allocate new slot if room permits
  if (cgramCount < 8) {
    int slot = cgramCount;
    activeCgramEmojis[slot] = emojiIdx;

    byte pattern[8];
    for (int b = 0; b < 8; b++) {
      pattern[b] = pgm_read_byte(&(emojis[emojiIdx][b]));
    }
    lcd.createChar(slot, pattern);
    cgramCount++;
    return slot;
  }

  // Fallback if all 8 custom slots are filled
  return 0;
}

// =====================================================
// TEXT VARIABLES
// =====================================================

char textBuffer[32];
int cursorPos = 0;

int lastKey = -1;
int tapIndex = 0;
unsigned long lastPressTime = 0;
const unsigned long MULTITAP_TIMEOUT = 900;

// =====================================================
// MODES
// =====================================================

bool alphabetMode = true;
bool capitalMode = true;

// =====================================================
// EMOJI VARIABLES
// =====================================================

bool emojiSelecting = false;
int emojiIndex = 0;
unsigned long emojiLastPress = 0;
const unsigned long EMOJI_TIMEOUT = 900;

// =====================================================
// CURSOR & LCD DISPLAY
// =====================================================

void setCursorPosition() {
  if (cursorPos < 16) {
    lcd.setCursor(cursorPos, 0);
  } else {
    lcd.setCursor(cursorPos - 16, 1);
  }
}

void redrawLCD() {
  lcd.clear();

  lcd.setCursor(0, 0);
  for (int i = 0; i < 16; i++) {
    lcd.write(textBuffer[i]);
  }

  lcd.setCursor(0, 1);
  for (int i = 16; i < 32; i++) {
    lcd.write(textBuffer[i]);
  }

  setCursorPosition();
}

// =====================================================
// CONFIRMATIONS
// =====================================================

void confirmCharacter() {
  if (lastKey != -1) {
    if (cursorPos < 31) {
      cursorPos++;
    }
    lastKey = -1;
    tapIndex = 0;
    setCursorPosition();
  }
}

void confirmEmoji() {
  if (emojiSelecting) {
    int slot = getOrAllocCgramSlot(emojiIndex);

    // Store CGRAM character code in buffer (slot 0..7)
    textBuffer[cursorPos] = (char)slot;
    emojiSelecting = false;

    if (cursorPos < 31) {
      cursorPos++;
    }

    setCursorPosition();
    Serial.println("EMOJI CONFIRMED");
  }
}

// =====================================================
// DISPLAY CURRENT EMOJI PREVIEW
// =====================================================

void displayEmoji() {
  int tempSlot = getOrAllocCgramSlot(emojiIndex);

  setCursorPosition();
  lcd.write(byte(tempSlot));

  emojiLastPress = millis();

  Serial.print("Emoji preview index: ");
  Serial.println(emojiIndex + 1);
}

void handleEmojiButton() {
  confirmCharacter();

  if (!emojiSelecting) {
    emojiSelecting = true;
    emojiIndex = 0;
  } else {
    emojiIndex++;
    if (emojiIndex >= EMOJI_COUNT) {
      emojiIndex = 0;
    }
  }

  displayEmoji();
}

// =====================================================
// IR DECODER & MODE HELPERS
// =====================================================

int getNumber(uint8_t command) {
  switch (command) {
    case KEY_0: return 0;
    case KEY_1: return 1;
    case KEY_2: return 2;
    case KEY_3: return 3;
    case KEY_4: return 4;
    case KEY_5: return 5;
    case KEY_6: return 6;
    case KEY_7: return 7;
    case KEY_8: return 8;
    case KEY_9: return 9;
    default: return -1;
  }
}

void showMode() {
  lcd.noCursor();
  lcd.noBlink();
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("MODE:");

  lcd.setCursor(0, 1);
  if (!alphabetMode) {
    lcd.print("123 NUMBERS");
  } else if (capitalMode) {
    lcd.print("ABC CAPITAL");
  } else {
    lcd.print("abc small");
  }

  delay(600);
  redrawLCD();

  lcd.cursor();
  lcd.blink();
}

// =====================================================
// COMMAND PROCESSING
// Shared by both the IR receiver and incoming Serial bytes,
// so a click on the webpage produces exactly the same result
// as pressing the matching button on the physical remote.
//
// fromIR = true  -> came from the IR sensor, print debug line
// fromIR = false -> came from Serial (webpage), stay quiet so
//                    the webpage doesn't see its own echo
// =====================================================

void processCommand(uint8_t command, bool fromIR) {

  if (fromIR) {
    Serial.print("Button: 0x");
    Serial.println(command, HEX);
  }

  // OK -> EMOJI SELECTOR
  if (command == KEY_OK) {
    handleEmojiButton();
  }

  // MUTE -> ABC / 123 TOGGLE
  else if (command == KEY_MUTE) {
    confirmEmoji();
    confirmCharacter();

    alphabetMode = !alphabetMode;

    if (alphabetMode) {
      Serial.println("ALPHABET MODE");
    } else {
      Serial.println("NUMBER MODE");
    }

    showMode();
  }

  // UP -> CAPITAL MODE
  else if (command == KEY_UP) {
    confirmEmoji();
    confirmCharacter();

    alphabetMode = true;
    capitalMode = true;

    Serial.println("ABC CAPITAL MODE");
    showMode();
  }

  // DOWN -> LOWERCASE MODE
  else if (command == KEY_DOWN) {
    confirmEmoji();
    confirmCharacter();

    alphabetMode = true;
    capitalMode = false;

    Serial.println("abc small mode");
    showMode();
  }

  // BACKSPACE
  else if (command == KEY_BACK) {
    if (emojiSelecting) {
      emojiSelecting = false;
      redrawLCD();
    } else if (lastKey != -1) {
      lastKey = -1;
      tapIndex = 0;
      textBuffer[cursorPos] = ' ';
      redrawLCD();
    } else if (cursorPos > 0) {
      cursorPos--;
      textBuffer[cursorPos] = ' ';
      redrawLCD();
    }

    Serial.println("BACKSPACE");
  }

  // LEFT
  else if (command == KEY_LEFT) {
    confirmEmoji();
    confirmCharacter();

    if (cursorPos > 0) {
      cursorPos--;
    }

    setCursorPosition();
    Serial.println("LEFT");
  }

  // RIGHT
  else if (command == KEY_RIGHT) {
    confirmEmoji();
    confirmCharacter();

    if (cursorPos < 31) {
      cursorPos++;
    }

    setCursorPosition();
    Serial.println("RIGHT");
  }

  // NUMBER / LETTER KEYS
  else {
    int number = getNumber(command);

    if (number != -1) {
      confirmEmoji();

      // NUMBER MODE
      if (!alphabetMode) {
        confirmCharacter();

        textBuffer[cursorPos] = '0' + number;
        redrawLCD();

        if (cursorPos < 31) {
          cursorPos++;
        }

        setCursorPosition();
      }

      // ALPHABET MODE
      else {
        // KEY 0 -> SPACE
        if (number == 0) {
          confirmCharacter();

          textBuffer[cursorPos] = ' ';
          redrawLCD();

          if (cursorPos < 31) {
            cursorPos++;
          }

          setCursorPosition();
        }

        // KEY 1 -> NUMBER 1
        else if (number == 1) {
          confirmCharacter();

          textBuffer[cursorPos] = '1';
          redrawLCD();

          if (cursorPos < 31) {
            cursorPos++;
          }

          setCursorPosition();
        }

        // KEYS 2-9 -> MULTI-TAP LETTERS
        else {
          if (number == lastKey && (millis() - lastPressTime < MULTITAP_TIMEOUT)) {
            tapIndex++;
            int length = strlen(keypad[number]);

            if (tapIndex >= length) {
              tapIndex = 0;
            }
          } else {
            confirmCharacter();
            lastKey = number;
            tapIndex = 0;
          }

          char letter = keypad[number][tapIndex];

          if (!capitalMode) {
            letter = tolower(letter);
          }

          textBuffer[cursorPos] = letter;
          redrawLCD();

          lastPressTime = millis();
        }
      }
    }
  }
}

// =====================================================
// SETUP
// =====================================================

void setup() {
  Serial.begin(9600);

  for (int i = 0; i < 32; i++) {
    textBuffer[i] = ' ';
  }

  lcd.init();
  lcd.backlight();
  lcd.clear();

  lcd.setCursor(1, 0);
  lcd.print("IR TEXTING");
  lcd.setCursor(2, 1);
  lcd.print("ABC MODE");

  delay(1200);

  lcd.clear();
  lcd.cursor();
  lcd.blink();

  IrReceiver.begin(IR_PIN, DISABLE_LED_FEEDBACK);
  Serial.println("IR TEXTING READY");
}

// =====================================================
// LOOP
// =====================================================

void loop() {

  // Auto-confirm multi-tap letter timeout
  if (lastKey != -1) {
    if (millis() - lastPressTime > MULTITAP_TIMEOUT) {
      confirmCharacter();
    }
  }

  // Auto-confirm emoji selection timeout
  if (emojiSelecting) {
    if (millis() - emojiLastPress > EMOJI_TIMEOUT) {
      confirmEmoji();
    }
  }

  // IR Signal Receiver
  if (IrReceiver.decode()) {
    if (!(IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT)) {
      uint8_t command = IrReceiver.decodedIRData.command;
      processCommand(command, true);
    }
    IrReceiver.resume();
  }

  // Serial Input (from the webpage / Web Serial)
  // Send ONE raw byte equal to a KEY_ code above, e.g. 0x1C for OK.
  if (Serial.available() > 0) {
    uint8_t command = (uint8_t)Serial.read();
    processCommand(command, false);
  }
}
