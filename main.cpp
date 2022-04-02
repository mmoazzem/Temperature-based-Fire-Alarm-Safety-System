/*
 *
 * Author                   : Mohammad Tamzeed Moazzem
 *
 * Purpose                  : A temperature/humidity based fire alert system that can be programmed using Nucleo L4R5ZI, DHT-11 temperature-humidity sensor, 4x4 keypad, an 1802 LCD 
 *                            panel and a buzzer
 *
 * Modules/Subroutines      : void col1_isr_handler(void); void col2_isr_handler(void); void col3_isr_handler(void); void col4_isr_handler(void);
 *                            void keypad_cycle(void); void set_celsius_threshold(void); void set_fahrenheit_threshold(void); void set_humidity_threshold(void);
 *                            void print_sensor_data(void); void check_sensor_data(void); void siren (void)
 *
 *
 * Inputs                   : 4x4 Keypad, DHT-11 sensor
 *
 * Outputs                  : 1802 LCD, LEDs, Buzzer
 *
 * Constraints              : Temperature must be displayed in °F/°C.
 *                            Humidity must be displayed in percentage.
 *                            User must use a prompt to enter thresholds.
 *                            The system will run forever.
 *
 * Sources/References       : https://os.mbed.com/docs/mbed-os/v6.15/apis/index.html
 *
 */



#include "mbed.h"
#include <string>
#include "1802.h"
#include "DHT11.h"

// Time delay to address bounce in microseconds
#define BOUNCE_DELAY_US 50000

// Watchdog timeout
#define TIMEOUT_MS 10000

// Interrupt Handler functions
void col1_isr_handler(void);
void col2_isr_handler(void);
void col3_isr_handler(void);
void col4_isr_handler(void);

// Keypad polling function
void keypad_cycle(void);

// Set temperature threshold in celsius
void set_celsius_threshold(void);

// Set temperature threshold in Fahrenheit
void set_fahrenheit_threshold(void);

// Set humidity threshold
void set_humidity_threshold(void);

// Prints current temperature and humidity in LCD panel
void print_sensor_data(void);

// Checks current temperature and humidity with the thresholds
void check_sensor_data(void);

// Handles buzzer sound
void siren (void);

// Interrupt Objects. Establishes an interrupt triggered by button on keypad.
InterruptIn col1(PE_2, PullDown);
InterruptIn col2(PE_4, PullDown);
InterruptIn col3(PE_5, PullDown);
InterruptIn col4(PE_6, PullDown);

// LCD Object with initialization
CSE321_LCD lcd(16, 2, LCD_5x8DOTS, PB_9, PB_8);

// DigitalOut object
DigitalOut siren_led(PD_7);

// DHT-11 sensor object with initialization
DHT11 sensor(PF_13);

// Buzzer object with initialization
PwmOut buzzer(PD_14);

// Thread object to print on LCD display
Thread print_thread;

// Thread object to check temperature and humidity
Thread check_thread;

// Mutex object 
Mutex mutex;

// Creates a event queue to print sensor data
EventQueue print_queue (32 * EVENTS_EVENT_SIZE);

// Creates a event queue to check sensor data
EventQueue check_queue (32 * EVENTS_EVENT_SIZE);

// Gets a reference to the single Watchdog instance.
Watchdog &watchdog = Watchdog::get_instance();

int row = 0; // Row counter
float temperature_threshold = 0; // Temperature threshold holder
int humidity_threshold = 0; // Humidity threshold holder
float current_fahrenheit = 0; // Current temperature holder (in fahrenheit)
int current_celsius = 0; // Current temperature holder (in celsius)
int current_humidity = 0; // Current humidity holder

string key = ""; // Empty string
string to_print = ""; // Empty string
bool is_key_pressed = false; // Button press flag. True when a button is pressed.
bool flag_celsius = false; // Celsius unit enable flag.
bool flag_decimal_point = false; // Decimal point flag. True when B is pressed.
bool flag_threshold = false; // Threshold flag. True when D is press to enter threshold
char degree = (char)223; // Degree Sign charecter
const char *degree_sign = &degree; // Pointer to degree variale

