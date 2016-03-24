/***************************************************************************************************************
 *  SMS send demo version 1.0                                      
 *  This sketch is an example that sends SMS messages using the     
 *  Telit CE910-DUAL, DE910-DUAL, and LE910-SVG modem.              
 *								    
 *  Circuit:						            
 *  * Arduino Leonardo					            
 *  * Skywire development kit: NL-SWDK and NL-SW-1xRTT, 	    
 *    NL-SW-EVD0, or NL-SW-LTE-TSVG			            
 *								    
 *  Created 9/27/2013					            
 *  by Kurt Larson // NimbeLink.com				    
 *  
 *  Modified 07/13/2015
 *  by Kyle Rodgers // NimbeLink.com
 *								    								    
 *  This example is in the public domain.			    
 *  
 *  CHANGELOG
 *  07/07/2015
 *  - Added "#define"s to allow the user to select which modem
 *  they are using, and have the program respond accordingly
 *  - Modified code to work with more modems
 *  - Tested code with CE910, DE910, LE910-SVG   
 *  07/10/2015
 *  - Removed LTE modems from SIMDET command
 *  - Added GPRS modem to SIMDET command
 *  
 *  GENERAL INSTRUCTIONS:
 *  1. Uncomment the Skywire Modem that you are using
 *  2. Change DESTINATION_PHONE_NUMBER to the phone number you would
 *      like to send the text message to
 *  3. Change SMS_MESSAGE to your desired text message
 *  4. Compile, upload to your device, and open the serial terminal
 *  
 *  Copyright (C) 2015 nimbelink.com, MIT License
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 *  and associated documentation files (the "Software"), to deal in the Software without restriction,
 *  including without limitation the rights to use, copy, modify, merge, publish, distribute,
 *  sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all copies or
 *  substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 *  BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 *  DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 ***************************************************************************************************************/

/* 
 *  Define modem model
 *  Uncomment only the modem that you are using. 
 *  Make sure only one modem is uncommented! 
 */
//#define NL_SW_1xRTT_V     // Verizon 2G Modem - CE910-DUAL
//#define NL_SW_1xRTT_S     // Sprint 2G Modem - CE910-DUAL
//#define NL_SW_1xRTT_A     // Aeris 2G Modem - CE910-DUAL
//#define NL_SW_GPRS        // AT&T/T-Mobile 2G Modem
//#define NL_SW_EVDO_V	    // Verizon 3G Modem
//#define NL_SW_EVDO_A	    // Aeris 3G Modem
//#define NL_SW_HSPAP	      // AT&T/T-Mobile 3G Modem
//#define NL_SW_HSPAPG	    // AT&T/T-Mobile 3G Modem w/ GPS
//#define NL_SW_HSPAPE	    // GSM 3G Modem, EU
//#define NL_SW_LTE_TSVG    // Verizon 4G LTE Modem
//#define NL_SW_LTE_TNAG    // AT&T/T-Mobile 4G LTE Modem
//#define NL_SW_LTE_TEUG    // GSM 4G LTE Modem, EU	

/*	
 *  Assign destination phone number
 *  Destination phone number of 1-555-444-3333 should be formated like: 15554443333
 */
 /* -- ENTER DESTINATION PHONE NUMBER HERE -- */
#define DESTINATION_PHONE_NUMBER "15554443333"

// SMS message you want to send, limited to 160 characters
String SMS_MESSAGE = "Sent from Arduino with Skywire embedded cellular modem.";

// initialize a few variables
int incomingByte = 0;
boolean connectionGood = false;
String currentString = "";

