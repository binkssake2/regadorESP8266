#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "RPG_WIFI";
const char* password = "chegouops4";

void setup () { 

  Serial.begin(115200);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    
    delay(1000);
    Serial.print("Connecting..");
  
  }

}

void loop() {

if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status
  
  HTTPClient http;  //Declare an object of class HTTPClient
  
  http.begin("http://worldtimeapi.org/api/ip");  //Specify request destination
  int httpCode = http.GET();                                                                  //Send the request
  
  if (httpCode > 0) { //Check the returning code
    
   String payload = http.getString();   //Get the request response payload
    
   const size_t capacity = JSON_OBJECT_SIZE(15) + 300;
   DynamicJsonDocument doc(capacity);
  
  //const char* json = "{\"week_number\":8,\"utc_offset\":\"-03:00\",\"utc_datetime\":\"2020-02-18T20:50:23.142655+00:00\",\"unixtime\":1582059023,\"timezone\":\"America/Sao_Paulo\",\"raw_offset\":-10800,\"dst_until\":null,\"dst_offset\":0,\"dst_from\":null,\"dst\":false,\"day_of_year\":49,\"day_of_week\":2,\"datetime\":\"2020-02-18T17:50:23.142655-03:00\",\"client_ip\":\"177.208.17.194\",\"abbreviation\":\"-03\"}";
  
  deserializeJson(doc, payload);
  
  int week_number = doc["week_number"]; // 8
  const char* utc_offset = doc["utc_offset"]; // "-03:00"
  const char* utc_datetime = doc["utc_datetime"]; // "2020-02-18T20:50:23.142655+00:00"
  long unixtime = doc["unixtime"]; // 1582059023
  const char* timezone = doc["timezone"]; // "America/Sao_Paulo"
  int raw_offset = doc["raw_offset"]; // -10800
  int dst_offset = doc["dst_offset"]; // 0
  bool dst = doc["dst"]; // false
  int day_of_year = doc["day_of_year"]; // 49
  int day_of_week = doc["day_of_week"]; // 2
  const char* datetime = doc["datetime"]; // "2020-02-18T17:50:23.142655-03:00"
  const char* client_ip = doc["client_ip"]; // "177.208.17.194"
  const char* abbreviation = doc["abbreviation"]; // "-03"

  
  Serial.print("DateTime:"); 
  Serial.println(datetime);
  
    
  }
  
  http.end();   //Close connection

}

delay(5000);    //Send a request every 30 seconds

}
