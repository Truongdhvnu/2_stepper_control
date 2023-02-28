//void test1() {
//  stepperY.moveTo(-400);
//  stepperY.setSpeed(-400);
//  stepperY.runSpeed();
//}
//
//void test2() {
//  liftPenDown();
//  moveToPoint(4800, 0);
//  delay(1000);
//  moveToPoint(0, 0);
//  delay(1000);
//  moveToPoint(4800,320);
//  delay(1000);
//  moveToPoint(0, 0);
//  delay(1000);
//  moveToPoint(4800,150);
//  delay(1000);
//  moveToPoint(0, 0);
//  delay(1000);
//  moveToPoint(4800,96);
//  delay(1000);
//  moveToPoint(0, 0);
//  delay(1000);
//  moveToPoint(4800,48);
//  delay(1000);
//  moveToPoint(0, 0);
//  delay(1000);
//}
//
//void test3() {
//  stepperX.moveTo(4000);
//  stepperX.setSpeed(400);
//  while(stepperX.distanceToGo() != 0) {
//    stepperX.runSpeedToPosition();
//  }
//  delay(1000);
//  stepperX.moveTo(0);
//  stepperX.setSpeed(-902.5);
//  while(stepperX.distanceToGo() != 0) {
//    stepperX.runSpeedToPosition();
//  }
//  delay(1000);
//}
//
//void test4() {
//  stepperY.moveTo(4000);
//  stepperX.moveTo(4000);
//  stepperY.setSpeed(300);
//  stepperX.setSpeed(300);
//  while(stepperY.distanceToGo() != 0 || stepperX.distanceToGo() != 0) {
//          stepperX.runSpeed();
//          stepperY.runSpeed();
//  }
//  delay(1000);
//  
//  stepperY.moveTo(0);
//  stepperX.moveTo(0);
//  stepperY.setSpeed(-300);
//  stepperX.setSpeed(-300);
//  while(stepperY.distanceToGo() != 0 || stepperX.distanceToGo() != 0) {
//    stepperX.runSpeed();
//    stepperY.runSpeed();
//  }
//  delay(1000);
//}
