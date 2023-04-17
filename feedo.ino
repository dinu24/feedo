
// ---------Libraries---------------
#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "credentials.h"

// ---------AddOns------------------
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

//---------Defines-----------------
#define temp_read 4

//---------Fascism-----------------
FirebaseData fbdo;
FirebaseData stream;
FirebaseAuth auth;
FirebaseConfig config;
OneWire oneWire(temp_read);
DallasTemperature sensors(&oneWire);

//---------Godown----------
unsigned long sendDataPrevMillis = 0;
bool signupOK = false;
volatile bool dataChanged = false;
int count = 0;

//---------On-demand-volunteer-----
void streamCallback(FirebaseStream data) {
  Serial.printf("stream path, %s\nevent path, %s\ndata type, %s\nevent type, %s\n\n",
                data.streamPath().c_str(),
                data.dataPath().c_str(),
                data.dataType().c_str(),
                data.eventType().c_str());
  printResult(data);
  Serial.println();
  Serial.printf("Received stream payload size: %d (Max. %d)\n\n", data.payloadLength(), data.maxPayloadLength());
  dataChanged = true;
}

//---------Referee-----------------
void streamTimeoutCallback(bool timeout) {
  if (timeout)
    Serial.println("stream timed out, resuming...\n");

  if (!stream.httpConnected())
    Serial.printf("error code: %d, reason: %s\n\n", stream.httpCode(), stream.errorReason().c_str());
}

//-----------Alpha----------------
void setup() {
  //-------Appetizer--------
  Serial.begin(115200);
  sensors.begin();
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("ok");
    signupOK = true;
  } else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  if (!Firebase.RTDB.beginStream(&stream, "base/params"))
    Serial.printf("stream begin error, %s\n\n", stream.errorReason().c_str());

  Firebase.RTDB.setStreamCallback(&stream, streamCallback, streamTimeoutCallback);  // change checker
}

void loop() {

  //--------Sensor--------------
  sensors.requestTemperatures();
  float temperatureC = sensors.getTempCByIndex(0);
  float temperatureF = sensors.getTempFByIndex(0);
  Serial.print(temperatureC);
  Serial.println("ºC");
  Serial.print(temperatureF);
  Serial.println("ºF");

  //------Start-feast-if-table-ready------------------
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();

//-------------push-temp-------------
    if (!(Firebase.RTDB.setFloat(&fbdo, "base/tempC", temperatureC))){
      Serial.println("TempC append failed.");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.setFloat(&fbdo, "base/pH", 0.01 + random(6, 8))) {
      Serial.println("PASSED");
      // Serial.println("PATH: " + fbdo.dataPath());
      // Serial.println("TYPE: " + fbdo.dataType());
    } else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
  }
  // if (Firebase.RTDB.getInt(&fbdo, "/test/int")) {
  //   if (fbdo.dataType() == "int") {
  //     int intValue = fbdo.intData();
  //     Serial.println(intValue);
  //   }
  // } else {
  //   Serial.println(fbdo.errorReason());
  // }
  // if (Firebase.RTDB.getFloat(&fbdo, "/test/float")) {
  //   if (fbdo.dataType() == "float") {
  //     float floatValue = fbdo.floatData();
  //     Serial.println(floatValue);
  //   }
  // } else {
  //   Serial.println(fbdo.errorReason());
  // }
}
