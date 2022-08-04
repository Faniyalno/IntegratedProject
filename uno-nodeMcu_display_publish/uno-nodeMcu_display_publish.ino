#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"
// Provide the token generation process info.
#include "addons/TokenHelper.h"

//set Firebase
#define DATABASE_URL "INSERT_DATABASE_URL"
#define API_KEY "INSERT_API_KEY"

// Insert Authorized Email and Corresponding Password
#define USER_EMAIL "INSERT_EMAIL"
#define USER_PASSWORD "INSERT_PASS"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
FirebaseJson json;

// Variable to save USER UID
String uid;

//set wifi
#define WIFI_SSID "WIFI_NAME"
#define WIFI_PASS  "WIFI_PASS"

//set ntp
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

#define LED D1 // set led to D1 in NodeMcu
String arrData[4]; //4 data sensor
unsigned long epochTime;
String databasePath;
String timePath = "/timestamp";
String timePathF = "/formattedTime";
String ldrPath = "/LDR";
String HumidityPath = "/Humidity";
String RainPath = "/Rain";
String TempPath = "/Temperature";
String parentPath; //Parent Node to be updated in every loop
unsigned long interval = 60000, current;
unsigned long previous = 0;

unsigned long getTime() {
  /*to set offsite is to use second not milisecond*/
  timeClient.setTimeOffset(25200); //offset to UTC +7
  timeClient.update();
  unsigned long now = timeClient.getEpochTime();
  return now;
}

void readdata() {
  current = millis();
  if (Firebase.ready()) {
    if (current - previous >= interval){
    //read data in nodemcu
    //var for store the data
    String data = "";
    while (Serial.available() > 0) {
      //take data from serial uno
      data += char(Serial.read());
    }
    //trim the data
    data.trim();
    //show all the data
    //Serial.println(data);
    int index = 0;
    for (int i = 0; i < data.length(); i++) {
      char delimiter = '#'; //divider
      if (data[i] != delimiter)
        arrData[index] += data[i];
      else
        index++;
    }

    int chInt = arrData[1].toInt(); // change the data type on array [1] Rain
    // from String to Int
    Serial.println("");
    if (chInt < 600) {
      Serial.println("Its Raining....");
      digitalWrite(LED, HIGH);
    }
    else {
      digitalWrite(LED, LOW);
    }
    epochTime = getTime();
    String formatted = timeClient.getFormattedTime();
    Serial.print("Unix Time: ");
    Serial.println(epochTime);
    Serial.println("Formatted Time: " + formatted);
    Serial.println("LDR: " + arrData[0]);
    Serial.println("Rain: " + arrData[1]);
    Serial.println("Temperature (C): " + arrData[2]);
    Serial.println("Humidity: " + arrData[3]);

    json.set(ldrPath,arrData[0]);
    json.set(RainPath,arrData[1]);
    json.set(TempPath,arrData[2]);
    json.set(HumidityPath,arrData[3]);
    parentPath = databasePath+"/"+String(epochTime);
    json.set(timePath, String(epochTime));
    json.set(timePathF, formatted);
    Serial.printf("Set json %s\n", Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json)?"ok":fbdo.errorReason().c_str());
    
    arrData[0] = "";
    arrData[1] = "";
    arrData[2] = "";
    arrData[3] = "";
    previous = current;
    }
  }
  else {
    Serial.println("FAILED");
    Serial.println("REASON: " + fbdo.errorReason());
  }
}

void setup() {
  Serial.begin(9600);
  timeClient.begin();
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  pinMode(LED, OUTPUT);

  // Assign the user sign in credentials
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  // Assign the api key (required)
  config.api_key = API_KEY;

  //assign the token database secret & RTDB URL
  //config.signer.tokens.legacy_token = DATABASE_SECRET;
  config.database_url = DATABASE_URL;
  
  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  
  // Assign the maximum retry of token generation
  config.max_token_generation_retry = 5;
  

  Firebase.reconnectWiFi(true);
  Firebase.begin(&config, &auth);
  
  // Getting the user UID might take a few seconds
  Serial.println("Getting User UID");
  while ((auth.token.uid) == "") {
    Serial.print('.');
    delay(1000);
  }

  // Print user UID
  uid = auth.token.uid.c_str();
  Serial.print("User UID: ");
  Serial.println(uid);

  // Update database path
  databasePath = "/UsersData/" + uid + "/readings";
}

void loop() {
  readdata();
}
