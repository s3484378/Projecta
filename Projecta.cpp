#include <Projecta.h>
#include <Wire.h>
static uint8_t _numObjects = 0;

/* Constructor
 */
Projecta::Projecta(){
    _numObjects++; // Keep track of no of objects (Max 2 i2c ports on the ESP32, 1 Master, 1 slave)
    _projNo = _numObjects;
    setButtonVoltCallback(NULL);
    setButtonBatteryCallback(NULL);
    setButtonChargeCallback(NULL);
    setButtonReconditionCallback(NULL);
}

uint8_t Projecta::getNo(){
    return _projNo;
}

/* Function to set last byte of the data to send.
 * The screen returns an error if this is not done
 * when any other byte in the send array is changed
 * @input -> NULL
 * @returns -> NULL
 */
void Projecta::setLastByte(void){
    uint8_t j=0;
    for(int i=0;i<9;i++){
        j += _sendBytes[i];
    }
    j &= 0xff;
    _sendBytes[9] = j;
}

/* Function to clear all values from the screen
 * @input -> NULL
 * @returns -> projecta_error:
 *      PROJ_OK
 */
projecta_error Projecta::clearScreen(){
    for(int i=0; i<9; i++){
        _sendBytes[i] = 0x00;
    }
    setLastByte();
    return PROJ_OK;
}


/* Begin Function to Initialise the Wire library
 * and clear the screen.
 * @input -> NULL
 * @returns -> projecta_error:
 *      PROJ_OK
 *      PROJ_I2C_ERROR 
 */
projecta_error Projecta::begin(){
    _proj_mode = PROJECTA_MASTER;
    switch(_projNo){
        case 1:
            Wire.begin();
            clearScreen();
            Wire.beginTransmission(0x65);
            if (Wire.endTransmission()){
                return PROJ_I2C_ERROR;
            }else{
                return PROJ_OK;
            }
            break;
        #ifdef MULTI_I2C
        case 2:
            Wire1.begin();
            clearScreen();
            Wire1.beginTransmission(0x65);
            if (Wire1.endTransmission()){
                return PROJ_I2C_ERROR;
            }else{
                return PROJ_OK;
            }
            break;
        #endif
        default:
            return PROJ_LIMIT_2_EXCEEDED;
            break;
    }
}

#ifdef MULTI_I2C
/* Begin Function to Initialise the Wire library
 * and clear the screen.
 * @input -> sda and scl pins (For espressif chips)
 * @returns -> projecta_error:
 *      PROJ_OK
 *      PROJ_I2C_ERROR 
 */
projecta_error Projecta::begin(int sda, int scl){
    _proj_mode = PROJECTA_MASTER;
    switch(_projNo){
        case 1:
            Wire.begin(sda, scl);
            clearScreen();
            Wire.beginTransmission(0x65);
            if (Wire.endTransmission()){
                return PROJ_I2C_ERROR;
            }else{
                return PROJ_OK;
            }
            break;
        case 2:
            Wire1.begin(sda, scl);
            clearScreen();
            Wire1.beginTransmission(0x65);
            if (Wire1.endTransmission()){
                return PROJ_I2C_ERROR;
            }else{
                return PROJ_OK;
            }
            break;
        default:
            return PROJ_LIMIT_2_EXCEEDED;
            break;
    }
}

/* Begin Function to Initialise the Wire library
 * and clear the screen.
 * @input -> sda and scl pins (For espressif chips),
 * i2c frequency
 * @returns -> projecta_error:
 *      PROJ_OK
 *      PROJ_I2C_ERROR 
 */
projecta_error Projecta::begin(int sda, int scl, int freq){
    _proj_mode = PROJECTA_MASTER;
    switch(_projNo){
        case 1:
            Wire.begin(sda, scl, freq);
            clearScreen();
            Wire.beginTransmission(0x65);
            if (Wire.endTransmission()){
                return PROJ_I2C_ERROR;
            }else{
                return PROJ_OK;
            }
            break;
        case 2:
            Wire1.begin(sda, scl, freq);
            clearScreen();
            Wire1.beginTransmission(0x65);
            if (Wire1.endTransmission()){
                return PROJ_I2C_ERROR;
            }else{
                return PROJ_OK;
            }
            break;
        default:
            return PROJ_LIMIT_2_EXCEEDED;
            break;
    }
}
#endif

/* Function to assign a callback for the volt/amp button presses
 * @input -> the button callback function
 * @returns -> Null to the user
 */
Projecta& Projecta::setButtonVoltCallback(BUTTON_VOLT_CALLBACK_SIGNATURE){
    this->voltCallback = voltCallback;
    return *this;
}

/* Function to assign a callback for the battery-type button presses
 * @input -> the button callback function
 * @returns -> Null to the user
 */
