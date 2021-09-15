

/*
 *Controle de estufa
 
 *Biblioteca temperature.h é bibliteca padrão do arduino 
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
#include "RTClib.h" //INCLUSÃO DA BIBLIOTECA
#include <Wire.h> //INCLUSÃO DA BIBLIOTECA
#include <OneWire.h>
#include <DallasTemperature.h>


//Inicializa o display no endereco 0x27
LiquidCrystal_I2C lcd(0x27,16,2);

//Inicializa o DHT11
DHTStable DHT;

#define DHT11_PIN 2

//OBJETO DO TIPO RTC_DS1307
RTC_DS1307 rtc; 



// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 3
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);
// arrays to hold device address
DeviceAddress insideThermometer;




// Variáveis
int RU_MAX = 90; // Umidade Relativa máxima de trabalho
float TEMP; // Guardar a variável temperatura
float Temp0;
float Temp1;
float RU;   // Guardar a variável Umidade Relativa do Ar
float PO;  // Guardar o ponto de Orvalho 
char daysOfTheWeek[7][12] = {"Domingo", "Segunda", "Terça", "Quarta", "Quinta", "Sexta", "Sábado"};

// Psicrometro usando dois termometros

float PSIC(float Ts, float Tu, float P, char Tipo='N'){
  float A = 0.0008;  //Sem Ventilação forçada
  if (Tipo == 'F'){
    A = 0.000667; //Com Ventilação forçada
  }
  
  float Es1 = 0.6108*pow(10,((7.5*Ts)/(237.3+Ts)));
  float Es2 = 0.6108*pow(10,((7.5*Tu)/(237.3+Tu)));
  float Ea = Es2 - A*P*(Ts-Tu);
  float UR = Ea*100/Es1;
  return UR;
  
  }





 
void setup()
{
 
 Serial.begin(9600); //INICIALIZA A SERIAL
 sensors.begin(); // Sensor de temperatura
 // set the resolution to 9 bit (Each Dallas/Maxim device is capable of several different resolutions)
  sensors.setResolution(insideThermometer, 9);
  
 lcd.init();
 lcd.clear();
 if (! rtc.begin()) { // SE O RTC NÃO FOR INICIALIZADO, FAZ
    Serial.println("DS1307 não encontrado"); //IMPRIME O TEXTO NO MONITOR SERIAL
    while(1); //SEMPRE ENTRE NO LOOP
  }
 if (! rtc.isrunning()) { //SE RTC NÃO ESTIVER SENDO EXECUTADO, FAZ
    Serial.println("DS1307 rodando!"); //IMPRIME O TEXTO NO MONITOR SERIAL
    //REMOVA O COMENTÁRIO DE UMA DAS LINHAS ABAIXO PARA INSERIR AS INFORMAÇÕES ATUALIZADAS EM SEU RTC
    //rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); //CAPTURA A DATA E HORA EM QUE O SKETCH É COMPILADO
    //rtc.adjust(DateTime(2018, 7, 5, 15, 33, 15)); //(ANO), (MÊS), (DIA), (HORA), (MINUTOS), (SEGUNDOS)
  }



 
}
 
void loop()
{


sensors.requestTemperatures();
  Temp0 = sensors.getTempCByIndex(0);
  Temp1 = sensors.getTempCByIndex(1);
  Serial.println("Temperaturas");
  Serial.println(Temp0);
  Serial.println(Temp1);
  float UR = PSIC(Temp0,Temp1,101.325);
  Serial.println("\nUmidade"+String(UR)+"\n");

    
  int chk = DHT.read11(DHT11_PIN); // Leitura dos sensores
  TEMP = DHT.getTemperature();
  RU = DHT.getHumidity();
  PO = dewPoint(Temp0, UR);
  Serial.println(String(RU,1)+";"+ String(TEMP,1)+";"+String(PO,1)); //IMPRIME NA SERIAL O VALOR DE UMIDADE MEDIDO
  LCD(TEMP,UR,PO);
  UMD(TEMP,RU,PO);
  delay(2000); //INTERVALO DE 2 SEGUNDOS * NÃO DIMINUIR ESSE VALOR
  
  

  Serial.println("Temperaturas");

    DateTime now = rtc.now(); //CHAMADA DE FUNÇÃO
    Serial.print("Data: "); //IMPRIME O TEXTO NO MONITOR SERIAL
    Serial.print(now.day(), DEC); //IMPRIME NO MONITOR SERIAL O DIA
    Serial.print('/'); //IMPRIME O CARACTERE NO MONITOR SERIAL
    Serial.print(now.month(), DEC); //IMPRIME NO MONITOR SERIAL O MÊS
    Serial.print('/'); //IMPRIME O CARACTERE NO MONITOR SERIAL
    Serial.print(now.year(), DEC); //IMPRIME NO MONITOR SERIAL O ANO
    Serial.print(" / Dia: "); //IMPRIME O TEXTO NA SERIAL
    Serial.print(daysOfTheWeek[now.dayOfTheWeek()]); //IMPRIME NO MONITOR SERIAL O DIA
    Serial.print(" / Horas: "); //IMPRIME O TEXTO NA SERIAL
    Serial.print(now.hour(), DEC); //IMPRIME NO MONITOR SERIAL A HORA
    Serial.print(':'); //IMPRIME O CARACTERE NO MONITOR SERIAL
    Serial.print(now.minute(), DEC); //IMPRIME NO MONITOR SERIAL OS MINUTOS
    Serial.print(':'); //IMPRIME O CARACTERE NO MONITOR SERIAL
    Serial.print(now.second(), DEC); //IMPRIME NO MONITOR SERIAL OS SEGUNDOS
    Serial.println(); //QUEBRA DE LINHA NA SERIAL
    delay(1000); //INTERVALO DE 1 SEGUNDO


  
  
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
  lcd.clear();  
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
