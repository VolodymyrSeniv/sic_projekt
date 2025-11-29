#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>

struct DataFrame
{
    uint16_t measurement;
    uint16_t senderID;
    uint32_t pathMask;
    uint16_t crc;
};

#define S1_NR 0
#define S2_NR 1
#define S3_NR 2
#define S4_NR 3
#define S5_NR 4
#define W1_NR 5
#define W2_NR 6
#define W3_NR 7
#define W4_NR 8
#define W0_NR 9

#define ID_S1 (1 << 0)
#define ID_S2 (1 << 1)
#define ID_S3 (1 << 2)
#define ID_S4 (1 << 3)
#define ID_S5 (1 << 4)
#define ID_W1 (1 << 5)
#define ID_W2 (1 << 6)
#define ID_W3 (1 << 7)
#define ID_W4 (1 << 8)
#define ID_W0 (1 << 9)

uint16_t calculateCRC(uint8_t *data, size_t len)
{
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < len; i++)
    {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++)
        {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xA001;
            else
                crc >>= 1;
        }
    }
    return crc;
}