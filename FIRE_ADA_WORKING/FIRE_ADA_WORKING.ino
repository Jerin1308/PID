#include <ESP8266WiFi.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#include <Firebase_ESP_Client.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define WIFI_SSID "A0596"
#define WIFI_PASS "12345678"

#define AIO_SERVER "io.adafruit.com"
#define AIO_PORT   1883
#define AIO_USER   "YOUR_ADAFRUIT_IO_USERNAME_HERE"  // Replace with your Adafruit IO username
#define AIO_KEY    "YOUR_ADAFRUIT_IO_KEY_HERE"

#define API_KEY "YOUR_FIREBASE_API_KEY_HERE"
#define DATABASE_URL "YOUR_FIREBASE_DATABASE_URL_HERE"

#define WATER_SENSOR D0
#define CHEM_SENSOR  D3
#define VOLT_PIN     A0

#define PUMP_1 D6
#define PUMP_2 D7

#define gsm Serial1   // GSM uses Serial1

LiquidCrystal_I2C lcd(0x27, 16, 2);

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_PORT, AIO_USER, AIO_KEY);

Adafruit_MQTT_Subscribe modeFeed(&mqtt, AIO_USER "/feeds/mode");
Adafruit_MQTT_Publish waterFeed(&mqtt, AIO_USER "/feeds/water");
Adafruit_MQTT_Publish chemFeed(&mqtt, AIO_USER "/feeds/chemical");
Adafruit_MQTT_Publish voltFeed(&mqtt, AIO_USER "/feeds/voltage");

unsigned long lastUpload = 0;
unsigned long fbTimer = 0;
unsigned long lcdTimer = 0;
unsigned long lastSubCheck = 0;
const unsigned long subInterval = 100;

volatile bool modeChanged = false;
volatile char newMode = 'X';

bool alertSent = false;
char lastMode = 'L', lastCmd = 'X';

void tokenStatusCallback(TokenInfo info) {}

float R1 = 30020.0, R2 = 7501.0;

float readVoltage() {
  int raw = analogRead(VOLT_PIN);
  float vout = (raw * 3.3) / 1024.0;
  return vout / (R2 / (R1 + R2));
}

void sendAlert(const char *number, const char *msg) {
  gsm.println(F("AT"));
  delay(1000);

  gsm.println(F("AT+CMGF=1"));
  delay(1000);

  gsm.print(F("AT+CMGS=\"+91"));
  gsm.print(number);
  gsm.println(F("\""));
  delay(1000);

  gsm.print(msg);
  gsm.write(26);
  delay(5000);
  yield();

}

void sendAlert_call(String phoneNumber) {
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("Call" + phoneNumber);
  delay(1000);
  gsm.println("AT");
  delay(1000);
  gsm.println("ATD" + phoneNumber + ";");
  delay(30000);
  gsm.print("ATH");
  delay(2000);
}

void mqttConnect() {
  if (mqtt.connected()) return;

  int8_t ret;
  uint8_t retries = 5;
  while ((ret = mqtt.connect()) != 0 && retries--) {
    mqtt.disconnect();
    delay(1000);
  }

  if (mqtt.connected()) {
    mqtt.subscribe(&modeFeed);
  }
}


void handleAdafruitMode() {
  mqttConnect();
  Adafruit_MQTT_Subscribe *subscription;

  while ((subscription = mqtt.readSubscription(5000))) {
    if (subscription == &modeFeed) {
      if (modeFeed.lastread == nullptr || strlen((char*)modeFeed.lastread) == 0) {
        continue;
      }

      char payload = toupper(((char*)modeFeed.lastread)[0]);
      if (payload == 'L' || payload == 'A') {
        if (payload != lastMode) {
          lastMode = payload;

          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print(F("MODE: "));
          lcd.print(lastMode == 'A' ? "A" : "L");

          Serial.write(payload);
        }
      }
    }
    yield();
  }
}

unsigned long pumpStart = 0;
unsigned long pumpDuration = 0;
bool pumpActive = false;

void pumpStop() {
  digitalWrite(PUMP_1, LOW);
  digitalWrite(PUMP_2, LOW);
}

void pumpStartWater(unsigned long duration) {
  digitalWrite(PUMP_1, LOW);
  digitalWrite(PUMP_2, HIGH);

  unsigned long start = millis();
  while (millis() - start < duration) {
    mqtt.processPackets(200);
    mqtt.ping();
    yield();
  }

  pumpStop();
}

void pumpStartChem(unsigned long duration) {
  digitalWrite(PUMP_1, HIGH);
  digitalWrite(PUMP_2, LOW);

  unsigned long start = millis();
  while (millis() - start < duration) {
    mqtt.processPackets(200);
    mqtt.ping();
    yield();
  }

  pumpStop();
}

