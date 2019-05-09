//pinmode(14,input);                   // set the pin to input
//int BUTTON = digitalread(14);        // read analog pin 0 as digital and set it to a variable
//pinMode(A0, INPUT_PULLUP);

// id tttt xxxx yyyy zzzz
// All numbers are 14 bits encodded as 2 times 7 bit in little endiam mode. Bits 7 of all bytes is ALWAYS LOW
// t is positive only
// x/y/z are signed
// id's bit 7 is ALWAYS high...
// once the move is completed, will reply by sending id back


// Led1, Led2, dx(3), dy(3)
// -> x, y

unsigned int MotorPos[3]= {0, 0, 0};

void setup() 
{
  //Serial.begin(9600);
  Serial.begin(115200);
  // Pins 2 to 13 are output low
  for (int i= 2; i<=19; i++) { pinMode(i, OUTPUT); digitalWrite(i, LOW); }
  Serial.println("Awake");
}

void updateSteps(int motor);

void loop() 
{
  int nb= Serial.available();
  if (nb==0) return;
  byte id= Serial.read();
  if ((id&0x80)==0) return; // not a start marker
  while (true)
  {
    if (nb>=8) break;
    delayMicroseconds(1000);
    nb= Serial.available();
  }
  byte data[8]; Serial.readBytes(data, 8);
  for (int i=0; i<8; i++) if ((data[i]&0x80)!=0) { Serial.println("Bad Data"); return; } // check that all data has 0 at tope
  unsigned long t= data[0]+(((int)data[1])<<7); t*= 1000; // t in micro seconds
  int x= data[2]+(((int)data[3])<<7); if ((x&0x2000)!=0) x|= 0xc000; // sign extend
  int y= data[4]+(((int)data[5])<<7); if ((y&0x2000)!=0) y|= 0xc000; // sign extend
  int z= data[6]+(((int)data[7])<<7); if ((z&0x2000)!=0) z|= 0xc000; // sign extend

  //analogWrite(PWM_Pin, PWM_Value);

  // move x, y, z in t micro seconds
  unsigned long dtx= (x==0) ? t+1 : t/abs(x);
  unsigned long dty= (y==0) ? t+1 : t/abs(y);
  unsigned long dtz= (z==0) ? t+1 : t/abs(z);
  unsigned long nextx= (x==0) ? t+1 : 0;
  unsigned long nexty= (y==0) ? t+1 : 0;
  unsigned long nextz= (z==0) ? t+1 : 0;

  //Serial.print("t:"); Serial.print(t);
  //Serial.print(" x:"); Serial.print(x);
  //Serial.print(" y:"); Serial.print(y);
  //Serial.print(" z:"); Serial.print(z);
  //Serial.print(" dx:"); Serial.print(dtx);
  //Serial.print(" dy:"); Serial.print(dty);
  //Serial.print(" dz:"); Serial.println(dtz);

  unsigned long now= 0;
  while (now<t)
  {
    if (now>=nextx) { MotorPos[0]+= x>0 ? 1 : -1; nextx+= dtx; updateSteps(0); }
    if (now>=nexty) { MotorPos[1]+= y>0 ? 1 : -1; nexty+= dty; updateSteps(1); }
    if (now>=nextz) { MotorPos[2]+= z>0 ? 1 : -1; nextz+= dtz; updateSteps(2); }
    unsigned long next= min(min(nextx, nexty), nextz)-now;
    delayMicroseconds(next);
    now+= next;
  }
  Serial.write(id);
}


void updateSteps(int motor)
{
  int pin= motor*4+2; // initial motor pin
  if (motor==2) pin= 14;
  switch (MotorPos[motor]&7) {
    case 0: digitalWrite(pin, LOW); digitalWrite(pin+1, LOW); digitalWrite(pin+2, LOW); digitalWrite(pin+3, HIGH); break;
    case 1: digitalWrite(pin, LOW); digitalWrite(pin+1, LOW); digitalWrite(pin+2, HIGH); digitalWrite(pin+3, HIGH); break;
    case 2: digitalWrite(pin, LOW); digitalWrite(pin+1, LOW); digitalWrite(pin+2, HIGH); digitalWrite(pin+3, LOW); break;
    case 3: digitalWrite(pin, LOW); digitalWrite(pin+1, HIGH); digitalWrite(pin+2, HIGH); digitalWrite(pin+3, LOW); break; 
    case 4: digitalWrite(pin, LOW); digitalWrite(pin+1, HIGH); digitalWrite(pin+2, LOW); digitalWrite(pin+3, LOW); break;
    case 5: digitalWrite(pin, HIGH); digitalWrite(pin+1, HIGH); digitalWrite(pin+2, LOW); digitalWrite(pin+3, LOW); break;
    case 6: digitalWrite(pin, HIGH); digitalWrite(pin+1, LOW); digitalWrite(pin+2, LOW); digitalWrite(pin+3, LOW); break;
    case 7: digitalWrite(pin, HIGH); digitalWrite(pin+1, LOW); digitalWrite(pin+2, LOW); digitalWrite(pin+3, HIGH); break;
  }
}
