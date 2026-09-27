#ifndef CS1237_H
#define CS1237_H

#include <Arduino.h>

class CS1237 {
public:
    CS1237(uint8_t sclkPin, uint8_t doutPin);

    void begin();
    void powerDown();
    void powerUp();
    long readData();
    int  getRegister();
    void setRegister(int registertowrite, int valuetowrite);
    void setGain(uint8_t gain);
    void setDataRate(uint8_t rate);

private:
    uint8_t _sclkPin;
    uint8_t _doutPin;

    void sendCommand(uint8_t command);
};

#endif
