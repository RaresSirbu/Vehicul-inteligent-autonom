#include <Servo.h>
#include <SoftwareSerial.h>

SoftwareSerial BT(A4,A3);

const int ledSS=11;
const int ledSD=12;
const int ledFS=A1;
const int ledFD=A2;

const int ENA=2;
const int IN1=3;
const int IN2=4;
const int ENB=7;
const int IN3=5;
const int IN4=6;

const int SERVO_PIN=8;
const int ECHO_PIN=9;
const int TRIG_PIN=10;
const int BUZZER_PIN=13;

Servo servoCap;

bool misiuneTerminata=false;
bool avariiActive=false;
bool luminiFataActive=false;

int vitezaSoc=160;
int socuriSpateNumar=0;

void setup(){
  Serial.begin(9600);
  BT.begin(9600);

  pinMode(ledSS,OUTPUT);
  pinMode(ledSD,OUTPUT);
  pinMode(ledFS,OUTPUT);
  pinMode(ledFD,OUTPUT);

  pinMode(ENA,OUTPUT);
  pinMode(IN1,OUTPUT);
  pinMode(IN2,OUTPUT);
  pinMode(ENB,OUTPUT);
  pinMode(IN3,OUTPUT);
  pinMode(IN4,OUTPUT);

  pinMode(TRIG_PIN,OUTPUT);
  pinMode(ECHO_PIN,INPUT);
  pinMode(BUZZER_PIN,OUTPUT);

  servoCap.attach(SERVO_PIN);
  servoCap.write(90);

  for(int i=0;i<3;i++){
    setAvarii(HIGH);
    digitalWrite(BUZZER_PIN,HIGH);
    delay(166);
    setAvarii(LOW);
    digitalWrite(BUZZER_PIN,LOW);
    delay(166);
  }
  delay(500);
}

void loop(){
  int dist=masoaraDistanta();
  if(dist<30&&dist>0){
    stopMotoare();
    opresteDirectie();
    avariiActive=false;
    setAvarii(LOW);
    digitalWrite(ledSS,LOW);
    digitalWrite(ledSD,LOW);
    digitalWrite(ledFS,HIGH);
    digitalWrite(ledFD,HIGH);
    delay(200);
    startManevraOcolire();
    misiuneTerminata=false;
    servoCap.write(90);
    setAvarii(LOW);
    return;
  }

  if(BT.available()){
    char comanda=BT.read();
    switch(comanda){
      case 'f':
        if(luminiFataActive){digitalWrite(ledSS,HIGH);digitalWrite(ledSD,HIGH);}
        daSocMotorFata();
        break;
      case 'b':
        digitalWrite(ledFS,HIGH);
        digitalWrite(ledFD,HIGH);
        digitalWrite(IN1,HIGH);
        digitalWrite(IN2,LOW);
        analogWrite(ENA,vitezaSoc);
        delay(150);
        analogWrite(ENA,0);
        digitalWrite(ledFS,LOW);
        digitalWrite(ledFD,LOW);
        break;
      case 'l':
        vireazaStangaMaxim();
        digitalWrite(ledSS,HIGH);
        digitalWrite(ledFS,HIGH);
        break;
      case 'r':
        vireazaDreaptaMaxim();
        digitalWrite(ledSD,HIGH);
        digitalWrite(ledFD,HIGH);
        break;
      case 'S':
        stopMotoare();
        opresteDirectie();
        if(!avariiActive&&!luminiFataActive)setAvarii(LOW);
        if(luminiFataActive){digitalWrite(ledSS,HIGH);digitalWrite(ledSD,HIGH);}
        break;
      case 'W':
        luminiFataActive=true;
        digitalWrite(ledSS,HIGH);
        digitalWrite(ledSD,HIGH);
        break;
      case 'w':
        luminiFataActive=false;
        digitalWrite(ledSS,LOW);
        digitalWrite(ledSD,LOW);
        break;
      case 'U':
        digitalWrite(ledFS,HIGH);
        digitalWrite(ledFD,HIGH);
        break;
      case 'u':
        digitalWrite(ledFS,LOW);
        digitalWrite(ledFD,LOW);
        break;
      case 'V':
        digitalWrite(BUZZER_PIN,HIGH);
        break;
      case 'v':
        digitalWrite(BUZZER_PIN,LOW);
        break;
      case 'X':
        avariiActive=true;
        setAvarii(HIGH);
        break;
      case 'x':
        avariiActive=false;
        setAvarii(LOW);
        break;
    }
  }
}

