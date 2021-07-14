/*
  ****************************************
              SimpleBlink.h
   Script para ESP8266MOD (Caixa D'água)
   Autor: Raphael Nunes
   E-mail: raphaelnunes67@gmail.com
  ****************************************
*/
// Blink Any LED
void BlinkLED(const int pin) {
  digitalWrite(pin, LOW);
  delay(500);
  digitalWrite(pin, HIGH);
  delay(500);
}
// END - Blink Any LED

//Leds during upgrade
void UpgradeBlinkLEDS(const int pin1,const int pin2){
  for (int i = 0; i < 3; i++) {
      digitalWrite(pin1, LOW);digitalWrite(pin2, LOW);
      delay(700);
      digitalWrite(pin1, HIGH);digitalWrite(pin2, HIGH);
      delay(700);
    }
    digitalWrite(pin1, LOW); digitalWrite(pin2, LOW);
}
// END - Leds during upgrade

void MQTTBlinkLED(const int pin){
  
  digitalWrite(pin, LOW);
  delay(100);
  digitalWrite(pin,HIGH);
  delay(100);
  digitalWrite(pin, LOW);
  delay(100);
  digitalWrite(pin,HIGH);
  delay(100);
  digitalWrite(pin, LOW);
  delay(100);
  digitalWrite(pin,HIGH);
}