// main() runs in its own thread in the OS
int main()
{   
    // Enable clock to Port D
    RCC->AHB2ENR |= 0x8;

    // Enable pin PD_3, PD_4, PD_5 and PD_6 as output
    GPIOD->MODER |= 0x1540;
    GPIOD->MODER &= ~(0x2A80);

    // Set LCD to correct state
    lcd.begin();

    buzzer.write(0.0);

    // Attach the isr_row1() function's address to the rising edge
    col1.rise(&col1_isr_handler);

    // Attach the isr_row2() function's address to the rising edge
    col2.rise(&col2_isr_handler);

    // Attach the isr_row3() function's address to the rising edge
    col3.rise(&col3_isr_handler);

    // Attach the isr_row4() function's address to the rising edge
    col4.rise(&col4_isr_handler);

    // Clear LCD panel
    lcd.clear();
    
    // Display text in LCD
    lcd.print("Press D to enter");

    // Sets LCD cursor to row 1 and column 3
    lcd.setCursor(3, 1);

    // Display text in LCD
    lcd.print("thresholds");

    // Spins a loop
    while (true) {

        // Cycles power through keypad row pins
        keypad_cycle();

        // Checks if D is pressed
        if(is_key_pressed && key.compare("D")==0){
            is_key_pressed = false;
            key = "";
            break;
        }
    }    

    // Clears the LCD panel
    lcd.clear();

    // Display text in LCD
    lcd.print("Press C for ");
    lcd.print(degree_sign);
    lcd.print("C");

    // Sets LCD cursor to row 1 and column 0
    lcd.setCursor(0, 1);

    // Display text in LCD
    lcd.print("Press B for ");
    lcd.print(degree_sign);
    lcd.print("F");

    // Spins a loop
    while (true) {

        // Cycles power through keypad row pins
        keypad_cycle();
        
        // Checks if C is pressed
        if(is_key_pressed && key.compare("C")==0){
            flag_celsius = true;
            is_key_pressed = false;
            key = "";
            break;
        }

        // Checks if B is pressed
        if(is_key_pressed && key.compare("B")==0){
            is_key_pressed = false;
            flag_threshold = true;
            key = "";
            break;
        }
    }

    // Checks if flag_celsius is true
    if(flag_celsius){
        // Invokes the function to set temperature threshold in celsius.
        set_celsius_threshold();

    }else {
        // Invokes the function to set temperature threshold in fahrenheit.
        set_fahrenheit_threshold();
    }

    // Clears the LCD panel
    lcd.clear();

    // Display text in LCD 
    lcd.print("Humidity (%): ");

    // Invokes the function to set humidity threshold in fahrenheit.
    set_humidity_threshold();

    // Start a thread to print sensor data in LCD
    print_thread.start(callback(&print_queue, &EventQueue::dispatch_forever));

    // Start a thread to check sensor data
    check_thread.start(callback(&check_queue, &EventQueue::dispatch_forever));

    // Calls an print_sensor_data on the queue every second.
    print_queue.call_every(2000ms, &print_sensor_data);
    
    //Start the Watchdog timer.
    watchdog.start(TIMEOUT_MS);

    // Calls an check_sensor_data on the queue every second. 
    check_queue.call_every(2000ms, &check_sensor_data);
    
    // Spins a main loop
    while (true) {
    }
    return 0;
}

/* This function reads sensor and prints temperature in celsius or in fahrenheit
   and humidity in percentage in LCD. The function uses mutex to to synchronize
   the access to its critical section.
*/ 
void print_sensor_data(void){
    
    mutex.lock(); // Wait until a Mutex becomes available. 
    sensor.read(); // Update sensor data
    
    float temp_f = sensor.getFahrenheit(); // Gets temperature in fahrenheit.
    int temp_c = sensor.getCelsius(); // Gets temperature in celsius.

    if(temp_f <=(current_fahrenheit + 2.0) || temp_f >= (current_fahrenheit - 2.0) )
    {
        current_fahrenheit = temp_f; 
    }
    if(temp_c <= (current_celsius + 2.0) || temp_c >= (current_celsius - 2.0) )
    {
        current_celsius = temp_c; 
    }
    
    
    current_humidity = sensor.getHumidity(); // Gets humidity in percent
    
    /* Checks if flag_celsius is true and print temeperature in celsius unit otherwise prints
       prints temperature in fahrenheit unit
    */
    if (flag_celsius) 
    {
      
      to_print = "Temp.: " + to_string(current_celsius) + degree + "C";
      
      // Clears the LCD panel
      lcd.clear();

      // Display text in LCD
      lcd.print(to_print.c_str());
    } 
    else 
    {
      to_print = "Temp.: " + to_string(current_fahrenheit) + degree + "F";
      
      // Clears the LCD panel
      lcd.clear();

      // Display text in LCD
      lcd.print(to_print.c_str());
    }

    // Sets LCD cursor to row 1 and column 0
    lcd.setCursor(0, 1);
    
    to_print = "Humidity: " + to_string(current_humidity) + "%";
    
    // Clears the LCD panel
    lcd.print(to_print.c_str());

    mutex.unlock(); // Unlock a mutex that has been locked by the same thread previously.
    
    Watchdog::get_instance().kick();
}

