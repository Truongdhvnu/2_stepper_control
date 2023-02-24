#include <AccelStepper.h>
#include <Servo.h>     
Servo myservo;

#define liftPenUp() {myservo.write(80); delay(100);}
#define liftPenDown() {myservo.write(40); delay(200);}
  
int dirX = 4;
int stepX = 5;
int enaX = 6;
AccelStepper stepperX(1, stepX, dirX, enaX);

int dirY = 7;
int stepY = 8;
int enaY = 9;
AccelStepper stepperY(1, stepY, dirY, enaY);

// ratio: toc do toi da chay = maxSpeed() * ratio
#define ratio 0.95

// time to settling down the system after polling_to_paint before navigation
#define hold 500

#define step_length 0.001

#define nstep_gather 20

void setup() {
  Serial.begin(9600);
  myservo.attach(12);
  liftPenUp();
  stepperX.setMaxSpeed(950);
  stepperY.setMaxSpeed(950);
  pinMode(enaX, OUTPUT);
  pinMode(enaY, OUTPUT);
  digitalWrite(enaY, HIGH);
  digitalWrite(enaX, HIGH);
  stepperX.setCurrentPosition(0);
  stepperY.setCurrentPosition(0);
  pinMode(13, OUTPUT);
  delay(5000);
  digitalWrite(13, HIGH);
  digitalWrite(enaY, LOW);
  digitalWrite(enaX, LOW);
  delay(1000);
}
/*
 * (a, b) : point to move the pen to.
 * move the pen from current position to point (a, b) in a straight line
 * do not implement liftPenUp/liftPenDown function
 */
void moveToPoint(int a, int b) {
  int disX = a - stepperX.currentPosition();
  int disY = b - stepperY.currentPosition();
  if(a < 0 || b < 0 || (disX == 0 && disY == 0)) {
    //
  } else {
     int dauX = 1;
     int dauY = 1;
     if(disX < 0) {
        disX = -1 * disX;
        dauX = -1;
     }
     if(disY < 0) {
        disY = -1 * disY;
        dauY = -1;
     }
     stepperX.moveTo(a);
     stepperY.moveTo(b);   
     if(disX > disY) {
        stepperX.setSpeed(dauX * stepperX.maxSpeed() * ratio);
        stepperY.setSpeed(dauY * stepperX.maxSpeed() * ratio * disY / disX);
     } else {
        stepperY.setSpeed(dauY * stepperY.maxSpeed() * ratio);
        stepperX.setSpeed(dauX * stepperY.maxSpeed() * ratio * disX / disY);
     }
     
     while(stepperX.distanceToGo() != 0 || stepperY.distanceToGo() != 0) {
        if(stepperX.distanceToGo() != 0) {
          stepperX.runSpeed();
        }
        if(stepperY.distanceToGo() != 0) {
          stepperY.runSpeed();
        }
     }
     delay(hold);
  }
}

/*
 * (a, b) : start point
 *  (c, d) : end point
 *  creat a line from (a, b) to (c, d).
 */
void line(int a, int b, int c, int d) {
  moveToPoint(a, b);
  liftPenDown();
  moveToPoint(c, d);
  liftPenUp();
}

/*
 * (a - s, a + s), ( b, b + 2 * s) must be in the work area
 * (a, b) la toa do dinh tren cung cua ngoi sao
 * s la size cua ngoi sao: chieu ngang cua ngoi sao la 2 * s * step_length chieu cao cung la 2*s*step_length
 * step_length = 0.8/200/4 = 1e-3cm (dung vitme T8 - quarter step)
 */
void star(int a, int b, int s) {
  moveToPoint(a, b);
  liftPenDown();
  moveToPoint(a - s, b + 2 * s);
  moveToPoint(a + s, round(b + 0.76 * s));
  moveToPoint(a - s, round(b + 0.76 * s));
  moveToPoint(a + s, b + 2 * s);
  moveToPoint(a, b);
  liftPenUp();
}

