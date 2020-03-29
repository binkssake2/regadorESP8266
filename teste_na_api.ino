#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <vector>
#include <Wire.h>       //I2C library
#include <RtcDS3231.h>  //RTC library
#include <LiquidCrystal.h>

const int RS = D3, EN = D5, d4 = D6, d5 = D7, d6 = D8, d7 = D9;
LiquidCrystal lcd(RS, EN, d4, d5, d6, d7);

RtcDS3231<TwoWire> rtcObject(Wire);

DynamicJsonDocument db(11424);

WiFiServer server(80);

unsigned long millis_db = 0;
unsigned long millis_rtc = 0;
unsigned long millis_db_display = 0;
int k = 0;

void add_att_RTC() {
  if (WiFi.status() == WL_CONNECTED) { //checa conexão wifi

    HTTPClient http;

    http.begin("http://worldtimeapi.org/api/ip");  //pra onde
    int httpCode = http.GET();

    if (httpCode > 0) { //chega o retorno

      String payload = http.getString();   //da o get e bota no payload

      const size_t capacity = JSON_OBJECT_SIZE(15) + 300;
      DynamicJsonDocument doc(capacity);

      deserializeJson(doc, payload);

      const char* datetime = doc["datetime"]; // "2020-02-18T17:50:23.142655-03:00"

      String infos_date = datetime;

      int ano = 1000 * (int(infos_date[0]) - 48) + 100 * (int(infos_date[1]) - 48) + 10 * (int(infos_date[2]) - 48) + (int(infos_date[3]) - 48);
      int mes = 10 * (int(infos_date[5]) - 48) + (int(infos_date[6]) - 48);
      int dia = 10 * (int(infos_date[8]) - 48) + (int(infos_date[9]) - 48);
      int hora = 10 * (int(infos_date[11]) - 48) + (int(infos_date[12]) - 48);
      int minuto = 10 * (int(infos_date[14]) - 48) + ((infos_date[15]) - 48);

      RtcDateTime currentTime = RtcDateTime(ano, mes, dia, hora, minuto, 0); 
      rtcObject.SetDateTime(currentTime);

    }

    http.end();

  }
}


DynamicJsonDocument add_att_db(DynamicJsonDocument db) {

  const size_t capacity = 11424;
  DynamicJsonDocument doc(capacity);
  if (WiFi.status() == WL_CONNECTED) {

    HTTPClient http;

    http.begin("http://binkssake.pythonanywhere.com/db");
    int httpCode = http.GET();

    if (httpCode > 0) {

      String payload = http.getString();

      deserializeJson(doc, payload);
      Serial.println(payload);
    }

    http.end();
    return doc;

  }
  return db;
}



void setup() {
  pinMode(D4, OUTPUT);
  digitalWrite(D4, LOW);
  Wire.begin();
  rtcObject.Begin();
  
  Serial.begin(115200);

  lcd.begin(16, 2);
  lcd.print("Config Wi-Fi...");

  WiFiManager wifiManager;

  wifiManager.autoConnect("Regador de Fror");

  Serial.println("conectouu :)");
  server.begin();
  add_att_RTC();
  db = add_att_db(db);
  
  lcd.clear();
  lcd.print("Conectado!");
  
  Serial.println("setup over");
  delay(3000);

}

void loop() {
  RtcDateTime currentTime = rtcObject.GetDateTime();
  int len_db = db.size();

  //SERIAL DEBUG
  Serial.print(currentTime.Hour());
  Serial.print(":");
  Serial.print(currentTime.Minute());
  Serial.println(" ");
  delay(1000);  

  //LCD PRINT DA HORA
  lcd.setCursor(0, 1);
  lcd.print("Hora:");
  lcd.print(currentTime.Hour());
  lcd.print(":");
  lcd.print(currentTime.Minute());
  lcd.print(":");
  lcd.print(currentTime.Second());

  //LCD PRINT DB
  
  if (millis() - millis_db_display >= 5000) {
    lcd.clear();
    lcd.print("I ");
    lcd.print(int(db[k][0][0]));
    lcd.print(":");
    lcd.print(int(db[k][0][1]));
    lcd.print(" ");
    lcd.print("Dur ");
    lcd.print(int(db[k][1][0]));
    lcd.print(":");
    lcd.print(int(db[k][1][1]));
    millis_db_display = millis();
    k++;
  }
    if(k == len_db) {
      k = 0;
    }
  



  
  ///////////att tempo RTC de 24 em 24 horas e banco de dados de 5 em 5 min///////////
  if (millis() - millis_rtc >= 86400000) { //24 horas    = 86400000, 1 hora = 3600000
    add_att_RTC();
    millis_rtc = millis();
    Serial.println("att rtc");
  }
  
   if (millis() - millis_db >= 300000) { //24 horas    = 86400000, 1 hora = 3600000
    db = add_att_db(db);
    millis_db = millis();
    Serial.println("att db");
  }

  
  

  //ROTINA DE TEMPO DO REGADOR PARA FAZER O PROCESSO PROPRIAMENTE DITO
  for (int i = 0; i < len_db; i++) {
   if ((int(db[i][0][0])) == currentTime.Hour()) { //*******************horas
      if ((int(db[i][0][1])) == currentTime.Minute()) { //****************minutos
        Serial.println("ENTROU");
        
        int tempo_segundos = 60 * int((db[i][1][1])) + 3600 * int ((db[i][1][0]));
        Serial.println(tempo_segundos);
        int millis_rega = millis();
        
        while (millis() - millis_rega < tempo_segundos * 1000) {
          digitalWrite(D4, HIGH);
          Serial.println(millis() - millis_rega);
          Serial.println(digitalRead(D4));
          delay(1000);
        }
        Serial.println("Saiu");
        digitalWrite(D4,LOW);
        millis_rega = millis();
      }
    }
  }
}
