// ============================================================================
//  neuer Aufbau Waveshare RP2040 One
//  
//  Softwarebereinigung begonnen am 300824
//  Arduino IDE settings:
//  board: Waveshare RP2040 One
//  Tempoanpassung 190825 3 Grundtempi  !!!!!
//  Schaltung _VbandKeyer_Skizze.pdf
//  Space Zeile 95/96
//  https://www.arduino.cc/reference/en/language/functions/usb/keyboard/keyboardmodifiers/
//  programmer: none
// 
// ============================================================================

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "Keyboard.h"
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
//------Pinbelegung Waveshare One---------------
int PKT      = 14   ;// GPIO   Paddle Punkt
int STR      = 15   ;// GPIO   Paddle Strich
int M_Ton    =   8  ;// GPIO    Mithörton Piezo
int LED_Cw   =  16  ;// Zielboard

int SpeedPin = A0;  // AD-Wandler eingang(Poti)
//------------------------------

//------Pinbelegung---Ende----------------------------




//-----Zähler--------------------------------------------------
int32_t time_d     = 0   ;//zur Speed ermittlung
int16_t time_w     = 0   ;
int32_t zeit_d     = 0   ;
int zeit           = 0;      // für die Pausenzeit
int i,ii,iii = 0;  // allgemeine Zähler
//---------------------------------

uint16_t Hz        = 850 ;// frequenz Mithörton                              
int16_t wpm100     = 10  ;// Startzeit in msec ca 500 BpM - 200 Bpm
int16_t wpm100_d   = 10  ;// Deltazeit in usec
int16_t wpm60      = 20  ;// Startzeit in msec ca 250 BpM - 125 Bpm
int16_t wpm60_d    = 10  ;// Deltazeit in usec
int16_t wpm40      = 40  ;// Startzeit in msec ca 130Bpm -30 BpM
int16_t wpm40_d    = 40  ;// Deltazeit in usec

//---------------------------------

int8_t Merker = 0;  //Merkerbyte
int PKT_m     = 0;  // Merker Punkt 1= nicht gedrückt
int STR_m     = 1;  // Merker Strich
int Zz_m      = 2;  // Merker für Zeichenzähler  4
int Wo_ab_m   = 3;  // Merker Wortabstand  8               
//int lambicBU  = 4;  // Merker lambicMode Bu-erkennung 
//int lambicB_W = 5;  // Merker lambicMode Wortabstand nur einmal!! 
int8_t Speed = 0; // verryHighSpeed = 1 HighSpeed =2 LowSpeed

uint8_t Z_z      = 0;  //Zeichenzaehler
uint8_t Cod      = 0;  //Codemuster
uint16_t BuCoZ   = 0;  // zur schnelle Ausgabe 170724
uint16_t BuCoZ_e = 0;  //core2

#define NUMPIXELS 1  // Zielboard
Adafruit_NeoPixel pixels(NUMPIXELS, LED_Cw, NEO_GRB + NEO_KHZ800);// Zielboard

//---------------------------------
void LED_aus(){
  Keyboard.releaseAll();
   pixels.setPixelColor(0, pixels.Color(0, 0, 0));  pixels.show();
   noTone(M_Ton);// Zielboard
  
}     
void LED_ein(){
 Keyboard.press(KEY_RIGHT_CTRL);
  pixels.setPixelColor(0, pixels.Color(0, 0, 20)); pixels.show();
  tone(M_Ton,Hz);// Zielboard
}  

void Bu_a(){
          BuCoZ = Z_z<<8;              //170524
          BuCoZ = BuCoZ |Cod;         //170524
         // Serial.println(BuCoZ);
          rp2040.fifo.push(BuCoZ);    //170724
        Z_z = 0;
        Cod   = 0 ;//#Codemuster
        BuCoZ =0;//170724
       bitClear(Merker,Zz_m);                     // zum schnelleren kodieren
         if(Merker ==0){Pause();}// Zeichenabstand   310724  drei Punkte Abstand
         if(Merker ==0){Pause();}// Zeichenabstand   310724  ==
         if(Merker ==0){Pause();}// Zeichenabstand   310724  Wortende
       //if((Merker ==0)and digitalRead(noSpace) ==HIGH)rp2040.fifo.push(5555);//170524  ###100924### ICOM == Space
       if((Merker ==0) )rp2040.fifo.push(5555);// ICOM == Space
    
}

//---------------------------------

//---------------------------------
void Pause_w(){
  zeit=(analogRead(SpeedPin));
  zeit = 1048-zeit;  //Poti drehrichtung umkehr
  
  //Serial.println(zeit);  //#################
  delay(time_w); 
   for(ii=0; ii<= zeit;ii++){  
      if ((digitalRead(PKT) == LOW) ) {bitSet(Merker, PKT_m);}
      if ((digitalRead(STR) == LOW) )  {bitSet(Merker, STR_m);}
      delayMicroseconds(time_d);
    }
} 
void Pause(){
  zeit=(analogRead(SpeedPin));
  zeit = 1048-zeit;  //Poti DrehrichtungumkehrC Q TE S T NT KI K Q MET 
  //Serial.println(zeit_d);
  delay(time_w); 
   for(ii=0; ii<= zeit;ii++){  
      if ((digitalRead(PKT) == LOW) ) {bitSet(Merker, PKT_m);}
      if ((digitalRead(STR) == LOW) ) {bitSet(Merker, STR_m);}
      delayMicroseconds(time_d);// verz in usec!!
    }
} 
//----------------------------------------------------------------
void Strich(){
  LED_ein();
      Pause(); Pause(); Pause();        // Strich  
  LED_aus();
  Pause();
  Cod = Cod |(1 << Z_z);                  //# Bit setzen 1=Strich
  Z_z =Z_z+1;                             // Zeichenzähler
  bitSet(Merker,Zz_m);                    // zum schnelleren kodieren                                     
  bitClear(Merker,STR_m);                // immer letze Aktion
  bitClear(Merker,Wo_ab_m);               // immer letze Aktion Wortabstand löschen
}
//------------------------------------------------------------------
void Punkt(){
 LED_ein();
      Pause();                               // Punkt
 LED_aus();
  Pause();
  Z_z =Z_z+1;                                // Zeichenzähler
  bitSet(Merker,Zz_m);                       // zum schnelleren kodieren
  bitClear(Merker,PKT_m);                    // immer letze Aktion Punktmerker löschen
  bitClear(Merker,Wo_ab_m);                  // immer letze Aktion Wortabstand löschen
}

