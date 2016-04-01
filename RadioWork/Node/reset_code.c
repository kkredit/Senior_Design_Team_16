
/* Add the following lines where you define pin assignments */
#define RESET_GND A0
#define RESET_PIN A1



/* Add the following functions (comments to be added) */
void hardReset(){
  pinMode(RESET_PIN, OUTPUT);
  digitalWrite(RESET_PIN, HIGH);
  delay(1000);
  // arduino resets here
}

void refreshReset(){
  pinMode(RESET_PIN, OUTPUT);  
  digitalWrite(RESET_PIN, LOW);
  delay(50); // 95% charged: 3*tau = 3*RC = 3*200*47*10^-6 = 28 ms
  //digitalWrite(RESETPIN, LOW); // to turn off internal pullup
  pinMode(RESET_PIN, INPUT);
}



/* Add the following lines where you initialize pins (preferably near the beginning of setup()) */
  // RESET_PIN -- set to low immediately to discharge capacitor
  pinMode(RESET_PIN, OUTPUT);
  digitalWrite(RESET_PIN, LOW);
  
  
  
/* Add the following lines at the end of setup() */
  // enable self-resetting ability
  delay(50);  // allow capacitor to discharge if was previously charged before enabling autoreset again
              // 95% = 3*tau = 3*RC = 3*200*100*10^-6 = 60ms -- but never gets fully charged, and has
              //    been dicharging during previous setup, so 50ms is sufficient
  pinMode(RESET_PIN, INPUT);
  pinMode(RESET_GND, OUTPUT);
  digitalWrite(RESET_GND, LOW);
  

  
/* Add the following lines in loop() */
  // refresh the reset
  refreshReset();