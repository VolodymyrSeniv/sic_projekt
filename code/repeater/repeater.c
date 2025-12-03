#include "../common.h"
#include <Wire.h>
#include <SPI.h>

const uint32_t MY_NODE_ID = ID_W1;

// Lista pinów CS dla poszczególnych sensorów
const int SENSOR_CS_PINS[] = {4, 5}; 
const int NUM_SENSORS = sizeof(SENSOR_CS_PINS) / sizeof(SENSOR_CS_PINS[0]);

void setup() {
    Serial.begin(UART_BAUD_RATE);
    Wire.begin();
    
    // Konfiguracja pinów CS
    for(int i = 0; i < NUM_SENSORS; i++) {
        pinMode(SENSOR_CS_PINS[i], OUTPUT);
        digitalWrite(SENSOR_CS_PINS[i], HIGH); // Stan wysoki = nieaktywny
    }

    // SPI start - domyślnie Master
    SPI.begin();
}

void loop() {
    // Pętla odpytująca każdy sensor po kolei
    for (int i = 0; i < NUM_SENSORS; i++) {
        readFromSensor(SENSOR_CS_PINS[i]);
        delay(100); // Krótka przerwa między odczytami sensorów
    }
    
    delay(1000); // Główny interwał pętli
}

void readFromSensor(int csPin) {
    DataFrame frame;
    // Czyścimy bufor ramki
    memset(&frame, 0, sizeof(DataFrame));

    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
    
    // Aktywacja konkretnego Sensora
    digitalWrite(csPin, LOW);
    
    // Transfer - wysyłamy 0x00, odbieramy dane od Slave'a
    // Musimy wiedzieć ile bajtów odebrać
    uint8_t* pFrame = (uint8_t*)&frame;
    for (size_t k = 0; k < sizeof(DataFrame); k++) {
        pFrame[k] = SPI.transfer(0x00); // Dummy byte to generate clock
    }
    
    // Dezaktywacja Sensora
    digitalWrite(csPin, HIGH);
    
    SPI.endTransaction();

    // Weryfikacja danych (np. czy ID nie jest puste lub CRC się zgadza)
    // Uwaga: Slave może zwrócić same 0 lub 0xFF jeśli nie był gotowy
    if (frame.senderID != 0 && frame.senderID != 0xFFFFFFFF) {
        processFrame(frame);
    }
}

void processFrame(DataFrame &frame) {
    // Logika routingu i przekazywania (z twojego oryginalnego kodu)
    frame.pathMask |= MY_NODE_ID;
    
    // Przelicz CRC jeśli modyfikujesz ramkę
    // frame.crc = calculateCRC(...) 

    Serial.print("Odebrano od ID: ");
    Serial.println(frame.senderID);
    Serial.print("Pomiar: ");
    Serial.println(frame.measurement);

    // Przekazanie dalej po I2C lub Serial (zgodnie z )
    if (MY_NODE_ID == ID_W1) {
       Wire.beginTransmission(W4_NR);
       Wire.write((uint8_t*)&frame, sizeof(DataFrame));
       Wire.endTransmission();
    }
}