//Bibliotecas
#include <analogWrite.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include "UbidotsEsp32Mqtt.h"
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include "time.h"


//Definição de pinos e sensor
#define LED 33
#define BUZZER 12
#define DHTPIN 13
#define DHTTYPE DHT22


//Configurações do Ubidots
#define UBIDOTS_TOKEN  ""  
#define WIFI_SSID  ""      
#define WIFI_PASS  ""
#define DEVICE_LABEL  ""  
#define VARIABLE_LABEL_T  "temperatura"  
#define PUBLISH_FREQUENCY_T  600000
#define VARIABLE_LABEL_H  "umidade"  
#define PUBLISH_FREQUENCY_H  601995
Ubidots ubidots(UBIDOTS_TOKEN);


//Configurações de data e hora
const char* ntpServer =  "south-america.pool.ntp.org";
const long  gmtOffset_sec = -3*3600;
const int   daylightOffset_sec = 3600;


//Timers
unsigned long timert;
unsigned long timerh;


//Configurações do LCD
int lcdColumns = 16;
int lcdRows = 4;
int bemv=0;
LiquidCrystal_I2C lcd(0x3F, lcdColumns, lcdRows);


//Configurações do DHT22
DHT_Unified dht(DHTPIN, DHTTYPE);
uint32_t delayMS;


//Função do Ubidots
void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}


//Função de data e hora
void printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  lcd.setCursor(1,0);
  lcd.print(&timeinfo, "%B %d  %H:%M");
}


void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  dht.begin();
  pinMode(BUZZER,OUTPUT);
  pinMode(LED,OUTPUT);
  sensor_t sensor;
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  delayMS = sensor.min_delay / 1000;
  ubidots.connectToWifi(WIFI_SSID, WIFI_PASS);
  ubidots.setCallback(callback);
  ubidots.setup();
  ubidots.reconnect();
  timert = timerh = millis();
}

void loop() {
  delay(delayMS);
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  int temp = event.temperature;
  if(bemv==0)
  {
    lcd.setCursor(3, 1);
    lcd.print("Bem-vindo(a)!");
    delay(1000);
    bemv++;
    lcd.clear();
  }
  printLocalTime();
  lcd.setCursor(1, 2);
  lcd.print("Temperatura: ");
  lcd.print(temp);
  lcd.print((char)223);
  lcd.print("C");
  if (!ubidots.connected())
    ubidots.reconnect();
  if (millis() - timert > PUBLISH_FREQUENCY_T) 
  {
    ubidots.add(VARIABLE_LABEL_T, temp);
    ubidots.publish(DEVICE_LABEL);
    timert = millis();
  }
  ubidots.loop();
  if(temp>25)
  {
    analogWrite(BUZZER,150);
    digitalWrite(LED, HIGH);
  }
  if(temp<12)
  {
    analogWrite(BUZZER,150);
    digitalWrite(LED, HIGH);
  }
  dht.humidity().getEvent(&event);
  int umidade = event.relative_humidity;
  lcd.setCursor(3,3);
  lcd.print("Umidade: ");
  lcd.print(umidade);
  lcd.print("%");
  if (!ubidots.connected())
    ubidots.reconnect();  
  if (millis() - timerh > PUBLISH_FREQUENCY_H) 
  {
    delay(5);
    ubidots.add(VARIABLE_LABEL_H, umidade);
    ubidots.publish(DEVICE_LABEL);
    timerh = timert = millis();
  }
  ubidots.loop();
  if(umidade>70)
  {
    analogWrite(BUZZER,150);
    digitalWrite(LED, HIGH);
  }
  if(umidade<55)
  {
    analogWrite(BUZZER,150);
    digitalWrite(LED, HIGH);
  }
  if(!(temp>25||temp<12||umidade>70||umidade<55))
  {
    analogWrite(BUZZER,0);
    digitalWrite(LED, LOW);
  }
}
