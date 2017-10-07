//======================= Include ===============================//
#include <ESP8266WiFi.h>
#include <Scheduler.h>
#include <FirebaseArduino.h>

//===================== Config Firebase =========================//
#define FIREBASE_HOST "firstplant-project.firebaseio.com"
#define FIREBASE_AUTH "492QWTQJHReTcF4ZOFfgjjpNHZQSRv8SCKbzvwff"

//==================== Config connect WiFi ======================//
const char* ssid = "Connectify-";
const char* password = "warongrat";
int moisture = 0, light = 0, rain = 0;
int soilPin = D1;
int lightPin = D2;
int rainPin = D3;

void aio_rt() {
  int moisture = digitalRead(soilPin);
  int rain = digitalRead(rainPin);
  int light = digitalRead(lightPin);
  if (light == 1 && rain == 1 && moisture == 1)  // dry cool
    Firebase.setInt("/AutoIrrigation/Warning/", 1);
  else if (light == 0 || rain == 0 || moisture == 0)  // wet hot
    Firebase.setInt("/AutoIrrigation/Warning/", 0);
}

void ton_i () {
  digitalWrite(D1, 1);
  digitalWrite(D3, 1);
}

void ton_f () {
  digitalWrite(D1, 1);
  digitalWrite(D3, 1);
  digitalWrite(D2, 1);
  digitalWrite(D4, 1);
}

void tof () {
  digitalWrite(D1, 0);
  digitalWrite(D3, 0);
  digitalWrite(D2, 0);
  digitalWrite(D4, 0);
}

void setup() {
  pinMode(D2, OUTPUT);
  pinMode(D4, OUTPUT);
  pinMode(D1, OUTPUT);
  pinMode(D3, OUTPUT);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}

void loop() {
  String AFstat = Firebase.getString("/AutoFertilization/Status/");
  int AIstat = Firebase.getInt("/AutoIrrigation/Warning/");
  String Fstat = Firebase.getString("/Fertilization/Status/");
  String Istat = Firebase.getString("/Irrigation/Status/");

  if (AFstat == "Enable") {
    int AFvolume =  Firebase.getInt("/AutoFertilization/Volume/") * 1000;
    ton_f();
    delay(AFvolume);
    tof();
    Firebase.setString("/AutoFertilization/Alert/", "Disable");
    Firebase.setString("/AutoFertilization/Status/", "Disable");
    Firebase.setString("/AutoFertilization/Notification/", "Enable");
    delay(100);
    Firebase.setString("/AutoFertilization/Notification/", "Disable");

  }
  else if (AIstat == 1)
    ton_i();

  else if (Fstat == "Enable") {
    int Fvolume =  Firebase.getInt("/Fertilization/Volume/") * 1000;
    ton_f();
    delay(Fvolume);
    tof();
    Firebase.setString("/Fertilization/Notification/", "Enable");
    Firebase.setString("/Fertilization/Status/", "Disable");
    delay(100);
    Firebase.setString("/Fertilization/Notification/", "Disable");
  }
  else if (Istat == "Enable") {
    int Itime =  Firebase.getInt("/Irrigation/Time/") * 1000;
    ton_i();
    delay(Itime);
    tof();
    Firebase.setString("/Irrigation/Notification/", "Enable");
    Firebase.setString("/Irrigation/Status/", "Disable");
    delay(100);
    Firebase.setString("/Irrigation/Notification/", "Disable");
  }
  else
    tof();
}