Projecta& Projecta::setButtonBatteryCallback(BUTTON_BATTERY_CALLBACK_SIGNATURE){
    this->batteryCallback = batteryCallback;
    return *this;
}

/* Function to assign a callback for the charge-rate button presses
 * @input -> the button callback function
 * @returns -> Null to the user
 */
Projecta& Projecta::setButtonChargeCallback(BUTTON_CHARGE_CALLBACK_SIGNATURE){
    this->chargeCallback = chargeCallback;
    return *this;
}

/* Function to assign a callback for the recondition button presses
 * @input -> the button callback function
 * @returns -> Null to the user
 */
Projecta& Projecta::setButtonReconditionCallback(BUTTON_RECONDITION_CALLBACK_SIGNATURE){
    this->reconditionCallback = reconditionCallback;
    return *this;
}

/* Function which decodes the button presses and returns
 * the value to the user's callback function
 * @input -> byte[0] of incoming i2c data (called in loop)
 * @returns -> NULL
 */
void Projecta::decodeButtons(uint8_t rawBut){
    uint8_t change = rawBut ^ lastButtonState;
    switch(change){
        case 0x01: // Charge rate
            if(chargeCallback){
                chargeCallback(rawBut & 0x01);
            }
            break;
        case 0x02: // Volt amp
            if(voltCallback){
                voltCallback(rawBut & 0x02);
            }
            break;
        case 0x04: // Battery Type
            if(batteryCallback){
                batteryCallback(rawBut & 0x04);
            }
            break;
        case 0x08: // Recondition
            if(reconditionCallback){
                reconditionCallback(rawBut & 0x08);
            }
            break;
    }
    lastButtonState = rawBut;
}

/* Loop function which user must place in the main
 * loop of their code otherwise the i2c communication
 * will not work
 * @input -> NULL
 * @returns -> NULL
 */ 
void Projecta::loop(){
    switch(_projNo){
        case 1:
            for(int i=0;i<5;i++){
                Wire.requestFrom(0x65,3);
                while(Wire.available()){
                    Wire.readBytes(_receiveBytes,3);
                }
                decodeButtons(_receiveBytes[0]);
            }
            Wire.beginTransmission(0x65);
            for(int i=0;i<10;i++){
                Wire.write(_sendBytes[i]);
            }
            Wire.endTransmission();
            break;
    #ifdef MULTI_I2C
        case 2:
            for(int i=0;i<5;i++){
                Wire1.requestFrom(0x65,3);
                while(Wire1.available()){
                    Wire1.readBytes(_receiveBytes,3);
                }
                decodeButtons(_receiveBytes[0]);
            }
            Wire1.beginTransmission(0x65);
            for(int i=0;i<10;i++){
                Wire1.write(_sendBytes[i]);
            }
            Wire1.endTransmission();
            break;
    #endif
            default:
                break;
    }
}

/* Function to encode a double variable to the 3x seven segment
 * displays
 * @input -> the double value to be encoded
 * @returns -> projecta_error:
 *      PROJ_OK
 *      PROJ_NUMBER_TOO_BIG
 */
projecta_error Projecta::sevenSegEncoder(double val){
    const uint8_t numbers[]={0xEB,0x60,0xC7,0xE5,0x6C,0xAD,0xAF,0xE0,0xEF,0xED};
    int h,t,o;
    if(val < 10){
        val = round(val*100);
        _sendBytes[3] = 0x10; // Decimal Place
        _sendBytes[2] = 0x00;
    }else if(val < 100){
        val = round(val*10);
        _sendBytes[3] = 0x00;
        _sendBytes[2] = 0x10; // Decimal Place
    }else if(val < 1000){
        val = round(val);
        _sendBytes[3] = 0x00;
        _sendBytes[2] = 0x00; // Decimal Place
    }
    if(val < 1000){
        h = (int)val/100;
        t = (int)(val/10-(h*10))%10;
        o = (int)val-h*100-t*10;
        _sendBytes[3] |= numbers[h];
        _sendBytes[2] |= numbers[t];
        _sendBytes[1] = numbers[o];
        return PROJ_OK;
    }else{
        _sendBytes[3] = 0x04;
        _sendBytes[2] = 0x04;
        _sendBytes[1] = 0x04;
        return PROJ_NUMBER_TOO_BIG;
    }
    
}

