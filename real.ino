//======================= Include ===============================//
#include <ESP8266WiFi.h>
#include <Scheduler.h>
#include <FirebaseArduino.h>
#include "DHT.h"

//===================== Config Firebase =========================//
#define FIREBASE_HOST "firstplant-project.firebaseio.com"
#define FIREBASE_AUTH "492QWTQJHReTcF4ZOFfgjjpNHZQSRv8SCKbzvwff"

//==================== Config connect WiFi ======================//
const char* ssid = "Connectify-";
const char* password = "warongrat";

//======================= Intinial ===============================//
#define DHTPIN D4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
int moisture = 0, light = 0, rain = 0;
float humidity = 0, temperature = 0, heat_index = 0;
int soilPin = D1;
int lightPin = D2;
int rainPin = D3;

int red = D5;
int yellow = D6;
int green = D7;

//============================ Function ==========================//
void aio() {
  int moisture = digitalRead(soilPin);
  int rain = digitalRead(rainPin);
  int light = digitalRead(lightPin);
  if (light == 1 && rain == 1 && moisture == 1) { // dry cool
    digitalWrite(green, 1);
    digitalWrite(red, 0);

  } else if (light == 0 || rain == 0 || moisture == 0) { // wet hot
    digitalWrite(green, 0);
    digitalWrite(red, 1);
    if (light == 0 && rain == 0) {
      Firebase.setInt("/Weather/Sunlight/", 1);
      Firebase.setInt("/Weather/Raindrop/", 1);
    } else if (light == 0 && rain == 1)
      Firebase.setInt("/Weather/Sunlight/", 1);
    else if (rain == 0 && light == 1)
      Firebase.setInt("/Weather/Raindrop/", 1);
  }
}

void weather() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  float heat_index = dht.computeHeatIndex(temperature, humidity, false);
  Firebase.setFloat("/Weather/Heatindex/", heat_index);
  Firebase.setFloat("/Weather/Humidity/", humidity);
  Firebase.setFloat("/Weather/Temperature/", temperature);
}

void merry_weather() {
  int moisture = digitalRead(soilPin);
  int rain = digitalRead(rainPin);
  int light = digitalRead(lightPin);
  if (light == 1 && rain == 1 && moisture == 1) { // dry cool
    Firebase.setInt("/Weather/Sunlight/", 0);
    Firebase.setInt("/Weather/Raindrop/", 0);
  }
  else if (light == 0 || rain == 0 || moisture == 0) { // wet hot
    if (light == 0 && rain == 0) {
      Firebase.setInt("/Weather/Sunlight/", 1);
      Firebase.setInt("/Weather/Raindrop/", 1);
    } else if (light == 0 && rain == 1)
      Firebase.setInt("/Weather/Sunlight/", 1);
    else if (rain == 0 && light == 1)
      Firebase.setInt("/Weather/Raindrop/", 1);
  }
}

//========================== task =============================//
class StartTask : public Task {
  public:
    void loop()  {


      String AFstat = Firebase.getString("/AutoFertilization/Status/");
      String AIstat = Firebase.getString("/AutoIrrigation/Status/");
      String Fstat = Firebase.getString("/Fertilization/Status/");
      String Istat = Firebase.getString("/Irrigation/Status/");

      if (AFstat == "Enable") {
        int AFvolume =  Firebase.getInt("/AutoFertilization/Volume/") * 1000;
        digitalWrite(green, 1);
        digitalWrite(yellow, 1);
        digitalWrite(red, 0);
        delay(AFvolume);
        digitalWrite(green, 0);
        digitalWrite(yellow, 0);
        digitalWrite(red, 1);
        Firebase.setString("/AutoFertilization/Alert/", "Disable");
        Firebase.setString("/AutoFertilization/Status/", "Disable");
        Firebase.setString("/AutoFertilization/Notification/", "Enable");
        delay(100);
        Firebase.setString("/AutoFertilization/Notification/", "Disable");

      }
      else if (AIstat == "Enable")
        aio();

      else if (Fstat == "Enable") {
        int Fvolume =  Firebase.getInt("/Fertilization/Volume/") * 1000;
        digitalWrite(green, 1);
        digitalWrite(yellow, 1);
        digitalWrite(red, 0);
        delay(Fvolume);
        digitalWrite(green, 0);
        digitalWrite(yellow, 0);
        digitalWrite(red, 1);
        Firebase.setString("/Fertilization/Notification/", "Enable");
        Firebase.setString("/Fertilization/Status/", "Disable");
        delay(100);
        Firebase.setString("/Fertilization/Notification/", "Disable");
      }
      else if (Istat == "Enable") {
        int Itime =  Firebase.getInt("/Irrigation/Time/") * 1000;
        digitalWrite(green, 1);
        digitalWrite(yellow, 1);
        digitalWrite(red, 0);
        delay(Itime);
        digitalWrite(green, 0);
        digitalWrite(yellow, 0);
        digitalWrite(red, 1);
        Firebase.setString("/Irrigation/Notification/", "Enable");
        Firebase.setString("/Irrigation/Status/", "Disable");
        delay(100);
        Firebase.setString("/Irrigation/Notification/", "Disable");
      }
      else {
        digitalWrite(green, 0);
        digitalWrite(yellow, 0);
        digitalWrite(red, 1);
      }
    }
} start_task;

class weatherTask : public Task {
  protected:
    void loop()  {
      weather();
      delay(15000);
    }
} weather_task;

class merryTask : public Task {
  protected:
    void loop()  {
      merry_weather();
      delay(1000);
    }
} merry_task;

//============================== Main =============================//
void setup() {
  Serial.begin(115200);
  pinMode(red, OUTPUT);
  pinMode(yellow, OUTPUT);
  pinMode(green, OUTPUT);
  dht.begin();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Scheduler.start(&start_task);
  Scheduler.start(&merry_task);
  Scheduler.start(&weather_task);
  Scheduler.begin();
}

void loop() {
}