/* This function checks temperature and humidity with the thresholds and if temperature
   is higher than the temperature threshold or humidity is less than the humidity threshold,
   it calls siren() function to activate the buzzer and turn on the red LED. Otherwise it 
   keeps checking. The function uses mutex to synchronize the access to its critical section.
*/ 
void check_sensor_data(void){

    mutex.lock(); // Wait until a Mutex becomes available.

    // Checks if flag_celsius is true. 
    if (flag_celsius){
        
        /* Checks if current temperature in celsius is greater than temperature threshold or
           current humidity less than humidity threshold and it calls siren() function 
           to activate the buzzer.
        */   
        if (current_celsius > temperature_threshold || current_humidity < humidity_threshold) {
            // 
            siren();
        }
        else{
            // Turn off the buzzer.
            buzzer.write(0.0);
            siren_led = 0;
        }
    }else{

        /* Checks if current temperature in fahrenheit is greater than temperature threshold or
           current humidity less than humidity threshold and it calls siren() function 
           to activate the buzzer.
        */ 
        if (current_fahrenheit > temperature_threshold || current_humidity < humidity_threshold) {
            siren();
        }else {
            // Turn off the buzzer
            buzzer.write(0.0);
            siren_led = 0;
        }
    }

    mutex.unlock(); // Unlock a mutex that has been locked by the same thread previously.
    
    //Refresh the Watchdog timer.
    Watchdog::get_instance().kick();
}

// Handles buzzer sound and red LED
void siren (void){
    siren_led = 1;
    for(int i =400; i<600; i++){
        buzzer.period(1.0/(float)i);
        buzzer.write(0.5);
        wait_us(10000);
    }
    wait_us(2000000);
    
    for(int i =600; i>400; i--){
        buzzer.period(1.0/(float)i);
        buzzer.write(0.5);
        wait_us(10000);
    }

    Watchdog::get_instance().kick();
}

/* This function handles user input from keypad to set humidity threshold */
void set_humidity_threshold(void){
    int count = 0;
    string humidity = ""; 
    humidity_threshold = 0;

    // Clears the LCD panel
    lcd.clear();

    // Display text in LCD
    lcd.print("Humidity (%): ");

    // Sets LCD cursor to row 1 and column 4
    lcd.setCursor(4, 1);

    // Spins a loop to check the user input if it is within the range.
    while (humidity_threshold < 20 || humidity_threshold > 80) {
      
      // Spins a loop and runs until count equals to 2.
      while (count < 2) {

        // Spins a loop.
        while (true) {

          // Invokes the keypad_cycles method to power through the row pins.
          keypad_cycle();

          // Checks if any key is pressed other than A, B, C, D, * and #.
          if (is_key_pressed && key.compare("A") != 0 && key.compare("B") != 0 && key.compare("C") != 0 && key.compare("D") != 0 && key.compare("*") != 0 && key.compare("#") != 0) {
            is_key_pressed = false;
            humidity = humidity + key; // Concatenate strings
            lcd.print(humidity.c_str()); // Display text in LCD.
            key = "";
            break;
          }
        }
        count += 1; // Increment count by 1.
        
        // Clears the LCD panel
        lcd.clear();

        // Display text in LCD
        lcd.print("Humidity (%): ");

        // Sets LCD cursor to row 1 and column 4
        lcd.setCursor(4, 1);

        // Display text in LCD
        lcd.print(humidity.c_str());
      }

      // Converts humidity to string and assign resultant to humidity_threshold
      humidity_threshold = stoi(humidity);

      /* Checks if humidity_threshold outside of the range. If true, then 
         resets the LCD panel.
      */   
      if(humidity_threshold < 20 || humidity_threshold > 80){
          lcd.clear(); // Clears the LCD panel
          lcd.print("Humidity (%): "); // Display text in LCD
          lcd.setCursor(4, 1); // Sets LCD cursor to row 1 and column 4
          humidity = "";
          count = 0;
      }
    }
}

