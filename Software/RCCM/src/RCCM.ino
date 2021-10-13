/*
 * Project RCCM
 * Description:
 * Author:
 * Date:
 */

//
#include "DataLog.h"

//
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