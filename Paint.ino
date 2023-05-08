#include <AccelStepper.h>
#include <Servo.h>   
  
Servo myservo;
boolean servo_cond = false;
#define liftPenUp() {myservo.write(25); delay(100); servo_cond = false;}
#define liftPenDown() {myservo.write(0); delay(100); servo_cond = true;}
  
int dirX = 4;
int stepX = 5;
int enaX = 6;
AccelStepper stepperX(1, stepX, dirX, enaX);
int dirY = 7;
int stepY = 8;
int enaY = 9;
AccelStepper stepperY(1, stepY, dirY, enaY);

// set vi tri hien tai la (0, 0)
#define resetPosition() {stepperX.setCurrentPosition(0); stepperY.setCurrentPosition(0);}

// ratio: toc do toi da chay = maxSpeed() * ratio.
// At "Setup" : vxMax = stepperX.maxSpeed() * ratio; vyMax = stepperY.maxSpeed() * ratio
#define ratio 0.95

float vxMax = 500; 
float vyMax = 500;

// time to settling down the system after polling_the_paint before navigation.
#define hold 150

// the length that the stepper forward or back after the other poll a step.
#define step_length 0.001

// the number of step gather to be moved with the same velocity.
#define nstep_gather 25

// vy/vx < max_v_ratio && vx/vy < max_v_ratio
#define max_v_ratio 40

// size of array save dv for maximun size_array next step
#define size_array 50

/*
 * (a, b) : point to move the pen to.
 * move the pen from current position to point (a, b) in a straight line
 * do not implement liftPenUp/liftPenDown function but shape of line ung voi servo_cond
 */
