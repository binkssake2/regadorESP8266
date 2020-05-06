#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <vector>
#include <Wire.h>
#include <DS3232RTC.h>
#include <LiquidCrystal.h>
#define BUTTON_RESET_WIFI D3
const int rele = D5;
const int RS = D0, EN = D4, d4 = D6, d5 = D7, d6 = D8, d7 = D9;
LiquidCrystal lcd(RS, EN, d4, d5, d6, d7);

DS3232RTC RTC;

DynamicJsonDocument db(11424);

WiFiServer server(80);

unsigned long millis_db = 0;
unsigned long millis_rtc = 0;
unsigned long millis_db_display = 0;
unsigned long millis_lcd_clear = 0;
int k = 0;

ICACHE_RAM_ATTR void reset_wifi() {
  WiFiManager wifiManager;
  wifiManager.resetSettings();
}

void precisa0(int digito) {
  if (digito < 10) {
    lcd.print('0');
  }
}

void reconnectWIFI() {
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Não foi possivel conectar ao servidor WEB ou banco de dados");
    lcd.setCursor(0, 0);
    lcd.print("Caiu, try recon");
    WiFi.reconnect();
    delay(3000);
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Reconectou");
      lcd.setCursor(0, 0);
      lcd.print("Reconectado!!!!");
      delay(3000);
    }
  }
}


void lcd_print_hora(int hora, int minuto, int segundo) {
  //LCD PRINT DA HORA
  lcd.setCursor(0, 1);
  lcd.print("Hora:");
  precisa0(hora);
  lcd.print(hora);
  lcd.print(":");
  precisa0(minuto);
  lcd.print(minuto);
  lcd.print(":");
  precisa0(segundo);
  lcd.print(segundo);
  delay(1000);
}

int lcd_print_db(int len_db, DynamicJsonDocument db, int k) {
  lcd.setCursor(0, 0);
  lcd.print("I ");
  precisa0(int(db[k][0][0]));
  lcd.print(int(db[k][0][0]));
  lcd.print(":");
  precisa0(int(db[k][0][1]));
  lcd.print(int(db[k][0][1]));
  lcd.print(" ");
  lcd.print("D ");
  precisa0(int(db[k][1][0]));
  lcd.print(int(db[k][1][0]));
  lcd.print(":");
  precisa0(int(db[k][1][1]));
  lcd.print(int(db[k][1][1]));
  millis_db_display = millis();
  return k + 1;
}


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
      int minuto = 10 * (int(infos_date[14]) - 48) + (int(infos_date[15]) - 48);
      int segundo = 10 * (int(infos_date[17] - 48)) + (int(infos_date[18]) - 48);

      setTime(hora, minuto, segundo, dia, mes, ano);
      RTC.set(now());
      lcd.setCursor(0, 0);
      lcd.print("TEMPO NA API");
      lcd.setCursor(0, 1);
      lcd.print("Hora:");
      lcd.print(hour());
      lcd.print(":");
      lcd.print(minute());
      lcd.print(":");
      lcd.print(second());
      lcd.print("   ");
      delay(5000);
      lcd.clear();

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
  pinMode(BUTTON_RESET_WIFI, INPUT_PULLUP);

  digitalWrite(rele, LOW);
  pinMode(rele, OUTPUT);

  Wire.begin();

  Serial.begin(115200);

  setSyncProvider(RTC.get);   // the function to get the time from the RTC
  if (timeStatus() != timeSet)
    Serial.println("Unable to sync with the RTC");
  else
    Serial.println("RTC has set the system time");

  lcd.begin(16, 2);
  lcd.print("Config Wi-Fi...");

  WiFiManager wifiManager;

  wifiManager.setTimeout(120);

  if (!wifiManager.autoConnect("Regador de Flor")) {
    Serial.println("Falhou ao conectar e deu timeout");
    delay(3000);
    ESP.reset();
    delay(5000);
  }

  Serial.println("conectouu :)");
  server.begin();

  add_att_RTC();

  delay(2000);

  db = add_att_db(db);
  lcd.setCursor(0, 0);
  lcd.print("!!!Conectado!!!");
  lcd.setCursor(0, 1);
  lcd.print("                ");

  Serial.println("setup over");
  delay(3000);

  attachInterrupt(digitalPinToInterrupt(BUTTON_RESET_WIFI), reset_wifi, FALLING);

}

void loop() {

  reconnectWIFI();

  int len_db = db.size();

  lcd_print_hora(hour(), minute(), second());

  if (millis() - millis_db_display >= 5000) {
    k = lcd_print_db(len_db, db, k);
  }
  if (k == len_db) {
    k = 0;
  }

  ///////////att tempo RTC de 24 em 24 horas e banco de dados de 5 em 5 min///////////
  if (millis() - millis_rtc >= 300000) { //24 horas    = 86400000, 1 hora = 3600000
    add_att_RTC();
    millis_rtc = millis();
    //lcd.clear();
    //lcd.print("att RTC         ");
    delay(2000);
    Serial.println("att rtc");
    lcd.clear();
  }

  if (millis() - millis_db >= 120000) { //24 horas    = 86400000, 1 hora = 3600000, 300000 5 min
    db = add_att_db(db);
    millis_db = millis();
    lcd.setCursor(0, 0);
    lcd.print("att DB          ");
    delay(3000);
    Serial.println("att db");
    k = 0;
    len_db = db.size();
  }




  //ROTINA DE TEMPO DO REGADOR PARA FAZER O PROCESSO PROPRIAMENTE DITO
  if (len_db > 0) {
    for (int i = 0; i < len_db; i++) {
      if ((int(db[i][0][0])) == hour()) { //*******************horas
        if ((int(db[i][0][1])) == minute()) { //****************minutos
          Serial.println("ENTROU");

          int tempo_segundos = 60 * int((db[i][1][1])) + 3600 * int ((db[i][1][0]));

          //lcd.clear();
          lcd.print("Rega por ");
          lcd.print(tempo_segundos);
          lcd.print("seg");
          lcd.setCursor(0, 1);
          lcd.print("Inicio:");
          lcd.print(int(db[i][0][0]));
          lcd.print(":");
          lcd.print(int(db[i][0][1]));
          lcd.print("     ");


          Serial.println(tempo_segundos);
          int millis_rega = millis();

          while (millis() - millis_rega < tempo_segundos * 1000) {
            digitalWrite(rele, HIGH);
            Serial.println(millis() - millis_rega);
            Serial.println(digitalRead(rele));
            delay(1000);
          }
          Serial.println("Saiu");
          digitalWrite(rele, LOW);
          millis_rega = millis();
          i = 0;
        }
      }
    }
  }
}
