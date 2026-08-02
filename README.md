IR-TXT9000 — Arduino IR Remote Texting Console

A Nokia-style text input system built using an Arduino, IR remote, IR receiver, and a 16×2 I2C LCD.

The project allows text to be entered using a normal IR remote with classic mobile-phone multi-tap typing. It supports uppercase and lowercase letters, numbers, spaces, cursor movement, backspace, and custom 5×8 LCD emojis. A browser-based interface (ir.html) can also communicate with the Arduino through Web Serial at 9600 baud.

Features

Nokia-style multi-tap typing using keys 2–9

Uppercase and lowercase alphabet modes

Number mode

Space input

Backspace/delete

Left and right cursor movement

16 custom 5×8 CGRAM emojis

Emoji cycling using the OK button

16×2 I2C LCD output

IR remote control

Serial debugging at 9600 baud

Browser-based LCD/remote simulator

Web Serial communication with Arduino

Physical remote and browser interface use the same IR command mapping

Hardware Required

Component

Quantity

Arduino Uno / compatible board

1

16×2 LCD with I2C module

1

IR receiver module

1

IR remote

1

Breadboard

1

Jumper wires

As required

USB cable

1

Connections

16×2 I2C LCD → Arduino Uno

LCD Pin

Arduino Uno

GND

GND

VCC

5V

SDA

A4

SCL

A5

The project uses the LCD I2C address:

LiquidCrystal_I2C lcd(0x27, 16, 2);

If your display does not respond, scan the I2C bus because some modules use another address such as 0x3F.

IR Receiver → Arduino

IR Receiver Pin

Arduino

OUT / Signal

D2

VCC

5V

GND

GND

The IR input pin is configured as:

#define IR_PIN 2

Check the pin order printed on your particular IR receiver module before connecting it. Receiver pinouts are not identical across every module.

Remote Control Functions



The project uses the following IR commands:

Remote Button

IR Command

Function

0

0x19

0 / Space

1

0x45

1

2

0x46

ABC

3

0x47

DEF

4

0x44

GHI

5

0x40

JKL

6

0x43

MNO

7

0x07

PQRS

8

0x15

TUV

9

0x09

WXYZ

OK

0x1C

Cycle/select emoji

* / MUTE

0x16

Toggle Alphabet ↔ Number mode

UP

0x18

Capital-letter mode

DOWN

0x52

Small-letter mode

LEFT

0x08

Move cursor left

RIGHT

0x5A

Move cursor right

BACK

0x0D

Backspace/delete

The supplied remote image should be stored in the repository as exactly:

rc.png

GitHub will then automatically display it in this README.

Multi-Tap Typing

The alphabet mode works like an old Nokia keypad.

Key

Characters

2

A B C

3

D E F

4

G H I

5

J K L

6

M N O

7

P Q R S

8

T U V

9

W X Y Z

0

Space

For example:

2       → A
2 2     → B
2 2 2   → C

7       → P
7 7     → Q
7 7 7   → R
7 7 7 7 → S

Repeated presses must occur before the multi-tap timeout. The project uses approximately 900 ms.

Input Modes

Capital Letters

Press UP.

ABC CAPITAL

Example:

HELLO

Small Letters

Press DOWN.

abc small

Example:

hello

Number Mode

Press MUTE to switch between alphabet and number input.

In number mode, pressing a numeric key directly inserts that number.

Space

In alphabet mode, press 0.

Backspace

Press the BACK button to remove the previous/current character.

Cursor Control

Use:

LEFT  → move cursor left
RIGHT → move cursor right

The text buffer represents the full 32 character positions of the 16×2 LCD.

Emoji System

The HD44780-compatible LCD cannot display normal Unicode emojis directly. Instead, the project creates custom 5×8 pixel characters using CGRAM.

The browser interface contains 16 custom designs, including:

Happy

Sad

Heart

Angry

Surprised

Wink

Laugh

Cool

Star

Check

Music

Bell

Up arrow

Down arrow

Battery

Alien

Press OK repeatedly to cycle through the available emoji designs. Stop pressing to confirm the selected emoji.

CGRAM Limitation

A standard HD44780 LCD provides only 8 CGRAM custom-character slots at one time. Therefore, although the project contains a larger emoji library, only a limited number of unique custom glyphs can be loaded simultaneously.

Arduino Libraries

Install the following libraries through Arduino IDE → Library Manager:

LiquidCrystal_I2C
IRremote

The sketch also uses Arduino's built-in:

#include <Wire.h>

Typical includes are:

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <IRremote.hpp>
#include <ctype.h>
#include <string.h>

Serial Connection

The Arduino communicates at:

