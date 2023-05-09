#include <Nextion.h>
#include "DHT.h"
#include <Wire.h>
#include <radio.h>
#include <TEA5767.h>
#include <Arduino.h>
#include <SoftwareSerial.h>

SoftwareSerial nextion(10,11);
Nextion myNextion(nextion,9600);

#define DHTPIN 2
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

int var_aux=0;

#define FIX_BAND RADIO_BAND_FM 
TEA5767 radio;

//DHT11 resistencia 4.7-10 kOhmnios: https://www.youtube.com/watch?v=bJFvmG5IlSA&ab_channel=codigoelectronica

int inc = 10;
int FREQ = 10270;
String em = "";

uint8_t test1;
byte test2;

boolean bajarCanal;
boolean subirCanal;

boolean en1;
int cont;

boolean pg3, pg2, pg1;

void setup() {
  
  Serial.begin(57600);
  dht.begin();
  myNextion.init();
  Wire.begin();
  radio.init();

  myNextion.setComponentText("page3.emisora","102.7");

  sensor_measurements();

  subirCanal = false;
  bajarCanal = false;

  en1 = false;
  cont = 0;

  pg3, pg2, pg1 = false;
  
  radio.setMute(false);

  radio.setBandFrequency(FIX_BAND, FREQ);
  radio.setMono(false);
  escribirEmisora();
}

void loop() {

    sensor_measurements();

    pg3 = myNextion.getComponentValue("pg2b1");
    pg2 = myNextion.getComponentValue("pg3b0");
    pg1 = myNextion.getComponentValue("pg1b0");
    //page3 = pg3 true y pg2 false y pg1 true

    while(!(pg3==true && pg2==false && pg1==true)){
      radio.setMute(true);
      pg3 = myNextion.getComponentValue("pg2b1");
      pg2 = myNextion.getComponentValue("pg3b0");
      pg1 = myNextion.getComponentValue("pg1b0");
      sensor_measurements();
      var_aux=1;
      escribirEmisora();
    }

      en1 = myNextion.getComponentValue("onoff");
      if(en1==true)cont++;
    
      if(en1==true && cont%2!=0){
        delay(200);
        encenderRadio();
      }else if(en1==true && cont%2==0){
        delay(200);
        radio.setMute(true);
      }

    subirCanal = myNextion.getComponentValue("page3.canalmas");
    if(subirCanal==true)cMasPushCallBack();

    bajarCanal = myNextion.getComponentValue("page3.canalmenos");
    if(bajarCanal==true)cMenosPushCallBack();

    sensor_measurements();
}

void sensor_measurements(){
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  float hin = dht.computeHeatIndex(temp,hum,false);

  myNextion.setComponentText("page2.t02",String(temp));
  myNextion.setComponentText("page2.t12",String(hin));
}

void cMenosPushCallBack(){
  //Bajar canal -> Incrementos de 0.10
  bajarCanal=false;
  if(FREQ==8750){
    FREQ=10800;
  }
  FREQ = FREQ - inc;
  var_aux=1;
  escribirEmisora();
  radio.setBandFrequency(FIX_BAND, FREQ);
}

void cMasPushCallBack(){
  //Subir canal
  subirCanal=false;
  if(FREQ==10800){
    FREQ=8750;
  }
  FREQ = FREQ + inc;
  var_aux=1;
  escribirEmisora();
  radio.setBandFrequency(FIX_BAND, FREQ);
}

void escribirEmisora(){
  if(var_aux==1){
    em="";
    int len = 4;
    
    if(FREQ>9999) len = 5;

    char buf[len];
  
    itoa(FREQ,buf,10);

  if(len == 4){
    em += buf[0];
    em += buf[1];
    em += '.';
    em += buf[2];
    em += buf[3];
  } else if (len == 5){
    em += buf[0];
    em += buf[1];
    em += buf[2];
    em += '.';
    em += buf[3];
    em += buf[4];
  } else{
    em = "Resintonice";
  }
  
  myNextion.setComponentText("page3.emisora",em);
  
  }
  var_aux=0;
}

void encenderRadio(){
  radio.setMute(false);

  radio.setBandFrequency(FIX_BAND, FREQ);
  radio.setMono(false);
  escribirEmisora();

  //Wire.endTransmission();
}