/* This function cycles through the keypad by providing power to one column 
   and turning off other columns.  
*/
void keypad_cycle(void) {

  if (row == 0) 
  { 
    // Sets Pin PD_6 to 1 and PD_3, PD_4 and PD_5 to 0.
    GPIOD->ODR |= 0x40;
    GPIOD->ODR &= ~(0x38);
    wait_us(BOUNCE_DELAY_US);
  } 
  else if (row == 1) 
  {
    // Sets Pin PD_5 to 1 and PD_3, PD_4 and PD_6 to 0.
    GPIOD->ODR |= 0x20;
    GPIOD->ODR &= ~(0x58);
    wait_us(BOUNCE_DELAY_US);
  } 
  else if (row == 2) 
  {
    // Sets Pin PD_4 to 1 and PD_3, PD_5 and PD_6 to 0.
    GPIOD->ODR |= 0x10;
    GPIOD->ODR &= ~(0x68);
    wait_us(BOUNCE_DELAY_US);
  } 
  else if (row == 3) 
  {
    // Sets Pin PD_3 to 1 and PD_4, PD_5 and PD_6 to 0.
    GPIOD->ODR |= 0x8;
    GPIOD->ODR &= ~(0x70);
    wait_us(BOUNCE_DELAY_US);
  }
  row += 1; // Increment row by 1.
  row %= 4; 
}

/* This functions run when interrupt is triggered by column #1 button's rising edge. */
void col1_isr_handler(void)
{
    if (row == 0) {
    is_key_pressed = true;
    key = "1";
    wait_us(BOUNCE_DELAY_US);

  } else if (row == 1) {
    is_key_pressed = true;
    key = "4";
    wait_us(BOUNCE_DELAY_US);

  } else if (row == 2) {
    is_key_pressed = true;
    key = "7";
    wait_us(BOUNCE_DELAY_US);

  } else if (row == 3) {
    is_key_pressed = true;
    if (flag_threshold){
        flag_decimal_point = true;
    }
    key = "*";
    wait_us(BOUNCE_DELAY_US);
  }

}

/* This functions run when interrupt is triggered by column #2 button's rising edge. */
void col2_isr_handler(void)
{

    if (row == 0) {
    is_key_pressed = true;
    key = "2";
    wait_us(BOUNCE_DELAY_US);

  } else if (row == 1) {
    is_key_pressed = true;
    key = "5";
    wait_us(BOUNCE_DELAY_US);

  } else if (row == 2) {
    is_key_pressed = true;
    key = "8";
    wait_us(BOUNCE_DELAY_US);

  } else if (row == 3) {
    is_key_pressed = true;
    key = "0";
    wait_us(BOUNCE_DELAY_US);
  }

}

/* This functions run when interrupt is triggered by column #3 button's rising edge. */
void col3_isr_handler(void)
{

    if (row == 0) {
    is_key_pressed = true;
    key = "3";
    wait_us(BOUNCE_DELAY_US);

  } else if (row == 1) {
    is_key_pressed = true;
    key = "6";
    wait_us(BOUNCE_DELAY_US);

  } else if (row == 2) {
    is_key_pressed = true;
    key = "9";
    wait_us(BOUNCE_DELAY_US);

  } else if (row == 3) {
    is_key_pressed = true;
    key = "#";
    wait_us(BOUNCE_DELAY_US);
  }

}

/* This functions run when interrupt is triggered by column #4 button's rising edge. */
void col4_isr_handler(void)
{

    if (row == 0) {
    is_key_pressed = true;
    key = "A";
    wait_us(BOUNCE_DELAY_US);
  } else if (row == 1) {
    is_key_pressed = true;
    key = "B";
    wait_us(BOUNCE_DELAY_US);
  } else if (row == 2) {
    is_key_pressed = true;
    key = "C";
    wait_us(BOUNCE_DELAY_US);
  } else if (row == 3) {
    is_key_pressed = true;
    key = "D";
    wait_us(BOUNCE_DELAY_US);
  }

}