9600 baud

The firmware initializes serial communication with:

Serial.begin(9600);

IR button events can be printed in a format such as:

Button: 0x46
Button: 0x46
Button: 0x47

This format is also understood by the browser interface.

Web Interface

ir.html provides a cyberpunk-style browser interface that simulates the 16×2 LCD and remote.

It includes:

Virtual 16×2 dot-matrix LCD

On-screen remote

Multi-tap text entry

Emoji preview

Cursor indicator

CGRAM slot indicator

Physical keyboard input

Web Serial connection

Synchronization with Arduino IR commands

The interface expects the LCD configuration 0x27 / 16×2 and a serial connection at 9600 baud.

Running the Web Interface

For Web Serial, use a Chromium-based desktop browser such as Google Chrome or Microsoft Edge.

Connect the Arduino to the computer using USB.

Upload the Arduino sketch.

Close the Arduino Serial Monitor if it is holding the serial port.

Open ir.html in Chrome or Edge.

Click Connect Arduino.

Select the Arduino's serial/COM port from the browser dialog.

The status should change to:

connected · 9600 baud

Which COM Port?

The COM-port number is not hard-coded into the project.

For example, Windows may identify the Arduino as:

COM3
COM5
COM6
COM10

The exact number depends on the computer and USB connection.

You can find it in:

Arduino IDE
→ Tools
→ Port

For the web interface, simply select that same Arduino port when Chrome/Edge asks which serial port to connect to.

Browser Keyboard Controls

The web interface also supports keyboard input.

Keyboard Key

Action

Enter

OK / Emoji

Backspace

Delete

←

Cursor left

→

Cursor right

Tab

Mute / mode switch

A–Z / a–z

Direct letter input

0–9

Direct number input

Space

Space

Project Files

Recommended GitHub repository structure:

IR-TXT9000/
│
├── README.md
├── ir.html
├── rc.png
├── IR_Texting.ino
│
└── images/
    └── project-photo.jpg       # optional

You can rename IR_Texting.ino if your Arduino sketch already has another name. Update this README accordingly.

Uploading to Arduino

Install the required libraries.

Connect the Arduino using USB.

Open the .ino sketch in Arduino IDE.

Select:

Tools → Board → Arduino Uno

Select the correct port:

Tools → Port → COMx

Click Upload.

After uploading, the LCD should initialize and the IR receiver will wait for commands.

Troubleshooting

LCD is blank

Check:

LCD VCC and GND

SDA → A4

SCL → A5

I2C address

LCD contrast potentiometer

lcd.backlight()

IR remote does nothing

Check:

Receiver signal → D2

Receiver VCC/GND

Remote battery

Correct IR command codes

Correct receiver pinout

Open Serial Monitor at 9600 baud and press remote buttons. You should see button codes.

Browser cannot connect to Arduino

Check:

Use Chrome or Edge on desktop

Arduino is connected through USB

Correct serial port is selected

Serial Monitor is closed

Another application is not using the port

Baud rate is 9600

Wrong letters appear

The remote codes can vary between different IR remotes. If another remote is used, read its command values through Serial Monitor and replace the command definitions in the Arduino code and REMOTE_CODES mapping in ir.html.

How It Works

             IR REMOTE
                 │
                 │ Infrared command
                 ▼
          ┌──────────────┐
          │ IR RECEIVER  │
          └──────┬───────┘
                 │ D2
                 ▼
          ┌──────────────┐
          │   ARDUINO    │
          │              │
          │ Multi-Tap    │
          │ Mode Control │
          │ Emoji/CGRAM  │
          └──────┬───────┘
                 │ I2C
                 ▼
          ┌──────────────┐
          │  16×2 LCD    │
          └──────────────┘

                 ↕ USB Serial @ 9600

          ┌──────────────┐
          │   ir.html    │
          │ Web Console  │
          └──────────────┘

The Arduino receives an IR command, identifies the corresponding remote button, processes it according to the current input mode, and updates the LCD.

For letters, repeated presses of the same number cycle through the letters assigned to that key. The UP and DOWN buttons control letter case, while MUTE switches alphabet/number mode. The OK button is used for the custom emoji selector.

Notes

LCD: 16 columns × 2 rows

I2C address used: 0x27

IR receiver signal pin: D2

Serial baud rate: 9600

Multi-tap timeout: 900 ms

Custom character size: 5×8 pixels

HD44780 CGRAM capacity: 8 custom characters simultaneously

Web Serial is intended for Chrome/Edge desktop browsers.

License

This project is intended for educational and experimental use. Add a license such as the MIT License if you want others to freely reuse and modify the project.
