#include <arduino.h>

const byte ROWS = 4;
const byte COLLUMS = 4;

byte rowPins[ROWS] = {6, 7, 8, 9};
byte collumPins[COLLUMS] = {2, 3, 4, 5};

char keypad[ROWS][COLLUMS]{
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};

// char euqualCharWithKeypad(char c)
// {
// }

// void readKeypad(char userChoice)
// {
// }

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
}

void loop()
{
    for (int r = 0; r < ROWS; r++)
    {   
        pinMode(rowPins[r], OUTPUT);
        digitalWrite(rowPins[r], LOW);

        Serial.println("checking " + String(r));

        delayMicroseconds(20);

        for (int c = 0; c < COLLUMS; c++)
        {
            int collumSignal = digitalRead(collumPins[c]);

            if (collumSignal == LOW)
            {
                Serial.println("find sygnal in collum");
                Serial.println("(" + String(r) + " , " + String(c) + ")");
                while (digitalRead(collumPins[c]) == LOW); // czekaj aż puścisz przycisk
                delay(100);
                break;
            }
        }
        pinMode(rowPins[r], INPUT);
    }
}