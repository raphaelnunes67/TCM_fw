/*
  ****************************************
           Driver_ConfigTCM.h
   Script para ESP8266MOD (Caixa D'água)
   Autor: Raphael Nunes
   E-mail: raphaelnunes67@gmail.com
  ****************************************
*/

// Save data in FS
void SaveData(String ssid, String password, String br_add, String port, String br_usr, String br_pass, String subs, String pubs) {
  DynamicJsonDocument doc (512);
  String input = "{}";
  deserializeJson(doc, input);
  JsonObject obj = doc.as<JsonObject>();
  obj[String("ssid")] = ssid;
  obj[String("password")] = password;
  obj[String("br_add")] = br_add;
  obj[String("port")] = port;
  obj[String("br_usr")] = br_usr;
  obj[String("br_pass")] = br_pass;
  obj[String("subs")] = subs;
  obj[String("pubs")] = pubs;
  String output;
  serializeJson(doc, output);
  file = SPIFFS.open("/register_config.json", "w"); // Save the Network and MQTT definitions
  file.print(output);
  file.close();

#ifdef DEBUG
  file = SPIFFS.open("/register_config.json", "r"); // Modo de leitura para printar os dados
  Serial.printf("Nome: %s - %u bytes\n", file.name(), file.size());
  Serial.println(file.readString());
  file.close();
#endif
}
// END - Save data in FS

//Web Server Functions
//Check if header is present and correct
bool is_authentified() {
#ifdef DEBUG
  Serial.println("Enter is_authentified");
#endif
  if (server.hasHeader("Cookie")) {
#ifdef DEBUG
    Serial.print("Found cookie: ");
#endif
    String cookie = server.header("Cookie");
#ifdef DEBUG
    Serial.println(cookie);
#endif
    if (cookie.indexOf("ESPSESSIONID=1") != -1) {
#ifdef DEBUG
      Serial.println("Authentification Successful");
#endif
      return true;
    }
  }
#ifdef DEBUG
  Serial.println("Authentification Failed");
#endif
  return false;
}

//Login page, also called for disconnect
void handleLogin() {
  file = SPIFFS.open("/new_network.html", "r");
  String content  = file.readString();
  file.close();

  if (server.hasHeader("Cookie")) {
#ifdef DEBUG
    Serial.print("Found cookie: ");
#endif
    String cookie = server.header("Cookie");
#ifdef DEBUG
    Serial.println(cookie);
#endif
  }
  if (server.hasArg("DISCONNECT")) {
#ifdef DEBUG
    Serial.println("Disconnection");
#endif
    server.sendHeader("Location", "/login");
    server.sendHeader("Cache-Control", "no-cache");
    server.sendHeader("Set-Cookie", "ESPSESSIONID=0");
    server.send(301);
    return;
  }
  if (server.hasArg("SSID") && server.hasArg("PASSWORD") && server.hasArg("BROKER ADDRESS")
      && server.hasArg("BROKER PORT") && server.hasArg("BROKER USER") && server.hasArg("BROKER PASS")
      && server.hasArg("TOPIC SUB") && server.hasArg("TOPIC PUB")) {

    ssid = server.arg("SSID");
    password = server.arg("PASSWORD");
    br_add = server.arg("BROKER ADDRESS");
    port = server.arg("BROKER PORT");
    br_usr = server.arg("BROKER USER");
    br_pass = server.arg("BROKER PASS");
    subs = server.arg("TOPIC SUB");
    pubs = server.arg("TOPIC PUB");

    int len_password = password.length();
    int len_ssid = ssid.length();
    int len_br_add = br_add.length();
    int len_port = port.length();
    int len_br_usr = br_usr.length();
    int len_br_pass = br_pass.length();
    int len_subs = subs.length();
    int len_pubs = pubs.length();

    if (
      (((len_ssid > 0) and (len_password >= 6)) or ((len_ssid > 0) and (len_password == 0)))
      and (((len_br_usr > 0) and (len_br_pass >= 6)) or ((len_br_usr == 0) and (len_br_pass == 0)) or ((len_br_usr > 0) and (len_br_pass == 0)))
      and ((len_subs and len_pubs and len_br_add) > 0)
    ) {

      file = SPIFFS.open("/confirm.html", "r");
      content = file.readString();
      file.close();
      server.send(200, "text/html", content);
#ifdef DEBUG
      Serial.println("Dados enviados com sucesso...");
#endif
      SaveData(ssid, password, br_add, port, br_usr, br_pass, subs, pubs);
      delay(5000);
      ESP.restart();
    }
  }
  server.send(200, "text/html", content);
}

//Root page can be accessed only if authentification is ok

//no need authentification
void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void ConfigureTCM() {

  digitalWrite(WIFI_LED_pin, LOW);
  digitalWrite(MQTT_LED_pin, HIGH);
  WiFi.mode(WIFI_OFF);
  delay(1000);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP("Configure_TCM");
  dnsServer.start(DNS_PORT, "*", apIP);
#ifdef DEBUG
  Serial.println("Acess Point iniciado!");
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());
#endif
  //server.on("/", handleLogin);
  server.onNotFound(handleLogin);
  const char * headerkeys[] = {"User-Agent", "Cookie"} ;
  size_t headerkeyssize = sizeof(headerkeys) / sizeof(char*);
  server.collectHeaders(headerkeys, headerkeyssize);
  server.begin();
#ifdef DEBUG
  Serial.println("Web Server de cadastro iniciado!");
#endif
  if (Set_Timer == true) {
    unsigned long count = 0;
    unsigned long timer = millis();
    while (count < ap_time_ms) { //Access point is open for 3 minutes
      count = millis() - timer;
      server.handleClient();
      dnsServer.processNextRequest();
    }
    ESP.restart();
  }
  else if (Set_Timer == false) {
    while (true) {
      server.handleClient();
      dnsServer.processNextRequest();
    }
  }
}
// END - Web Server Functions

// Verify if register_config.json exists
void VerifyConfigJSON() {
  if (!SPIFFS.exists("/register_config.json")) {
#ifdef DEBUG
    Serial.println("\nArquivo \'register_config.json\' não existe");
#endif
    ConfigureTCM();
  }
  else {
#ifdef DEBUG
    Serial.println("\nArquivo \'register_config.json\' existe");
#endif
  }
}
// END - Verify if register_config.json exists