//void circle(int a, int b, int r) {
//  r = (r/nstep_gather) * nstep_gather;
//  int k = r / 2 / nstep_gather;
//  r = r * 0.001;
//  int i = nstep_gather;
//  float dx = 0;
//  float dy = 0;
//  while(i <= k * nstep_gather) {
//    dx += sqrt((i-1) * step_length * (2*r-(i-1)*step_length)) - sqrt(i * step_length * (2*r-i*step_length));
//    dy += -(sqrt(r*r - (i-1)*(i-1)*step_length*step_length) - sqrt(r*r - i*i*step_length*step_length));
//    if(dy/dx > 0.01) {
//      break;
//    } else {
//      i += nstep_gather;
//    }
//  }
//  int flag = k - i/nstep_gather + 1;
//  float temp_v[k - i/nstep_gather + 1];
//  float temp_p[k - i/nstep_gather + 1];
//  temp_p[0] = i;
//  temp_v[0] = dy/dx;
//  int j;
//  int next = 1;
//  for(j = i + nstep_gather; j <= k * nstep_gather; j += nstep_gather) {
//    dx = sqrt((j-1) * step_length * (2*r-(j-1)*step_length)) - sqrt(j * step_length * (2*r-j*step_length));
//    dy = -(sqrt(r*r - (j-1)*(j-1)*step_length*step_length) - sqrt(r*r - j*j*step_length*step_length));
//    temp_v[next] = dy/dx;
//    temp_p[next++] = j;
//  }
//  for (i = 0; i < flag; i++) {
//    Serial.print(temp_v[i]);
//    Serial.print("---");
//    Serial.println(temp_p[i]);
//  }
//  moveToPoint(a, b);
//}

