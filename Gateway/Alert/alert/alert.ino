#include "C:/Users/Antonivs/Desktop/Arbeit/Undergrad/Senior_Design/repo/RadioWork/Shared/SharedDefinitions.h"

Garden_Status gardenStatus;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while (!Serial);

//  checkAlerts(DAILY_REPORT,0);
//  checkAlerts(BAD_FLOW_RATE,1);
//  checkAlerts(MESH_DOWN);
//  checkAlerts(GATEWAY_RESET);
//  checkAlerts(BAD_VOLTAGE_STATE);
}

void loop() {
  // put your main code here, to run repeatedly:

}

void checkAlerts(uint8_t opcode, uint8_t nodeNum) {
  String myAlert;
  // daily report
  if (opcode == 0) {
    myAlert = "0" + (String) opcode + "%" + (String) gardenStatus.percentAwake
    + (String) gardenStatus.percentMeshUptime + (String) gardenStatus.percent3GUptime;

    // use for loop to create information about each registered node in {}
    for(uint8_t node = 1; node<=16; node++) {
      String myNodeInfo;
      // if registered
      if(gardenStatus.nodeStatusPtrs[node] != NULL) {
        myNodeInfo += "{";
        myNodeInfo += "%";
        myNodeInfo += (String) node;
        myNodeInfo += "%";
        myNodeInfo += (String) gardenStatus.nodeStatusPtrs[node]->meshState;
        myNodeInfo += "%";
        myNodeInfo += (String) gardenStatus.nodeStatusPtrs[node]->percentAwake;
        myNodeInfo += "%";
        myNodeInfo += (String) gardenStatus.nodeStatusPtrs[node]->accumulatedFlow;
        myNodeInfo += "%";
        myNodeInfo += (String) gardenStatus.nodeStatusPtrs[node]->maxedOutFlowMeter;
      
        for(uint8_t valve = 1; valve <= 4; valve++) {
          String myValveInfo;
          if(gardenStatus.nodeStatusPtrs[node]->valveStates[valve].isConnected) {
            myValveInfo += "%";
            myValveInfo += "[";
            myValveInfo += "%";
            myValveInfo += (String) valve;
            myValveInfo += "%";
            float myValveTime = gardenStatus.nodeStatusPtrs[node]->valveStates[valve].timeSpentWatering;
            myValveTime = myValveTime / 60;   // convert to minutes
            myValveInfo += (String) myValveTime;
            myValveInfo += "]";
            myNodeInfo += myValveInfo;
          }
        }
        myNodeInfo += "%";
        myNodeInfo += "}";
        myAlert += myNodeInfo;
      }
    }
   
  // bad valve state
  } else if (opcode == 1) {
    myAlert = "0" + (String) opcode;
    myAlert += "%";
    myAlert += nodeNum;
    myAlert += "%";
    myAlert += (String) gardenStatus.nodeStatusPtrs[nodeNum]->flowState;
    myAlert += "%";
    myAlert += (String) gardenStatus.nodeStatusPtrs[nodeNum]->currentFlowRate;
    
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
