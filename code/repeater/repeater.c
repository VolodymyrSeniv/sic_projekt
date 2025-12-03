#include "../common.h"
#include <Wire.h>
#include <SPI.h>

const uint32_t MY_NODE_ID = ID_W1; // lub ID_W2, zależnie od urządzenia
const int SENSOR_CS_PINS[] = {4, 5};
const int NUM_SENSORS = sizeof(SENSOR_CS_PINS) / sizeof(SENSOR_CS_PINS[0]);

const uint8_t TARGET_I2C_ADDR = (MY_NODE_ID == ID_W1) ? W4_NR : W3_NR;

void setup()
{
    Serial.begin(UART_BAUD_RATE);
    Wire.begin(); // Inicjalizacja I2C jako Master

    SPI.begin();

    for (int i = 0; i < NUM_SENSORS; i++)
    {
        pinMode(SENSOR_CS_PINS[i], OUTPUT);
        digitalWrite(SENSOR_CS_PINS[i], HIGH);
    }
}

void loop()
{
    for (int i = 0; i < NUM_SENSORS; i++)
    {
        DataFrame frame;
        if (readFromSensor(SENSOR_CS_PINS[i], frame))
        {
            processAndForward(frame);
        }
        delay(50);
    }
    delay(1000);
}

bool readFromSensor(int csPin, DataFrame &frame)
{
    memset(&frame, 0, sizeof(DataFrame));

    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
    digitalWrite(csPin, LOW);

    uint8_t *pFrame = (uint8_t *)&frame;
    for (size_t k = 0; k < sizeof(DataFrame); k++)
    {
        pFrame[k] = SPI.transfer(0x00); // Clock generation
    }

    digitalWrite(csPin, HIGH);
    SPI.endTransaction();

    if (frame.senderID == 0 || frame.senderID == 0xFFFFFFFF)
    {
        return false;
    }
    return true;
}

void processAndForward(DataFrame &frame)
{
    frame.pathMask |= MY_NODE_ID;

    frame.crc = calculateCRC((uint8_t *)&frame, sizeof(DataFrame) - sizeof(uint16_t));

    Serial.write((uint8_t *)&frame, sizeof(DataFrame));

    Wire.beginTransmission(TARGET_I2C_ADDR);
    Wire.write((uint8_t *)&frame, sizeof(DataFrame));
    Wire.endTransmission();
}