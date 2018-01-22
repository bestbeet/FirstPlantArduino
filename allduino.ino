//======================= Include ===============================//
#include <ESP8266WiFi.h>
#include <Scheduler.h>
#include <FirebaseArduino.h>
#include "DHT.h"

//===================== Config Firebase =========================//
#define FIREBASE_HOST "firstplant-project.firebaseio.com"
#define FIREBASE_AUTH "492QWTQJHReTcF4ZOFfgjjpNHZQSRv8SCKbzvwff"
String ID = "/007";

//==================== Config connect WiFi ======================//
const char* ssid = "Connectify-";
const char* password = "warongrat";

//======================= Intinial ===============================//
#define DHTPIN D4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
int soilPin = D5;
int lightPin = D2;
int rainPin = D3;
byte sensorInterrupt = D1;
byte sensorPin       = D1;
volatile int pulseCount = 0;
int Calc;
int warning;
//============================ Function ==========================//
void readFlow() {
  int Fvolume =  Firebase.getInt(ID + "/Fertilization/Volume/");
  sei();
  delay (1000);
  cli();
  Calc = ((pulseCount / 7.5) / 60) * 1000;

  Serial.println(Calc);
  if (Calc < Fvolume)
    ton_f();
  else {
    Calc = 0;
    pulseCount = 0;
    ton_f();
    Firebase.setString(ID + "/Fertilization/Notification/", "Enable");
    Firebase.setString(ID + "/Fertilization/Status/", "Disable");
    delay(100);
    Firebase.setString(ID + "/Fertilization/Notification/", "Disable");
  }
}

void readFloww() {
  int Fvolume =  Firebase.getInt(ID + "/AutoFertilization/Volume/");
  sei();
  delay (1000);
  cli();
  Calc = ((pulseCount / 7.5) / 60) * 1000;

  Serial.println(Calc);
  if (Calc < Fvolume)
    ton_f();
  else {
    Calc = 0;
    pulseCount = 0;
    ton_f();
    Firebase.setString(ID + "/AutoFertilization/Notification/", "Enable");
    Firebase.setString(ID + "/AutoFertilization/Status/", "Disable");
    delay(100);
    Firebase.setString(ID + "/AutoFertilization/Notification/", "Disable");
  }
}

void pulseCounter(){
  pulseCount++;}

void weather() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  float heat_index = dht.computeHeatIndex(temperature, humidity, false);
  int moisture = digitalRead(soilPin);
  int rain = digitalRead(rainPin);
  int light = digitalRead(lightPin);

  Firebase.setFloat(ID + "/Weather/Heatindex/", heat_index);
  Firebase.setFloat(ID + "/Weather/Humidity/", humidity);
  Firebase.setFloat(ID + "/Weather/Temperature/", temperature);
  if (light == 1 && rain == 1 && moisture == 1) { // dry cool
    Firebase.setInt(ID + "/Weather/Sunlight/", 0);
    Firebase.setInt(ID + "/Weather/Raindrop/", 0);
    Firebase.setInt(ID + "/AutoIrrigation/Warning/", 0);
    warning = 0;
  }
  else if (light == 0 || rain == 0 || moisture == 0) { // wet hot
    Firebase.setInt(ID + "/AutoIrrigation/Warning/", 1);
    warning = 1;
    if (light == 0 && rain == 0) {
      Firebase.setInt(ID + "/Weather/Sunlight/", 1);
      Firebase.setInt(ID + "/Weather/Raindrop/", 1);
    } else if (light == 0 && rain == 1)
      Firebase.setInt(ID + "/Weather/Sunlight/", 1);
    else if (rain == 0 && light == 1)
      Firebase.setInt(ID + "/Weather/Raindrop/", 1);
  }
}

//========================== task =============================//
class weatherTask : public Task {
  protected:
    void loop()  {
      weather();
      delay(1000);
    }
} weather_task;

class offilineTask : public Task {
  protected:
    void loop()  {
      if (WiFi.status() != WL_CONNECTED) {
        if (warning == 0)
          ton_i();
        else
          tof();
      }
    }
} offline_task;

class startTask : public Task {
  protected:
    void loop()  {
      String AFstat = Firebase.getString(ID + "/AutoFertilization/Status/");
      String AIstat = Firebase.getString(ID + "/AutoIrrigation/Status/");
      int AIwarning = Firebase.getInt(ID + "/AutoIrrigation/Warning/");
      String Fstat = Firebase.getString(ID + "/Fertilization/Status/");
      String Istat = Firebase.getString(ID + "/Irrigation/Status/");

      if (AFstat == "Enable")
        readFloww();
      else if (AIstat == "Enable") {
        if (AIwarning == 0)
          ton_i();
        else
          tof();
      }
      else if (Fstat == "Enable")
        readFlow();
      else if (Istat == "Enable") {
        int Itime =  Firebase.getInt(ID + "/Irrigation/Time/") * 1000;
        ton_i();
        delay(Itime);
        tof();
        Firebase.setString(ID + "/Irrigation/Notification/", "Enable");
        Firebase.setString(ID + "/Irrigation/Status/", "Disable");
        delay(100);
        Firebase.setString(ID + "/Irrigation/Notification/", "Disable");
      }
      else
        tof();
    }
} start_task;

//============================== Main =============================//
void setup() {
  Serial.begin(115200);
  pinMode(D6, OUTPUT);// Fertilizer
  pinMode(D7, OUTPUT);// Water
  pinMode(D8, OUTPUT);// Pump
  pinMode(sensorPin, INPUT);
  digitalWrite(sensorPin, HIGH);
  dht.begin();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  attachInterrupt(sensorInterrupt, pulseCounter, RISING);
  Scheduler.start(&weather_task);
  Scheduler.start(&offline_task);
  Scheduler.start(&start_task);
  Scheduler.begin();
}

void loop() {
}

void ton_i () {
  digitalWrite(D7, 1);
  digitalWrite(D8, 1);
}

void ton_f () {
  digitalWrite(D6, 1);
  digitalWrite(D7, 1);
  digitalWrite(D8, 1);
}

void tof () {
  digitalWrite(D6, 0);
  digitalWrite(D7, 0);
  digitalWrite(D8, 0);
}