void circle(int a, int b, int r) {
  int end_point = r/3.414213562, start = 0, k = 0;
  float dv0 = 0;
  while(dv0 < 0.02) {
    k += nstep_gather;
    dv0 = k / sqrt(1.0 * k * (2*r - k));
  }
  start = k;
  int arr_size = (int)((end_point - start) / nstep_gather + 2);
  float dv[arr_size];
  dv[0] = dv0;
  int i = 0;
  while(k < end_point) {
    k += nstep_gather;
    i++;
    dv[i] = nstep_gather/(sqrt(1.0 * k * (2*r - k)) - sqrt(1.0 * (k-nstep_gather) * (2*r - k + nstep_gather)));   
  }
  k = k - nstep_gather;
  dv[arr_size - 1] = (end_point - k)/(sqrt(1.0 * end_point * (2*r - end_point)) - sqrt(1.0 * k * (2*r - k)));
//    dv[arr_size - 1] = 1;


  for(i = 0; i < arr_size; i++) {
    Serial.println(dv[i], 5);
  }
  Serial.println(start);
  Serial.println(end_point);
  moveToPoint(a, b);
  liftPenDown();

  int sign_vx = -1;
  int sign_vy = 1;
  int target_pos = 0;
  // ve tu A --> B
  stepperY.setCurrentPosition(0);
  stepperX.setCurrentPosition(0);
  sign_vx = -1;
  sign_vy = 1;
  stepperX.setSpeed(stepperX.maxSpeed() * ratio * sign_vx);
  target_pos = (start - nstep_gather) * sign_vy;
  for(i = 0; i < arr_size - 1; i++) {
    target_pos += nstep_gather * sign_vy;
    stepperY.moveTo(target_pos);
    stepperY.setSpeed(dv[i] * sign_vy * stepperX.maxSpeed() * ratio);
    while(stepperY.distanceToGo() != 0) {
      stepperY.runSpeed();
      stepperX.runSpeed();
    }
  }
  stepperY.moveTo(end_point * sign_vy);
  stepperY.setSpeed(dv[i] * sign_vy * stepperX.maxSpeed() * ratio);
  while(stepperY.distanceToGo() != 0) {
      stepperX.runSpeed();
      stepperY.runSpeed();
    }
  delay(50);
  
  // Ve tu B --> C
  stepperY.setCurrentPosition(0);
  stepperX.setCurrentPosition(0);
  sign_vx = -1;
  sign_vy = 1;
  stepperY.setSpeed(sign_vy * stepperY.maxSpeed() * ratio);
  target_pos = (end_point % nstep_gather - nstep_gather )* sign_vx;
  for(i = arr_size - 1; i >= 0; i--) {
    if( i == 0 ) {
      target_pos = end_point * sign_vx;
    } else {
      target_pos += nstep_gather * sign_vx;
    }
    stepperX.moveTo(target_pos);
    stepperX.setSpeed(dv[i] * sign_vx * stepperY.maxSpeed() * ratio);
    while(stepperX.distanceToGo() != 0) {
      stepperX.runSpeed();
      stepperY.runSpeed();
    }
  }
  delay(50);
  
  // ve tu C --> D
  stepperY.setCurrentPosition(0);
  stepperX.setCurrentPosition(0);
  sign_vx = 1;
  sign_vy = 1;
  stepperY.setSpeed(sign_vy * stepperY.maxSpeed() * ratio);
  target_pos = (start - nstep_gather) * sign_vx;
  for(i = 0; i < arr_size - 1; i++) {
    target_pos += nstep_gather * sign_vx;
    stepperX.moveTo(target_pos);
    stepperX.setSpeed(dv[i] * sign_vx * stepperY.maxSpeed() * ratio);
    while(stepperX.distanceToGo() != 0) {
      stepperX.runSpeed();
      stepperY.runSpeed();
    }
  }
  stepperX.moveTo(end_point * sign_vx);
  stepperX.setSpeed(dv[i] * sign_vx * stepperY.maxSpeed() * ratio);
  while(stepperX.distanceToGo() != 0) {
      stepperY.runSpeed();
      stepperX.runSpeed();
  }
  delay(50);

  // Ve tu D --> E
  stepperY.setCurrentPosition(0);
  stepperX.setCurrentPosition(0);
  sign_vx = 1;
  sign_vy = 1;
  target_pos = (end_point % nstep_gather - nstep_gather )* sign_vy;
  stepperX.setSpeed(stepperX.maxSpeed() * ratio * sign_vx);
  for(i = arr_size - 1; i >= 0; i--) {
    if( i == 0 ) {
      target_pos = end_point * sign_vy;
    } else {
      target_pos += nstep_gather * sign_vy;
    }
    stepperY.moveTo(target_pos);
    stepperY.setSpeed(dv[i] * sign_vy * stepperX.maxSpeed() * ratio);
    while(stepperY.distanceToGo() != 0) {
      stepperX.runSpeed();
      stepperY.runSpeed();
    }
  }
  delay(50);
  
  // Ve tu E --> F
  stepperY.setCurrentPosition(0);
  stepperX.setCurrentPosition(0);
  sign_vx = 1;
  sign_vy = -1;
  target_pos = (start - nstep_gather) * sign_vy;
  stepperX.setSpeed(stepperX.maxSpeed() * ratio * sign_vx);
  for(i = 0; i < arr_size - 1; i++) {
    target_pos += nstep_gather * sign_vy;
    stepperY.moveTo(target_pos);
    stepperY.setSpeed(dv[i] * sign_vy * stepperX.maxSpeed() * ratio);
    while(stepperY.distanceToGo() != 0) {
      stepperY.runSpeed();
      stepperX.runSpeed();
    }
  }
  stepperY.moveTo(end_point * sign_vy);
  stepperY.setSpeed(dv[i] * sign_vy * stepperX.maxSpeed() * ratio);
  while(stepperY.distanceToGo() != 0) {
      stepperX.runSpeed();
      stepperY.runSpeed();
    }
  delay(50);
  
  // Ve tu F --> G
  stepperY.setCurrentPosition(0);
  stepperX.setCurrentPosition(0);
  sign_vx = 1;
  sign_vy = -1;
  stepperY.setSpeed(sign_vy * stepperY.maxSpeed() * ratio);
  target_pos = (end_point % nstep_gather - nstep_gather )* sign_vx;
  for(i = arr_size - 1; i >= 0; i--) {
    if( i == 0 ) {
      target_pos = end_point * sign_vx;
    } else {
      target_pos += nstep_gather * sign_vx;
    }
    stepperX.moveTo(target_pos);
    stepperX.setSpeed(dv[i] * sign_vx * stepperY.maxSpeed() * ratio);
    while(stepperX.distanceToGo() != 0) {
      stepperX.runSpeed();
      stepperY.runSpeed();
    }
  }
  delay(50);

  // Ve tu G --> H
  stepperY.setCurrentPosition(0);
  stepperX.setCurrentPosition(0);
  sign_vx = -1;
  sign_vy = -1;
  stepperY.setSpeed(sign_vy * stepperY.maxSpeed() * ratio);
  target_pos = (start - nstep_gather) * sign_vx;
  for(i = 0; i < arr_size - 1; i++) {
    target_pos += nstep_gather * sign_vx;
    stepperX.moveTo(target_pos);
    stepperX.setSpeed(dv[i] * sign_vx * stepperY.maxSpeed() * ratio);
    while(stepperX.distanceToGo() != 0) {
      stepperY.runSpeed();
      stepperX.runSpeed();
    }
  }
  delay(50);

  // Ve tu H -> A
  stepperY.setCurrentPosition(0);
  stepperX.setCurrentPosition(0);
  sign_vx = -1;
  sign_vy = -1;
  stepperX.setSpeed(stepperX.maxSpeed() * ratio * sign_vx);
  target_pos = (end_point % nstep_gather - nstep_gather )* sign_vy;
  for(i = arr_size - 1; i >= 0; i--) {
    if( i == 0 ) {
      target_pos = end_point * sign_vy;
    } else {
      target_pos += nstep_gather * sign_vy;
    }
    stepperY.moveTo(target_pos);
    stepperY.setSpeed(dv[i] * sign_vy * stepperX.maxSpeed() * ratio);
    while(stepperY.distanceToGo() != 0) {
      stepperX.runSpeed();
      stepperY.runSpeed();
    }
  }
  delay(50);
  stepperX.setCurrentPosition(a);
  stepperY.setCurrentPosition(b);
}
void test1() {
  stepperY.moveTo(-400);
  stepperY.setSpeed(-400);
  stepperY.runSpeed();
}

