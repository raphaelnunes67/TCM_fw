/*
  ****************************************
              Driver_RWS.h
   Script para ESP8266MOD (Caixa D'água)
   Autor: Raphael Nunes
   E-mail: raphaelnunes67@gmail.com
  ****************************************
*/
// Solenoid controller
void CommandSolenoid(bool Command) {
  Solenoid_State = Command;
  digitalWrite(SOLENOID_pin, Solenoid_State);
}
// END - Motor controller

// Make a JSON Serialized
String GetJSONSerialized(String value0, String value1) {
  DynamicJsonDocument doc (512);
  String input = "{}";
  deserializeJson(doc, input);
  JsonObject obj = doc.as<JsonObject>();
  obj[String("SolenoidStatus")] = String(Solenoid_State);
  obj[String("FloatSwitchStatus")] = value0;
  obj[String("WaterLevel")] = value1;
 
  String output;
  serializeJson(doc, output);
  return output;
}
// END - Make a JSON Serialized

// Get Water Level(Sensor SR04T)
String GetWaterLevel() {
  //  long Level;
  //  Level = random(5, 121);
  unsigned int o = 0, uS;
  for (int i = 0; i <7; i++){
  delay(50);
  uS = sonar.ping();
  filter.in(uS);
  o = filter.out();
  }
#ifdef DEBUG
  Serial.println(o);
#endif
  return String(o / US_ROUNDTRIP_CM);
}
// END - Get Water Level

//Get Float Switch
String GetFloatSwitchStatus(){

  FloatSwitchStatus = digitalRead(FLOATSWITCH_pin);
  return String (FloatSwitchStatus);

}
//END - Get Float Switch

// Aux function to readValues
String ReadValues() {
  FloatSwitchStatus = GetFloatSwitchStatus();
  WaterLevel = GetWaterLevel();

  return GetJSONSerialized(FloatSwitchStatus, WaterLevel);
}
// - END Aux function

char SaveTime_upgrade(char intORfw) {
  //ntpClient.update();
  ntpClient.begin();
  delay(2000);
  ntpClient.forceUpdate();
  String TIME = ntpClient.getFormattedDate();
  file = SPIFFS.open("/system_info.json", "r");
  String system_info = file.readString();
  file.close();
  DynamicJsonDocument doc (512);
  deserializeJson(doc, system_info);
  JsonObject obj = doc.as<JsonObject>();
  if (intORfw == 'i') {
    obj[("last_update_int")] = TIME;
  }
  else if (intORfw == 'f') {
    obj[("last_update_fw")] = TIME;
  }
  system_info = "";
  serializeJson(doc, system_info);
  file = SPIFFS.open("/system_info.json", "w");
  file.print(system_info);
  file.close();
}
String UpdateVC_link(String New_VC_link) {
  file = SPIFFS.open("/system_info.json", "r");
  String system_info = file.readString();
  file.close();
  DynamicJsonDocument doc (512);
  deserializeJson(doc, system_info);
  JsonObject obj = doc.as<JsonObject>();
  obj[("VC_link")] = New_VC_link;
  system_info = "";
  serializeJson(doc, system_info);
  file = SPIFFS.open("/system_info.json", "w");
  file.print(system_info);
  file.close();
}
