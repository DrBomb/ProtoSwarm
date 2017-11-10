#define DEBUG 1



#include "SoftwareSerial.h"

char buff[10], order, iden;
uint8_t len;

SoftwareSerial xSerial(9, 10);

void setup() {
  // put your setup code here, to run once:
  xSerial.begin(115200);
  #ifdef DEFINE
  Serial.begin(115200);
  #endif
  pinMode(A1,INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(xSerial.available() > 3){
    char delim = Serial.read();
    if(delim == '('){
      order = Serial.read();
      iden = Serial.read();
      len = (uint8_t)Serial.read();
      Serial.readBytes(buff, len);
    } else {
      
      while(Serial.available()) Serial.read();
    }
  }
  switch(order){
    case '\0':
      break;
    case '\1':
      sendProx();
      order = '\0';
      break;
    case '\2':
      sendEcho();
      order = '\0';
      break;  
  }
}

void sendEcho(){
  xSerial.write('(');
  xSerial.write(order);
}

void sendProx(){
  int reading = analogRead(PROX_PIN);
  xSerial.write('(');
  xSerial.write(order);
  xSerial.write(iden);
  xSerial.write(2);
  xSerial.write((char)(reading & 0xFF));
  xSerial.write((char)((reading >> 8) & 0xFF);
}
