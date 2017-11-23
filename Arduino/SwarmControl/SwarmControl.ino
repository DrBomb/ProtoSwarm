#define DEBUG 1
#define BUFFSIZE 10
#define FLOOR_R A3
#define FLOOR_L A5
#define WHEEL_R A2
#define WHEEL_L A4
#define PROX_PIN A1

#include <AFMotor.h>
#include <QTRSensors.h>
#include "SoftwareSerial.h"

char buff[BUFFSIZE], order, iden;
uint8_t len;
unsigned int sensorValues[4];

QTRSensorsRC qtr((unsigned char[]) {FLOOR_R, FLOOR_L, WHEEL_R, WHEEL_L}, 4);
AF_DCMotor motorR(1);
AF_DCMotor motorL(2);
SoftwareSerial xSerial(10, 9);

struct Motor {
  unsigned long deadline;
  bool forward = false;
  AF_DCMotor *motor;
  uint8_t speed;
} MotorR, MotorL;

void setup() {
  MotorR.motor = &motorR;
  MotorL.motor = &motorL;
  
  // put your setup code here, to run once:
  xSerial.begin(115200);
  #if DEBUG
  Serial.begin(115200);
  #endif
  pinMode(A1,INPUT);
  for (int i = 0; i < 250; i++)  // make the calibration take about 5 seconds
  {
    qtr.calibrate();
    delay(20);
  }
  #if DEBUG
  Serial.println("INIT");
  #endif
}

void loop() {
  // put your main code here, to run repeatedly:
  if(xSerial.available() > 3){
    char delim = xSerial.read();
    if(delim == '('){
      order = xSerial.read();
      iden = xSerial.read();
      len = (uint8_t)xSerial.read();
      xSerial.readBytes(buff, len);
    } else {
      while(xSerial.available()) xSerial.read();
    }
    #if DEBUG
    Serial.print("Order: ");
    Serial.print(order, HEX);
    Serial.print(" Iden: ");
    Serial.print(iden, HEX);
    Serial.print(" Len: ");
    Serial.print(len);
    Serial.print(" Data: ");
    for(int i = 0; i < len; i++){
      Serial.print(buff[i]);
      Serial.print(" ");
    }
    Serial.println();
    #endif
  }
  qtr.readCalibrated(sensorValues);
  doMotor(MotorR);
  doMotor(MotorL);
  switch(order){
    case (char)0:
      break;
    case (char)1: //Enviar Lectura de Sensor Frontal
      sendProx();
      break;
    case (char)2: // Enviar Eco
      sendEcho();
      break;
    case (char)3: // Velocidad Motor Derecho
      motorSpeed(MotorR);
      break;
    case (char)4: // Velocidad Motor Izquierdo
      motorSpeed(MotorL);
      break;
    case (char)5: // Control Motor Primer Byte Derecho 1 - Forward, 2 - Backward, Otro - Release. Segundo y tercer byte, tiempo en ms
      motorSet(MotorR);
      break;
    case (char)6: // Control Motor Primer Byte Derecho 1 - Forward, 2 - Backward, Otro - Release. Segundo y tercer byte, tiempo en ms
      motorSet(MotorL);
      break;
    case (char)7: // Sensor Piso Derecho
      sendFloorR();
      break;
    case (char)8: // Sensor Piso Izquierdo
      sendFloorL();
      break;
    case (char)9: // Sensor Rueda Derecha
      sendWheelR();
      break;
    case (char)10: // Sensor Rueda Izquierda
      sendWheelL();
      break;
  }
  order = '\0';
  iden = '\0';
  memset(buff, 0, BUFFSIZE);
}

void sendFloorR(){
  xSerial.write('(');
  xSerial.write(order);
  xSerial.write(iden);
  xSerial.write((uint8_t)2);
  xSerial.write((uint8_t)((sensorValues[2]>>8)&0xFF));
  xSerial.write((uint8_t)(sensorValues[2]&0xFF));
}

void sendFloorL(){
  xSerial.write('(');
  xSerial.write(order);
  xSerial.write(iden);
  xSerial.write((uint8_t)2);
  xSerial.write((uint8_t)((sensorValues[3]>>8)&0xFF));
  xSerial.write((uint8_t)(sensorValues[3]&0xFF));
}

void sendWheelR(){
  xSerial.write('(');
  xSerial.write(order);
  xSerial.write(iden);
  xSerial.write((uint8_t)2);
  xSerial.write((uint8_t)((sensorValues[0]>>8)&0xFF));
  xSerial.write((uint8_t)(sensorValues[0]&0xFF));
}

void sendWheelL(){
  xSerial.write('(');
  xSerial.write(order);
  xSerial.write(iden);
  xSerial.write((uint8_t)2);
  xSerial.write((uint8_t)((sensorValues[1]>>8)&0xFF));
  xSerial.write((uint8_t)(sensorValues[1]&0xFF));
}

void motorSet(Motor m){
  uint8_t mode = (uint8_t) buff[0];
  uint16_t time;
  time = (uint16_t)((buff[1]<<8)|buff[2]);
  switch(mode){
    case 1:
      m.deadline = millis() + time;
      m.forward = true;
      break;
    case 2:
      m.deadline = millis() + time;
      m.forward = false;
      break;
    default:
      m.deadline = 0;
  }
  xSerial.write('(');
  xSerial.write(order);
  xSerial.write(iden);
  xSerial.write((uint8_t)3);
  xSerial.write(mode);
  xSerial.write(buff[1]);
  xSerial.write(buff[2]);
}

void motorSpeed(Motor m){
  (*m.motor).setSpeed((uint8_t)buff[0]);
}

void doMotor(Motor m){
  if(m.deadline > millis()){
    if(m.forward){
      (*m.motor).run(FORWARD);
    } else {
      (*m.motor).run(BACKWARD);
    }
  } else {
    (*m.motor).run(RELEASE);
  }
}

void sendEcho(){
  xSerial.write('(');
  xSerial.write(order);
  xSerial.write(iden);
  xSerial.write((uint8_t)0);
}

void sendProx(){
  int reading = analogRead(PROX_PIN);
  xSerial.write('(');
  xSerial.write(order);
  xSerial.write(iden);
  xSerial.write((uint8_t)2);
  xSerial.write((char)((reading >> 8) & 0xFF));
  xSerial.write((char)(reading & 0xFF));
}
