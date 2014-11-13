/* 
(c) Hector Martinez III
    hmartineziii@gmail.com
    
Uses the temp sensor to output to serial LED display
*/

#include <SoftwareSerial.h>
#include <OneWire.h>

// These are the Arduino pins required to create a software serial
//  instance. We'll actually only use the TX pin.
const int softwareTx = 8;
const int softwareRx = 7;

// initializes the temp sensor o the analog output pin 2
int DS18S20_Pin = 2;


//initialize class SoftwareSerial with instance s7s
SoftwareSerial s7s(softwareRx, softwareTx);

//initialize class OneWire temp sensor with instance ds
OneWire ds(DS18S20_Pin);

char tempString[10];  // Will be used with sprintf to create strings

void setup()
{
  //Serial in this case is to the console log
  Serial.begin(9600);
  // Must begin s7s software serial at the correct baud rate.
  //  The default of the s7s is 9600.
  s7s.begin(9600);
  // Clear the display, and then turn on all segments and decimals
  clearDisplay();  // Clears display, resets cursor
  s7s.print("HOLA");  // Displays -HI- on all digits
  setDecimals(0b000000);  // Do not turn on any decimals, colon, apos
  // Flash brightness values at the beginning
  //setBrightness(255);  // Lowest brightness
  //delay(1500);
  //setBrightness(127);  // Medium brightness
  //delay(1500);
  setBrightness(0);  // High brightness
  delay(1500);
  // Clear the display before jumping into loop
  clearDisplay();  
}


void loop()
{
  float temperature = getTemp();
  // Magical sprintf creates a string for us to send to the s7s.
  //  The %4d option creates a 3-digit integer. the %c creates a digit for F or C
  Serial.println(temperature);
  int td_f_temp = (temperature*1.8+32)*10;
  int td_c_temp = temperature*10; //not printed
  sprintf(tempString, "%3d%c", td_f_temp,'F');
  Serial.println(td_f_temp);
  Serial.println(tempString);
  s7s.print(tempString); // This will output the tempString to the S7S
  setDecimals(0b00100010);  // Sets digit 2 decimal on and apostrophe
  delay(1000);  // This will make the display update at 100Hz.
}



//-----------------------------------------------------//
//  Functions for 7 series LED display                 //
// Send the clear display command (0x76)
//  This will clear the display and reset the cursor
void clearDisplay()
{
  s7s.write(0x76);  // Clear display command
}

// Set the displays brightness. Should receive byte with the value
//  to set the brightness to
//  dimmest------------->brightest
//     0--------127--------255
void setBrightness(byte value)
{
  s7s.write(0x7A);  // Set brightness command byte
  s7s.write(value);  // brightness data byte
}

// Turn on any, none, or all of the decimals.
//  The six lowest bits in the decimals parameter sets a decimal 
//  (or colon, or apostrophe) on or off. A 1 indicates on, 0 off.
//  [MSB] (X)(X)(Apos)(Colon)(Digit 4)(Digit 3)(Digit2)(Digit1)
void setDecimals(byte decimals)
{
  s7s.write(0x77);
  s7s.write(decimals);
}


//-------------------------------------------------------------//
//  Temp sensor function for getting temperature in Celsius    //
float getTemp(){
  //returns the temperature from one DS18S20 in DEG Celsius
  byte data[12];
  byte addr[8];
  if ( !ds.search(addr)) {
      //no more sensors on chain, reset search
      ds.reset_search();
      return -1000;
  }
  if ( OneWire::crc8( addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return -1000;
  }
  if ( addr[0] != 0x10 && addr[0] != 0x28) {
      Serial.print("Device is not recognized");
      return -1000;
  }
  ds.reset();
  ds.select(addr);
  ds.write(0x44,1); // start conversion, with parasite power on at the end
  byte present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE); // Read Scratchpad
  for (int i = 0; i < 9; i++) { // we need 9 bytes
    data[i] = ds.read();
  }
  ds.reset_search();
  byte MSB = data[1];
  byte LSB = data[0];
  float tempRead = ((MSB << 8) | LSB); //using two's compliment
  float TemperatureSum = tempRead / 16;
  return TemperatureSum; 
}
