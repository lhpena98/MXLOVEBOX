#include "settings.h"

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <Servo.h>
#include <TZ.h>
#include "SH1106Wire.h"
#include <Wire.h>

const int fetchIntervalMillis = _fetchIntervalSeconds * 1000;
const char* ssid = _ssid;
const char* password = _password;
const String url = _url;
unsigned long previousMillis = 0; 
const long interval = 500;  

SH1106Wire oled(0x3c, SDA, SCL);
Servo myservo;
//int lightValueThreshold = 30;
int lightValueThresholdmorning = 50;
int lightValueThresholdday = 150;
int lightValueThresholdnight = 38;
int pos = 90;
int increment = -1;
int lightValue;
String line;
String mode;
char idSaved = '0';
bool wasRead = true;

void drawMessage(const String& message) {
  Serial.print("Drawing message....");
  oled.clear();
  // differentiat between *t*ext and image messages
  if (mode[0] == 't') {
    oled.drawStringMaxWidth(0, 0, 128, message);
  } 
  else if (mode[0] == 'b') {
    for (int i = 0; i <= message.length(); i++) {
      int x = i % 129;
      int y = i / 129;

      if (message[i] == '1') {
        oled.setPixel(x, y);
      }
    }
  }
  else if (mode[0] == 'm') {
    int mes = message.toInt();
    lightValue = analogRead(0);
    String lv = String(lightValue);
    lightValueThresholdmorning = mes;
    String thr = String(lightValueThresholdmorning);
    Serial.print("Light Sensor morning value now = ");
    Serial.println(lightValueThresholdmorning);
    oled.drawStringMaxWidth(0, 0, 128, "Value Updated to:");
    oled.drawStringMaxWidth(84, 0, 128, thr);
    oled.drawStringMaxWidth(0, 20, 128, "Current measurement:");
    oled.drawStringMaxWidth(113, 20, 128, lv);
    }
  else if (mode[0] == 'd') {
    int mes = message.toInt();
    lightValue = analogRead(0);
    String lv = String(lightValue);
    lightValueThresholdday = mes;
    String thr = String(lightValueThresholdday);
    Serial.print("Light Sensor midday value now = ");
    Serial.println(lightValueThresholdday);
    oled.drawStringMaxWidth(0, 0, 128, "Value Updated to:");
    oled.drawStringMaxWidth(84, 0, 128, thr);
    oled.drawStringMaxWidth(0, 20, 128, "Current measurement:");
    oled.drawStringMaxWidth(103, 20, 128, lv);
    }
   else if (mode[0] == 'n') {
    int mes = message.toInt();
    lightValue = analogRead(0);
    String lv = String(lightValue);
    lightValueThresholdnight = mes;
    String thr = String(lightValueThresholdnight);
    Serial.print("Light Sensor night value now = ");
    Serial.println(lightValueThresholdnight);
    oled.drawStringMaxWidth(0, 0, 128, "Value Updated to:");
    oled.drawStringMaxWidth(84, 0, 128, thr);
    oled.drawStringMaxWidth(0, 20, 128, "Current measurement:");
    oled.drawStringMaxWidth(103, 20, 128, lv);
    }
  oled.display();
  Serial.println("done.");
}

// Set time via NTP, as required for x.509 validation
void setClock() {
  // General C time functions: https://en.wikipedia.org/wiki/C_date_and_time_functions
  // Pick a timezone from https://github.com/esp8266/Arduino/blob/master/cores/esp8266/TZ.h
  configTime(TZ_Europe_Paris, "pool.ntp.org");

  Serial.print("Waiting for NTP time sync: ");
  time_t now = time(nullptr);
  // 1546300800 = 01/01/2019 @ 12:00am (UTC)
  while (now < 1546300800) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println();
  time_t nowUtc = mktime(gmtime(&now));

  Serial.print("Local time: ");
  Serial.print(ctime(&now));
  Serial.print("UTC:        ");
  Serial.print(ctime(&nowUtc));
  Serial.println();
}

void wifiConnect() {
  Serial.printf("Connecting to WiFi '%s'...", ssid);
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
  }
  Serial.print("..done. IP ");
  Serial.println(WiFi.localIP());
}

