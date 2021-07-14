/*
  ****************************************
             Sleep_Functions.h
   Script para ESP8266MOD (Caixa D'água)
   Autor: Raphael Nunes
   E-mail: raphaelnunes67@gmail.com
  ****************************************
*/
// Sleep Function
void ESP_Sleep() {
  #ifdef DEBUG
  Serial.println("Indo dormir em 3s...");
  #endif
  delay(3000);
  ESP.deepSleep(time_sleep_s * 1e6, WAKE_RF_DEFAULT); // Sleep 300s
  
}
// END - Sleep Function
