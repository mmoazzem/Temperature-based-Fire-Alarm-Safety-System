# Temperature-based-Fire-Alarm-Safety-System

-------------------
About
-------------------

The goal of this project is to develop a temperature-based fire alarm embedded safety system that can be programmed using the Nucleo L4R5ZI, a DHT-11 temperature-humidity sensor, an LCD panel, a 4x4 keypad, and a buzzer. The DHT-11 temperature-humidity sensor will be used as an input to read temperature and humidity. The LCD panel serves as an output and displays the temperature and humidity, while the keypad serves as an input for setting temperature and humidity thresholds. The buzzer also serves as an output, alerting the user in the event of a fire or the possibility of a fire.

-------------------
Contributors
-------------------

Mohammad Tamzeed Moazzem

--------------------
Inputs
--------------------
* DHT-11 Temperature/Humidity Sensor
* 4x4 matrix Keypad

--------------------
Outputs
--------------------
* 1802 LCD Display
* Buzzer
* Red LED

-------------------
Specifications
-------------------

* Temperature will be displayed in LCD panel.
* Humidity will be displayed in LCD panel.
* User can press D to enter thresholds. 
* User can set temperature threshold.
* User can set humidity threshold.
* User can press C to select celsius unit.
* User can press B to select fahrenheit unit.
* When temperature and humidity cross the threshold, a buzzer and red LED will turn on.

-------------------
Constraints
-------------------

* Temperature must be displayed in **°F/°C**
* Humidity must be displayed in percentage.
* User must use a prompt to enter thresholds.
* The system will run **“forever”**.

--------------------
Features
--------------------

* A 4x4 keypad to input time values
	* Uses 4 columns and 4 row to interact with Nucleo
	* Rows connect to Nucleo as output.
	* Columns connect to Nucelo as interrupt input. 

* An LCD panel
	* Displays time and text prompts.
	* Displays user inputs.
	* Uses I2C bus to communicate with Nucelo.
	* An API has been provided.

* Red LEDs 
	* To notify the user when current temperature and humidity are beyond threshold point.

* DHT-11 Temperature/Humidity sensor 
	* To read temperature and humidity

* Buzzer  
	* To notify the user with sound when current temperature and humidity are beyond threshold point .

--------------------
Required Materials
--------------------

* Nucelo L4R5ZI
* 1802 LCD Display
* DHT-11 Temperature/Humidity Sensor
* 4x4 matrix Keypad
* Buzzer
* Red LED
* 1k ohm’s resistors
* Jumper wires
* Breadboard

--------------------
Resources and References
--------------------

Mbed OS API: https://os.mbed.com/docs/mbed-os/v6.15/apis/index.html

--------------------
Getting Started
--------------------
* To get started, install and open Mbed Studio and connect Nucleo L4R5ZI board to the PC. 
* Using Mbed Studio create a mbed-os-empty program and copy the code from CSE321_project3_mmoazzem_main.cpp and place it in main.cpp. 
* Next copy CSE321_project3_mmoazzem_1802.h, CSE321_project3_mmoazzem_1802.cpp, CSE321_project3_mmoazzem_DHT11.h and CSE321_project3_mmoazzem_DHT11.cpp to mbed-os-empty directoty.
* Open **mbed_lib.json** from the directory mbed-os-empty->mbed-os->platform->mbed_lib.json.
* Change value of **minimal-printf-enable-floating-point** to true and value of **minimal-printf-set-floating-point-max-decimals** to 2.
* Then upload the program in Nucleo L4R5ZI board and the system will start working.


--------------------
Pin Connections
--------------------
* Connect first 4 pins of the keypad to PD_6, PD_5, PD_4 and PD_3. 
* Then connect remaining 4 pins to PE_2, PE_4, PE_5 and PE_6.
* Connect positve side (longer leg) of Red LED to PD_7 and negetive side (shorter leg) to ground in series with a 1k Ohms resistor. 
* Next, connect the LCD pins GND, VCC, SDA and SCL to ground, 3.3/5V, PB_9(SDA) and PB_8(SCL) respectively.
* Connect the DHT-11 sensor pins positve(+), out and negetive(-) to 3.3/5V, PF_13(Out) and ground respectively.
* Next, connect buzzer pin GND, I/O and VCC to ground, PD_14(I/O) and 3.3/5V respectively.

--------------------
Files Needed
--------------------

* CSE321_project3_mmoazzem_DHT11.cpp
* CSE321_project3_mmoazzem_DHT11.h
* CSE321_project3_mmoazzem_1802.cpp
* CSE321_project3_mmoazzem_1802.h
* CSE_321_project3_mmoazzem_main.cpp

----------
Things Declared
----------
* Header file:
	* mbed.h
	* CSE321_project2_mmoazzem_1802.h
	* CSE321_project3_mmoazzem_DHT11.h
	* string

* Objects:
	* lcd
	* siren_led
	* sensor
	* buzzer
	* print_thread
	* check_thread
	* mutex
	* print_queue
	* check_queue
	* watchdog
	* col1
	* col2
	* col3
	* col4

* Functions:
	* void col1_isr_handler(void) 
	* void col2_isr_handler(void)
	* void col3_isr_handler(void)
	* void col4_isr_handler(void)
	* void keypad_cycle(void)
	* void set_celsius_threshold(void)
	* void set_fahrenheit_threshold(void)
	* void set_humidity_threshold(void)
	* void print_sensor_data(void)
	* void check_sensor_data(void)
	* void siren (void)

----------
API and Built In Elements Used
----------
* InterruptIn
* string
* CSE321_LCD
* rise
* to_string
* c_str
* begin
* clear
* setcursor
* print
* read
* getFahrenheit
* getCelsius
* getHumidity
* period
* write
* wait_us
* get_instance
* start
* kick

----------
Custom Functions
----------
* void col1_isr_handler(void)
	* This function runs when interrupt is triggered by column #1 button's rising edge.
* void col2_isr_handler(void)
	* This function runs when interrupt is triggered by column #2 button's rising edge.
* void col3_isr_handler(void)
	* This function runs when interrupt is triggered by column #3 button's rising edge.
* void col4_isr_handler(void)
	* This functions run when interrupt is triggered by column #4 button's rising edge.
* void keypad_cycle(void)
	* This function cycles through the keypad by providing power to one column and turning off other columns. 
* void set_celsius_threshold(void)
  * This function handles user input from keypad to set temperature threshold in celsius.
* void set_fahrenheit_threshold(void)
  * This function handles user input from keypad to set temperature threshold in fahrenheit.
* void set_humidity_threshold(void)
  * Handles user input from keypad to set humidity threshold.
* void print_sensor_data(void)
  * This function reads sensor and prints temperature in celsius or in fahrenheit and humidity in percentage in LCD. The function uses mutex to to synchronize
    the access to its critical section.
* void check_sensor_data(void)
  * This function checks temperature and humidity with the thresholds and if temperature is higher than the temperature threshold or humidity is less than the humidity
    threshold, it calls siren() function to activate the buzzer. Otherwise it keeps checking. The function uses mutex to synchronize the access to its critical section.
* void siren (void)
  * Handles buzzer sound.

----------
Improvement Options
----------
To improve the system, a DHT-22 or AM2320 temperature sensor can be used to get more precise reading of temperature and humidity. Also instead of a passive buzzer, using a active buzzer or a speaker would increase the sound quality and will provide more flexible way to control the device. 
