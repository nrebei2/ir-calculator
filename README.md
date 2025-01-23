This repo holds a sketch implementation of a functional calculator on an Arduino, using an IR remote control as input and an LCD display as output. 

## Features

- Entering digits.
- Addition (+) and subtraction (-).
- Move the cursor left or right to edit the input.
- Scroll left or right if the input exceeds the screen width.
- History to view past calculations.
- Scroll up/down through history.
- Append selected history items directly to the current input.
- Blinking cursor for UI.
- All memory is statically allocated, there is no heap usage.

Basically, all I could fit on my Elegoo IR Remote.

[IR Library](https://github.com/Arduino-IRremote/Arduino-IRremote), [LCD Library](https://github.com/arduino-libraries/LiquidCrystal)

![pic](https://github.com/user-attachments/assets/0bfb0d9b-8f2b-42f1-86c5-ebb3738b6c5a)
(Excuse the wires...)
