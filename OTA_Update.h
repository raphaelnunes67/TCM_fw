/*
  ****************************************
            OTA_Upgrade.h
   Script para ESP8266MOD (Caixa D'água)
   Autor: Raphael Nunes
   E-mail: raphaelnunes67@gmail.com
  ****************************************
*/

/*
 ******************************************************************
                      OTA Arduino IDE UPDATE
 ******************************************************************
*/
String s;

void OTA_arduino_update() {
  ArduinoOTA.setHostname("TCM-ESP8266");
  ArduinoOTA.onStart([]() {
    digitalWrite(WIFI_LED_pin, HIGH);
    digitalWrite(MQTT_LED_pin, HIGH);
    UpgradeBlinkLEDS(WIFI_LED_pin, MQTT_LED_pin);
    if (ArduinoOTA.getCommand() == U_FLASH) {
      // Atualizar sketch
      s = "Sketch";
    } else { // U_SPIFFS
      // Atualizar SPIFFS
      s = "SPIFFS";
      // SPIFFS deve ser finalizada
      SPIFFS.end();
    }
#ifdef DEBUG
    Serial.println("Iniciando OTA - " + s);
#endif
  });

  ArduinoOTA.onEnd([]() {
#ifdef DEBUG
    Serial.println("\nOTA Concluído.");
#endif
    digitalWrite(WIFI_LED_pin, HIGH);
    digitalWrite(MQTT_LED_pin, HIGH);
    UpgradeBlinkLEDS(WIFI_LED_pin, MQTT_LED_pin);
    #ifdef DEBUG
    Serial.println (s);
    #endif
    if (s == "SPIFFS"){
      SaveTime_upgrade('i');
    }
    else if(s == "Sketch"){
      SaveTime_upgrade('f');
    }
    
    ESP.restart();
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
#ifdef DEBUG
    Serial.print(progress * 100 / total);
    Serial.print(" ");
#endif
  });
  ArduinoOTA.onError([](ota_error_t error) {
#ifdef DEBUG
    Serial.print("Erro " + String(error) + " ");
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Falha de autorização");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Falha de inicialização");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Falha de conexão");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Falha de recebimento");
    } else if (error == OTA_END_ERROR) {
      Serial.println("Falha de finalização");
    } else {
      Serial.println("Falha desconhecida");
    }
#endif
    for (int i = 0; i < 4; i++) {
      BlinkLED(WIFI_LED_pin);
    }
    ESP.restart();
  });

  ArduinoOTA.begin();

#ifdef DEBUG
  Serial.println("Atualização via OTA disponível.");
  Serial.println("Hostname: TCM-ESP8266");
#endif
}
// END - OTA Arduino IDE Update

/*
 ******************************************************************
                         OTA HTTPS UPDATE
 ******************************************************************
*/
char SaveTime_upgrade(char intORfw);

