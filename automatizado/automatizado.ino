





/*Biblioteca temperature.h é bibliteca padrão do arduino 
 * Temperature
 * by Rob Tillaart versão 0.2.4
 * Usado para calcular o ponto de orvalho da estufa.
 * 
 * Bibliteca LiquidCrystal_I2C é uma biblioteca padrão do arduino
 * LiquidCrystal I2C
 * by Marco Schwartz Versão 1.1.2
 * 
 * DHTStable é uma biblioteca padrão do arduino
 * DHTStable
 * by Rob Tillaart Versão 1.0.0
 */
 
#include "temperature.h"
#include <LiquidCrystal_I2C.h>
#include "DHTStable.h"


//Inicializa o display no endereco 0x27
LiquidCrystal_I2C lcd(0x27,16,2);

//Inicializa o DHT11
DHTStable DHT;
#define DHT11_PIN 2

// Variáveis
int RU_MAX = 90; // Umidade Relativa máxima de trabalho
float TEMP; // Guardar a variável temperatura
float RU;   // Guardar a variável Umidade Relativa do Ar
float PO;  // Guardar o ponto de Orvalho 

 
void setup()
{
 lcd.init();
 Serial.begin(9600); //INICIALIZA A SERIAL
}
 
void loop()
{
    
  int chk = DHT.read11(DHT11_PIN); // Leitura dos sensores
  TEMP = DHT.getTemperature();
  RU = DHT.getHumidity();
  PO = dewPoint(TEMP, RU);
  Serial.println(String(RU,1)+";"+ String(TEMP,1)+";"+String(PO,1)); //IMPRIME NA SERIAL O VALOR DE UMIDADE MEDIDO
  LCD(TEMP,RU,PO);
  UMD(TEMP,RU,PO);
  delay(2000); //INTERVALO DE 2 SEGUNDOS * NÃO DIMINUIR ESSE VALOR
  
}

void UMD(float TEMP, float RU, float PO){
  if (PO <= TEMP){
    Serial.println("Desliga");
    }
    else{
      if (RU <= RU_MAX){
        Serial.println("Liga");
        }
        else{
          Serial.println("Desliga2");
          }
      }
  }


// PRintar no LCD os dados
void LCD (float TEMP, float RU, float PO){
    
  lcd.setBacklight(HIGH);
  lcd.setCursor(0,0);
  lcd.print("T/PO");
  lcd.print((char)223);
  lcd.print("C");
  lcd.print(":");
  lcd.print(TEMP,1);
  lcd.print("/");
  lcd.print(PO,1);
  lcd.setCursor(0,1);
  lcd.print("Umidade: ");
  lcd.print(RU,1);
  lcd.print("%");  
  }