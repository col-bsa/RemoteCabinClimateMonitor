/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "Particle.h"
#line 1 "/home/zach/GitHub/RemoteCabinClimateMonitor/Software/RCCM/src/RCCM.ino"
/*
 * Project RCCM
 * Description:
 * Author:
 * Date:
 */

//
#include "DataLog.h"

//
void setup();
void loop();
#line 12 "/home/zach/GitHub/RemoteCabinClimateMonitor/Software/RCCM/src/RCCM.ino"
DataLog dataLog(100);


//
const pin_t MY_LED = D7;

// 
void setup() {
  //
  pinMode(MY_LED, OUTPUT);


}

// 
void loop() {
  // 
	digitalWrite(MY_LED, HIGH);
	delay(1s);
	digitalWrite(MY_LED, LOW);
	delay(1s);

}