void OTA_https_update() {
  //client.disconnect(); // Turn off MQTT comunication
  //digitalWrite(MQTT_LED_pin, HIGH); // Turn off LED MQTT notification
  WiFiClientSecure client_http;
  client_http.setInsecure();
  HTTPClient http;
  //Read VC link in FS
  DynamicJsonDocument doc0 (512);
  file = SPIFFS.open("/system_info.json", "r");
  String output = file.readString();
  file.close();
  deserializeJson(doc0, output);
  JsonObject obj0 = doc0.as<JsonObject>();
  String VC_link = obj0[("VC_link")].as<String>();
  //END - Read VC link
  http.begin(client_http, VC_link);
  int httpCode = http.GET();
  String content_http = http.getString();

  // In case of redirect, find a direct link
  while (content_http.indexOf("Moved Temporarily") != -1) {
    int pFrom = content_http.indexOf("<A HREF=\"") + 9;
    int pTo = content_http.indexOf("\">here</A>");
    String VC_new_link = content_http.substring(pFrom, pTo);
    httpCode = http.GET();
    http.begin(client_http, VC_new_link);
    content_http = http.getString();
  }
  // END - Find a direct link
#ifdef DEBUG
  Serial.println("Link direto do VC.json: ");
  Serial.println(content_http);
#endif

  http.end();

  // Read versions JSON from FS
  String fw_ver = obj0[("fw_ver")].as<String>();
  String int_ver = obj0[("int_ver")].as<String>();
  // END - Read JSON from FS

  // Read versions JSON from WEB
  DynamicJsonDocument doc1 (512);
  deserializeJson(doc1, content_http);
  JsonObject obj1 = doc1.as<JsonObject>();
  String fw_new_ver = obj1[("fw_ver")].as<String>();
  String int_new_ver = obj1[("int_ver")].as<String>();
  // END - Read JSON from WEB

#ifdef DEBUG
  Serial.print("Versão de FW do dispositivo: ");
  Serial.println(fw_ver);
  Serial.print("Versão de FW disponível: ");
  Serial.println(fw_new_ver);
  Serial.print("Versão da interface do dispositivo: ");
  Serial.println(int_ver);
  Serial.print("Versão de interface disponível: ");
  Serial.println(int_new_ver);
#endif

  // Compare versions, if don't have a new version return
  if ((fw_new_ver == fw_ver) and (int_new_ver == int_ver)) {
#ifdef DEBUG
    Serial.println ("Nenhuma atualização disponível");
#endif
    return;
  }
  //END - Compare versions

  // HTTPS OTA Upgrade definitions
  ESPhttpUpdate.onStart([] {
    digitalWrite(WIFI_LED_pin, HIGH);
    UpgradeBlinkLEDS(WIFI_LED_pin, MQTT_LED_pin);
#ifdef DEBUG
    Serial.println("Atualização via repositório iniciada");
#endif
  });

  ESPhttpUpdate.onEnd([] {
#ifdef DEBUG
    Serial.println("\nAtualização via repositório concluída ");
#endif
    digitalWrite(WIFI_LED_pin, HIGH);
    digitalWrite(MQTT_LED_pin, HIGH);
    UpgradeBlinkLEDS(WIFI_LED_pin, MQTT_LED_pin);
  });

  ESPhttpUpdate.onError([](int erro) {
#ifdef DEBUG
    Serial.println("\nErro na atualização, código: " + String(erro));
#endif
    for (int i = 0; i < 4; i++) {
      BlinkLED(WIFI_LED_pin);
    }
    ESP.restart();
  });

  ESPhttpUpdate.onProgress([](size_t progresso, size_t total) {
#ifdef DEBUG
    Serial.print(progresso * 100 / total);
    Serial.print(" ");
#endif
  });

  ESPhttpUpdate.rebootOnUpdate(false); // Define false. In case of SPIFFS it is not necessary reboot
  //END - HTTPS OTA Upgrade definitions

  /* ******************************************************************************
   * *                         FS UPDATE                                          *
   * ******************************************************************************
  */

  if (int_new_ver != int_ver) {

#ifdef DEBUG
    Serial.println("\nAtualização do FS");
#endif

    String int_link = obj1[("int_link")].as<String>();
#ifdef DEBUG
    Serial.println("Link direto da nova versão de interface: ");
    Serial.println(int_link);
#endif
    http.begin(client_http, int_link);
    int httpCode = http.GET();
    String content_int_upgrade_link = http.getString();
    // In case of redirect, find a direct link
    while (content_int_upgrade_link.indexOf("Moved Temporarily") != -1) {
      int pFrom = content_int_upgrade_link.indexOf("<A HREF=\"") + 9;
      int pTo = content_int_upgrade_link.indexOf("\">here</A>");
      int_link = content_int_upgrade_link.substring(pFrom, pTo);
      http.begin(client_http, int_link);  // Just Upgrade Interface
      httpCode = http.GET();
      content_int_upgrade_link = http.getString();
    }
    // END - Find a direct link
    http.end();

#ifdef DEBUG
    Serial.println("Link direto da nova versão de interface: ");
    Serial.println(int_link);
#endif
    file = SPIFFS.open("/register_config.json", "r"); //Open register_config.json
    String config_json = file.readString(); //Make a copy
    file.close(); //Close file
    SPIFFS.end(); // Finish SPIFFS to make the upgrade

    ESPhttpUpdate.updateSpiffs(client_http, int_link); // Get the new FS (UPGRADE)

    SPIFFS.begin();
    //Create and save register_config.json
    file = SPIFFS.open("/register_config.json", "w");
    file.print(config_json);
    file.close();
    //END - Create and save register_config.json

    //Create a new file system_info.json
    file = SPIFFS.open("/system_info.json", "a");
    file.print("{\"fw_ver\":\""); file.print(fw_ver); file.print("\",");
    file.print("\"int_ver\":\""); file.print(int_new_ver); file.print("\",");
    file.print("\"VC_link\":\""); file.print(VC_link); file.print("\"");
    file.print("}");
    file.close();
    //END - New file system_info.h
    SaveTime_upgrade('i');
#ifdef DEBUG
    file = SPIFFS.open("/system_info.json", "r");
    Serial.println(file.readString());
    file.close();
#endif

#ifdef DEBUG
    Serial.println("system_info.json atualizado");
#endif
  }
  // END - Upgrade interface

  /* ******************************************************************************
   * *                         FIRMWARE UPDATE                                    *
   * ******************************************************************************
  */
  if (fw_new_ver != fw_ver) {
#ifdef DEBUG
    Serial.println("\nAtualização do Firmware");
#endif
    String fw_link = obj1[("fw_link")].as<String>(); // Take file link

#ifdef DEBUG
    Serial.println("Link da nova versão de FW: ");
    Serial.println(fw_link);
#endif

    http.begin(client_http, fw_link); // Make a request
    int httpCode = http.GET(); // Get the request http code
    String content_fw_upgrade_link = http.getString(); // Get the returned content
    // In case of redirect, find a direct link
    while (content_fw_upgrade_link.indexOf("Moved Temporarily") != -1) {
      int pFrom = content_fw_upgrade_link.indexOf("<A HREF=\"") + 9;
      int pTo = content_fw_upgrade_link.indexOf("\">here</A>");
      fw_link = content_fw_upgrade_link.substring(pFrom, pTo);
      http.begin(client_http, fw_link);
      httpCode = http.GET();
      content_fw_upgrade_link = http.getString();
    }
    http.end();
    // END - Find a direct link
#ifdef DEBUG
    Serial.println("Link direto da nova versão de FW");
    Serial.println(fw_link);
#endif
    ESPhttpUpdate.update(client_http, fw_link); // Get the nem FW version (UPGRADE)

    //Create a new file system_info.json
    SPIFFS.remove("/system_info.json");
    file = SPIFFS.open("/system_info.json", "a");
    file.print("{\"fw_ver\":\""); file.print(fw_new_ver); file.print("\",");
    file.print("\"int_ver\":\""); file.print(int_new_ver); file.print("\",");
    file.print("\"VC_link\":\""); file.print(VC_link); file.print("\"");
    file.print("}");
    file.close();
    //END - Create a new file system_info.json
    SaveTime_upgrade('f');
#ifdef DEBUG
    file = SPIFFS.open("/system_info.json", "r");
    Serial.println(file.readString());
    file.close();
#endif

    ESP.restart();
  }
  //END - Update FW
}
