/*
  ****************************************
              Driver_MQTT.h
   Script para ESP8266MOD (Caixa D'água)
   Autor: Raphael Nunes
   E-mail: raphaelnunes67@gmail.com
  ****************************************
*/
// Send the data through MQTT
void SendValues() {
  message = ReadValues();
#ifdef DEBUG
  Serial.println(message);
#endif
  int len = message.length() + 1;
  char msg[len];
  message.toCharArray(msg, len);
#ifdef DEBUG
  Serial.print("Mensagem enviada: ");
  Serial.print(msg);
  Serial.print(" tam.:");
  Serial.print(len);
  Serial.println("bytes");
#endif
  char topic_pubs [pubs.length() + 1];
  pubs.toCharArray(topic_pubs, pubs.length() + 1);
  client.publish(topic_pubs, msg);
  delay(time_pub);
}
// END - Send the data through MQTT

void Send_system_info() {
  file = SPIFFS.open("/system_info.json", "r");
  String FW_info = file.readString();
  char FW_info_char [FW_info.length() + 1];
  FW_info.toCharArray(FW_info_char, FW_info.length() + 1);
  char topic_pubs [pubs.length() + 1];
  pubs.toCharArray(topic_pubs, pubs.length() + 1);
  client.publish(topic_pubs, FW_info_char);
}

// Callback MQTT payloads
void callback(char* topic, byte* payload, unsigned int length) {
#ifdef DEBUG
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
#endif
  payload[length] = '\0';
  String cb_message = String((char*)payload);
#ifdef DEBUG
  Serial.println(cb_message);
#endif
 

  if (cb_message == "OPEN"){CommandSolenoid(false);}
  
  else if (cb_message == "CLOSE"){CommandSolenoid(true);}
  
  else if (cb_message == "UPDATE"){SendValues();}
  
  else if (cb_message == "GS"){ESP_Sleep();}
  
  else if (cb_message == "OTA"){OTA_https_update();}
  
  else if (cb_message == "SI"){Send_system_info();}
  
  else if(cb_message.indexOf("http") != -1){UpdateVC_link(cb_message);}
}
//END - Callback MQTT payloads

// Configure MQTT connection
void initMQTT() {
  
  file = SPIFFS.open("/register_config.json", "r");
  String output = file.readString();
  file.close();
  DynamicJsonDocument doc (512);
  deserializeJson(doc, output);
  JsonObject obj = doc.as<JsonObject>();
  br_add = obj[("br_add")].as<String>();
  port = obj[("port")].as<String>();
  br_usr = obj[("br_usr")].as<String>();
  br_pass = obj[("br_pass")].as<String>();
  subs = obj[("subs")].as<String>();
  pubs = obj[("pubs")].as<String>();

  char mqtt_server [br_add.length() + 1];
  char mqtt_login [br_usr.length() + 1];
  char mqtt_pass [br_pass.length() + 1];

  char topic_subs [subs.length() + 1];

  br_add.toCharArray(mqtt_server, br_add.length() + 1);
  int mqtt_port = port.toInt();
  br_usr.toCharArray(mqtt_login, br_usr.length() + 1);
  br_pass.toCharArray(mqtt_pass, br_pass.length() + 1);
  subs.toCharArray(topic_subs, subs.length() + 1);
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  unsigned long count;
  unsigned long timer = millis();
#ifdef DEBUG
  Serial.println("Tentando se conectar ao Broker... ");
#endif
  while (client.connect("TCM-master", mqtt_login, mqtt_pass) == false) {
    MQTTBlinkLED(MQTT_LED_pin);
    ArduinoOTA.handle();
    count = millis() - timer;
    if (count > try_connect_ms) { // Tries to reconnect for 2 minutes
      Set_Timer = true;
      ConfigureTCM();
    }
    reconnectWIFI();
  }
#ifdef DEBUG
  Serial.println("Conectado!");
  Serial.print("IP Broker: "); Serial.print(mqtt_server); Serial.print(". Porta: "); Serial.println(mqtt_port);
#endif
  Send_system_info();
  digitalWrite(MQTT_LED_pin, LOW);
  client.publish("test/TCM", "System ON!");
  delay(500);
  client.subscribe(topic_subs);

#ifdef DEBUG
  Serial.println("Servico MQTT inicializado.");
  Serial.print("Inscrito em: [");
  Serial.print(subs);
  Serial.println("]");
  Serial.print("Publicando em: [");
  Serial.print(pubs);
  Serial.println("]");
#endif

}
// END - Configure MQTT connection

// Configure MQTT reconnection
void reconnectMQTT() {
  if ((client.connected() == false) and (WiFi.status() == WL_CONNECTED)) {
#ifdef DEBUG
    Serial.println("Perca de comunicacao MQTT");
#endif
    initMQTT();
  }
  else {
  }
}
// END - Configure MQTT reconnection