void pumpStartSingle(unsigned long duration) {
  digitalWrite(PUMP_1, LOW);   // OFF
  digitalWrite(PUMP_2, HIGH);  // ON

  unsigned long start = millis();
  while (millis() - start < duration) {
    mqtt.processPackets(200);
    mqtt.ping();
    yield();
  }

  pumpStop();
}



void handleFirebaseCommands() {
  if (millis() - fbTimer < 1000) return;
  fbTimer = millis();

  if (!Firebase.RTDB.getString(&fbdo, "/predictions/data")) {
    return;
  }

  String data = fbdo.stringData();
  data.trim();
  if (data.length() == 0) return;

  char cmd = toupper(data.charAt(0));
  if (cmd == 'X') return;

  if (cmd != lastCmd) {

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("CMD: "));
    lcd.print(cmd);
    Serial.write(cmd);
    delay(100);
    Firebase.RTDB.setString(&fbdo, "/predictions/data", "X");
    switch (cmd) {
      case 'U': delay(2500); yield(); pumpStartWater(5000); pumpStartChem(5000); break;
      case 'D': pumpStartWater(7500); yield(); pumpStartChem(7500); break;
      case 'B': pumpStartChem(4000); delay(2200); yield(); pumpStartWater(13000); break;
      case 'H': pumpStartSingle(15000); break;
      default: return;
    }
  }

  pumpStop();
  Serial.write('X');
  lastCmd = cmd;
  yield();

}


void setup() {
  Serial.begin(115200);
  Serial1.begin(115200);

  lcd.init();
  lcd.backlight();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("System Starting");
  lcd.setCursor(0, 1);
  lcd.print("Please wait...");
  delay(1000);

  pinMode(WATER_SENSOR, INPUT_PULLUP);
  pinMode(CHEM_SENSOR, INPUT_PULLUP);
  pinMode(PUMP_1, OUTPUT);
  pinMode(PUMP_2, OUTPUT);
  digitalWrite(PUMP_1, LOW);
  digitalWrite(PUMP_2, LOW);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Sensors Ready");
  delay(1000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting WiFi");

  WiFi.begin(WIFI_SSID, WIFI_PASS);

  int dotPos = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    lcd.setCursor(dotPos, 1);
    lcd.print(".");
    dotPos++;
    if (dotPos > 15) dotPos = 0;
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi Connected");
  delay(1000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Firebase Init");

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  config.token_status_callback = tokenStatusCallback;

  Firebase.signUp(&config, &auth, "", "");
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  lcd.setCursor(0, 1);
  lcd.print("Firebase Ready");
  delay(1000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("MQTT Connecting");

  mqtt.subscribe(&modeFeed);
  mqttConnect();

  lcd.setCursor(0, 1);
  lcd.print("MQTT Ready");
  delay(1000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("System Ready");
  lcd.setCursor(0, 1);
  lcd.print("LEAF DETECTED");
  delay(1000);

  lcd.clear();
}



void loop() {
  mqtt.processPackets(1000);
  mqtt.ping();
  mqttConnect();
  handleAdafruitMode();

  int water = digitalRead(WATER_SENSOR);
  int chem  = digitalRead(CHEM_SENSOR);
  float volt = readVoltage();

  if (water == 0 && chem == 0 && !alertSent) {
    sendAlert("7904640664", "BOTH TANK EMPTY ALERT!");
    sendAlert_call("7904640664");
    alertSent = true;
  }
  else if (chem == 0 && !alertSent) {
    sendAlert("7904640664", "CHEMICAL TANK EMPTY ALERT!");
    sendAlert_call("7904640664");
    alertSent = true;
  }
  else if (water == 0 && !alertSent) {
    sendAlert("7904640664", "WATER TANK EMPTY ALERT!");
    sendAlert_call("7904640664");
    alertSent = true;
  }

  if (water == 1 && chem == 1) {
    alertSent = false;
  }

  if (millis() - lcdTimer > 1500) {
    lcdTimer = millis();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("W : "));
    lcd.print(String(water));
    lcd.print(F(" C : "));
    lcd.print(String(chem));
    lcd.setCursor(0, 1);
    lcd.print(F("V : "));
    lcd.print(String(volt, 2));
    lcd.print("   ");
  }

  if (millis() - lastUpload > 15000) {
    waterFeed.publish(String(water).c_str());
    chemFeed.publish(String(chem).c_str());
    voltFeed.publish(String(volt, 2).c_str());
    lastUpload = millis();
  }

  handleFirebaseCommands();
  yield();

}
