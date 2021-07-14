/*
  ****************************************
           GeneralDefinitions.h
   Script para ESP8266MOD (Cisterna)
   Autor: Raphael Nunes
   E-mail: raphaelnunes67@gmail.com
  ****************************************
*/
//Define pins name
const int WIFI_LED_pin = 12; // Red
const int MQTT_LED_pin = 2; //Blue
//const int AP_LED_pin = 2; // Yellow
const int INTERRUPT_pin = 0;
const int SOLENOID_pin = 4;
const int FLOATSWITCH_pin = 5;
const int WL_ECHO_pin = 15;
const int WL_TRIG_pin = 13;
//Web Server and DNS server
const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 1, 1);
DNSServer dnsServer;
ESP8266WebServer server(80); // Web Sever on port 80
//Variables
String WaterLevel, FloatSwitchStatus;
String ssid, password, port, br_add, br_usr, br_pass, subs, pubs;
String message;
bool Solenoid_State = false;
bool Set_Timer = false; //
const int time_pub = 1000; // Delay of each MQTT publication (1s)
const int time_sleep_s = 300; // Sleep time in seconds
const int ap_time_ms = 180000; //Access point is open for 3 minutes
const int try_connect_ms = 120000; // Time to try a connection (MQTT and WIFI); (ms)
//File System
File file;
//MQTT PubSubClient config.
WiFiClient EspClient;
PubSubClient client(EspClient);
//Time config
WiFiUDP ntpUDP;
const int timeZone = -4;
NTPClient ntpClient(ntpUDP, "a.ntp.br", timeZone*3600, 60000);
//Water Level Sensor
const int MAX_DISTANCE = 120;
NewPing sonar (WL_TRIG_pin, WL_ECHO_pin, MAX_DISTANCE);
MedianFilter filter (31,0);


// Set pins IO and initial values
void SetPins() {
  pinMode(WIFI_LED_pin, OUTPUT); digitalWrite(WIFI_LED_pin, HIGH);
  pinMode(MQTT_LED_pin, OUTPUT); digitalWrite(MQTT_LED_pin, HIGH);
  pinMode(SOLENOID_pin, OUTPUT); digitalWrite(SOLENOID_pin, LOW);
  pinMode(FLOATSWITCH_pin, INPUT);
  pinMode(WL_ECHO_pin, INPUT);
  pinMode(WL_TRIG_pin, OUTPUT); digitalWrite(WL_TRIG_pin, LOW); 
  pinMode(INTERRUPT_pin, INPUT_PULLUP);
#ifdef DEBUG
  Serial.println("\nPinos setados.");
#endif
}
// END - Set and define initial pin values

// Show int and fw version in serial
void ShowSystemVersion() {
  if (SPIFFS.exists("/system_info.json")) {
    file = SPIFFS.open("/system_info.json", "r");
    String output = file.readString();
    file.close();
    DynamicJsonDocument doc (512);
    deserializeJson(doc, output);
    JsonObject obj = doc.as<JsonObject>();
    String FW_version = obj[("fw_ver")].as<String>();
    String Interface_version = obj[("int_ver")].as<String>();
    Serial.print("Versão do Firmware: ");
    Serial.println(FW_version);
    Serial.print("Versão da interface de cadastro: ");
    Serial.println(Interface_version);
  }
}
//END - ShowSystemVersion