// code that initializes the serial ports and modem, waits for valid network connection
void setup() 
{
  // initialize serial debug communication with PC over USB connection
  Serial.begin(9600);
  while (!Serial) ; // wait for serial debug port to connect to PC
  for (int q = 5; q > 0; q--)
  { 
    Serial.println(q, DEC);
    delay(250);
  }
  Serial.println("SMS Send Example");

  // Start cellular modem
  Serial.println("Starting Cellular Modem");
  
  /*
   *	Arduino I/O pin 12 is connected to modem ON_OFF signal.
   *	ON_OFF has internal 200k pullup resister and needs to be driven low 
   *	by external signal for >1s to startup.
   *	Arduino defaults to I/O pin as input with no pullup on reset.
   */
    //Set I/O pin 12 to low.
    digitalWrite(12, LOW);
  
    // Configure I/O pin 12 as output
    pinMode(12, OUTPUT);
  
    // Drive I/O pin 12 low
    digitalWrite(12, LOW);
    
  #if defined NL_SW_LTE_TSVG || defined NL_SW_LTE_TNAG || defined NL_SW_LTE_TEUG
    delay(5100); // modem requires >5s pulse
  #else
    delay(1100); // modem requires >1s pulse
  #endif

    // Return I/O pin 12 to input/hi-Z state  
    pinMode(12, INPUT);
    
  #if defined NL_SW_LTE_TSVG || defined NL_SW_LTE_TNAG || defined NL_SW_LTE_TEUG
    // Wait > 15 seconds for initialization
    delay(15100);
  #endif
      
  // Initialize serial port to communicate with modem
  Serial.println("Initializing modem COM port");
  Serial1.begin(115200);
  while (!Serial1) ;

  // Soft reset of modem
  Serial.println("Reseting modem");
  Serial1.println("ATZ");
  delay(500);
  while(PrintModemResponse() > 0);

  //  SIM-based Skywire-specific setup     
  #if defined NL_SW_GPRS || NL_SW_HSPAP || defined NL_SW_HSPAPG || defined NL_SW_HSPAPE
    // activate SIM detect
    Serial.println("Activating SIM card detect");
    Serial1.println("AT#SIMDET=1");
    delay(500);
    while(PrintModemResponse() > 0);
  #endif
  
  // send command to modem to get firmware version
  Serial1.println("AT+CGMR");
  delay(10);
  while(PrintModemResponse() > 0);

  // Check for network connection
  Serial.println("Waiting for network connection");
  connectionGood = false; 
  while(!connectionGood)
  {
    // send command to modem to get network status
    #if defined NL_SW_1xRTT_A || defined NL_SW_1xRTT_S || defined NL_SW_1xRTT_V || defined NL_SW_EVDO_A || defined NL_SW_EVDO_V
      Serial1.println("AT+CREG?");
    #else
      Serial1.println("AT+CGREG?");
    #endif
    currentString = "";
    delay(500);
  
    // Read serial port buffer for UART connected to modem and print that message back out to debug serial over USB
    while(Serial1.available() >0) 
    {
      //read incoming byte from modem
      incomingByte=Serial1.read();
      //write byte out to debug serial over USB
      Serial.write(incomingByte);
  
      // add current byte to the string we are building
      currentString += char(incomingByte);
  
      // check currentString to see if network status is "0,1" or "0,5" which means we are connected
      if((currentString.substring(currentString.length()-3, currentString.length()) == "0,1") || 
         (currentString.substring(0, currentString.length()) == "0,5"))
      {
        connectionGood = true;
        Serial.println(); 
        Serial.println("Connection successful!");  
      }
    }
  }
  
  // get modem response, if any
  while(PrintModemResponse() > 0);
}

// main loop that sends the SMS message
void loop() 
{
  // put modem into text mode
  Serial1.println("AT+CMGF=1");
  delay(250);
  while(PrintModemResponse() > 0);
 
  // assemble SMS message and send
  Serial1.print("AT+CMGS=\"+");
  Serial1.print(DESTINATION_PHONE_NUMBER);
  Serial1.print("\"\r");
  delay(250);
  while(PrintModemResponse() > 0);
  
  // check SMS message length and trim to 160bytes if necessary
  if (SMS_MESSAGE.length() > 160)
  {
    Serial.println("SMS too long, cropping to 160 bytes");
    SMS_MESSAGE = SMS_MESSAGE.substring(0, 160);
  }
  Serial1.print(SMS_MESSAGE);
  Serial1.write(26); // special "CTRL-Z" character
  Serial1.write("\r");
  delay(1000);
  while(PrintModemResponse() > 0);
  
  // let user know message has been sent
  Serial.println("SMS sent");

  // wait for Arduino reset
  while(1);
}

int PrintModemResponse()
{
  while(Serial1.available() > 0) 
  {
    //read incoming byte from modem and write byte out to debug serial over USB
    Serial.write(Serial1.read());
  } 
  Serial.println();
  //return number of characters in modem response buffer -- should be zero, but some may have come in since last test
  return Serial1.available();
}