void startManevraOcolire(){
  stopMotoare();
  for(int i=0;i<3;i++){
    digitalWrite(ledSS,HIGH);
    digitalWrite(ledSD,HIGH);
    digitalWrite(ledFS,HIGH);
    digitalWrite(ledFD,HIGH);
    digitalWrite(BUZZER_PIN,HIGH);
    delay(166);
    digitalWrite(ledSS,LOW);
    digitalWrite(ledSD,LOW);
    digitalWrite(ledFS,LOW);
    digitalWrite(ledFD,LOW);
    digitalWrite(BUZZER_PIN,LOW);
    delay(166);
  }

  servoCap.write(0);
  delay(400);

  int masuratori[19];
  int idx=0;
  for(int u=0;u<=180;u+=10){
    servoCap.write(u);
    delay(300);
    masuratori[idx]=masoaraDistanta();
    idx++;
  }

  int sumaStanga=0,sumaDreapta=0,countStanga=0,countDreapta=0;
  for(int i=0;i<19;i++){
    int unghi=i*10;
    if(unghi<=80){sumaStanga+=masuratori[i];countStanga++;}
    else if(unghi>=100){sumaDreapta+=masuratori[i];countDreapta++;}
  }

  int mediaStanga=(countStanga>0)?sumaStanga/countStanga:0;
  int mediaDreapta=(countDreapta>0)?sumaDreapta/countDreapta:0;

  servoCap.write(90);
  delay(300);

  bool ocolirePrinDreapta=(mediaDreapta>mediaStanga);
  socuriSpateNumar=0;

  if(ocolirePrinDreapta)vireazaDreaptaMaxim();
  else vireazaStangaMaxim();

  bool obstacolDepasit=false;
  int incercariConsecutiveFaraMiscare=0;

  while(!obstacolDepasit&&incercariConsecutiveFaraMiscare<3){
    servoCap.write(90);
    delay(80);
    int distanta=masoaraDistanta();
    if(distanta<30&&distanta>0){
      digitalWrite(ledSS,HIGH);
      digitalWrite(ledSD,HIGH);
      digitalWrite(ledFS,HIGH);
      digitalWrite(ledFD,HIGH);
      stopMotoare();
      opresteDirectie();
      delay(200);

      if(ocolirePrinDreapta)vireazaStangaMaxim();
      else vireazaDreaptaMaxim();
      delay(100);

      digitalWrite(IN1,HIGH);
      digitalWrite(IN2,LOW);
      analogWrite(ENA,129);
      digitalWrite(BUZZER_PIN,HIGH);
      delay(200);
      digitalWrite(BUZZER_PIN,LOW);
      analogWrite(ENA,0);
      socuriSpateNumar++;

      delay(400);
      stopMotoare();
      opresteDirectie();
      digitalWrite(ledSS,LOW);
      digitalWrite(ledSD,LOW);
      digitalWrite(ledFS,LOW);
      digitalWrite(ledFD,LOW);
      delay(200);

      if(ocolirePrinDreapta)vireazaDreaptaMaxim();
      else vireazaStangaMaxim();
      incercariConsecutiveFaraMiscare=0;
    }else{
      if(ocolirePrinDreapta){
        digitalWrite(ledSD,HIGH);
        digitalWrite(ledFD,HIGH);
        digitalWrite(ledSS,LOW);
        digitalWrite(ledFS,LOW);
      }else{
        digitalWrite(ledSS,HIGH);
        digitalWrite(ledFS,HIGH);
        digitalWrite(ledSD,LOW);
        digitalWrite(ledFD,LOW);
      }
      daSocMotorFata();
      delay(400);
      digitalWrite(ledSS,LOW);
      digitalWrite(ledSD,LOW);
      digitalWrite(ledFS,LOW);
      digitalWrite(ledFD,LOW);

      servoCap.write(90);
      delay(80);
      distanta=masoaraDistanta();
      if(distanta>40||distanta==0)obstacolDepasit=true;
      else if(distanta<30&&distanta>0)incercariConsecutiveFaraMiscare=0;
      else incercariConsecutiveFaraMiscare++;
    }
  }

  digitalWrite(ledSS,LOW);
  digitalWrite(ledSD,LOW);
  digitalWrite(ledFS,LOW);
  digitalWrite(ledFD,LOW);
  stopMotoare();
  opresteDirectie();

  servoCap.write(0);
  delay(250);
  masoaraDistanta();
  servoCap.write(90);
  delay(250);
  masoaraDistanta();
  servoCap.write(180);
  delay(250);
  masoaraDistanta();
  servoCap.write(90);
  delay(250);

  digitalWrite(ledSS,HIGH);
  digitalWrite(ledSD,HIGH);
  unsigned long tStart=millis();
  while(millis()-tStart<2000){
    digitalWrite(IN1,LOW);
    digitalWrite(IN2,HIGH);
    analogWrite(ENA,vitezaSoc);
    delay(150);
    analogWrite(ENA,0);
    int dist=masoaraDistanta();
    if(dist<20&&dist>0){
      digitalWrite(ledSS,LOW);
      digitalWrite(ledSD,LOW);
      digitalWrite(ledFS,HIGH);
      digitalWrite(ledFD,HIGH);
      delay(200);
      break;
    }
    delay(350);
  }

  digitalWrite(ledSS,LOW);
  digitalWrite(ledSD,LOW);
  digitalWrite(ledFS,LOW);
  digitalWrite(ledFD,LOW);
  stopMotoare();
  delay(300);

  if(ocolirePrinDreapta)vireazaStangaMaxim();
  else vireazaDreaptaMaxim();

  int socuriRevenire=socuriSpateNumar*2;
  if(socuriRevenire<3)socuriRevenire=3;
  if(socuriRevenire>8)socuriRevenire=8;

  for(int i=0;i<socuriRevenire;i++){
    if(ocolirePrinDreapta){
      digitalWrite(ledSS,HIGH);
      digitalWrite(ledFS,HIGH);
      digitalWrite(ledSD,LOW);
      digitalWrite(ledFD,LOW);
    }else{
      digitalWrite(ledSD,HIGH);
      digitalWrite(ledFD,HIGH);
      digitalWrite(ledSS,LOW);
      digitalWrite(ledFS,LOW);
    }
    daSocMotorFata();
    delay(200);
    digitalWrite(ledSS,LOW);
    digitalWrite(ledSD,LOW);
    digitalWrite(ledFS,LOW);
    digitalWrite(ledFD,LOW);
    delay(133);
  }

  stopMotoare();
  opresteDirectie();
  setAvarii(HIGH);
  digitalWrite(BUZZER_PIN,HIGH);
  delay(500);
  digitalWrite(BUZZER_PIN,LOW);
  setAvarii(LOW);
}

