#include <CS1237.h>

// Change these pins to match your wiring.
constexpr uint8_t SCLK_PIN = 18;
constexpr uint8_t DOUT_PIN = 19;

CS1237 adc(SCLK_PIN, DOUT_PIN);

void setup() {
    Serial.begin(115200);
    adc.begin();

    Serial.println("CS1237 Basic Read");
}

void loop() {
    long value = adc.readData();

    Serial.print("ADC: ");
    Serial.println(value);
}