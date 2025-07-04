#include <LiquidCrystal_I2C.h>
#include <arduino.h>

const byte ROWS = 4;
const byte COLLUMS = 4;

byte rowPins[ROWS] = {5, 4, 3, 2};
byte collumPins[COLLUMS] = {6, 7, 8, 9};

char keypad[ROWS][COLLUMS]{{'1', '2', '3', 'A'},
                           {'4', '5', '6', 'B'},
                           {'7', '8', '9', 'C'},
                           {'*', '0', '#', 'D'}};

char readKeypad();

LiquidCrystal_I2C lcd(0x27, 16, 2);

byte lineChar[8] = {B10000, B10000, B10000, B10000,
                    B10000, B10000, B10000, B10000};

byte squareChar[8] = {B11111, B11111, B11111, B11111,
                      B11111, B11111, B11111, B11111};

byte cursorPos[2] = {0, 1};

String pin;

bool isEditionMode = false;

bool validChar(char c);

void charImpulseOnDisplay(int charIndex, char c);

void displayCharOnLCD(char c);

void writeChar(char c);

void displayPulsingChar(int charIndex, char c);

void assignKeyFucntion(char c);

void handleAcceptButton();

void handleBackspaceButton();

void handleClearButton();

void handleCursorModeButton();

void handleForwardButton();

void handleBackwardButton();

void setup() {
    Serial.begin(9600);

    for (int r = 0; r < ROWS; r++) {
        pinMode(rowPins[r], INPUT);
    }

    for (int c = 0; c < COLLUMS; c++) {
        pinMode(collumPins[c], INPUT_PULLUP);
    }

    lcd.init();
    lcd.backlight();

    lcd.createChar(0, lineChar);
    lcd.createChar(1, squareChar);

    lcd.clear();
    lcd.print("Enter PIN: ");

    lcd.setCursor(cursorPos[0], cursorPos[1]);
}

unsigned long lastMillis;
unsigned long lastMillisWrite;

bool shouldShowChar = true;

int currentCharsNumber = 0;
int maxCharsNumber = 4;

bool shouldImpulse = true;

void loop() {
    char c = readKeypad();
    Serial.println(currentCharsNumber);

    if ((millis() - lastMillis) >= 500) {
        if (isEditionMode) {
            charImpulseOnDisplay(1, pin[cursorPos[0]]);
        } else {
            charImpulseOnDisplay(0, ' ');
        }
    }

    if (cursorPos[0] >= pin.length()) {
        isEditionMode = false;
        Serial.println(pin);
        Serial.println("chars number cursor  is: " + String(currentCharsNumber));
    }

    // if (!isEditionMode) {
    if (currentCharsNumber >= maxCharsNumber) {
        return;
    }

    displayCharOnLCD(c);
}

void displayCharOnLCD(char c) {
    if (validChar(c)) {
        if (isdigit(c)) {
            // if (!isEditionMode) {
            //     currentCharsNumber++;
            // }
            currentCharsNumber++;
            Serial.println("currentCharsNumber inscreased");
            Serial.println("displaying character");
            Serial.println(String(cursorPos[0]) + " " + String(cursorPos[1]));
            lastMillisWrite = millis();
            writeChar(c);
        } else
            assignKeyFucntion(c);
    }
}

void assignKeyFucntion(char c) {
    Serial.println("assigning key function...");

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

    shouldImpulse = true;
    delay(20);
}

void handleAcceptButton() {
    lcd.setCursor(0, 1);
    lcd.print("Accept");
};

void handleBackspaceButton() {
    lcd.setCursor(cursorPos[0], cursorPos[1]);
    cursorPos[0]--;
    lcd.print(" ");
    currentCharsNumber--;
    pin.remove(pin.length() - 1);
    Serial.println("Backspace");
}

void handleClearButton() {
    for (int i = 0; i < maxCharsNumber; i++) {
        lcd.setCursor(i, cursorPos[1]);
        lcd.print(" ");
        cursorPos[0] = 0;
        Serial.println("clear PIN");
    }
    currentCharsNumber = 0;
    pin = "";
}

void handleCursorModeButton() {
    if (isEditionMode) {
        currentCharsNumber = pin.length();
        lcd.setCursor(cursorPos[0], cursorPos[1]);
        lcd.print(pin[cursorPos[0]]);
        cursorPos[0] = pin.length();
        isEditionMode = false;
        // cursorPos[0]++;

        Serial.println(pin[cursorPos[0]]);
    } else {
        lcd.setCursor(cursorPos[0], cursorPos[1]);
        isEditionMode = true;
        cursorPos[0]--;
        lcd.print(" ");
        currentCharsNumber--;
    }
}

void handleForwardButton() {
    if (cursorPos[0] != 0) {
        lcd.setCursor(cursorPos[0], cursorPos[1]);
        lcd.print(pin[cursorPos[0]]);
    }
    cursorPos[0]++;
    currentCharsNumber++;
    // lcd.setCursor(cursorPos[0], cursorPos[1]);
}

void handleBackwardButton() {
    if (cursorPos[0] != 0) {
        lcd.setCursor(cursorPos[0], cursorPos[1]);
        lcd.print(pin[cursorPos[0]]);
        cursorPos[0]--;
        currentCharsNumber--;
    }
    // lcd.setCursor(cursorPos[0], cursorPos[1]);
}

void writeChar(char c) {
    Serial.println("writing char...");
    lcd.setCursor(cursorPos[0], cursorPos[1]);
    lcd.print(c);
    if (!isEditionMode) {
        pin += c;
    } else {
        pin[cursorPos[0]];
    }
    cursorPos[0] += 1;
    // delay(10);
    Serial.println("wrote char");
}

void charImpulseOnDisplay(int charIndex, char c) {
    Serial.println("waiting to impulse...");
    if (shouldImpulse) {
        lastMillis = millis();
        lcd.setCursor(cursorPos[0], cursorPos[1]);
        displayPulsingChar(charIndex, c);
        // Serial.print()
    }
    Serial.println("impulse done");
}

void displayPulsingChar(int charIndex, char c) {
    if (shouldShowChar) {
        shouldShowChar = false;
        lcd.write(charIndex);
        // delayMicroseconds(20);
    } else {
        shouldShowChar = true;
        lcd.print(c);
        // delayMicroseconds(20);
    }
}

char readKeypad() {
    char userChoice = '\0';

    for (int r = 0; r < ROWS; r++) {
        pinMode(rowPins[r], OUTPUT);
        digitalWrite(rowPins[r], LOW);

        // Serial.println("checking " + String(r));

        delayMicroseconds(20);

        for (int c = 0; c < COLLUMS; c++) {
            int collumSignal = digitalRead(collumPins[c]);

            if (collumSignal == LOW) {
                Serial.println("find sygnal in collum");
                Serial.println("(" + String(r) + " , " + String(c) + ")");

                userChoice = keypad[r][c];

                Serial.println(userChoice);

                while (digitalRead(collumPins[c]) == LOW);
                delay(100);
                break;
            }
        }
        pinMode(rowPins[r], INPUT);
    }

    return userChoice;
}

bool validChar(char c) {
    if (c != '\0') {
        Serial.println("validing char");
        return true;
    }

    return false;
}