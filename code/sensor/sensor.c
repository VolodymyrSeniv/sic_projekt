#include "../common.h"
#include <SPI.h>
#include "wiring_private.h" // Potrzebne do pinPeripheral

const uint32_t MY_NODE_ID = ID_S1;

volatile DataFrame txFrame;
volatile bool dataUpdated = false;

void setup()
{
    Serial.begin(UART_BAUD_RATE);

    txFrame.senderID = 1;
    txFrame.measurement = 0;

    pinMode(3, INPUT_PULLUP);

    spiSlaveInit();
}

void loop()
{
    txFrame.measurement = analogRead(A0);
    txFrame.pathMask = MY_NODE_ID;

    delay(100);
}

void spiSlaveInit()
{
    SPI.end();

    // Konfiguracja pinów pod SERCOM1
    pinPeripheral(10, PIO_SERCOM); // MISO
    pinPeripheral(8, PIO_SERCOM);  // MOSI
    pinPeripheral(9, PIO_SERCOM);  // SCK
    pinPeripheral(3, PIO_SERCOM);  // SS (Slave Select) - WAŻNE: Połącz to z CS Repeatera!

    // Konfiguracja zegara dla SERCOM1
    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(GCM_SERCOM1_CORE) |
                        GCLK_CLKCTRL_GEN_GCLK0 |
                        GCLK_CLKCTRL_CLKEN;
    while (GCLK->STATUS.bit.SYNCBUSY)
        ;

    // Konfiguracja rejestrów SERCOM1 w tryb SPI Slave
    SERCOM1->SPI.CTRLA.bit.ENABLE = 0; // Wyłącz przed konfiguracją
    while (SERCOM1->SPI.SYNCBUSY.bit.ENABLE)
        ;

    SERCOM1->SPI.CTRLA.reg = SERCOM_SPI_CTRLA_MODE_SPI_SLAVE; // Tryb Slave
    SERCOM1->SPI.CTRLA.bit.DIPO = 0x3;                        // Data In Pinout (zależy od PAD, MISO na PAD 3)
    SERCOM1->SPI.CTRLA.bit.DOPO = 0x0;                        // Data Out Pinout (MOSI na PAD 0)
    SERCOM1->SPI.CTRLA.bit.CPOL = 0;                          // Clock Polarity (zgodne z MODE0)
    SERCOM1->SPI.CTRLA.bit.CPHA = 0;                          // Clock Phase (zgodne z MODE0)
    SERCOM1->SPI.CTRLA.bit.IBON = 1;                          // Input Buffer On

    SERCOM1->SPI.CTRLB.reg = SERCOM_SPI_CTRLB_RXEN; // Włącz odbiornik (wymagane do działania)
    while (SERCOM1->SPI.SYNCBUSY.bit.CTRLB)
        ;

    SERCOM1->SPI.INTENSET.bit.DRE = 1; // Włącz przerwanie "Data Register Empty"

    // Włącz SERCOM1
    SERCOM1->SPI.CTRLA.bit.ENABLE = 1;
    while (SERCOM1->SPI.SYNCBUSY.bit.ENABLE)
        ;

    // Włącz przerwania w NVIC
    NVIC_EnableIRQ(SERCOM1_IRQn);
}

volatile uint8_t byteIndex = 0;

void SERCOM1_Handler()
{
    if (SERCOM1->SPI.INTFLAG.bit.DRE)
    {
        uint8_t *pData = (uint8_t *)&txFrame;

        SERCOM1->SPI.DATA.reg = pData[byteIndex];

        byteIndex++;
        if (byteIndex >= sizeof(DataFrame))
        {
            byteIndex = 0;
        }
    }

    if (SERCOM1->SPI.INTFLAG.bit.RXC)
    {
        uint8_t data = SERCOM1->SPI.DATA.reg;
    }
}