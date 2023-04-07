#include "DHT.h"
#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <LiquidCrystal_I2C.h>
#include <string.h>
#include <TimeLib.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

const char* ntpServerName = "id.pool.ntp.org";
const int timeZone = 0;

WiFiUDP Udp;
NTPClient timeClient(Udp, ntpServerName, timeZone * 3600, 60000);

LiquidCrystal_I2C lcd(0x27,16,2);
#define LDR A0
#define DHTPIN D7
#define DHTTYPE DHT22
#define GREEN D0
#define BLUE D3
#define RED D5
#define trigPin D6
#define echoPin D8
#define WIFI_SSID "ESP8266"
#define WIFI_PASSWORD "12345678"
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  lcd.init();
  lcd.clear();         
  lcd.backlight();
  dht.begin();
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);
  pinMode(RED, OUTPUT);
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP().toString());
  Serial.println();
  setSyncProvider(&getNtpTime);
  setSyncInterval(300);
}

void loop() {
  // Mendapatkan Nilai Dari Sensor
  int h = dht.readHumidity();
  int t = dht.readTemperature();
  int lux = analogRead(LDR);

  Serial.print(F("\nHumidity      : "));
  Serial.print(h);
  Serial.print(F("%"));
  Serial.print(F("\nTemperature   : "));
  Serial.print(t);
  Serial.print(F("°C"));
  delay(1000);

  // Menampilkan Suhu LCD
  lcd.home();
  lcd.clear();
  lcd.setCursor(0,0);   //Set cursor to character 0 on line 0
  lcd.print(F("Celcius    : "));
  lcd.print(t); 
  lcd.print(F("C"));
  int f = (t * 1.8) + 32;
  lcd.setCursor(0,1);   //Move cursor to character 0 on line 1
  lcd.print(F("Fahrenheit : "));
  lcd.print(f); 
  lcd.print(F("F"));
  delay(1000);

  // Menampilkan Waktu LCD
  lcd.home();
  lcd.clear();
  if (timeStatus() == timeSet) {
    lcd.setCursor(16/2 - 4/2, 0); // Set the cursor to the center of the first row
    lcd.print("TIME");
    lcd.setCursor(4,1);
    int jam = hour();
    jam = jam + 7;
    printDigits(jam);
    lcd.print(":");
    printDigits(minute());
    lcd.print(":");
    printDigits(second());
  }
  delay(1000);

  // Sensor Ultrasonik
  long duration, jarak;
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  jarak = duration * 0.034 / 2;
  Serial.print(F("\nDistance      : "));
  Serial.print(jarak);
  Serial.print(F(" cm"));
  delay(1000);

  // Sensor LDR
  Serial.print(F("\nIllumination  : "));
  Serial.print(lux);
  Serial.print(F(" lux"));
  Serial.println();
  delay(1000);

  // Klasifikasi
  if(t > 18 && t <=24){
    //dingin
    digitalWrite(BLUE, HIGH);
    delay(100);
    digitalWrite(BLUE, LOW);
    delay(100);
  }
  else if(t <= 18){
    //dingin
    digitalWrite(GREEN, HIGH);
    delay(100);
    digitalWrite(GREEN, LOW);
    delay(100);
  }
  else if(t >= 30){
    //dingin
    digitalWrite(RED, HIGH);
    delay(100);
    digitalWrite(RED, LOW);
    delay(100);
  }
}

time_t getNtpTime() {
  byte packetBuffer[NTP_PACKET_SIZE];
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  packetBuffer[0] = 0b11100011;
  packetBuffer[1] = 0;
  packetBuffer[2] = 6;
  packetBuffer[3] = 0xEC;
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  Udp.begin(123);
  Udp.beginPacket(ntpServerName, 123);
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
  delay(1000);
  int cb = Udp.parsePacket();
  if (!cb) {
    Serial.println("NTP no packet yet");
    return 0;
  }
  Udp.read(packetBuffer, NTP_PACKET_SIZE);
  unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
  unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
  unsigned long secsSince1900 = highWord << 16 | lowWord;
  return secsSince1900 - 2208988800UL + timeZone * 3600;
}

void printDigits(int digits) {
  if (digits < 10) {
  lcd.print("0");
  }
  lcd.print(digits);
}