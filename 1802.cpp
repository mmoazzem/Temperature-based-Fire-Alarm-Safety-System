#include "1802.h"
#include "mbed.h"

// modified from https://os.mbed.com/users/Yar/code/CSE321_LCD_for_Nucleo/
// modified from:
// https://os.mbed.com/users/cmatz3/code/Grove_LCD_RGB_Backlight_HelloWorld/

// constructor
CSE321_LCD::CSE321_LCD(unsigned char lcd_cols, unsigned char lcd_rows,
                       unsigned char charsize, PinName sda, PinName scl)
    : i2c(sda, scl) {
  _addr = LCD_ADDRESS_1802;
  _cols = lcd_cols;
  _rows = lcd_rows;
  _charsize = charsize;
  _backlightval = LCD_BACKLIGHT;
}

void CSE321_LCD::begin() {

  // Initialize displayfunction parameter for setting up LCD display
  _displayfunction |= LCD_2LINE;
  _displayfunction |= LCD_5x8DOTS;

  // Wait for more than 30 ms after power rises above 4.5V per the data sheet
  thread_sleep_for(50);

  // Send first function set command. Wait longer that 39 us per the data
  // sheet
  sendCommand(LCD_FUNCTIONSET | _displayfunction);
  thread_sleep_for(45);

  // turn the display on
  displayON();

  // clear the display
  clear();

  // Initialize backlight
  setReg(0, 0);
  setReg(1, 0);
  setReg(0x08, 0xAA);

  //   }
}

void CSE321_LCD::clear() {
  sendCommand(LCD_CLEARDISPLAY);
  wait_us(2000);
}

void CSE321_LCD::sendCommand(char value) {
  char data[2] = {0x80, value};
  i2c.write(_addr, data, 2);
}

// set color thing for seeed
void CSE321_LCD::setRGB(char r, char g, char b) {

  this->setReg(RED_REG, r);
  this->setReg(GREEN_REG, g);
  this->setReg(BLUE_REG, b);
}

void CSE321_LCD::displayON() {
  _displaycontrol |= LCD_DISPLAYON;
  this->sendCommand(LCD_DISPLAYCONTROL | _displaycontrol);
}

void CSE321_LCD::setReg(char addr, char val) {
  char data[2];
  data[0] = addr;
  data[1] = val;
  i2c.write(RGB_ADDRESS, data, 2);
}

void CSE321_LCD::setCursor(unsigned char col, unsigned char row) {
  // change the cordinate of where the next charecter will be put
  if (row == 0) {
    col = col | 0x80;
  } else {
    col = col | 0xc0;
  }

  char data[2];
  data[0] = 0x80;
  data[1] = col;
  i2c.write(_addr, data, 2);
}

int CSE321_LCD::print(const char *text) { // output a string to the LCD
  char data[2];
  data[0] = 0x40;
  while (*text) {
    data[1] = *text;
    i2c.write(_addr, data, 2);
    text++;
  }
  return 0;
}