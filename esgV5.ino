// ESG Sensor Data Capture and send to ThingSpeak
// Via GSM or WiFi
// Coded by IT Team KISMEC
// Coder 01 : Override

// board:  LilyGo T-Display
// port : com3
//
// Library: 
// 1. DHT sensor library by Adafruit
// 2. TinyGSM by Volodymyr Shymanskyy
// 3. WiFiManager by tzapu
// 4. Sensirion I2C SCD30
//
// Serial Output
// Open Serial Monitor
// Set baud rate to 115200
//
// pin 4  = DHT22
// pin 34 = MQ135 Air quality sensor
// pin 21 = SDA (SCD30)
// pin 22 = SCL (SCD30)

#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiManager.h>
#include "DHT.h"
#define TINY_GSM_MODEM_SIM800
#include <TinyGsmClient.h>
#include <Wire.h>
#include <SensirionI2cScd30.h>

// --- Select connection type ---
bool useGSM = false;   // Set true to use GSM
bool useWiFi = true;   // Set true to use WiFi

// --- GSM Configuration ---
#define GSM_BAUD 9600
const char* apn = "internet";  
const char* gprsUser = "";     
const char* gprsPass = "";

HardwareSerial SerialAT(1); 
TinyGsm modem(SerialAT);
TinyGsmClient gsmClient(modem);

// --- ThingSpeak API ---
const char* api_key = "MOC3KU8DJ7NXRGWR";
const char* server = "api.thingspeak.com";

// --- DHT Sensor ---
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// --- MQ135 Sensor ---
#define MQ135_PIN 34

// --- SCD30 Sensor ---
SensirionI2cScd30 scd30;
float scd30CO2 = 0, scd30Temp = 0, scd30Hum = 0;

// --- WiFi Setup ---
void setupWiFi() {
  WiFiManager wifiManager;
  if (!wifiManager.autoConnect("AutoConnectAP")) {
    Serial.println("Failed to connect to WiFi. Restarting...");
    ESP.restart();
  }
  Serial.println("WiFi connected.");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

// --- GSM Setup ---
void setupGSM() {
  SerialAT.begin(GSM_BAUD, SERIAL_8N1, 16, 17);
  Serial.println("Initializing GSM...");
  if (!modem.restart()) {
    Serial.println("GSM restart failed!");
    while (true);
  }
  Serial.print("Connecting to GSM network...");
  if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
    Serial.println("GSM GPRS connect failed!");
    while (true);
  }
  Serial.println("GSM connected.");
}

// --- Read Air Quality ---
float readAirQuality() {
  int analogValue = analogRead(MQ135_PIN);
  float airQuality = map(analogValue, 0, 4095, 0, 100); // scale 0-100%
  return airQuality;
}

// --- Send data via WiFi ---
void sendDataWiFi(float dhtTemp, float dhtHum, float mq135Value,
                  float scd30CO2, float scd30Temp, float scd30Hum) {
  HTTPClient http;
  String url = String("http://") + server + "/update?api_key=" + api_key +
               "&field1=" + String(dhtTemp) +
               "&field2=" + String(dhtHum) +
               "&field3=" + String(mq135Value) +
               "&field4=" + String(scd30CO2) +
               "&field5=" + String(scd30Temp) +
               "&field6=" + String(scd30Hum);

  http.begin(url);
  int httpResponseCode = http.GET();
  if (httpResponseCode > 0) {
    Serial.print("WiFi → ThingSpeak HTTP Response code: ");
    Serial.println(httpResponseCode);
  } else {
    Serial.print("WiFi → Error sending data: ");
    Serial.println(httpResponseCode);
  }
  http.end();
}

// --- Send data via GSM ---
void sendDataGSM(float dhtTemp, float dhtHum, float mq135Value,
                 float scd30CO2, float scd30Temp, float scd30Hum) {
  if (!modem.isNetworkConnected()) {
    Serial.println("GSM not connected.");
    return;
  }

  TinyGsmClient& client = gsmClient;

  String url = "/update?api_key=" + String(api_key) +
               "&field1=" + String(dhtTemp) +
               "&field2=" + String(dhtHum) +
               "&field3=" + String(mq135Value) +
               "&field4=" + String(scd30CO2) +
               "&field5=" + String(scd30Temp) +
               "&field6=" + String(scd30Hum);

  if (!client.connect(server, 80)) {
    Serial.println("GSM → Connection to ThingSpeak failed!");
    return;
  }

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + server + "\r\n" +
               "Connection: close\r\n\r\n");

  Serial.println("GSM → Data sent to ThingSpeak");

  unsigned long timeout = millis();
  while (client.connected() && millis() - timeout < 5000) {
    while (client.available()) {
      char c = client.read();
      Serial.write(c);
    }
  }

  client.stop();
}

// --- Setup ---
void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(MQ135_PIN, INPUT);

  // Init SCD30
  Wire.begin(21, 22); 
  scd30.begin(Wire, 0x61);

  int16_t error;
  error = scd30.startPeriodicMeasurement(0); // 0 = no ambient pressure compensation
  if (error) {
    Serial.print("SCD30 start error code: ");
    Serial.println(error);
  } else {
    Serial.println("SCD30 started periodic measurement.");
  }

  if (useWiFi) {
    setupWiFi();
  } else if (useGSM) {
    setupGSM();
  } else {
    Serial.println("No connection method selected!");
    while (true);
  }
}

// --- Main Loop ---
void loop() {
  float dhtTemp = dht.readTemperature();
  float dhtHum = dht.readHumidity();

  if (isnan(dhtTemp) || isnan(dhtHum)) {
    Serial.println("DHT22 read failed!");
    delay(2000);
    return;
  }

  float mq135Value = readAirQuality();

  // Read SCD30
  float co2 = 0, temperature = 0, humidity = 0;
  int16_t error = scd30.readMeasurementData(co2, temperature, humidity);

  if (!error && co2 > 0) { // valid data
    scd30CO2 = co2;
    scd30Temp = temperature;
    scd30Hum = humidity;
  } else if (error) {
    Serial.print("SCD30 read error code: ");
    Serial.println(error);
  }

  // Print data locally
  Serial.println("----- Sensor Data -----");
  Serial.print("DHT22 Temp: "); Serial.println(dhtTemp);
  Serial.print("DHT22 Hum : "); Serial.println(dhtHum);
  Serial.print("MQ135 Value: "); Serial.println(mq135Value);
  Serial.print("SCD30 CO2 : "); Serial.println(scd30CO2);
  Serial.print("SCD30 Temp: "); Serial.println(scd30Temp);
  Serial.print("SCD30 Hum : "); Serial.println(scd30Hum);

  // Send to ThingSpeak
  if (useWiFi && WiFi.status() == WL_CONNECTED) {
    sendDataWiFi(dhtTemp, dhtHum, mq135Value, scd30CO2, scd30Temp, scd30Hum);
  } else if (useGSM) {
    sendDataGSM(dhtTemp, dhtHum, mq135Value, scd30CO2, scd30Temp, scd30Hum);
  } else {
    Serial.println("No active connection method.");
  }

  delay(60000); // Wait 60 seconds before next reading
}

