#include <arduino.h>
#include <LiquidCrystal_I2C.h>

const byte ROWS = 4;
const byte COLLUMS = 4;

byte rowPins[ROWS] = {5, 4, 3, 2};
byte collumPins[COLLUMS] = {6, 7, 8, 9};

char keypad[ROWS][COLLUMS]{
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};

char readKeypad();

LiquidCrystal_I2C lcd(0x27, 16, 2);

byte lineChar[8] = {
    B10000,
    B10000,
    B10000,
    B10000,
    B10000,
    B10000,
    B10000,
    B10000};

byte cursorPos[2] = {0, 1};

bool validChar(char c);

void lineCharImpulse();

void writeCharOnLCD(char c);

void setup()
{
    Serial.begin(9600);

    for (int r = 0; r < ROWS; r++)
    {
        // pinMode(rowPins[r], OUTPUT);
        // digitalWrite(rowPins[r], LOW);
        pinMode(rowPins[r], INPUT);
    }

    for (int c = 0; c < COLLUMS; c++)
    {
        pinMode(collumPins[c], INPUT_PULLUP);
    }

    lcd.init();
    lcd.backlight();

    lcd.createChar(0, lineChar);

    lcd.clear();
    lcd.print("Enter PIN: ");

    lcd.setCursor(cursorPos[0], cursorPos[1]);
}

unsigned long lastMillis;
unsigned long lastMillisWrite;

bool shouldShow = true;

void loop()
{
    char c = readKeypad();

    if ((millis() - lastMillis) >= 500)
    {
        lineCharImpulse();
    }

    if (validChar(c))
    {
        Serial.println("displaying character");
        Serial.println(String(cursorPos[0]) + " " + String(cursorPos[1]));

        lastMillisWrite = millis();
        writeCharOnLCD(c);
    }
}

void lineCharImpulse()
{
    if (shouldShow)
    {
        Serial.println("waiting to impulse...");
        shouldShow = false;
        lastMillis = millis();
        lcd.setCursor(cursorPos[0], cursorPos[1]);
        lcd.write(0);
        Serial.println("impulse done");
    }
    else
    {
        Serial.println("waiting to impulse...");
        shouldShow = true;
        lastMillis = millis();
        lcd.setCursor(cursorPos[0], cursorPos[1]);
        lcd.print(" ");
        Serial.println("impulse done");
    }
}

char readKeypad()
{
    char userChoice = '\0';

    for (int r = 0; r < ROWS; r++)
    {
        pinMode(rowPins[r], OUTPUT);
        digitalWrite(rowPins[r], LOW);

        // Serial.println("checking " + String(r));

        delayMicroseconds(20);

        for (int c = 0; c < COLLUMS; c++)
        {
            int collumSignal = digitalRead(collumPins[c]);

            if (collumSignal == LOW)
            {
                Serial.println("find sygnal in collum");
                Serial.println("(" + String(r) + " , " + String(c) + ")");

                userChoice = keypad[r][c];

                Serial.println(userChoice);

                while (digitalRead(collumPins[c]) == LOW)
                    ;
                delay(100);
                break;
            }
        }
        pinMode(rowPins[r], INPUT);
    }

    return userChoice;
}

bool validChar(char c)
{
    if (c != '\0')
    {
        Serial.println("validing char");
        return true;
    }

    return false;
}

void writeCharOnLCD(char c)
{
    Serial.println("writing char...");
    lcd.setCursor(cursorPos[0], cursorPos[1]);
    lcd.print(c);
    delay(10);
    cursorPos[0] += 1;
    Serial.println("wrote char");
}