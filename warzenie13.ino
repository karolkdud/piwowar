#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <OneWire.h>



#define ONE_WIRE_BUS 12
#define TEMPERATURE_PRECISION 12
#define TIMER_DELAY 60000 // ma być 60 000
#define TIMER_REFRESH 100

LiquidCrystal_I2C lcd_d(0x20, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress insideThermometer, outsideThermometer;

void PreKey(){
  digitalWrite(2,LOW);
  digitalWrite(3,LOW);
  digitalWrite(4,LOW);
  digitalWrite(5,LOW);
  digitalWrite(13,LOW);
}
char KEY[4][4] = {{'1','4','7','*'},
                  {'2','5','8','0'},
                  {'3','6','9','#'},
                  {'A','B','C','D'}};
                               //0123456789ABCDEF   0123456789ABCDEF
char Wyswietlenie[7][2][17] ={{" WARZENIE       "," GOTOWANIE      "}, // 0
                              {"CYKL    "        ,"Czas    mT    'C"}, // 1
                              {" START          "," KOLEJNY CYKL   "}, // 2
                              {" PRZERWAC W.    "," KONTYNUUJ W.   "}, // 3
                              {"GOTOWANIE"       ,"Czas    m      "}, // 4
                              {" !!! ALARM !!!  ","  !!! ALARM !!! "}, // 5 
                              {" EMPTY          "," EMPTY          "}};// 6
struct Warzenie {
  float temp=0; // możliwa redukcja pamięci dynamicznej programu : liczba dziesiętnych części temperatury. np 1005 = 100.5 'C
  byte czas=0;
};
Warzenie stackWarzenie[10];
byte stackWarzenieCounter=0;

struct Gotowanie {
  byte czas=0;
};

Gotowanie stackGotowanie[2];
byte stackGotowanieCounter=0;

boolean stackProces[13];
byte stackProcesCounter=0;

char key='W';
byte x,y;
boolean czyDol=0,czyPrawo=0;

//nagłówki funkcji
void procesWarzenia();
void procesGotowania();
//|O|O|O|O|O|O|O|O| - Wejscia klawiatury
//|2|3|4|5|6|7|8|9| - Piny Arduino

void setup(){
  pinMode(2,OUTPUT);
  pinMode(3,OUTPUT);
  pinMode(4,OUTPUT);
  pinMode(5,OUTPUT);
  pinMode(6,INPUT);      //Pull down
  pinMode(7,INPUT);      //Pull down
  pinMode(8,INPUT);      //Pull down
  pinMode(9,INPUT);      //Pull down
  pinMode(10,OUTPUT);    //Grzałka
  digitalWrite(10,HIGH);
  pinMode(11,OUTPUT);    //Mieszadło
  digitalWrite(11,HIGH);
  
  pinMode(12,OUTPUT);    //termometr
  digitalWrite(12,LOW);
  pinMode(13,OUTPUT);
  PreKey();
  Serial.begin(115200);
  lcd_d.begin(16,2);   // Inicjalizacja LCD 2x16

  lcd_d.backlight(); // zalaczenie podwietlenia 

  lcd_d.blink();

  sensors.begin();
  if (!sensors.getAddress(insideThermometer, 0)) Serial.println("Unable to find address for Device 0"); 
  if (!sensors.getAddress(outsideThermometer, 1)) Serial.println("Unable to find address for Device 1"); 
  sensors.setResolution(insideThermometer, TEMPERATURE_PRECISION);
  sensors.setResolution(outsideThermometer, TEMPERATURE_PRECISION);

}
  
void loop(){
      printTemperature(insideThermometer);
  Ekrany(3);
  while(Ekrany(2)){
    if(Ekrany(0)) {EKRAN4();} 
    else {EKRAN1();}
 }
  stackGotowanieCounter=1;
  stackWarzenieCounter=1;
  for(byte i=1;i<=stackProcesCounter;i++){
    lcd_d.clear();
    lcd_d.setCursor(0xB,0);
    lcd_d.print(i,DEC);
    lcd_d.setCursor(0xD,0);
    lcd_d.print('/');
    lcd_d.print(stackProcesCounter);
    if(stackProces[i]==1){
      procesWarzenia();
      stackWarzenieCounter++;
    }
    else {
      procesGotowania();
      stackGotowanieCounter++;
      }
  }
  delay(1000);
  stackGotowanieCounter=0;
  stackWarzenieCounter=0;
  stackProcesCounter=0;
}

  char GetKey(){
    byte ret=255,i,j,k,l;
    while(ret==255){
    for (i=2;i<=5;i++){
      digitalWrite(i,HIGH);
      for(j=6;j<=9;j++){
        while(digitalRead(j)==HIGH){
          ret=0;
          k=i;
          l=j;
          digitalWrite(13,HIGH);
          delay(100);
          digitalWrite(13,LOW);
          while(digitalRead(j)==HIGH){}
        }
      }
   digitalWrite(i,LOW);
   }
   }
  return KEY[l-6][k-2];
  }
  //wyswietla ekran z w/w tablicy
  void EKRAN(byte nr){
    for(byte i=0;i<2;i++){
      lcd_d.setCursor(0,i);
      lcd_d.print(Wyswietlenie[nr][i]);
    }
  }
//Ekrany wyboru Dóra - Dół 0- warzenie/gotowanie,2 - start/nastepny,3- przerwac?/kontynuowac? 
boolean Ekrany(byte e){
  lcd_d.noBlink();
  lcd_d.clear();
  while(key!='*'){
    EKRAN(e);
    lcd_d.setCursor(0,czyDol);
    lcd_d.print('>');
    key=GetKey();
    if(key=='C' || key=='D'){
      czyDol = !czyDol;
    }
    else {};
  }
  key='W';
  return czyDol;
}
//Warzenie ustawienia
void EKRAN1(){
  lcd_d.blink();
  stackProcesCounter++;
  stackProces[stackProcesCounter]=1;
  stackWarzenieCounter++;
  lcd_d.clear();
  while((key!='*') || ((stackWarzenie[stackWarzenieCounter].czas==0) || (stackWarzenie[stackWarzenieCounter].temp==0))){
  EKRAN(1);
  lcd_d.setCursor(6,0);
  lcd_d.print(stackWarzenieCounter,DEC);  
  lcd_d.setCursor(6,1);
  lcd_d.print(stackWarzenie[stackWarzenieCounter].czas,DEC);
  lcd_d.setCursor(0xA,1);
  lcd_d.print(stackWarzenie[stackWarzenieCounter].temp,1);
  if(czyPrawo==0){
    lcd_d.setCursor(6,1);
    }
  else {lcd_d.setCursor(0xA,1);}
    
      key=GetKey();  
      if((key=='A') || (key=='B')) {
        czyPrawo = !czyPrawo;     

      }
      else {
        if((key=='C') || (key=='D') || (key=='#') || (key=='*')){}
        else {
          if (czyPrawo==0){ //wpisz czas
            lcd_d.print(key);
            stackWarzenie[stackWarzenieCounter].czas=10*(key-48);
            key=GetKey();
            if ((key=='0') || (key=='1') || (key=='2') || (key=='3') || (key=='4') || (key=='5') || (key=='6') || (key=='7') || (key=='8') || (key=='9')){
              lcd_d.print(key);
              stackWarzenie[stackWarzenieCounter].czas+=(key-48);
              czyPrawo=!czyPrawo;
            }
          }
          else{ //wpisz temperature
            lcd_d.print(key);
            stackWarzenie[stackWarzenieCounter].temp=10*(key-48); // dziesiątki
            key=GetKey();
            if ((key=='0') || (key=='1') || (key=='2') || (key=='3') || (key=='4') || (key=='5') || (key=='6') || (key=='7') || (key=='8') || (key=='9')){
              lcd_d.print(key);
              stackWarzenie[stackWarzenieCounter].temp+=(key-48); // jednostki
              lcd_d.print('.');
              key=GetKey();
              if ((key=='0') || (key=='5')){
                lcd_d.print(key);
                stackWarzenie[stackWarzenieCounter].temp+=0.1*(key-48); // dziesiętne
                czyPrawo=!czyPrawo;
              }
            }
          }
        }
      }
    
    
  }
  key='W';
}
//gotowanie ustawienia
void EKRAN4(){
  lcd_d.blink();
  stackProcesCounter++;
  stackProces[stackProcesCounter]=0;
  stackGotowanieCounter++;
  lcd_d.clear();
  while((key!='*') || (stackGotowanie[stackGotowanieCounter].czas==0)){
    EKRAN(4);
    lcd_d.setCursor(0xB,0);
    lcd_d.print(stackGotowanieCounter,DEC);  
    lcd_d.setCursor(6,1);
    lcd_d.print(stackGotowanie[stackGotowanieCounter].czas,DEC);
    key=GetKey();  
    if((key=='C') || (key=='D') || (key=='#') || (key=='*') || (key=='A') || (key=='B')){}
    else {
      lcd_d.setCursor(6,1);
      lcd_d.print(key);
      stackGotowanie[stackGotowanieCounter].czas=10*(key-48);
      key=GetKey();
      if ((key=='0') || (key=='1') || (key=='2') || (key=='3') || (key=='4') || (key=='5') || (key=='6') || (key=='7') || (key=='8') || (key=='9')){
        lcd_d.print(key);
        stackGotowanie[stackGotowanieCounter].czas+=(key-48);
      }
    }
  }
  key='W';
}
//ALARM!!!!!!!!!!!1
void EKRAN5(){
  lcd_d.noBlink();
  lcd_d.clear();
  EKRAN(5);
  while(1){
    lcd_d.noBacklight();
    delay(500);
    lcd_d.backlight();
    lcd_d.noDisplay();
    delay(125);
    lcd_d.display();
    delay(125);
    lcd_d.noDisplay();
    delay(125);
    lcd_d.display();
    delay(125);
  }
}
// function to print the temperature for a device
float printTemperature(DeviceAddress deviceAddress)
{
  sensors.requestTemperatures();
  float tempC = sensors.getTempC(deviceAddress);
  Serial.print("Temp C: ");
  Serial.print(tempC);
  Serial.println();
  return tempC;
}

void Grzanie(boolean stan){ //high/low grzanie
if(stan==true) {digitalWrite(10,LOW);}
else {digitalWrite(10,HIGH);}
}
void GrzanieZMieszaniem(boolean stan){ //high/low mieszanie

if(stan==true) {
  digitalWrite(11,LOW);
  delay(10000);
  digitalWrite(10,LOW);
}
else {
  digitalWrite(10,HIGH);
  delay(10000);
  digitalWrite(11,HIGH);};

}

void procesGotowania(){
  Grzanie(true);
   EKRAN(4);
  while((printTemperature(insideThermometer)<=100.5)){delay(TIMER_REFRESH);}//99.5
//  Grzanie(LOW);
  //BEEEEEEEEEEEEEEEEEP
   unsigned long time0=millis(),timeNow=time0,timeTill=stackGotowanie[stackGotowanieCounter].czas;
   timeTill*=TIMER_DELAY;
   timeTill+=time0;
   lcd_d.noBlink();
   while(timeTill>=timeNow){
   
    lcd_d.setCursor(0x9,0);
    lcd_d.print(stackGotowanieCounter);
    lcd_d.setCursor(5,1);
    lcd_d.print(stackGotowanie[stackGotowanieCounter].czas,DEC);
    //tresc procedury
    Serial.print("czas: ");
    Serial.println(stackGotowanie[stackGotowanieCounter].czas,DEC);
    delay(TIMER_REFRESH);
    timeNow=millis();
    stackGotowanie[stackGotowanieCounter].czas=byte((timeTill-timeNow)/TIMER_DELAY);
    if (printTemperature(insideThermometer)<=30.5) {Grzanie(true);} //99.5
    if (printTemperature(insideThermometer)>=31.5) {Grzanie(false);}//100.5
    //dodaćregulację mocy grzałek!!
    delay(1);
   }
   Grzanie(false);
}
void procesWarzenia(){
  EKRAN(1);
  if(printTemperature(insideThermometer)<=(stackWarzenie[stackWarzenieCounter].temp-0.5)) {
    GrzanieZMieszaniem(true);
    while(printTemperature(insideThermometer)<=(stackWarzenie[stackWarzenieCounter].temp-0.5)){delay(TIMER_REFRESH);}
    }
  else {
    while(printTemperature(insideThermometer)<=(stackWarzenie[stackWarzenieCounter].temp-0.5)){delay(TIMER_REFRESH);}
        }   
//   Mieszanie(true);

   unsigned long time0=millis(),timeNow=time0,timeTill=stackWarzenie[stackWarzenieCounter].czas;
   timeTill*=TIMER_DELAY;
   timeTill+=time0;
   lcd_d.noBlink();
   while(timeTill>=timeNow){

    lcd_d.setCursor(0x9,0);
    lcd_d.print(stackWarzenieCounter);
    lcd_d.setCursor(5,1);
    lcd_d.print(stackWarzenie[stackWarzenieCounter].czas,DEC);
    lcd_d.setCursor(0xA,1);
    lcd_d.print(stackWarzenie[stackWarzenieCounter].temp,1);
    //tresc procedury
    Serial.print("czas: ");
    Serial.println(stackWarzenie[stackWarzenieCounter].czas,.2);
    delay(TIMER_REFRESH);
    timeNow=millis();
    stackWarzenie[stackWarzenieCounter].czas=byte((timeTill-timeNow)/TIMER_DELAY);
    if (printTemperature(insideThermometer)<=(stackWarzenie[stackWarzenieCounter].temp-0.5)) {GrzanieZMieszaniem(true);}
    if (printTemperature(insideThermometer)>=(stackWarzenie[stackWarzenieCounter].temp+0.5)) {GrzanieZMieszaniem(false);}    
    delay(1);
   }
}
