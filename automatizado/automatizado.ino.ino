#include <dht.h>

//Programa: Display LCD 16x2 e modulo I2C
//Autor: Arduino e Cia
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
//Inicializa o display no endereco 0x27
LiquidCrystal_I2C lcd(0x27,16,2);

dht DHT;
 
void setup()
{
 lcd.init();
 Serial.begin(9600); //INICIALIZA A SERIAL
}
 
void loop()
{
    
  DHT.read11(2); //LÊ AS INFORMAÇÕES DO SENSOR
  Serial.println(String(DHT.humidity)+";"+ String(DHT.temperature)); //IMPRIME NA SERIAL O VALOR DE UMIDADE MEDIDO
  delay(2000); //INTERVALO DE 2 SEGUNDOS * NÃO DIMINUIR ESSE VALOR
  
  lcd.setBacklight(HIGH);
  lcd.setCursor(0,0);
  lcd.print("Temp: ");
  lcd.print(DHT.temperature);
  lcd.print((char)223);
  lcd.print("C");
  lcd.setCursor(0,1);
  lcd.print("Umidade: ");
  lcd.print(DHT.humidity);
  lcd.print("%");
  delay(1000);
  //lcd.setBacklight(LOW);
  //delay(1000);
}