projecta_error Projecta::sevenSegEncoder(char* let){
    const uint8_t chars[] = {
        0xEE, //a
        0x2F, //b
        0x8B, //c
        0x67, //d
        0x8F, //e
        0x8E, //f
        0xED, //g
        0x2E, //h
        0x0A, //i
        0x63, //j
        0x6E, //k
        0x0B, //l
        0xA2, //m
        0x26, //n
        0xEB, //o
        0xCE, //p
        0xEC, //q
        0x06, //r
        0xAD, //s
        0x0F, //t
        0x6B, //u
        0x23, //v
        0x49, //w
        0x6E, //x
        0x6D, //y
        0xC7, //z
        0x00  // space
    };
    char toPrint[strlen(let)];
    for(int i=0; i<(int)strlen(let); i++){
        if(let[i] >= 'A' && let[i] <= 'Z'){
            toPrint[i] = let[i] + 32;
        }else if(let[i] >= 'a' && let[i] <= 'z'){
            toPrint[i] = let[i];
        }else if(let[i] == ' '){ // Space
            toPrint[i] = 0x5B;
        }
        else{
            return PROJ_CHARACTER_INVALID;
        }
    }
    _sendBytes[1] = chars[(uint8_t)toPrint[2] - 'a'];
    _sendBytes[2] = chars[(uint8_t)toPrint[1] - 'a'];
    _sendBytes[3] = chars[(uint8_t)toPrint[0] - 'a'];
    setLastByte();
    return PROJ_OK;
}

/* Set Voltage Function. Will update the screen
 * value with the value input and will display the
 * 'V' symbol
 * @input -> double voltage value
 * @returns -> projecta_error:
 *      PROJ_OK
 */
projecta_error Projecta::setVoltage(double volt){
    sevenSegEncoder(volt);
    _sendBytes[0] = 0x08; // V
    setLastByte();
    return PROJ_OK;
}

/* Set Current Function. Will update the screen
 * value with the value input and will display the
 * 'A' symbol
 * @input -> double current value
 * @returns -> projecta_error:
 *      PROJ_OK
 */
projecta_error Projecta::setCurrent(double amp){
    sevenSegEncoder(amp);
    _sendBytes[0] = 0x80; // A
    setLastByte();
    return PROJ_OK;
}

/* Set Power Function. Will update the screen
 * value with the value input and will display the
 * 'W' symbol if less than 1000, or 'KW' symbol
 * if greater than 1000
 * @input -> double wattage value
 * @returns -> projecta_error:
 *      PROJ_OK
 */
projecta_error Projecta::setWatts(double watt){
    if(watt < 1000){
        sevenSegEncoder(watt);
        _sendBytes[0] = 0x20; // W
    }else{
        sevenSegEncoder(watt/1000);
        _sendBytes[0] = 0x22; // W
    }
    setLastByte();
    return PROJ_OK;
}

/* Set Temperature Function. Will update the screen
 * value with the value input and will display the
 * '&degC' symbol
 * @input -> double temperature value
 * @returns -> projecta_error:
 *      PROJ_OK
 */
projecta_error Projecta::setTemperature(double temp){
    sevenSegEncoder(temp);
    _sendBytes[0] = 0x04;
    setLastByte();
    return PROJ_OK;
}

/* Set Percentage Function. Will update the screen
 * value with the value input and will display the
 * '%' symbol
 * @input -> double percentage value
 * @returns -> projecta_error:
 *      PROJ_OK
 */
projecta_error Projecta::setPercent(double percent){
    sevenSegEncoder(percent);
    _sendBytes[0] = 0x40;
    setLastByte();
    return PROJ_OK;
}

/* Set Amp Hour Function. Will update the screen
 * value with the value input and will display the
 * 'AH' symbol
 * @input -> double Amp Hour value
 * @returns -> projecta_error:
 *      PROJ_OK
 */
projecta_error Projecta::setAh(double ah){
    sevenSegEncoder(ah);
    _sendBytes[0] = 0x11;
    setLastByte();
    return PROJ_OK;
}

/* Set KWH Function. Will update the screen
 * value with the value input and will display the
 * 'KWH' symbol
 * @input -> double KWH value
 * @returns -> projecta_error:
 *      PROJ_OK
 */
projecta_error Projecta::setWh(double wh){
    if(wh < 1000){
        sevenSegEncoder(wh);
        _sendBytes[0] = 0x30; // Wh
    }else{
        sevenSegEncoder(wh/1000);
        _sendBytes[0] = 0x32; // KWh
    }
    // sevenSegEncoder(wh);
    // _sendBytes[0] = 0x32;
    setLastByte();
    return PROJ_OK;
}

/* Set Hour Function. Will update the screen
 * value with the value input and will display the
 * 'H' symbol
 * @input -> double Hour value
 * @returns -> projecta_error:
 *      PROJ_OK
 */
projecta_error Projecta::setHours(double h){
    sevenSegEncoder(h);
    _sendBytes[0] = 0x1;
    setLastByte();
    return PROJ_OK;    
}

/* Function to set the battery type on the screen
 * @input -> projecta_battery_type enum
 * @returns -> projecta_error:
 *      PROJ_OK
 *      PROJ_BATTERY_TYPE_INVALID
 */
