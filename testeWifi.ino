#define THINGER_SERIAL_DEBUG

#define TdsSensorPin 27
#define SensorTurbidez 26
#define VREF 3.3

#include <ThingerESP32.h>
#include "secrets.h"
#include "esp_random.h"
#include <Arduino.h>
#include <Wire.h>

ThingerESP32 thing (USERNAME, DEVICE_ID, DEVICE_CREDENTIAL);

int temp_var, tds_var, turbid_var_volt, turbid_var_ntu;

float temperature = 25.0; // current temperature

void setup() {
    Serial.begin(115200);
    pinMode(2, OUTPUT);
    thing.add_wifi(SSID, SSID_PASSWORD);
    thing["GPIO_2"] << digitalPin(2);

    pinMode(TdsSensorPin, INPUT);
    pinMode(SensorTurbidez, INPUT);

    thing["Temperature_sensor"] >> [](pson & out){
        out = temp_var;
    };

    thing["tds_sensor"] >> [](pson & out){
        out = tds_var;
    };

    thing["Turbidity_sensor_volt"] >> [](pson & out){
        out = turbid_var_volt;
    };

    thing["Turbidity_sensor_ntu"] >> [](pson & out){
        out = turbid_var_ntu;
    };
}

// Helper function for rounding
float ArredondarPara(float ValorEntrada, int CasaDecimal) {
    float multiplicador = pow(10.0, CasaDecimal);
    ValorEntrada = round(ValorEntrada * multiplicador) / multiplicador;
    return ValorEntrada;
}

// Turbidity reading function
void readTurbid() {
    float voltagem = 0;
    for (int i = 0; i < 800; i++) {
        voltagem += ((float)analogRead(SensorTurbidez) / 4095.0) * 3.3; // Use 4095 for ESP32 ADC and 3.3V reference
    }

    voltagem = voltagem / 800;
    voltagem = ArredondarPara(voltagem, 1);

    float NTU;
    if (voltagem < 2.5) {
        NTU = 3000;
    } else if (voltagem > 4.2) {
        NTU = 0;
        voltagem = 4.2;
    } else {
        NTU = -1120.4 * sq(voltagem) + 5742.3 * voltagem - 4353.8;
    }

    turbid_var_volt = voltagem;
    turbid_var_ntu = NTU;
}

// TDS reading function
void readTDS() {
    int analogValue = analogRead(TdsSensorPin);
    float voltage = analogValue * (float)VREF / 4096.0;
    float compensationCoefficient = 1.0 + 0.02 * (temperature - 25.0);
    float compensationVoltage = voltage / compensationCoefficient;
    float TdsValue = (133.42 * compensationVoltage * compensationVoltage * compensationVoltage 
                     - 255.86 * compensationVoltage * compensationVoltage 
                     + 857.39 * compensationVoltage) * 0.5;

    tds_var = TdsValue;

}

void loop() {
    thing.handle();
    temp_var = 25; // variavel temporária até o sensor estar funcionando
    readTurbid();
    readTDS();
    delay(2000);
}
