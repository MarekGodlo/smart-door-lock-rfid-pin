#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

#define DEBUG_MODE  // switch to debug mode

#ifdef DEBUG_MODE
#define DEBUG(x) Serial.println(x)
#else
#define DEBUG(x)
#endif

const int clockPin = 13;
const int latchPin = 12;
const int dataPin = 11;

// keypad
const byte ROWS = 4;
const byte COLUMNS = 4;

// byte rowPins[ROWS] = {5, 4, 3, 2};

byte collumPins[COLUMNS] = {2, 3, 4, 5};

char keypad[ROWS][COLUMNS]{
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};

// lcd
LiquidCrystal_I2C lcd(0x27, 16, 2);

byte lineChar[8] = {B10000, B10000, B10000, B10000,
                    B10000, B10000, B10000, B10000};

byte squareChar[8] = {B11111, B11111, B11111, B11111,
                      B11111, B11111, B11111, B11111};

byte cursorPos[2] = {0, 1};

bool isEditionMode = false;

unsigned long lastMillis;
// unsigned long lastMillisWrite;

// pulsing char
bool shouldShowChar = true;

// pin
int currentCharsNumber = 0;
int maxCharsNumber = 4;

String pin;

void updateShiftRegister(byte value);

bool checkPin(int pin, int correctPin);

char readKeypad();

void charImpulseOnDisplay(int charIndex, char c);

void displayCharOnLCD(char c);

byte outputState = 0;

void setup() {
    Serial.begin(9600);

    DEBUG("Serial monitor configured");

    // sets shift register
    pinMode(clockPin, OUTPUT);
    pinMode(latchPin, OUTPUT);
    pinMode(dataPin, OUTPUT);

    DEBUG("Shift register congigured");

    // sets keypad
    // for (int r = 0; r < ROWS; r++) {
    //     pinMode(rowPins[r], INPUT);
    // }

    for (int c = 0; c < COLUMNS; c++) {
        pinMode(collumPins[c], INPUT_PULLUP);
    }
    DEBUG("Keypad configured");

    // sets lcd
    lcd.init();
    lcd.backlight();

    DEBUG("LCD Initialized");

    lcd.createChar(0, lineChar);
    lcd.createChar(1, squareChar);

    DEBUG("Created custom signs");

    lcd.clear();
    lcd.print("Configuration...");

    lcd.setCursor(cursorPos[0], cursorPos[1]);

    delay(1000);

    lcd.clear();
    lcd.print("Enter PIN: ");

    DEBUG("LCD configured");
    DEBUG("==============================================================================");
    DEBUG(" ");
}

void loop() {
    char c = readKeypad();

    if ((millis() - lastMillis) >= 500) {
        if (isEditionMode) {
            charImpulseOnDisplay(1, pin[cursorPos[0]]);
        } else {
            charImpulseOnDisplay(0, ' ');
        }
    }

    if (cursorPos[0] >= pin.length()) {
        isEditionMode = false;
    }

    if (currentCharsNumber >= maxCharsNumber) {
        checkPin(pin.toInt(), 1234);
        return;
    }

    displayCharOnLCD(c);
}

void updateShiftRegister(byte value) {
    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin, MSBFIRST, value);
    digitalWrite(latchPin, HIGH);
}

bool checkPin(int pin, int correctPin) {
    if (pin == correctPin) {
        DEBUG("Entered correct pin");
    } else {
        DEBUG("Entered incorrect pin");
    }
}

// ===============================================================================================================
// Keypad
// ===============================================================================================================

void selectRow(byte row) {
    outputState |= 0b00011110;
    outputState &= ~(1 << (row + 1));

    updateShiftRegister(outputState);
}

// reads the value from keypad
char readKeypad() {
    char userChoice = '\0';

    // checks all rows
    for (int r = 0; r < ROWS; r++) {
        // pinMode(rowPins[r], OUTPUT);
        // digitalWrite(rowPins[r], LOW);
        selectRow(r);

        // DEBUG("checking " + String(r));

        delayMicroseconds(20);

        // check all columns
        for (int c = 0; c < COLUMNS; c++) {
            int columnsSignal = digitalRead(collumPins[c]);

            if (columnsSignal == LOW) {
                DEBUG("find sygnal in collum");
                DEBUG("(" + String(r) + " , " + String(c) + ")");

                userChoice = keypad[r][c];

                Serial.print("Found user choice: ");
                DEBUG(userChoice);

                while (digitalRead(collumPins[c]) == LOW);
                delay(10);
                break;
            }
        }
    }

    return userChoice;
}

