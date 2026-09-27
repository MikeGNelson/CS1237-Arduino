#include "CS1237.h"


#define DELAY_455_NS asm volatile ("nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t") // I NEED 57 FOR MY ESP32

CS1237::CS1237(uint8_t sclkPin, uint8_t doutPin) : _sclkPin(sclkPin), _doutPin(doutPin) {}

void CS1237::begin() {
    pinMode(_sclkPin, OUTPUT);
    pinMode(_doutPin, INPUT);
    digitalWrite(_sclkPin, LOW);
}

void CS1237::powerDown() {
    digitalWrite(_sclkPin, HIGH);
    // delayMicroseconds(100);
    DELAY_455_NS;
}

void CS1237::powerUp() {
    digitalWrite(_sclkPin, LOW);
    // delayMicroseconds(100);
    DELAY_455_NS;
}

long CS1237::readData() {
    while (digitalRead(_doutPin) == HIGH);  // Wait for data ready

    long data = 0;
    for (uint8_t i = 0; i < 24; i++) {
        digitalWrite(_sclkPin, HIGH);
        DELAY_455_NS;
        data |= digitalRead(_doutPin) << (23 - i); //Read a bit and shift it up//= (data << 1) | digitalRead(_doutPin);
        digitalWrite(_sclkPin, LOW);
        DELAY_455_NS;
    }

    for (uint8_t i = 0; i < 3; i++)
    {
        digitalWrite(_sclkPin, HIGH);
        DELAY_455_NS; //t6
        digitalWrite(_sclkPin, LOW);
        DELAY_455_NS; //t6
    }

    // Convert from 24-bit two's complement
    if (data & 0x800000) {
        data |= 0xFF000000;
    }

    // if (data >> 23 == 1) //if the 24th bit (sign) is 1, the number is negative
    // {
    //     data = data - 16777216;  //conversion for the negative sign
    //     //"mirroring" around zero
    // }

    return data;
}

int CS1237::getRegister(){
    byte registerValue; //Variable that stores the config register value

    //Shift out 27 (24+3) bits
    long ADCreading = readData(); //32-bit variable that stores the whole ADC reading
    
    pinMode(_doutPin, OUTPUT); //After the 27th SCLK pulse, set DOUT to OUTPUT

    for (uint8_t i = 0; i < 2; i++) //Emit 2 pulses (28-29)
    {
        digitalWrite(_sclkPin, HIGH);
        DELAY_455_NS; //t6
        digitalWrite(_sclkPin, LOW);
        DELAY_455_NS; //t6
    } 

    for (uint8_t i = 0; i < 7; i++) //_sclkPin 30-36, sending READ word
    {
        digitalWrite(_sclkPin, HIGH);
        DELAY_455_NS;
        digitalWrite(_doutPin, ((0x56 >> (6 - i)) & 0b00000001)); //0x56 - READ
        digitalWrite(_sclkPin, LOW);
        DELAY_455_NS;
    }    

    for (uint8_t i = 0; i < 1; i++) //Send the 37th _sclkPin pulse
    {
        digitalWrite(_sclkPin, HIGH);
        DELAY_455_NS; //t6
        digitalWrite(_sclkPin, LOW);
        DELAY_455_NS; //t6
    } 
    
    //After the 37th _sclkPin pulse switch the direction of DOUT. 
    pinMode(_doutPin, INPUT_PULLUP); //we read, so dout becomes INPUT

    registerValue = 0; //Because we are reading

    for (uint8_t i = 0; i < 8; i++) //38-45 _sclkPin pulses
    {
        digitalWrite(_sclkPin, HIGH);
        DELAY_455_NS;        
        registerValue |= digitalRead(_doutPin) << (7 - i); //read out and shift the values into the register_value variable        
        digitalWrite(_sclkPin, LOW);
        DELAY_455_NS;
    }

    // send 1 clock pulse, to set the Pins of the ADCs to output and pull high
    for (uint8_t i = 0; i < 1; i++)
    {
        digitalWrite(_sclkPin, HIGH);
        DELAY_455_NS; //t6
        digitalWrite(_sclkPin, LOW);
        DELAY_455_NS; //t6
    }

    // At the 46th _sclkPin, switch DRDY / DOUT to output and pull up DRDY / DOUT. 
    pinMode(_doutPin, INPUT_PULLUP); //Ready to receive the DRDY to perform a new acquisition

    return registerValue;
}

