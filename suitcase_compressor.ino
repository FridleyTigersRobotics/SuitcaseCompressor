//Arduino Pro Mini ATmega328P (5V, 16 MHz)

#define PRESS_SW 7
#define RSL 3
#define COMPRESSOR 5 
#define BATTADCPIN A0
#define SHORTCYCLEDELAY 5000

int lightstatus=1;
bool lighton=false;
unsigned long lastblinktime = 0;
int switchstate=0;
bool lastswitchstate = LOW;
int reading=1;
unsigned long lastdebouncetime = 0;
unsigned long debouncedelay = 50;
unsigned long lastruntime=0;
unsigned long milliheartbeat=0;
bool nowrun = false;
//measured 1.11v at ADC pin at 12.52V battery
float adcmultiplier=(12.52/1.11);
float battvoltage=0;

void setup() {
  // put your setup code here, to run once:
Serial.begin(9600); //enable serial port for debugging if needed
pinMode(PRESS_SW, INPUT);//pressure switch
pinMode(RSL, OUTPUT);
pinMode(COMPRESSOR, OUTPUT);
pinMode(LED_BUILTIN,OUTPUT);
pinMode(BATTADCPIN, INPUT);
digitalWrite(COMPRESSOR, LOW);
digitalWrite(LED_BUILTIN, LOW);
//delay(1000);//delay startup

}

void loop() {
  // put your main code here, to run repeatedly:
  if(millis()>(milliheartbeat+1000)){
    milliheartbeat=millis();
    digitalWrite(LED_BUILTIN, (!digitalRead(LED_BUILTIN)));
    battvoltage=adcmultiplier*( (float) analogRead( BATTADCPIN ) * 5.0f / 1024.0f );
    Serial.print("battery voltage = ");
    Serial.print(battvoltage);
    Serial.println("");

  }

  if((millis()<lastdebouncetime) || (millis()<lastruntime)){
    //rollowver of millis
    lastdebouncetime=0;
    lastruntime=0;
    lightstatus=4;
    milliheartbeat=0;
  }

  if (lighton) {
    digitalWrite(RSL,HIGH);
  }
  else {
    digitalWrite(RSL,LOW);
  }
  switch (lightstatus) {
    case 1:
      //light off
      lighton=false;
      break;
    case 2:
      //slow blink
      if ((millis()-lastblinktime)>1000){
        lighton=!lighton;
        lastblinktime=millis();
      }
      break;
    case 3:
      //fast blink
      if ((millis()-lastblinktime)>200){
        lighton=!lighton;
        lastblinktime=millis();
      }
      break;
    case 4:
      //solid light
      lighton=true;
      break;
    case 5:
      //standby blink
      if (((millis()-lastblinktime)>100) && (lighton==true)){
        lighton=false;
        lastblinktime=millis();
      }
      if (((millis()-lastblinktime)>3000) && (lighton==false)){
        lighton=true;
        lastblinktime=millis();
      }
      break;
  }

  //debounce pressure switch
  reading = digitalRead(PRESS_SW);
  if (reading != lastswitchstate) {
    lastdebouncetime=millis();
  }
  if ((millis()-lastdebouncetime)>debouncedelay) {
    if (reading != switchstate) {
      switchstate=reading;
    }
  }
  lastswitchstate=reading;//save the reading. next time through the loop, it'll be the lastswitchstate
 // switchstate=digitalRead(PRESS_SW); //if there was no debounce used

  if(switchstate==LOW){
    //low pressure
    if((millis()-lastruntime)<SHORTCYCLEDELAY) //waiting to run
    {
      lightstatus=3;
      digitalWrite(COMPRESSOR, LOW);
    }
    if(((millis()-lastruntime)>SHORTCYCLEDELAY) || (nowrun==true)) //only run compressor if longer than SHORTCYCLEDELAY between last run time, or it has already started running and is still not up to pressure
    {
      lightstatus=2;
      nowrun=true;
      digitalWrite(COMPRESSOR, HIGH);
      lastruntime=millis();
    }
  }
  if(switchstate==HIGH){
    //switch at high is open, or full pressure
    nowrun=false;
    digitalWrite(COMPRESSOR, LOW);
    lightstatus=5;
  }
}

