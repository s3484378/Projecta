/* Projecta Master Library - Ben Soutter 2018
 * Can be used to control the remote screen
 * or to confirm the functionality of the Projecta
 * slave library without connecting to a charger. 
 */
#ifndef Projecta_h
#define Projecta_h
#include <Arduino.h>

// Defines to determine is multi I2C ports are available
#if defined(ESP32) || defined(ESP8266)
#define MULTI_I2C
#endif

#define BUTTON_VOLT_CALLBACK_SIGNATURE void (*voltCallback)(bool pressed)   // Function Callback Definition
#define BUTTON_BATTERY_CALLBACK_SIGNATURE void (*batteryCallback)(bool pressed)   // Function Callback Definition
#define BUTTON_CHARGE_CALLBACK_SIGNATURE void (*chargeCallback)(bool pressed)   // Function Callback Definition
#define BUTTON_RECONDITION_CALLBACK_SIGNATURE void (*reconditionCallback)(bool pressed)   // Function Callback Definition

typedef enum{
    PROJ_OK,
    PROJ_I2C_ERROR,
    PROJ_LIMIT_2_EXCEEDED,
    PROJ_BATTERY_BAR_INVALUD,
    PROJ_BATTERY_TYPE_INVALID,
    PROJ_LED_NOT_VALID,
    PROJ_NUMBER_TOO_BIG,
    PROJ_CHARACTER_INVALID,
    PROJ_UNKNOWN_ERROR
}projecta_error;

typedef enum{
    BUTTON_BATTERY_TYPE_PRESSED,
    BUTTON_BATTERY_TYPE_RELEASED,
    BUTTON_RECONDITION_PRESSED,
    BUTTON_RECONDITION_RELEASED,
    BUTTON_CHARGE_RATE_PRESSED,
    BUTTON_CHARGE_RATE_RELEASED,
    BUTTON_VOLT_AMP_PRESSED,
    BUTTON_VOLT_AMP_RELEASED
}projecta_button;

typedef enum{
    BATTERY_NONE = 0,
    BATTERY_POWER_SUPPLY,
    BATTERY_GEL,
    BATTERY_AGM,
    BATTERY_WET,
    BATTERY_CALCIUM
}projecta_battery_type;

typedef enum{
    LED_GREEN_RED_FLASHING = 0,
    LED_RED_FLASHING,
    LED_SOLID_RED,
    LED_GREEN_FLASHING,
    LED_SOLID_GREEN
}projecta_led;

typedef enum{
    PROJECTA_SLAVE = 0,
    PROJECTA_MASTER
}projecta_mode;

class Projecta{
    private:
        void setLastByte(void);
        void decodeButtons(uint8_t rawBut);
        projecta_error sevenSegEncoder(double);
        uint8_t _sendBytes[10];
        uint8_t _receiveBytes[3];
        BUTTON_VOLT_CALLBACK_SIGNATURE;
        BUTTON_BATTERY_CALLBACK_SIGNATURE;
        BUTTON_CHARGE_CALLBACK_SIGNATURE;
        BUTTON_RECONDITION_CALLBACK_SIGNATURE;
        uint8_t lastButtonState = 0;
        static const uint8_t numberDecodeArray[9][7];
        uint8_t _projNo; // Store the _numObjects val in here when instantiated
        projecta_mode _proj_mode;
    public:
        Projecta();
        projecta_error begin();
        #ifdef MULTI_I2C
        projecta_error begin(int sda, int scl);
        projecta_error begin(int sda, int scl, int freq);
        #endif
        Projecta& setButtonVoltCallback(BUTTON_VOLT_CALLBACK_SIGNATURE);
        Projecta& setButtonBatteryCallback(BUTTON_BATTERY_CALLBACK_SIGNATURE);
        Projecta& setButtonChargeCallback(BUTTON_CHARGE_CALLBACK_SIGNATURE);
        Projecta& setButtonReconditionCallback(BUTTON_RECONDITION_CALLBACK_SIGNATURE);
        projecta_error clearScreen();
        projecta_error setVoltage(double);
        projecta_error setCurrent(double);
        projecta_error setWatts(double);
        projecta_error setTemperature(double);
        projecta_error setPercent(double);
        projecta_error setAh(double);
        projecta_error setWh(double);
        projecta_error setHours(double);
        projecta_error setBatteryType(projecta_battery_type);
        projecta_error setBatteryBar(uint8_t);
        projecta_error setBuzzer(bool);
        projecta_error setLed(projecta_led, bool);
        String getErrorString(projecta_error);
        void loop();

        uint8_t getNo();

        projecta_error sevenSegEncoder(char*); //
};

#endif