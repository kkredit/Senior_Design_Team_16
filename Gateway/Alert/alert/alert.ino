#include "C:/Users/Antonivs/Desktop/Arbeit/Undergrad/Senior_Design/repo/RadioWork/Shared/SharedDefinitions.h"

Garden_Status gardenStatus;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while (!Serial);

  checkAlerts(DAILY_REPORT);
  checkAlerts(BAD_VALVE_STATE);
  checkAlerts(MESH_DOWN);
  checkAlerts(GATEWAY_RESET);
  checkAlerts(BAD_VOLTAGE_STATE);
}

void loop() {
  // put your main code here, to run repeatedly:

}

void checkAlerts(uint8_t opcode) {
  String myAlert;
  // daily report
  if (opcode == 0) {
    myAlert = "0" + (String) opcode + "%";
  // bad valve state
  } else if (opcode == 1) {
    myAlert = "0" + (String) opcode + "%";
  // mesh down
  } else if (opcode == 2) {
    myAlert = "0" + (String) opcode;
  // gateway self-reset
  } else if (opcode == 3) {
    myAlert = "0" + (String) opcode;
  // bad voltage state
  } else if (opcode == 4) {
    myAlert = "0" + (String) opcode + "%";
  }

  // print out alert message to serial port
  Serial.println(myAlert);
  
}
