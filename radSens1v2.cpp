#include "radSens1v2.h"

//gcc -o test test.cpp radSens1v2.cpp -lwiringPi

ClimateGuard_RadSens1v2::ClimateGuard_RadSens1v2()
{
    _sensor_address = 0x66;
}

ClimateGuard_RadSens1v2::~ClimateGuard_RadSens1v2()
{
}

/*Initialization function and sensor connection. Returns false if the sensor is not connected to the I2C bus.*/
bool ClimateGuard_RadSens1v2::radSens_init()
{
    Wire.begin();
    Wire.beginTransmission(_sensor_address); //safety check, make sure the sensor is connected
    Wire.write(0x0);
    if (Wire.endTransmission(true) != 0)
        return false;
    updateData();
    return true;
}

bool ClimateGuard_RadSens1v2::updateData()
{
    Wire.requestFrom(_sensor_address, 21);
    for (int i = 0; i < 21; i++)
    {
        _data[i] = Wire.read();
    }

    if (_data[0] == 0x7D)
    {
        _pulse_cnt += (_data[9] << 8) | _data[10];
        return true;
    }
    return false;
}

bool ClimateGuard_RadSens1v2::getData()
{
    if (updateData())
    {
        intensy_static = (((uint32_t)_data[6] << 16) | ((uint16_t)_data[7] << 8) | _data[8]) / 10.0;
        intensy_dyanmic = (((uint32_t)_data[3] << 16) | ((uint16_t)_data[4] << 8) | _data[5]) / 10.0;
        pulses = _pulse_cnt;
        return true;
    }
    return false;
}

/*Get chip id, default value: 0x7D.*/
uint8_t ClimateGuard_RadSens1v2::getChipId()
{
    _chip_id = _data[0];
    return _chip_id;
}

/*Get firmware version.*/
uint8_t ClimateGuard_RadSens1v2::getFirmwareVersion()
{
    _firmware_ver = _data[1];
    return _firmware_ver;
}

/*Get radiation intensity (dynamic period T < 123 sec).*/
float ClimateGuard_RadSens1v2::getRadIntensyDynamic()
{
    if (updateData())
    {
        float temp = (((uint32_t)_data[3] << 16) | ((uint16_t)_data[4] << 8) | _data[5]) / 10.0;
        return temp;
    }
    else
    {
        return 0;
    }
}

/*Get radiation intensity (static period T = 500 sec).*/
float ClimateGuard_RadSens1v2::getRadIntensyStatic()
{
    if (updateData())
    {
        return (((uint32_t)_data[6] << 16) | ((uint16_t)_data[7] << 8) | _data[8]) / 10.0;
    }
    else
    {
        return 0;
    }
}

/*Get the accumulated number of pulses registered by the module
since the last I2C data reading.*/
uint32_t ClimateGuard_RadSens1v2::getNumberOfPulses()
{
    if (updateData())
    {
        return _pulse_cnt;
    }
    else
    {
        return 0;
    }
}

/*Get sensor address.*/
uint8_t ClimateGuard_RadSens1v2::getSensorAddress()
{
    if (updateData())
    {
        _sensor_address = _data[16];
        return _sensor_address;
    }
    return 0;
}

/*Get state of high-voltage voltage Converter.*/
bool ClimateGuard_RadSens1v2::getHVGeneratorState()
{
    if (updateData())
    {
        if (_data[17] == 1)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    return false;
}

/*Get the value coefficient used for calculating the radiation intensity.*/
uint16_t ClimateGuard_RadSens1v2::getSensitivity()
{
    if (updateData())
    {
        return _data[19] * 256 + _data[18];
    }
    return 0;
}

/*Control register for a high-voltage voltage Converter. By
default, it is in the enabled state. To enable the HV generator,
write 1 to the register, and 0 to disable it. If you try to write other
values, the command is ignored.
bool state = *state**/
bool ClimateGuard_RadSens1v2::setHVGeneratorState(bool state)
{
    Wire.beginTransmission(0x66);
    Wire.write(RS_HV_GENERATOR_RG);
    if (state)
    {
        Wire.write(1);
    }
    else
    {
        Wire.write(0);
    }
    if (Wire.endTransmission(true) == 0)
        return true; //"true" sends stop message after transmission & releases I2C bus

    return false;
}

/*Contains the value coefficient used for calculating
the radiation intensity. If necessary (for example, when installing a different
type of counter), the necessary sensitivity value in
imp/MKR is entered in the register. The default value is 105 imp/MKR. At the end of
recording, the new value is stored in the non-volatile memory of the
microcontroller.
uint16_t sens = *coefficient**/
bool ClimateGuard_RadSens1v2::setSensitivity(uint16_t sens)
{
    Wire.beginTransmission(0x66);

    Wire.write(RS_SENSITIVITY_RG);
    Wire.write((uint8_t)(sens & 0xFF));
    Wire.endTransmission(true);
    delay(15);
    Wire.beginTransmission(0x66);
    Wire.write(RS_SENSITIVITY_RG + 0x01);
    Wire.write((uint8_t)(sens >> 8));
    bool err = Wire.endTransmission(true);
    delay(15);
    if (!err)
        return true;

    return false;
}

/*Control register for a indication diode. By
default, it is in the enabled state. To enable the indication,
write 1 to the register, and 0 to disable it. If you try to write other
values, the command is ignored.
bool state = *state**/
bool ClimateGuard_RadSens1v2::setLedState(bool state)
{
    Wire.beginTransmission(0x66);

    Wire.write(RS_LED_CONTROL_RG);
    if (state)
    {
        Wire.write(1);
    }
    else
    {
        Wire.write(0);
    }
    bool err = Wire.endTransmission(true);
    delay(15);
    if (!err)
        return true;

    return false;
}

/*Get state of led indication.*/
bool ClimateGuard_RadSens1v2::getLedState()
{
    if (updateData())
    {
        if (_data[20] == 1)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    return false;
}

