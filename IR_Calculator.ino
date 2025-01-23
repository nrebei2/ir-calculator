#include <LiquidCrystal.h>
#include <IRremote.h>
#include "Command.h"

#define SCREEN_WIDTH 16
#include "Calculator.h"

#define RECV_PIN 9
#define RS_PIN 7
#define ENABLE_PIN 8
#define D4_PIN 6
#define D5_PIN 5
#define D6_PIN 4
#define D7_PIN 3

LiquidCrystal lcd = LiquidCrystal(RS_PIN, ENABLE_PIN, D4_PIN, D5_PIN, D6_PIN, D7_PIN);

IRrecv irrecv(RECV_PIN);
decode_results results;

Calculator calc = Calculator();

void setup() {
  lcd.begin(SCREEN_WIDTH, 2);
  irrecv.enableIRIn();
  Serial.begin(9600);
}

void loop() {
  irrecv.resume();
  while (!irrecv.decode(&results)) {}

  switch (Command c = fromIRReading(results.value)) {
    case Zero:
    case One:
    case Two:
    case Three:
    case Four:
    case Five:
    case Six:
    case Seven:
    case Eight:
    case Nine:
      calc.handleDigit((char)c);
      break;
    case Plus:
      calc.handlePlus();
      break;
    case Minus:
      calc.handleMinus();
      break;
    case Left:
      calc.handleLeft();
      break;
    case Right:
      calc.handleRight();
      break;
    case Enter:
      calc.handleEnter();
      break;
    case Up:
      calc.handleUp();
      break;
    case Down:
      calc.handleDown();
      break;
    case Unused:
      // Serial.println(results.value, HEX);
      return;
  }

  calc.print(lcd);
}