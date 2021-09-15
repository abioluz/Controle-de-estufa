

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
 * 
 * 
 * 
 * https://www.usinainfo.com.br/blog/projeto-arduino-sd-card-leitura-e-escrita-de-dados-no-cartao-micro-sd/
 * https://blogmasterwalkershop.com.br/arduino/como-usar-com-arduino-modulo-bluetooth-hc-05-hc-06
 * https://www.filipeflop.com/blog/tutorial-modulo-bluetooth-com-arduino/
 */
 
#include "temperature.h"
#include <LiquidCrystal_I2C.h>
#include "RTClib.h" //INCLUSÃO DA BIBLIOTECA
#include <Wire.h> //INCLUSÃO DA BIBLIOTECA
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SD.h>
#include <SPI.h>

#include <SoftwareSerial.h> //INCLUSÃO DE BIBLIOTECA


const int pinoRX = 0; //PINO DIGITAL 2 (RX)
const int pinoTX = 2; //PINO DIGITAL 3 (TX)
int dadoBluetooth; //VARIÁVEL QUE ARMAZENA O VALOR ENVIADO PELO BLUETOOTH


SoftwareSerial bluetooth(pinoRX, pinoTX); //PINOS QUE EMULAM A SERIAL, ONDE
//O PINO 2 É O RX E O PINO 3 É O TX

//Inicializa o display no endereco 0x27
LiquidCrystal_I2C lcd(0x27,16,2);

//OBJETO DO TIPO RTC_DS1307 Para Relógio
RTC_DS1307 rtc; 

// Ativando Sensor de temperatura DS18B20
// Definindo a porta para a leitura
#define ONE_WIRE_BUS 4
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);
// arrays to hold device address
DeviceAddress insideThermometer;

// SD
File myFile;

// Variáveis
int pinoSS = 10;
int RU_MAX = 90; // Umidade Relativa máxima de trabalho
float Temp_seco_0; // Temperatura ambiente e do bulbo seco
float Temp_umido_1; // Temperatura do bulbo úmido
float RU;   // Guardar a variável Umidade Relativa do Ar
float PO;  // Guardar o ponto de Orvalho 
char daysOfTheWeek[7][4] = {"Dom", "Seg", "Ter", "Qua", "Qui", "Sex", "Sab"};
unsigned long time_inicio;
unsigned long time_inicio_gravacao;
unsigned long time_fim;


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
 bluetooth.begin(9600); //INICIALIZA A SERIAL DO BLUETOOTH
  
 time_inicio = millis();
 time_inicio_gravacao = millis();
 Serial.begin(9600); //INICIALIZA A SERIAL
 sensors.begin(); // Sensor de temperatura
 // set the resolution to 9 bit (Each Dallas/Maxim device is capable of several different resolutions)
  sensors.setResolution(insideThermometer, 9);

 // Iniciando o LCD 
 lcd.init();
 lcd.setBacklight(HIGH);
 lcd.clear();

 // Iniciando o RCT
 if (! rtc.begin()) { // SE O RTC NÃO FOR INICIALIZADO, FAZ
    Serial.println("DS1307 não encontrado"); //IMPRIME O TEXTO NO MONITOR SERIAL
    while(1); //SEMPRE ENTRE NO LOOP
  }
 if (rtc.isrunning()) { //SE RTC NÃO ESTIVER SENDO EXECUTADO, FAZ
    Serial.println("DS1307 rodando!"); //IMPRIME O TEXTO NO MONITOR SERIAL
    // REMOVA O COMENTÁRIO DE UMA DAS LINHAS ABAIXO PARA INSERIR AS INFORMAÇÕES ATUALIZADAS EM SEU RTC
    // rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); //CAPTURA A DATA E HORA EM QUE O SKETCH É COMPILADO
    // rtc.adjust(DateTime(2018, 7, 5, 1, 1, 1)); //(ANO), (MÊS), (DIA), (HORA), (MINUTOS), (SEGUNDOS)
  }
// rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  
}

 
void loop()
{
  //delay(495);
  DateTime now = rtc.now(); //CHAMADA DE FUNÇÃO
  sensors.requestTemperatures();
  Temp_seco_0 = sensors.getTempCByIndex(0);
  Temp_umido_1 = sensors.getTempCByIndex(1);
  RU = PSIC(Temp_seco_0, Temp_umido_1, 101.325);
  PO = dewPoint(Temp_seco_0, RU);

  if ( millis() - time_inicio_gravacao > 300000){ // 5 min = 300000
    time_inicio_gravacao = millis();
    String texto = String(now.day())+";"+String(now.month())+";"+String(now.year())+";"+String(now.hour())+";"+String(now.minute())+";"+String(now.second())+";"+String(RU,1)+";"+ String(Temp_seco_0,1)+";"+ String(Temp_umido_1,1)+";"+String(PO,1);
    Serial.println(texto); //IMPRIME NA SERIAL O VALOR DE UMIDADE MEDIDO
    escrever(texto);
    ler();

  }
  String temperatura = "T/PO"+String((char)223)+"C:"+String(Temp_seco_0,1)+"/"+String(PO,1);
  String umidade = "Umidade: "+String(RU,1)+"%";
  String data = String(daysOfTheWeek[now.dayOfTheWeek()])+": "+String(now.day())+"/"+String(now.month())+"/"+String(now.year());
  String hora = "    "+String(now.hour())+":"+String(now.minute())+":"+String(now.second());
  LCD(temperatura, umidade, data, hora);

  if(bluetooth.available()){ //SE O BLUETOOTH ESTIVER HABILITADO, FAZ
       dadoBluetooth = bluetooth.read(); //VARIÁVEL RECEBE O VALOR ENVIADO PELO BLUETOOTH
       Serial.println(dadoBluetooth);
       if (dadoBluetooth == 'V'){
        Serial.println("Deu certo");
        ler();
       }
       else{
        Serial.println("Ainda não");}
       
  }  

  
  //UMD(Temp_seco_0,RU,PO);
  
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


// Printar no LCD os dados
void LCD (String temperatura, String umidade, String data, String hora){
   if (millis()-time_inicio > 10000){
    time_inicio = millis();
  
  }
  lcd.clear();
  if (millis()-time_inicio <= 5000){
    lcd.setCursor(0,0);
    lcd.print(temperatura);
    lcd.setCursor(0,1);
    lcd.print(umidade);
    
  }
  if (millis()-time_inicio > 5000){
    lcd.setCursor(0,0);
    lcd.print(data);
    lcd.setCursor(0,1);
    lcd.print(hora);
  }
 
 
}

void escrever(String texto){
  pinMode(pinoSS, OUTPUT); // Declara pinoSS como saída
  
  if (SD.begin()) { // Inicializa o SD Card
    Serial.println("SD Card pronto para uso."); // Imprime na tela
  }
 
  else {
    Serial.println("Falha na inicialização do SD Card.");
    return;
  } 
  
  myFile = SD.open("estufa.txt", FILE_WRITE); // Cria / Abre arquivo .txt
  if (myFile) { // Se o Arquivo abrir imprime:
    myFile.println(texto); // Escreve no Arquivo
    myFile.close(); // Fecha o Arquivo após escrever
  }  
  
}

void ler(){
  if (SD.begin()) { // Inicializa o SD Card
    Serial.println("SD Card pronto para uso."); // Imprime na tela
  }
 
  else {
    Serial.println("Falha na inicialização do SD Card.");
    return;
  } 


  myFile = SD.open("estufa.txt"); // Abre o Arquivo
 
  if (myFile) { 
    while (myFile.available()) { // Exibe o conteúdo do Arquivo
      Serial.write(myFile.read());
    }
 
  myFile.close(); // Fecha o Arquivo após ler
  }
}

  