//---------------------------------
void Version(){
   Keyboard.print("  Waveshare HighSpeedKezer 180825  60");
   Keyboard.write(42);
   Keyboard.print("C");
   Keyboard.write(40);
   Keyboard.print("DL1ZAR  ");
  }

//---------------------------------
void Bu_a(int16_t BuCoZ_e){
  //  Serial.println(BuCoZ_e);
          switch (BuCoZ_e){
            case 514:
              Keyboard.write('A');break;
            case 1025:
              Keyboard.write('B');break;
            case 1029:
              Keyboard.write('C');break;
            case 769:
              Keyboard.write('D');break;
            case 256:
              Keyboard.write('E');break;
            case 1028:
              Keyboard.write('F');break;
            case 771:
              Keyboard.write('G');break;
            case 1024:
              Keyboard.write('H');break;
            case 512:
              Keyboard.write('I');break;
            case 1038:
              Keyboard.write('J');break;
            case 773:
              Keyboard.write('K');break;
            case 1026:
              Keyboard.write('L');break;
            case 515:
              Keyboard.write('M');break;
            case 513:
              Keyboard.write('N');break;
            case 775:
              Keyboard.write('O');break;
            case 1030:
              Keyboard.write('P');break;
            case 1035:
              Keyboard.write('Q');break;
            case 770:
              Keyboard.write('R');break;
            case 768:
              Keyboard.write('S');break;
            case 257:
              Keyboard.write('T');break;
            case 772:
              Keyboard.write('U');break;
            case 1032:
              Keyboard.write('V');break;
            case 774:
              Keyboard.write('W');break;
            case 1033:
              Keyboard.write('X');break;
            case 1037:
              Keyboard.write('Z');break;//keyboardlayout
            case 1027:
              Keyboard.write('Y');break;//keyboardlayout
            case 1310:
              Keyboard.write('1');break;
             case 1308:
              Keyboard.write('2');break;
            case 1304:
              Keyboard.write('3');break;
            case 1296:
              Keyboard.write('4');break;
            case 1280:
              Keyboard.write('5');break;
            case 1281:
              Keyboard.write('6');break;
            case 1283:
              Keyboard.write('7');break;
            case 1287:
              Keyboard.write('8');break;
            case 1295:
              Keyboard.write('9');break;
             case 1311:
              Keyboard.write('0');break;
             case 1289:
              Keyboard.write(38);break;//("/")
            case 1578:
              Keyboard.write('.');break;
            case 1587:
              Keyboard.write(',');break;
            case 1548:
              Keyboard.write('_');break;// ? Keyboardlayout
            case 1557:
              Keyboard.write(';');break;
            case 1589:
              Keyboard.write('!');break;
            case 1293:
              Keyboard.write(42);break;// ( Keyboardlayout
            case 1581:
              Keyboard.write(40);break;// ) Keyboardlayout
            case 1543:
              Keyboard.write(62);break;// ::
            case 1297:
              Keyboard.write(41);break;// 
            case 2503: // OSO
              Version();break;//
            case 5555:
                  Keyboard.write(' ');break;//
            break;//
            }
  // bitClear(Merker,lambicBU);// Test 230824
  }
        

//###################################
//-------Hauptschleifen----------
//###################################
void setup() {
  pinMode(PKT,        INPUT_PULLUP);
  pinMode(STR,        INPUT_PULLUP);
  pinMode(M_Ton,  OUTPUT);
  pinMode(LED_Cw, OUTPUT);
    //Serial.begin(115200);
    Speed = 0;  // VeryHighSpeed
   if ((digitalRead(PKT) == LOW)) Speed = 1;  // HighSpeed
   if ((digitalRead(STR) == LOW)) Speed = 2;  // NormalSpeed
}
//###################################

void loop() {
 if (Speed == 1){ time_w = wpm40;time_d = wpm40_d;}
 if (Speed == 2){time_w = wpm60;time_d = wpm60_d;}
 if (Speed == 0){time_w = wpm100;time_d = wpm100_d;}
 
    if (bitRead(Merker,PKT_m)){ Punkt();}
    if (bitRead(Merker,STR_m)){Strich();}
          if ((digitalRead(PKT) == LOW)) bitSet(Merker, PKT_m);
          if ((digitalRead(STR) == LOW))  bitSet(Merker, STR_m);   
    if((Merker ==4)){Bu_a();}  
 

  //Serial.println(digitalRead(Cw_deu));
}


//###################################
void setup1() {
 // pinMode(LED_BUILTIN, OUTPUT);
  Keyboard.begin();
          //Serial.begin(115200); // Zum Testen
}
//-#########-Ausgabe Remote##########-----
 void loop1() {
  if (rp2040.fifo.available()>0) {
 //   digitalWrite(LED_BUILTIN, HIGH);
    BuCoZ_e = rp2040.fifo.pop();
    Bu_a(BuCoZ_e);
  //  digitalWrite(LED_BUILTIN, LOW); //aus
    }
  
}