/* This function handles user input from keypad to set temperature threshold
   in celsius
*/
void set_celsius_threshold(void)
{   
    // Checks if flag_celsius is false
    if(!flag_celsius){
        return;
    }
    int count = 0;
    string temperature = "";
    temperature_threshold = 100;
    
    lcd.clear(); // Clears the LCD panel
    lcd.print("Temp. ("); // Display text in LCD
    lcd.print(degree_sign); // Display text in LCD
    lcd.print("C): "); // Display text in LCD
    lcd.setCursor(4, 1); // Sets LCD cursor to row 1 and column 4

    // Spins a loop and runs till temperature_threshold greater than 50 
    while (temperature_threshold > 50) {
        
        // Spins a loop and runs until count equals to 2.
        while (count<2) {

          //Spins a loop
          while (true) {

            // Invokes the keypad_cycles method to power through the row pins.
            keypad_cycle();

            // Checks if any key is pressed other than A, B, C, D, * and #.
            if (is_key_pressed && key.compare("A") != 0 && key.compare("B") != 0 && key.compare("C") != 0 && key.compare("D") != 0 && key.compare("#") != 0 && key.compare("*") != 0){
              is_key_pressed = false;
              temperature = temperature + key; // Concatenate strings.
              lcd.print(temperature.c_str()); // Display text in LCD.
              key = "";
              break;
            }
          }
          count += 1; // Increment count by 1.

          // Clears the LCD panel
          lcd.clear();

          // Display text in LCD
          lcd.print("Temp. (");
          lcd.print(degree_sign);
          lcd.print("C): ");

          // Sets LCD cursor to row 1 and column 4.
          lcd.setCursor(4, 1);

          // Display text in LCD
          lcd.print(temperature.c_str());
        }

        // Converts temperature to string and assign resultant to temperature_threshold
        temperature_threshold = stoi(temperature);

        /* Checks if humidity_threshold outside of the range. If true, then 
         resets the LCD panel.
        */ 
        if(temperature_threshold>50){
            lcd.clear(); // Clears the LCD panel
            lcd.print("Temp. ("); // Display text in LCD
            lcd.print(degree_sign); // Display text in LCD
            lcd.print("C): "); // Display text in LCD
            lcd.setCursor(4, 1); // Sets LCD cursor to row 1 and column 4
            temperature = "";
            count = 0;
        }
    }
}

/* This function handles user input from keypad to set temperature threshold
   in fahrenheit
*/
void set_fahrenheit_threshold(void)
{
    // Checks if flag_celsius is true
    if(flag_celsius){
        return;
    }
    int count = 0;
    string temperature = "";
    temperature_threshold = 123;

    lcd.clear(); // Clears the LCD panel
    lcd.print("Temp. ("); // Display text in LCD
    lcd.print(degree_sign); // Display text in LCD
    lcd.print("F): "); // Display text in LCD
    lcd.setCursor(4, 1); // Sets LCD cursor to row 1 and column 4

    // Spins a loop and runs till temperature_threshold greater than 122.
    while (temperature_threshold > 122) {
        
        // Spins a loop and runs until count equals to 5.
        while (count<5) {

          //Spins a loop
          while (true) {

            // Invokes the keypad_cycles method to power through the row pins.
            keypad_cycle();

            // Checks if any key is pressed other than A, B, C, D, and #.
            if (is_key_pressed && key.compare("A") != 0 && key.compare("B") != 0 && key.compare("C") != 0 && key.compare("D") != 0 && key.compare("#") != 0) {
              is_key_pressed = false;

              // Checks if * key is pressed. 
              if(key.compare("*") == 0){

                // Checks if flag_decimal_point is true.
                  if (flag_decimal_point){
                    temperature = temperature + "."; // Concatenate strings.
                    flag_decimal_point = false; 
                    flag_threshold = false;
                  count = 3;
                  }
              }else {
                  temperature = temperature + key; // Concatenate strings.
                  count += 1; // Increment count by 1.
              }
              key = "";
              break;
            }
          }

          // Clears the LCD panel
          lcd.clear();

          // Display text in LCD
          lcd.print("Temp. (");
          lcd.print(degree_sign);
          lcd.print("F): ");

          // Sets LCD cursor to row 1 and column 4.
          lcd.setCursor(4, 1);

          // Display text in LCD.
          lcd.print(temperature.c_str());
        }

        // Converts temperature to string and assign resultant to temperature_threshold
        temperature_threshold = stof(temperature);

        if (temperature_threshold>122){
          lcd.clear(); // Clears the LCD panel
          lcd.print("Temp. ("); // Display text in LCD
          lcd.print(degree_sign); // Display text in LCD
          lcd.print("F): "); // Display text in LCD
          lcd.setCursor(4, 1); // Sets LCD cursor to row 1 and column 4
          temperature = "";
          count = 0;
        }
    }
}