projecta_error Projecta::setBatteryType(projecta_battery_type bat){
    // Clear battery first (can remove this if for some reason more than 1 should be displayed)
    _sendBytes[5] &= 0x33;
    _sendBytes[6] &= 0xF7;
    switch(bat){
        case BATTERY_NONE:
            _sendBytes[5] &= 0x33;
            _sendBytes[6] &= 0xF7;
            break;
        case BATTERY_POWER_SUPPLY:
            _sendBytes[6] |= 0x08;
            break;
        case BATTERY_GEL:
            _sendBytes[5] |= 0x40;
            break;
        case BATTERY_AGM:
            _sendBytes[5] |= 0x04;
            break;
        case BATTERY_WET:
            _sendBytes[5] |= 0x80;
            break;
        case BATTERY_CALCIUM:
            _sendBytes[5] |= 0x08;
            break;
        default:
            return PROJ_BATTERY_TYPE_INVALID;
            break;
    }
    setLastByte();
    return PROJ_OK;
}

/* Function to set the battery status bar
 * 0 is off, and 1 - 4 bars
 * @input -> number of bars or off (uint8_t)
 * @returns -> projecta_error:
 *      PROJ_OK
 *      PROJ_BATTERY_BAR_INVALUD
 */
projecta_error Projecta::setBatteryBar(uint8_t batBar){
    _sendBytes[4] &= 0x0F;
    _sendBytes[6] &= 0xEF;
    switch(batBar){
        case 0:
            _sendBytes[4] &= 0x0F;
            _sendBytes[6] &= 0xEF;
            break;
        case 1:
            _sendBytes[4] |= 0x80;
            _sendBytes[6] |= 0x10;
            break;
        case 2:
            _sendBytes[4] |= 0xC0;
            _sendBytes[6] |= 0x10;
            break;
        case 3:
            _sendBytes[4] |= 0xE0;
            _sendBytes[6] |= 0x10;
            break;
        case 4:
            _sendBytes[4] |= 0xF0;
            _sendBytes[6] |= 0x10;
            break;
        default:
            return PROJ_BATTERY_BAR_INVALUD;
            break;
    }
    setLastByte();
    return PROJ_OK;
}

/* Set buzzer function - on or off
 * @input -> boolean value for the buzzer true = on
 * @returns projecta_error:
 *      PROJ_OK
 */
projecta_error Projecta::setBuzzer(bool buz){
    _sendBytes[7] ^= (-buz ^ _sendBytes[7]) & (1 << 5);
    setLastByte();
    return PROJ_OK;
}

/* Set LED function
 * @input -> projecta_led enum and the value for the led
 * true = on, false = off
 * @returns projecta_error:
 *      PROJ_LED_NOT_VALID
 *      PROJ_OK
 */
projecta_error Projecta::setLed(projecta_led led, bool onOff){
    // Disable Leds First
    _sendBytes[7] &= 0xE0;
    switch(led){
        case LED_GREEN_RED_FLASHING:
            _sendBytes[7] ^= (-onOff ^ _sendBytes[7]) & (1 << 4);
            break;
        case LED_RED_FLASHING:
            _sendBytes[7] ^= (-onOff ^ _sendBytes[7]) & (1 << 3);
            break;
        case LED_SOLID_RED:
            _sendBytes[7] ^= (-onOff ^ _sendBytes[7]) & (1 << 2);
            break;
        case LED_GREEN_FLASHING:
            _sendBytes[7] ^= (-onOff ^ _sendBytes[7]) & (1 << 1);
            break;
        case LED_SOLID_GREEN:
            _sendBytes[7] ^= (-onOff ^ _sendBytes[7]) & (1 << 0);
            break;
        default:
            return PROJ_LED_NOT_VALID;
            break;
    }
    setLastByte();
    return PROJ_OK;
}

String Projecta::getErrorString(projecta_error err){
    switch(err){
        case PROJ_OK:
            return "PROJ_OK";
            break;
        case PROJ_I2C_ERROR:
            return "PROJ_I2C_ERROR";
            break;
        case PROJ_LIMIT_2_EXCEEDED:
            return "PROJ_LIMIT_2_EXCEEDED";
            break;
        case PROJ_BATTERY_BAR_INVALUD:
            return "PROJ_BATTERY_BAR_INVALUD";
            break;
        case PROJ_BATTERY_TYPE_INVALID:
            return "PROJ_BATTERY_TYPE_INVALID";
            break;
        case PROJ_LED_NOT_VALID:
            return "PROJ_LED_NOT_VALID";
            break;
        case PROJ_NUMBER_TOO_BIG:
            return "PROJ_NUMBER_TOO_BIG";
            break;
        case PROJ_CHARACTER_INVALID:
            return "PROJ_CHARACTER_INVALID";
            break;
        case PROJ_UNKNOWN_ERROR:
            return "PROJ_UNKNOWN_ERROR";
            break;
        default:
            return "PROJ_UNKNOWN_ERROR";
    }
}