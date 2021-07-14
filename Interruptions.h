/*
  ****************************************
             Interruptions.h
   Script para ESP8266MOD (Caixa D'água)
   Autor: Raphael Nunes
   E-mail: raphaelnunes67@gmail.com
  ****************************************
*/
// Interrupt Function [Factory Reset button]
void ICACHE_RAM_ATTR FactoryReset() {
  if (SPIFFS.exists("/register_config.json")) {
    unsigned long PressTime = millis();
    while (digitalRead(INTERRUPT_pin) == LOW) {
      if (millis() - PressTime >= 3000) {
        SPIFFS.remove("/register_config.json");
        ESP.restart();
      }
    }
  }
}
// END - Interrupt Function

// Set Interrupt Function
void SetInterrupt() {
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_pin), FactoryReset, CHANGE);
}
// END - Set Interrupt Function