void getGistMessage() {
  Serial.println("Fetching message...");
  const int httpsPort = 443;
  const char* host = "gist.githubusercontent.com";

  BearSSL::WiFiClientSecure client;
  BearSSL::X509List cert(digicertRootCert);
  client.setTrustAnchors(&cert);
  if (!client.connect(host, httpsPort)) {
    Serial.println("Failed to connect to GitHub");
    return;
  }

  // add current millis as a cache-busting means
  client.print(String("GET ") + url + "?" + millis() + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: ESP8266\r\n" +
               "Connection: close\r\n\r\n");

  while (client.connected()) {
    String temp = client.readStringUntil('\n');
    if (temp == "\r") {
      break;
    }
  }
  String id = client.readStringUntil('\n');
  Serial.printf("\tid: '%s', last processed id: '%c'\n", id.c_str(), idSaved);
  if (id[0] != idSaved) { // new message
    wasRead = 0;
    idSaved = id[0];
    lightValue = analogRead(0);
    Serial.print("Waiting for NTP time sync: ");
    time_t now = time(nullptr);
    while (now < 1546300800) {
      delay(500);
      Serial.print(".");
      now = time(nullptr);
    }
    char *currenttime = ctime(&now);
    currenttime += 11;
    currenttime[8] = '\0';
    memmove(&currenttime[2], &currenttime[2 + 1], strlen(currenttime) - 2);
    memmove(&currenttime[4], &currenttime[4 + 1], strlen(currenttime) - 4);
    int currenttimeint = atoi(currenttime);
    Serial.print(currenttime);
    Serial.println();
    if ((currenttimeint > 20001) && (currenttimeint < 90000)) { //Morning time
      while(lightValue <= lightValueThresholdmorning){
        oled.clear();
        oled.drawString(20, 30, "NEW MESSAGE :O");
        oled.display();
        lightValue = analogRead(0);
        unsigned long currentMillis = millis();
        spinServo();
        if (currentMillis - previousMillis >= interval) {
          previousMillis = currentMillis;
        }
        oled.clear();
      }
    }
    else if ((currenttimeint > 90001) && (currenttimeint < 190000)) { //Day time
      while(lightValue <= lightValueThresholdday){
        oled.clear();
        oled.drawString(20, 30, "NEW MESSAGE :O");
        oled.display();
        lightValue = analogRead(0);
        unsigned long currentMillis = millis();
        spinServo();
        if (currentMillis - previousMillis >= interval) {
          previousMillis = currentMillis;
        }
        oled.clear();
      }
    }
    else if ((currenttimeint > 190001) && (currenttimeint < 235959)) { //Night time
      while(lightValue <= lightValueThresholdnight){
        oled.clear();
        oled.drawString(20, 30, "NEW MESSAGE :O");
        oled.display();
        lightValue = analogRead(0);
        unsigned long currentMillis = millis();
        spinServo();
        if (currentMillis - previousMillis >= interval) {
          previousMillis = currentMillis;
        }
        oled.clear();
      }
    }
    else if ((currenttimeint > 0) && (currenttimeint < 20000)) { //Night time
      while(lightValue <= lightValueThresholdnight){
        oled.clear();
        oled.drawString(20, 30, "NEW MESSAGE :O");
        oled.display();
        lightValue = analogRead(0);
        unsigned long currentMillis = millis();
        spinServo();
        if (currentMillis - previousMillis >= interval) {
          previousMillis = currentMillis;
        }
        oled.clear();
      }
    }
    mode = client.readStringUntil('\n');
    Serial.println("\tmode: " + mode);
    line = client.readStringUntil(0);
    Serial.println("\tmessage: " + line);
    drawMessage(line);
  } else {
    Serial.println("\t-> message id wasn't updated");
  }
}

void spinServo() {
  myservo.write(pos);
  delay(50);    // wait 50ms to turn servo

  if (pos == 75 || pos == 105) { // 75°-105° range
    increment *= -1;
  }
  pos += increment;
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\n\n");

  Serial.print("Attaching servo...");
  myservo.attach(16);       // Servo an D0
  Serial.println("done.");

  Serial.print("Initializing display...");
  oled.init();
  oled.flipScreenVertically();
  oled.setColor(WHITE);
  oled.setTextAlignment(TEXT_ALIGN_LEFT);
  oled.setFont(ArialMT_Plain_10);

  oled.clear();
  oled.drawString(5, 10, "<3 HOLA CACEROLA <3");
  oled.drawString(28, 20, "BOOTING UP...");
  oled.drawString(25, 30, "PLEASE WAIT ;)");
  oled.drawString(70, 50, "P.D. T <3 M");
  oled.display();
  Serial.println("done.");

  wifiConnect();
  setClock();

  Serial.printf("Initial state: last processed id '%c' %s read.\n", idSaved, (wasRead ? "was" : "wasn't"));
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    wifiConnect();
    setClock();
  }
  if (wasRead) {
    getGistMessage();
  }
  Serial.print("Waiting for NTP time sync: ");
  time_t now = time(nullptr);
  // 1546300800 = 01/01/2019 @ 12:00am (UTC)
  while (now < 1546300800) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  char *currenttime = ctime(&now);
  currenttime += 11;
  currenttime[8] = '\0';
  memmove(&currenttime[2], &currenttime[2 + 1], strlen(currenttime) - 2);
  memmove(&currenttime[4], &currenttime[4 + 1], strlen(currenttime) - 4);
  int currenttimeint = atoi(currenttime);
  Serial.print(currenttimeint);
  Serial.println();
  while (!wasRead) {
    yield();
    spinServo();
    if ((currenttimeint > 20001) && (currenttimeint < 90000)) { //Morning time
      lightValue = analogRead(0);
      if (lightValue > lightValueThresholdmorning) {
        Serial.printf("Analog read value (LDR) %d above threshold of morning %d -> consider message read.\n", lightValue, lightValueThresholdmorning);
        wasRead = 1;
      } 
    }
    else if ((currenttimeint > 90001) && (currenttimeint < 190000)) { //Day time
      lightValue = analogRead(0);
      if (lightValue > lightValueThresholdday) {
        Serial.printf("Analog read value (LDR) %d above threshold of day %d -> consider message read.\n", lightValue, lightValueThresholdday);
        wasRead = 1;
      } 
    }
    else if ((currenttimeint > 190001) && (currenttimeint < 235959)) { //Night time
      lightValue = analogRead(0);
      if (lightValue > lightValueThresholdnight) {
        Serial.printf("Analog read value (LDR) %d above threshold of night %d -> consider message read.\n", lightValue, lightValueThresholdnight);
        wasRead = 1;
      } 
    }
    else if ((currenttimeint > 0) && (currenttimeint < 20000)) { //Night time
      lightValue = analogRead(0);
      if (lightValue > lightValueThresholdnight) {
        Serial.printf("Analog read value (LDR) %d above threshold of night %d -> consider message read.\n", lightValue, lightValueThresholdnight);
        wasRead = 1;
      } 
    }
  }
  delay(fetchIntervalMillis);
}
