#include <system_configuration.h>
#include <unwind-cxx.h>
#include <utility.h>

// 3G Modem
#define NL_SW_EVDO_V      // Verizon 3G Modem
#define ThreeG  12        // I/O pin for 3G Modem
#define Modem_Serial Serial1  // TX1 & RX1 pins, which are connected to 3G Modem

// initialize a few variables
int incomingByte = 0;
boolean connectionGood = false;
String currentString = "";
int ErrorCount = 0;

 void setup() {
  // 3G Modem Testing
  Serial.begin(9600);
  while (!Serial) ; // wait for serial debug port to connect to PC
  for (int q = 5; q > 0; q--)
  { 
    Serial.println(q, DEC);
    delay(250);
  }

  // Start cellular modem
  Serial.println("Starting Cellular Modem");

  //Set I/O pin 12 to low.
  digitalWrite(ThreeG, LOW);
  
  // Configure I/O pin 12 as output
  pinMode(ThreeG, OUTPUT);
  
  // Drive I/O pin 12 low
  digitalWrite(ThreeG, LOW);

  delay(1100); // modem requires >1s pulse

  // Return I/O pin 12 to input/hi-Z state  
  pinMode(ThreeG, INPUT);

  // Initialize serial port to communicate with modem
  Modem_Serial.println("Initializing modem COM port");
  Modem_Serial.begin(115200);
  while (!Modem_Serial) ;

  // Soft reset of modem
  Serial.println("Reseting modem");
  Modem_Serial.println("ATZ");
  delay(500);
  while(PrintModemResponse() > 0);

  // send command to modem to get firmware version
  Modem_Serial.println("AT+CGMR");
  delay(10);
  while(PrintModemResponse() > 0);
  // Modem_Serial.println("AT#SKTD=0,13,'www.google.com',0");

  // Check for network connection
  Serial.println("Waiting for network connection...");
  connectionGood = false; 
  while(!connectionGood){
    // connect
    Modem_Serial.println("AT+CREG?");
    delay(500);
    while(Modem_Serial.available() > 0) {
      //read incoming byte from modem
      incomingByte=Modem_Serial.read();
      //write byte out to debug serial over USB
      Serial.write(incomingByte);
      // add current byte to the string we are building
      currentString += char(incomingByte);
      // check currentString to see if network status is "0,1" or "0,5" which means we are connected
      if((currentString.substring(currentString.length()-3, currentString.length()) == "0,1") || 
         (currentString.substring(0, currentString.length()) == "0,5")) {
        connectionGood = true;
        Serial.println(); 
        Serial.println("Connection successful!");  
      }
    }
  }
  // Modem_Serial.println("AT#SS"); // report socket status
  //Modem_Serial.println("AT#SCFG=?");
  Modem_Serial.println("AT#SGACT=1,1");
  delay(5000);
  while(1) {
    incomingByte = Modem_Serial.read();
    if (incomingByte != -1) {
      Serial.write(incomingByte);
        // delay(2000);
    } else {
      // Modem_Serial.println("AT#SGACT=1,0");
      break;
    }
  }
}

void loop() {    
  incomingByte = Modem_Serial.read();
  if (incomingByte != -1) {
    Serial.write(incomingByte);  
  } else {
    Modem_Serial.println("AT#SGACT=1,0");
    Serial.println("Modem test done!");
    while(1); 
  }
} 

int PrintModemResponse()
{
  while(Modem_Serial.available() > 0) 
  {
    //read incoming byte from modem and write byte out to debug serial over USB
    Serial.write(Modem_Serial.read());
  } 
  Serial.println();
  //return number of characters in modem response buffer -- should be zero, but some may have come in since last test
  return Modem_Serial.available();
}
