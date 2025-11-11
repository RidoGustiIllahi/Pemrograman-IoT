#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT_U.h>

// =========================
// KONFIGURASI WIFI
// =========================
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// =========================
// KONFIGURASI MQTT
// =========================
const char* mqttServer = "broker.mqttdashboard.com";
const int mqttPort = 1883;
const char* clientID = "55e294eb-d614-4d17-8688-5118eb3600c7";

const char* topicSuhu = "rido/suhu";
const char* topicKelembapan = "rido/kelembapan";
const char* topicLux = "rido/lux";
const char* topicLed = "rido/led";  

// =========================
// OBJEK WIFI DAN MQTT
// =========================
WiFiClient espClient;
PubSubClient client(espClient);

// =========================
// KONFIGURASI DHT22
// =========================
#define DHTPIN 12
#define DHTTYPE DHT22
DHT_Unified dht(DHTPIN, DHTTYPE);

// =========================
// KONFIGURASI LDR
// =========================
#define LDR_PIN 34  // pin ADC LDR di ESP32 (gunakan pin analog)

// =========================
// KONFIGURASI LED
// =========================
#define LED_PIN 14

// =========================
// VARIABEL WAKTU
// =========================
unsigned long previousMillis = 0;
const long interval = 2000; // kirim data setiap 2 detik

// =========================
// FUNGSI WIFI SETUP
// =========================
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

// =========================
// CALLBACK PESAN MASUK
// =========================
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);

  if (String(topic) == topicLed) {
    if (message == "on") {
      digitalWrite(LED_PIN, HIGH);
      Serial.println("LED ON");
    } else if (message == "off") {
      digitalWrite(LED_PIN, LOW);
      Serial.println("LED OFF");
    }
  }
}

// =========================
// RECONNECT MQTT
// =========================
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(clientID)) {
      Serial.println("connected");
      client.subscribe(topicLed);
      Serial.print("Subscribed to topic: ");
      Serial.println(topicLed);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

// =========================
// SETUP
// =========================
void setup() {
  Serial.begin(115200);
  setup_wifi();

  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);

  pinMode(LED_PIN, OUTPUT);
  pinMode(LDR_PIN, INPUT);

  dht.begin();
}

// =========================
// LOOP UTAMA
// =========================
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // ===== Baca suhu dan kelembapan =====
    sensors_event_t event;
    dht.temperature().getEvent(&event);
    float suhu = event.temperature;

    dht.humidity().getEvent(&event);
    float kelembapan = event.relative_humidity;

    // ===== Baca LDR =====
    int ldrValue = analogRead(LDR_PIN);
    float lux = map(ldrValue, 0, 4095, 0, 1000); 

    // ===== Kirim ke broker =====
    if (!isnan(suhu)) {
      String msgSuhu = String(suhu, 2);
      client.publish(topicSuhu, msgSuhu.c_str());
      Serial.print("Published suhu: ");
      Serial.println(msgSuhu);
    }

    if (!isnan(kelembapan)) {
      String msgKelembapan = String(kelembapan, 2);
      client.publish(topicKelembapan, msgKelembapan.c_str());
      Serial.print("Published kelembapan: ");
      Serial.println(msgKelembapan);
    }

    String msgLux = String(lux, 2);
    client.publish(topicLux, msgLux.c_str());
    Serial.print("Published lux: ");
    Serial.println(msgLux);
  }
}