void CS1237::setRegister(int registertowrite, int valuetowrite)
{
    //"Arbitrary" register numbers
    //0 - Channel
    //1 - PGA
    //2 - Speed
    //3 - REF

    //Config register structure
    //bit 0-1 : Channel. 00 - A, 01 - reserved, 10 - Temperature, 11 - internal short (maybe for offset calibration?)
    //bit 2-3 : PGA. 00 - 1, 01 - 2, 10 - 64, 11 - 128
    //bit 4-5 : speed. 00 - 10 Hz, 01 - 40 Hz, 10 - 640 Hz, 11 - 1280 Hz
    //bit 6   : Reference. Default is enabled which is 0.
    //bit 7   : reserved, don't touch
    //----------------------------------------------------------
    
    byte register_value = getRegister(); //Variable that stores the config register value. Fill it up with the current reg value

    int byteMask = 0b00000000; //Masking byte for writing only 1 register at a time

    switch (registertowrite)
    {
        case 0: //channel
        byteMask = 0b11111100; //when using & operator, we keep all, except channel bits
        register_value = register_value & byteMask; //Update the register_value with mask. This deletes the first two
        
            switch (valuetowrite)
            {
            case 0: // A // W0 0
                register_value = register_value | 0b00000000; //Basically keep everything as-is
                Serial.println("Channel = 0");
                break;
            case 1: // Reserved // W0 1
                //dont implement it!
                Serial.println("Channel = Reserved, invalid!");
                break;
            case 2: //Temperature // W0 2
                register_value = register_value | 0b00000010;
                Serial.println("Channel = Temp");
                break;
            case 3: //Internal short //W0 3
                register_value = register_value | 0b00000011;
                Serial.println("Channel = Short");
                break;
            }      
        break;
        //-------------------------------------------------------------------------------------------------------------

        case 1: //PGA
            byteMask = 0b11110011; //when using & operator, we keep all, except channel bits
            register_value = register_value & byteMask; //Update the register_value with mask. This deletes the first two

            switch (valuetowrite)
            {
            case 0: // PGA 1 //W1 0
                register_value = register_value | 0b00000000; //Basically keep everything as-is
                //pga_divider = 1;
                Serial.println("PGA = 1");
                break;
            case 1: // PGA 2 //W1 1
                register_value = register_value | 0b00000100;
                //pga_divider = 2;
                Serial.println("PGA = 2");
                break;
            case 2: //PGA 64 //W1 2
                register_value = register_value | 0b00001000;
                //pga_divider = 64;
                Serial.println("PGA = 64");
                break;
            case 3: //PGA 128 //W1 3
                register_value = register_value | 0b00001100;
                //pga_divider = 128;
                Serial.println("PGA = 128");
                break;
            }
            break;
            //-------------------------------------------------------------------------------------------------------------

        case 2: //DRATE
            byteMask = 0b11001111; //when using & operator, we keep all, except channel bits
            register_value = register_value & byteMask; //Update the register_value with mask. This deletes the first two

            switch (valuetowrite)
            {
            case 0: // 10 Hz //W2 0
                register_value = register_value | 0b00000000; //Basically keep everything as-is
                Serial.println("DRATE = 10 Hz");
                break;
            case 1: // 40 Hz //W2 1
                register_value = register_value | 0b00010000;
                Serial.println("DRATE = 40 Hz");
                break;
            case 2: //640 Hz //W2 2
                register_value = register_value | 0b00100000;
                Serial.println("DRATE = 640 Hz");
                break;
            case 3: //1280 Hz //W2 3
                register_value = register_value | 0b00110000;
                Serial.println("DRATE = 1280 Hz");
                break;
            }
            break;
            //-------------------------------------------------------------------------------------------------------------
        case 3: //VREF
            if (valuetowrite == 0) //W3 0
            {
                bitWrite(register_value, 6, 0); //Enable
                Serial.println("VREF ON");
            }
            else if (valuetowrite == 1) //W3 1
            {
                bitWrite(register_value, 6, 1); //Disable
                Serial.println("VREF OFF");
            }
            else {}//Other values wont trigger anything
            break;
    }

    //Shift out 27 (24+3) bits
    long ADCreading = readData(); //32-bit variable that stores the whole ADC reading

    pinMode(_doutPin, OUTPUT); //After the 27th _sclkPin pulse, set DOUT to OUTPUT

    for (uint8_t i = 0; i < 2; i++) //Emit 2 pulses (28-29)
    {
        digitalWrite(_sclkPin, HIGH);
        DELAY_455_NS; //t6
        digitalWrite(_sclkPin, LOW);
        DELAY_455_NS; //t6
    } 

    for (uint8_t i = 0; i < 7; i++) //_sclkPin 30-36, sending READ word
    {
        digitalWrite(_sclkPin, HIGH);
        DELAY_455_NS;
        digitalWrite(_doutPin, ((0x65 >> (6 - i)) & 0b00000001)); //0x65 - WRITE
        digitalWrite(_sclkPin, LOW);
        DELAY_455_NS;
    }

    for (uint8_t i = 0; i < 1; i++) //Send the 37th _sclkPin pulse
    {
        digitalWrite(_sclkPin, HIGH);
        DELAY_455_NS; //t6
        digitalWrite(_sclkPin, LOW);
        DELAY_455_NS; //t6
    } 

    for (uint8_t i = 0; i < 8; i++) //38-45 _sclkPin pulses
    {
        digitalWrite(_sclkPin, HIGH);
        DELAY_455_NS;
        digitalWrite(_doutPin, ((register_value >> (7 - i)) & 0b00000001)); //Write the register values        
        digitalWrite(_sclkPin, LOW);
        DELAY_455_NS;
    }

    // send 1 clock pulse, to set the Pins of the ADCs to output and pull high
    for (uint8_t i = 0; i < 1; i++)
    {
        digitalWrite(_sclkPin, HIGH);
        DELAY_455_NS; //t6
        digitalWrite(_sclkPin, LOW);
        DELAY_455_NS; //t6
    }

    // At the 46th _sclkPin, switch DRDY / DOUT to output and pull up DRDY / DOUT. 
    pinMode(_doutPin, INPUT_PULLUP);
}