void test2() {
  liftPenDown();
  moveToPoint(4800, 0);
  delay(1000);
  moveToPoint(0, 0);
  delay(1000);
  moveToPoint(4800,320);
  delay(1000);
  moveToPoint(0, 0);
  delay(1000);
  moveToPoint(4800,150);
  delay(1000);
  moveToPoint(0, 0);
  delay(1000);
  moveToPoint(4800,96);
  delay(1000);
  moveToPoint(0, 0);
  delay(1000);
  moveToPoint(4800,48);
  delay(1000);
  moveToPoint(0, 0);
  delay(1000);
}

void test3() {
  stepperX.moveTo(4000);
  stepperX.setSpeed(400);
  while(stepperX.distanceToGo() != 0) {
    stepperX.runSpeedToPosition();
  }
  delay(1000);
  stepperX.moveTo(0);
  stepperX.setSpeed(-902.5);
  while(stepperX.distanceToGo() != 0) {
    stepperX.runSpeedToPosition();
  }
  delay(1000);
}

void test4() {
  stepperY.moveTo(4000);
  stepperX.moveTo(4000);
  stepperY.setSpeed(300);
  stepperX.setSpeed(300);
  while(stepperY.distanceToGo() != 0 || stepperX.distanceToGo() != 0) {
          stepperX.runSpeed();
          stepperY.runSpeed();
  }
  delay(1000);
  
  stepperY.moveTo(0);
  stepperX.moveTo(0);
  stepperY.setSpeed(-300);
  stepperX.setSpeed(-300);
  while(stepperY.distanceToGo() != 0 || stepperX.distanceToGo() != 0) {
    stepperX.runSpeed();
    stepperY.runSpeed();
  }
  delay(1000);
}
void loop() {
  circle(1000, 1000, 2000);
  moveToPoint(0, 0);
  delay(3000);
}
