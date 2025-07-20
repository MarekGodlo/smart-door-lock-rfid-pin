#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <MFRC522.h>
#include <SPI.h>
#include <Servo.h>

#define DEBUG_MODE  // switch to debug mode

#ifdef DEBUG_MODE
#define DEBUG(x) Serial.println(x)
#else
#define DEBUG(x)
#endif

// MFRC522
const uint8_t SS_PIN = 10;
const uint8_t RST_PIN = 9;

MFRC522 mfrc(SS_PIN, RST_PIN);

// Sored UID for identify the card
uint8_t storedUID[4] = {
    0x5A, 0x0C, 0x1A, 0x02};

uint8_t currentUID[4];
const uint8_t CURRENT_UID_LENGHT = 4;

boolean isMFRCMode = true;

// Shift register
const uint8_t CLOCK_PIN = 8;
const uint8_t LATCH_PIN = 7;
const uint8_t DATA_PIN = 6;

// Keypad
const uint8_t ROWS = 4;
const uint8_t COLUMNS = 4;

uint8_t collumPins[COLUMNS] = {A0, 2, 3, 4};

char keypad[ROWS][COLUMNS]{
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};

// LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

uint8_t lineChar[8] = {B10000, B10000, B10000, B10000,
                       B10000, B10000, B10000, B10000};

uint8_t squareChar[8] = {B11111, B11111, B11111, B11111,
                         B11111, B11111, B11111, B11111};

uint8_t cursorPos[2] = {0, 1};

bool isEditionMode = false;

unsigned long lastMillis;

// Pulsing char
bool shouldShowChar = true;

// PIN
uint8_t currentCharsNumber = 0;
const uint8_t MAX_CHARS_NUMBER = 4;

String pin;
const int STORED_PIN = 1234;

uint8_t outputState = 0;

// Servo
Servo servo;
const uint8_t SERVO_PIN = 5;

boolean checkUID(uint8_t* uid1, uint8_t* uid2);
void cleanTable(uint8_t* table, uint8_t length);

char readKeypad();
void charImpulseOnDisplay(uint8_t charIndex, char c);
void displayCharOnLCD(char c);
bool checkPIN(int entered, int correct);

void activeServo(int angel, int speed);

void setup() {
    Serial.begin(9600);

    DEBUG("Serial monitor configured");

    // Sets mfrc
    SPI.begin();
    mfrc.PCD_Init();

    DEBUG("MFRC configured");

    // Sets servo
    servo.attach(SERVO_PIN);
    servo.write(90);

    DEBUG("Servo motor configured");

    // Sets shift register
    pinMode(CLOCK_PIN, OUTPUT);
    pinMode(LATCH_PIN, OUTPUT);
    pinMode(DATA_PIN, OUTPUT);

    DEBUG("Shift register congigured");

    // Sets keypad
    for (int c = 0; c < COLUMNS; c++) {
        pinMode(collumPins[c], INPUT_PULLUP);
    }
    DEBUG("Keypad configured");

    // Sets LCD
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
    lcd.print("Apply card");

    DEBUG("LCD configured");
    DEBUG("==============================================================================");
    DEBUG(" ");
}

void loop() {
    if (isMFRCMode) {
        // Waiting for card
        if (!mfrc.PICC_IsNewCardPresent()) return;
        if (!mfrc.PICC_ReadCardSerial()) return;

        Serial.print("UID: ");

        for (int i = 0; i < mfrc.uid.size; i++) {
            currentUID[i] = mfrc.uid.uidByte[i];

            if (currentUID[i] < 0x10) {
                Serial.print(0);
            }
            Serial.print(currentUID[i], HEX);
            Serial.print(" ");
        }

        Serial.println(" ");

        if (checkUID(currentUID, storedUID)) {
            DEBUG("Detected correct UID");
            lcd.clear();
            lcd.print("Correct card");
            activeServo(180, 3000);
            lcd.clear();
            lcd.print("Apply card");
        } else {
            DEBUG("Detected incorrect UID");
            DEBUG("Switched to keypad");

            isMFRCMode = false;
            lcd.clear();
            lcd.print("Incorrect card");

            delay(2000);

            lcd.clear();
            lcd.print("Enter PIN: ");
            cursorPos[0] = 0;
            lcd.setCursor(cursorPos[0], cursorPos[1]);
        }

        mfrc.PICC_HaltA();
        mfrc.PCD_StopCrypto1();
        cleanTable(currentUID, CURRENT_UID_LENGHT);
    } else {
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

        if (currentCharsNumber >= MAX_CHARS_NUMBER) {
            if (checkPIN(pin.toInt(), STORED_PIN)) {
                lcd.clear();
                lcd.print("Correct PIN");
                activeServo(180, 3000);
            } else {
                lcd.clear();
                lcd.print("Incorrect PIN");
                delay(2000);
            }

            lcd.clear();
            lcd.print("Apply card");

            currentCharsNumber = 0;
            pin = "";
            cursorPos[0] = 0;
            isMFRCMode = true;
            return;
        }

        displayCharOnLCD(c);
    }
}