// ===============================================================================================================
// Assigning function to buttons
// ===============================================================================================================

void handleAcceptButton() {
    lcd.setCursor(0, 1);
    lcd.print("Accept");
    DEBUG("finished");
};

// deletes last sign
void handleBackspaceButton() {
    lcd.setCursor(cursorPos[0], cursorPos[1]);
    cursorPos[0]--;
    lcd.print(" ");
    currentCharsNumber--;
    pin.remove(pin.length() - 1);
    DEBUG("finished backspace");
}

// deletes all signs in row
void handleClearButton() {
    for (int i = 0; i < maxCharsNumber; i++) {
        lcd.setCursor(i, cursorPos[1]);
        lcd.print(" ");
        cursorPos[0] = 0;
    }
    currentCharsNumber = 0;
    pin = "";

    DEBUG(" finished clear PIN");
}

// turns on or turns off edition mode
void handleCursorModeButton() {
    // turn off
    if (isEditionMode) {
        currentCharsNumber = pin.length();
        lcd.setCursor(cursorPos[0], cursorPos[1]);
        lcd.print(pin[cursorPos[0]]);
        cursorPos[0] = pin.length();
        isEditionMode = false;

        DEBUG("Finished turn off edition mode");
        // turn on
    } else {
        lcd.setCursor(cursorPos[0], cursorPos[1]);
        isEditionMode = true;
        cursorPos[0]--;
        lcd.print(" ");
        currentCharsNumber--;

        DEBUG("Finished turn on edition mode");
    }
}

// moves the cursor to the right
void handleForwardButton() {
    if (isEditionMode) {
        if (cursorPos[0] != 0) {
        }
        // delete cursor char
        lcd.setCursor(cursorPos[0], cursorPos[1]);
        lcd.print(pin[cursorPos[0]]);
        // change the cursor position
        cursorPos[0]++;
        currentCharsNumber++;

        DEBUG("Cursor has moved rigth");
    }
    DEBUG("Finished forward");
}

// moves the cursor to the left
void handleBackwardButton() {
    if (isEditionMode) {
        // checks if the cursor isn't on the first column;
        if (cursorPos[0] != 0) {
            // delete cursor char
            lcd.setCursor(cursorPos[0], cursorPos[1]);
            lcd.print(pin[cursorPos[0]]);
            // change the cursor position
            cursorPos[0]--;
            currentCharsNumber--;

            DEBUG("Cursor has moved left");
        }
    }
    DEBUG("Finished backward");
}

void assignKeyFunction(char c) {
    DEBUG("Assigning key function");

    switch (c) {
        case 'A':
            handleBackspaceButton();
            break;
        case 'B':
            handleClearButton();
            break;
        case 'C':
            handleCursorModeButton();
            break;
        case '*':
            handleBackwardButton();
            break;
        case '#':
            handleForwardButton();
            break;
        default:
            break;
    }

    DEBUG(" ");
}

// ===============================================================================================================
// LCD
// ===============================================================================================================

// checks if sign isn't null
bool validChar(char c) {
    if (c != '\0') {
        DEBUG("validing char");
        return true;
    }

    return false;
}

// prints the sign on the display
void writeChar(char c) {
    DEBUG("writing char");
    lcd.setCursor(cursorPos[0], cursorPos[1]);
    lcd.print(c);
    if (!isEditionMode) {
        pin += c;
    } else {
        pin[cursorPos[0]];
    }
    cursorPos[0] += 1;
    // delay(10);
    DEBUG("finished write char");
    DEBUG(" ");
}

// displays the digit from keypad or calls assignKeyFunction
void displayCharOnLCD(char c) {
    if (validChar(c)) {
        if (isdigit(c)) {
            currentCharsNumber++;

            DEBUG("displaying character on pos:");
            Serial.print(cursorPos[0]);
            Serial.print(" ");
            DEBUG(cursorPos[1]);

            // lastMillisWrite = millis();
            writeChar(c);
        } else
            assignKeyFunction(c);
    }
}

void displayPulsingChar(int charIndex, char c) {
    if (shouldShowChar) {
        shouldShowChar = false;
        lcd.write(charIndex);
    } else {
        shouldShowChar = true;
        lcd.print(c);
    }
}

// set sign that will be puls
void charImpulseOnDisplay(int charIndex, char c) {
    lastMillis = millis();

    lcd.setCursor(cursorPos[0], cursorPos[1]);
    displayPulsingChar(charIndex, c);
}