void CS1237::setGain(uint8_t gain) {
    uint8_t command = 0x65;  // Command to write configuration register
    sendCommand(command);
    // Sending configuration byte with gain setting
    digitalWrite(_sclkPin, LOW);
    DELAY_455_NS;
    for (uint8_t i = 0; i < 8; i++) {
        digitalWrite(_sclkPin, (gain >> (7 - i)) & 1);
        DELAY_455_NS;
        digitalWrite(_sclkPin, HIGH);
        DELAY_455_NS;
        digitalWrite(_sclkPin, LOW);
        DELAY_455_NS;
    }
}

void CS1237::setDataRate(uint8_t rate) {
    uint8_t command = 0x65;  // Command to write configuration register
    sendCommand(command);
    // Sending configuration byte with data rate setting
    digitalWrite(_sclkPin, LOW);
    DELAY_455_NS;
    for (uint8_t i = 0; i < 8; i++) {
        digitalWrite(_sclkPin, (rate >> (7 - i)) & 1);DELAY_455_NS;
        digitalWrite(_sclkPin, HIGH);
        DELAY_455_NS;
        digitalWrite(_sclkPin, LOW);
        DELAY_455_NS;
    }
}

void CS1237::sendCommand(uint8_t command) {
    digitalWrite(_sclkPin, LOW);
    DELAY_455_NS;
    for (uint8_t i = 0; i < 8; i++) {
        digitalWrite(_sclkPin, (command >> (7 - i)) & 1);
        DELAY_455_NS;
        digitalWrite(_sclkPin, HIGH);
        DELAY_455_NS;
        digitalWrite(_sclkPin, LOW);
        DELAY_455_NS;
    }
}
