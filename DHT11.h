// Code Adapted from DHT11 Library by Eric Fossum from the MBED Code Repository
// Copyright 2016 Eric Fossum
// SPDX-License-Identifier: Apache-2.0
// https://os.mbed.com/users/fossum_13/code/DHT11/
// Licensed with Apache http://www.apache.org/licenses/
// Depreciation issues addressed 11/26/2020

#ifndef DHT11_H
#define DHT11_H
 
#include "mbed.h"
 
#define DHTLIB_OK                0
#define DHTLIB_ERROR_CHECKSUM   -1
#define DHTLIB_ERROR_TIMEOUT    -2
 
/** Class for the DHT11 sensor.
 * 
 * Example:
 * @code
 * #include "mbed.h"
 * #include "DHT11.h"
 *
 * Serial pc(USBTX, USBRX);
 * DHT11 sensor(PTD7); //note this is from a different board
 * 
 * int main() {
 *     sensor.read()
 *     pc.printf("T: %f, H: %d\r\n", sensor.getFahrenheit(), sensor.getHumidity());
 * }
 * @endcode
 */
class DHT11
{
public:
    /** Construct the sensor object.
     *
     * @param pin PinName for the sensor pin.
     */
    DHT11(PinName const &p);
    
    /** Update the humidity and temp from the sensor.
     *
     * @returns
     *   0 on success, otherwise error.
     */
    int read();
    
    /** Get the temp(f) from the saved object.
     *
     * @returns
     *   Fahrenheit float
     */
    float getFahrenheit();
    
    /** Get the temp(c) from the saved object.
     *
     * @returns
     *   Celsius int
     */
    int getCelsius();
    
    /** Get the humidity from the saved object.
     *
     * @returns
     *   Humidity percent int
     */
    int getHumidity();
 
private:
    /// percentage of humidity
    int _humidity;
    /// celsius
    int _temperature;
    /// pin to read the sensor info on
    DigitalInOut _pin;
    /// times startup (must settle for at least a second)
    Timer _timer;
};
 
#endif