void moveToPoint(int a, int b) {
  int disX = a - stepperX.currentPosition();
  int disY = b - stepperY.currentPosition();
  if(a < 0 || b < 0 || (disX == 0 && disY == 0) || a > 15625 || b > 12500) {
    return;
  } else {
      int sign_vx = 1;
      int sign_vy = 1;
      if(disX < 0) {
          disX = -1 * disX;
          sign_vx = -1;
      }
      if(disY < 0) {
          disY = -1 * disY;
          sign_vy = -1;
      }
      stepperX.moveTo(a);
      stepperY.moveTo(b);  
      if(disY == 0) {
            stepperX.setSpeed(sign_vx * vxMax);
            while(stepperX.distanceToGo() != 0) {
                stepperX.runSpeed();
            }
      } else if(disX == 0) {
          stepperY.setSpeed(sign_vy * vyMax);
            while(stepperY.distanceToGo() != 0) {
                stepperY.runSpeed();
            }
      } else {
          if((!servo_cond) && (1.0 * disX/disY < 0.2 || 1.0 * disX/disY > 5)) {
              stepperX.setSpeed(sign_vx * vxMax);
              stepperY.setSpeed(sign_vy * vyMax);
              while(stepperY.distanceToGo() != 0) {
                  stepperY.runSpeed();
              }
              while(stepperX.distanceToGo() != 0) {
                  stepperX.runSpeed();
              }
          } else {
             if(disX > disY) {
                stepperX.setSpeed(sign_vx * vxMax);
                stepperY.setSpeed(sign_vy * vxMax * disY / disX);
             } else {
                stepperY.setSpeed(sign_vy * vyMax);
                stepperX.setSpeed(sign_vx * vyMax * disX / disY);
             }
             
             while(stepperX.distanceToGo() != 0 || stepperY.distanceToGo() != 0) {
                if(stepperX.distanceToGo() != 0) {
                  stepperX.runSpeed();
                }
                if(stepperY.distanceToGo() != 0) {
                  stepperY.runSpeed();
                }
             }
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

/*
 * Tao 1/8 duong tron
 * dv : mang dy/dx
 * arr_size: dv.size()
 * start: vi tri bat dau cua target_pos (xem target_pos tai ham circle)
 * end: vi tri ket thuc cua target_pos
 * sign_vx: chieu di chuyen tren truc x
 * sign_vy: chieu di chuyen tren truc y
 * vmax: == true neu set vx max, == false neu vy max
 * change_v: == true neu gia tri van toc vy hoac vx bien thien thay doi tu 0. --> 1 , == flale if 1 --> 0.
 */
void circle1_8(float *dv, int arr_size,int start, int end_point, int sign_vx, int sign_vy, boolean vmax, boolean change_v) {
  int target_pos = 0, i = 0;
  stepperY.setCurrentPosition(0);
  stepperX.setCurrentPosition(0);
  if(vmax == true) { // vx max, vy change
    stepperX.setSpeed(vxMax * sign_vx);
    if(change_v == true) { // vy change from 0. to 1 (*vx)
        target_pos = (start - nstep_gather) * sign_vy;
        for(i = 0; i < arr_size - 1; i++) {
          target_pos += nstep_gather * sign_vy;
          stepperY.moveTo(target_pos);
          stepperY.setSpeed(dv[i] * sign_vy * vxMax);
          while(stepperY.distanceToGo() != 0) {
            stepperY.runSpeed();
            stepperX.runSpeed();
          }
        }
        stepperY.moveTo(end_point * sign_vy);
        stepperY.setSpeed(dv[i] * sign_vy * vxMax);
        while(stepperY.distanceToGo() != 0) {
          stepperX.runSpeed();
          stepperY.runSpeed();
        }
        delay(50);
    } else { // vy change from 1 to 0. (*vx)
      target_pos = (end_point % nstep_gather - nstep_gather )* sign_vy;
      for(i = arr_size - 1; i >= 0; i--) {
        if( i == 0 ) {
          target_pos = end_point * sign_vy;
        } else {
          target_pos += nstep_gather * sign_vy;
        }
        stepperY.moveTo(target_pos);
        stepperY.setSpeed(dv[i] * sign_vy * vxMax);
        while(stepperY.distanceToGo() != 0) {
          stepperX.runSpeed();
          stepperY.runSpeed();
        }
      }
      delay(50);
    }
  } else { // vy max , vx change
    stepperY.setSpeed(sign_vy * vyMax);
    if(change_v == true) { // vx change from 0. to 1 (*vy);
      target_pos = (start - nstep_gather) * sign_vx;
      for(i = 0; i < arr_size - 1; i++) {
        target_pos += nstep_gather * sign_vx;
        stepperX.moveTo(target_pos);
        stepperX.setSpeed(dv[i] * sign_vx * vyMax);
        while(stepperX.distanceToGo() != 0) {
          stepperX.runSpeed();
          stepperY.runSpeed();
        }
      }
      stepperX.moveTo(end_point * sign_vx);
      stepperX.setSpeed(dv[i] * sign_vx * vyMax);
      while(stepperX.distanceToGo() != 0) {
        stepperY.runSpeed();
        stepperX.runSpeed();
      }
      delay(50);
    } else { // vx change from 1 to 0. (*vy);
      stepperY.setSpeed(sign_vy * vyMax);
      target_pos = (end_point % nstep_gather - nstep_gather )* sign_vx;
      for(i = arr_size - 1; i >= 0; i--) {
        if( i == 0 ) {
          target_pos = end_point * sign_vx;
        } else {
          target_pos += nstep_gather * sign_vx;
        }
        stepperX.moveTo(target_pos);
        stepperX.setSpeed(dv[i] * sign_vx * vyMax);
        while(stepperX.distanceToGo() != 0) {
          stepperX.runSpeed();
          stepperY.runSpeed();
        }
      }
      delay(50);
     }
  }
}

/*
 * ve hinh tron ban kinh r (don vi: step)
 * (a, b): toa do tren cung cua hinh tron
 */
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

  moveToPoint(a, b);
  liftPenDown();

  // from A -> B, B --> C, ...., H --> A
  circle1_8(dv, arr_size, start, end_point, -1, 1, true, true);
  circle1_8(dv, arr_size, start, end_point, -1, 1, false, false);
  circle1_8(dv, arr_size, start, end_point, 1, 1, false, true);
  circle1_8(dv, arr_size, start, end_point, 1, 1, true, false);
  circle1_8(dv, arr_size, start, end_point, 1, -1, true, true);
  circle1_8(dv, arr_size, start, end_point, 1, -1, false, false);
  circle1_8(dv, arr_size, start, end_point, -1, -1, false, true);
  circle1_8(dv, arr_size, start, end_point, -1, -1, true, false);
  
  stepperX.setCurrentPosition(a);
  stepperY.setCurrentPosition(b);
  liftPenUp();
  delay(hold);
}

/*
 * ve theo phuong trinh func voi x thuoc [start, end_point] (dv: step)
 */
void paint_func(int a, int b, int start, int end_point, float func(int)) {
    int sign_vx = 1;
    if(start > end_point) {
      sign_vx = -1;
    }
    moveToPoint(a,b);
    if(!servo_cond) {
      liftPenDown();
    }
    stepperX.setCurrentPosition(start);
    float dv[size_array];
    int pos[size_array];
    boolean finish = false;
    int k = start;
    int i = 0, dx = 0;
    float dy = 0;
    int dv_size = 0;
    while(!finish) {
        i = 0;
        while(i < size_array && !finish) {
            dx = 0;
            float temp = func(k);
            while(1) {
                k += nstep_gather * sign_vx;
                dx += nstep_gather;
                if((k >= end_point && sign_vx == 1) || (sign_vx == -1 && k <= end_point)) {
                    dx -= (k - end_point) * sign_vx;
                    k = end_point;
                } 
                dy = func(k) - temp; 
                if((fabs(dy/dx) > 1.0/max_v_ratio && fabs(dy/dx) < max_v_ratio) || k == end_point || i == size_array) {
                    dv[i] = dy/dx;
                    pos[i] = k;
                    i++;
                    if(k == end_point) {
                        finish = true;
                    }
                    break;
                }
            }
        }
        dv_size = i;
        if(fabs(dv[dv_size - 1]) < 1.0/max_v_ratio) {
            if(dv[dv_size - 1] < 0) {
                dv[dv_size - 1] = -1.0/max_v_ratio;
            } else {
                dv[dv_size - 1] = 1.0/max_v_ratio;
            }
        } else if(fabs(dv[dv_size - 1]) > max_v_ratio) {
            if(dv[dv_size - 1] < 0) {
                dv[dv_size - 1] = -1.0 * max_v_ratio;
            } else {
                dv[dv_size - 1] = max_v_ratio;
            }
        }
        for(i = 0; i < dv_size; i++) {
//            Serial.print(dv[i]);
//            Serial.print(" ");
//            Serial.println(pos[i]);
            stepperX.moveTo(pos[i]);
            if(fabs(dv[i]) < 1) {
              stepperX.setSpeed(vxMax * sign_vx);
              stepperY.setSpeed(vxMax * dv[i]);
            } else {
              if(dv[i] < 0) {
                  stepperY.setSpeed(vyMax * -1);
                  stepperX.setSpeed(vyMax * sign_vx / dv[i] * -1);
              } else {
                  stepperY.setSpeed(vyMax);
                  stepperX.setSpeed(vyMax * sign_vx / dv[i]);
              }
            }
            while(stepperX.distanceToGo() != 0) {
              stepperX.runSpeed();
              stepperY.runSpeed();
            }
        }
    }
    stepperX.setCurrentPosition(a + end_point - start);
    stepperY.setCurrentPosition(b + func(end_point) - func(start));
//    Serial.print(stepperX.currentPosition());
//    Serial.print(" ");
//    Serial.println(stepperY.currentPosition());
}

/*
 * x : toa do cua x (step)
 * return: tao do cua y ung voi phuong trinh y = x^2 (step)
 */
float parabol(int x) {
    float temp = x * step_length;
    temp = temp * temp;
    return temp / step_length;
}

float sin_2x(int x) {
    float temp = x * step_length;
    temp = sin(temp * 2.0);
    return temp / step_length;
}

// chuoi f1 --> f5 ve hinh trai tim
float f1(int x) {
  float temp = x * step_length;
  temp = sqrt(fabs(1.0 - 1.0*(temp - 3)*(temp - 3)));
  return temp / step_length;
}

float f2(int x) {
  float temp = x * step_length;
  temp = sqrt(fabs(1.0 - 1.0*(temp - 5)*(temp - 5)));
  return temp / step_length;
}

float f3(int x) {
  float temp = x * step_length;
  temp =-1.2 * sqrt(fabs(4.0 - 1.0*(temp - 4)*(temp - 4)));
  return temp / step_length;
}

float f4(int x) {
  float temp = x * step_length;
  temp = 0.4 * cos(2 * temp - 8);
  return temp / step_length;
}

float f5(int x) {
  float temp = x * step_length;
  temp = 0.4 * cos(2 * temp - 9);
  return temp / step_length;
}

void test1() {
  circle(2500, 2000, 2000);
  delay(1000);
  star(2000, 3000, 1000);
  delay(1000);
  moveToPoint(0, 0);
  paint_func(2000, 4000, -2000, 2000, parabol);
  moveToPoint(0, 0);
  delay(1000);
  paint_func(1000, 3000, -3000, 3000, sin_2x);
  moveToPoint(0, 0);
  delay(1000);
}

void test2() {
  paint_func(4000, 3000, 2000, 4000, f1);
  paint_func(6000, 3000, 4000, 6000, f2);
  paint_func(8000, 3000, 6000, 2000, f3);
  liftPenUp();
  paint_func(2000, 2300 + round(f4(2000)), 2000, 10000, f4);
  liftPenUp();
  paint_func(10000, 2300 + round(f5(10000)), 10000, 2000, f5);
  liftPenUp();
  moveToPoint(0, 0);
  delay(3000);
}
void setup() {
  Serial.begin(115200);
  myservo.attach(12);
  liftPenUp();
  stepperX.setMaxSpeed(950);
  stepperY.setMaxSpeed(950);
  vxMax = stepperX.maxSpeed() * ratio;
  vyMax = stepperY.maxSpeed() * ratio;
  pinMode(enaX, OUTPUT);
  pinMode(enaY, OUTPUT);
  digitalWrite(enaY, HIGH);
  digitalWrite(enaX, HIGH);
  stepperX.setCurrentPosition(0);
  stepperY.setCurrentPosition(0);
  pinMode(13, OUTPUT);
  delay(8000);
  digitalWrite(13, HIGH);
  digitalWrite(enaY, LOW);
  digitalWrite(enaX, LOW);
  delay(150);
}

const byte REQUEST_DATA = 1;
const byte REQUEST_CONNECT = 2;
int hight;
int low;
int result_x;
int result_y;
byte pos_receive[4];

void loop() {
//  while(state_connect[0] != REQUEST_CONNECT) {
//    Serial.write(REQUEST_CONNECT);
//    if(Serial.available() > 0) {
//      Serial.readBytes(state_connect, 1);
//    }
//    for(int i = 0; i <10000; i++);
//    if(Serial.available() > 0) {
//      Serial.readBytes(state_connect, 1);
//    }
//  }
//  if(state_connect[0] == REQUEST_CONNECT) {
//    Serial.write(REQUEST_DATA);
//    while(Serial.available() < 4) {};
//    Serial.readBytes(pos_receive, 4);
//    hight = (int)(pos_receive[1]);
//    hight = hight & 0x00FF;
//    low = (int)(pos_receive[0]);
//    low = low & 0x00FF;
//    result_x = hight * 256 + low;
//    hight = (int)(pos_receive[3]);
//    hight = hight & 0x00FF;
//    low = (int)(pos_receive[2]);
//    low = low & 0x00FF;
//    result_y = hight * 256 + low;
//    if((result_x == 0xFFFF) && (result_y == 0xFFFF)) {
//      Serial.write((byte)(6));
//      liftPenUp();
//    } else if ((result_x == 0xFEFE) && (result_y == 0xFEFE)) {
//      Serial.write((byte)(7));
//      liftPenDown();
//    } else {
//      Serial.write((byte)(8));
//      moveToPoint(result_x, result_y);
//    }
//  }
  test2();
}