void cleanTable(uint8_t* table, uint8_t lenght) {
    for (int i = 0; i < lenght; i++) {
        table[i] = 0;
    }
}

void activeServo(int angel, int speed) {
    servo.write(angel);
    delay(speed);
    servo.write(90);
    DEBUG("Actived servo");
}

boolean checkUID(uint8_t* uid1, uint8_t* uid2) {
    for (int i = 0; i < 4; i++) {
        if (uid1[i] != uid2[i]) {
            return false;
        }
    }
    return true;
}

// ===============================================================================================================
// Keypad
// ===============================================================================================================

void updateShiftRegister(uint8_t value) {
    digitalWrite(LATCH_PIN, LOW);
    shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, value);
    digitalWrite(LATCH_PIN, HIGH);
}

void selectRow(uint8_t row) {
    outputState |= 0b00011110;
    outputState &= ~(1 << (row + 1));

    updateShiftRegister(outputState);
}

// Reads the value from keypad
char readKeypad() {
    char userChoice = '\0';

    // Checks all rows
    for (int r = 0; r < ROWS; r++) {
        selectRow(r);
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

boolean checkPIN(int entered, int correct) {
    if (entered == correct) {
        DEBUG("Entered correct pin");
        return true;
    } else {
        DEBUG("Entered incorrect pin");
        return false;
    }
}

// ===============================================================================================================
// Assigning function to buttons
// ===============================================================================================================

// Deletes last sign
void handleBackspaceButton() {
    lcd.setCursor(cursorPos[0], cursorPos[1]);
    cursorPos[0]--;
    lcd.print(" ");
    currentCharsNumber--;
    pin.remove(pin.length() - 1);
    DEBUG("finished backspace");
}

// Deletes all signs in row
void handleClearButton() {
    for (int i = 0; i < MAX_CHARS_NUMBER; i++) {
        lcd.setCursor(i, cursorPos[1]);
        lcd.print(" ");
        cursorPos[0] = 0;
    }
    currentCharsNumber = 0;
    pin = "";

    DEBUG(" finished clear PIN");
}

// Turns on or turns off edition mode
void handleCursorModeButton() {
    // Turns off
    if (isEditionMode) {
        currentCharsNumber = pin.length();
        lcd.setCursor(cursorPos[0], cursorPos[1]);
        lcd.print(pin[cursorPos[0]]);
        cursorPos[0] = pin.length();
        isEditionMode = false;

        DEBUG("Finished turn off edition mode");
        // Turns on
    } else {
        lcd.setCursor(cursorPos[0], cursorPos[1]);
        isEditionMode = true;
        cursorPos[0]--;
        lcd.print(" ");
        currentCharsNumber--;

        DEBUG("Finished turn on edition mode");
    }
}

// Moves the cursor to the right
void handleForwardButton() {
    if (isEditionMode) {
        if (cursorPos[0] != 0) {
        }
        // Delete cursor char
        lcd.setCursor(cursorPos[0], cursorPos[1]);
        lcd.print(pin[cursorPos[0]]);
        // Change the cursor position
        cursorPos[0]++;
        currentCharsNumber++;

        DEBUG("Cursor has moved rigth");
    }
    DEBUG("Finished forward");
}

// Moves the cursor to the left
void handleBackwardButton() {
    if (isEditionMode) {
        // Checks if the cursor isn't on the first column;
        if (cursorPos[0] != 0) {
            // Delete cursor char
            lcd.setCursor(cursorPos[0], cursorPos[1]);
            lcd.print(pin[cursorPos[0]]);
            // Change the cursor position
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

// Checks if sign isn't null
bool validChar(char c) {
    if (c != '\0') {
        DEBUG("validing char");
        return true;
    }

    return false;
}

// Prints the sign on the display
void writeChar(char c) {
    DEBUG("writing char");
    lcd.setCursor(cursorPos[0], cursorPos[1]);
    lcd.print(c);
    if (!isEditionMode) {
        pin += c;
    } else {
        pin[cursorPos[0]] = c;
    }
    cursorPos[0] += 1;
    DEBUG("finished write char");
    DEBUG(" ");
}

// Displays the digit from keypad or calls assignKeyFunction
void displayCharOnLCD(char c) {
    if (validChar(c)) {
        if (isdigit(c)) {
            currentCharsNumber++;

            DEBUG("displaying character on pos:");
            Serial.print(cursorPos[0]);
            Serial.print(" ");
            DEBUG(cursorPos[1]);

            writeChar(c);
        } else
            assignKeyFunction(c);
    }
}

void displayPulsingChar(uint8_t charIndex, char c) {
    if (shouldShowChar) {
        shouldShowChar = false;
        lcd.write(charIndex);
    } else {
        shouldShowChar = true;
        lcd.print(c);
    }
}

// Set sign that will be puls
void charImpulseOnDisplay(uint8_t charIndex, char c) {
    lastMillis = millis();

    lcd.setCursor(cursorPos[0], cursorPos[1]);
    displayPulsingChar(charIndex, c);
}