void daSocMotorFata(){
  digitalWrite(IN1,LOW);
  digitalWrite(IN2,HIGH);
  analogWrite(ENA,vitezaSoc);
  delay(150);
  analogWrite(ENA,0);
}

int masoaraDistanta(){
  digitalWrite(TRIG_PIN,LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN,HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN,LOW);
  long durata=pulseIn(ECHO_PIN,HIGH,30000);
  if(durata==0)return 100;
  return durata*0.034/2;
}

void setAvarii(int stare){
  digitalWrite(ledSS,stare);
  digitalWrite(ledSD,stare);
  digitalWrite(ledFS,stare);
  digitalWrite(ledFD,stare);
}

void stopMotoare(){
  analogWrite(ENA,0);
  analogWrite(ENB,0);
  digitalWrite(IN1,LOW);
  digitalWrite(IN2,LOW);
}

void vireazaStangaMaxim(){
  digitalWrite(IN3,LOW);
  digitalWrite(IN4,HIGH);
  analogWrite(ENB,255);
}

void vireazaDreaptaMaxim(){
  digitalWrite(IN3,HIGH);
  digitalWrite(IN4,LOW);
  analogWrite(ENB,255);
}

void opresteDirectie(){
  digitalWrite(IN3,LOW);
  digitalWrite(IN4,LOW);
  analogWrite(ENB,0);
}
