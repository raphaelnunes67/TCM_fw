/*
  ****************************************
              Driver_WIFI.h
   Script para ESP8266MOD (Caixa D'água)
   Autor: Raphael Nunes
   E-mail: raphaelnunes67@gmail.com
  ****************************************
*/
// Configure WIFI connection
void initWIFI() {
  
  file = SPIFFS.open("/register_config.json", "r");
  String output = file.readString();
  file.close();
  DynamicJsonDocument doc (1024);
  deserializeJson(doc, output);
  JsonObject obj = doc.as<JsonObject>();
  ssid = obj[("ssid")].as<String>();
  password = obj[("password")].as<String>();
  WiFi.mode(WIFI_OFF);
  delay(100);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
#ifdef DEBUG
  Serial.print("Tentando se conectar a rede ");
  Serial.println(ssid);
#endif
  unsigned long count = 0;
  unsigned long timer = millis();
  while ((WiFi.status() != WL_CONNECTED)) {
    BlinkLED(WIFI_LED_pin);
    count = millis() - timer;
    if (count > try_connect_ms) {
      Set_Timer = true;
      ConfigureSMCC();
    }
  }
  digitalWrite(WIFI_LED_pin, LOW);
#ifdef DEBUG
  Serial.print("\nConectado na rede: ");
  Serial.println(ssid);
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
#endif
}
// END - Configure WIFI connection

// Configure WIFI reconnection
void reconnectWIFI() {
  if (WiFi.status() != WL_CONNECTED ) {
#ifdef DEBUG
    Serial.println("Perca de conexão WIFI. Tentando reconexão...");
#endif
    digitalWrite(MQTT_LED_pin, HIGH);
    digitalWrite(WIFI_LED_pin, HIGH);
    WiFi.mode(WIFI_OFF);
    delay(1000);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    unsigned long count;
    unsigned long timer = millis();
    while ((WiFi.status() != WL_CONNECTED)) {
      BlinkLED(WIFI_LED_pin);
      count = millis() - timer;
      if (count > try_connect_ms) {
        Set_Timer = true;
        ConfigureSMCC();
      }
    }
    digitalWrite(WIFI_LED_pin, LOW);
#ifdef DEBUG
    Serial.println("Reconectado ao WIFI!");
#endif
    void reconnectMQTT();
    reconnectMQTT();
  }
  else {
  }
}
// END - Configure WIFI reconnection
