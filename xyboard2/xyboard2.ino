//pinmode(14,input);                   // set the pin to input
//int BUTTON = digitalread(14);        // read analog pin 0 as digital and set it to a variable
//pinMode(A0, INPUT_PULLUP);

// Led1, Led2, operation(2), spd(3), x(3), y(3) // operation=0: goto x/y  operation=1: set current x/y  operation=2: stop move, operation=3: slack set
// spd is delay between steps...
// -> x, y

void setup() 
{
  //Serial.begin(9600);
  Serial.begin(115200);
  // Pins 2 to 13 are output low
  for (int i= 2; i<=19; i++) { pinMode(i, OUTPUT); digitalWrite(i, LOW); }
  Serial.println("Awake");
}

void updateSteps(int motor);

long getInt(byte *b)
{
  long r= b[0]+(((long)b[1])<<7)+(((long)b[2])<<14);
  if ((b[2]&0x40)!=0) r|= 0xffe00000;
  return r;
}

long x=0, y=0;
long dx=0, dy=0;
long slackx= 0, slacky= 0;
char xdir= 0, ydir= 0;
long stepSpeed= 1000;
byte data[12];
byte dataUsed= 0;
bool hasChanged= false;

void loop() 
{
  if (Serial.available()>=12-dataUsed)
  {
    Serial.readBytes(data+dataUsed, 12-dataUsed);
    data[0]^=0x80; // invert the start marker bit so that if it is not set it becomes set and falls into the error detection bellow...
    for (int i=0; i<12; i++) if ((data[i]&0x80)!=0) { Serial.println("Bad Data"); memcpy(data, data+i, 12-i); dataUsed-= i; goto bouge; } // check that all data has 0 at tope
    // get t, x and y
    stepSpeed= getInt(data+3); dx= getInt(data+6); dy= getInt(data+9);
    // update LEDs outputs
    analogWrite(10, data[0]<<1);
    analogWrite(11, data[1]<<1);
    // deal with command
    if ((data[2]&0x3)==0) 
    { // move, but check direction changes and absorb slack!
      char xdir2= 0; if (dx<x) xdir2= -1; else if (dx>x) xdir2= 1; if (xdir*xdir2<0) x-= xdir2*slackx; xdir= xdir2;
      char ydir2= 0; if (dy<y) ydir2= -1; else if (dy>y) ydir2= 1; if (ydir*ydir2<0) y-= ydir2*slacky; ydir= ydir2;
    } else if ((data[2]&0x3)==1) { x= dx; y= dy; } // set x/y
    else if ((data[2]&0x3)==2) { dx= x; dy= y; } // Stop moving... 
    else /*if ((data[2]&0x3)==1)*/ { slackx= dx; slacky= dy; dx= x; dy= y; } // set slack
    hasChanged= true;
  } bouge:

  if (dx!=x || dy!=y)
  {
    if (dx!=x) { if (dx<x) x--; else x++; updateSteps(0, x); hasChanged= true; }
    if (dy!=y) { if (dy<y) y--; else y++; updateSteps(1, y); hasChanged= true; }
    delayMicroseconds(stepSpeed);
  }
  
  if (hasChanged && Serial.availableForWrite()>=6)
  {
    // write x/y pos
    byte out[6]; 
    out[0]= (x&0x7f)|0x80; out[1]= (x>>7)&0x7f; out[2]= (x>>14)&0x7f; 
    out[3]= y&0x7f; out[4]= (y>>7)&0x7f; out[5]= (y>>14)&0x7f; 
    Serial.write(out, 6);
    hasChanged= false;
  }
}


void updateSteps(int motor, long pos)
{
  int pin= motor*4+2; // initial motor pin
  switch (pos&7) {
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
