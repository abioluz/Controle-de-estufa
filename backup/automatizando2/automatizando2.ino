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
 * 
 * 
 *    p é o ponteiro então *p é o valor guardado no endereço
 *    x é uma variável então &x é o endereço da variável
 *    
 *    int x = 5;
 *    int *p; // declarando ponteiro
 *    p = &x; // pasando o endereço do x para o ponteiro
 *    resultado = *p; // passando o valor do endereço salvo no ponteiro p
 *    Serial.print(resultado); // será mostrado o valor 5
 *   
 */
 
#include "temperature.h"            //Inclusão de biblioteca: Para o calculo do Ponto de orvalho. 
#include <LiquidCrystal_I2C.h>      //Inclusão de biblioteca: Para usar o LCD.
#include "RTClib.h"                 //Inclusão de biblioteca: Para poder usar o relógio.
#include <OneWire.h>                //Inclusão de biblioteca: Para poder usar os sensores de temperatura DS18B20.
#include <DallasTemperature.h>      //Inclusão de biblioteca: Para poder usar os sensores de temperatura DS18B20.
#include <SD.h>                     //Inclusão de biblioteca: Para usar o microSSD.
#include <SPI.h>                    //Inclusão de biblioteca: Para usar o microSSD.     


OneWire oneWire(4); // Termometro no pino quatro
DallasTemperature Temp(&oneWire);
DeviceAddress insideThermometer;
LiquidCrystal_I2C lcd(0x27,16,2); //Inicializa o display no endereco 0x27
RTC_DS1307 rtc; //OBJETO DO TIPO RTC_DS1307 Para Relógio

const PROGMEM byte RU_MAX = 92; // Umidade Relativa máxima de trabalho
bool sim_nao = 0;
char liga_desliga = 'L';
int start = 5; //minutos
unsigned long time_inicio;
int controle = 0;

// Psicrometro usando dois termometros
float PSIC(float Ts, float Tu, float P, char Tipo='N'){ 
  float A;
  if (Tipo == 'F'){
    A = 0.000667; //Com Ventilação forçada
  }
  else{
    A = 0.0008;  //Sem Ventilação forçada
  } 
  float Es1 = 0.6108*pow(10,((7.5*Ts)/(237.3+Ts)));
  float Es2 = 0.6108*pow(10,((7.5*Tu)/(237.3+Tu)));
  float Ea = Es2 - A*P*(Ts-Tu);
  float UR = Ea*100/Es1;
  return UR;
}

void inicio(){
  if (millis() <= 10000){
    
    DateTime data_hora = rtc.now();
    if (SD.begin()) {
      Serial.println("SD Card pronto para uso."); 
    }
    else {
      Serial.println("Falha na inicialização do SD Card.");
      return;
    } 
    File myFile = SD.open("estufa.txt", FILE_WRITE); 
    if (myFile) { 
      myFile.print(data_hora.day());
      myFile.print("/");
      myFile.print(data_hora.month());
      myFile.print("/");
      myFile.print(data_hora.year());
      myFile.print(";");
      myFile.print(data_hora.hour());
      myFile.print(":");
      myFile.print(data_hora.minute());
      myFile.print(":");
      myFile.print(data_hora.second());
      myFile.println(";NAN;NAN;NAN;NAN;START");
      Serial.println("Salvando dados");
    }
    else {     
      Serial.println("Erro ao Abrir Arquivo .txt");
    }
    myFile.close(); 
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("INICIALIZANDO!");
    lcd.setCursor(0,1);
    for (byte i=0; i < 16; i++){
      lcd.print(".");
      delay(500);
    }
    time_inicio = millis();
  }
}

void UMD(float TEMP, float RU, float PO){

  if (RU <= RU_MAX){
    Serial.println("Liga");
    digitalWrite(8, HIGH);
    if (liga_desliga != 'L'){
      liga_desliga = 'L';
    }
  }
  else{
    Serial.println("Desliga2");
    digitalWrite(8, LOW);
    if (liga_desliga != 'D'){
      liga_desliga = 'D';
    }
  }
}

void ler(){
  if (SD.begin()) {
    Serial.println("SD Card pronto para uso."); 
  }
  else {
    Serial.println("Falha na inicialização do SD Card.");
    return;
  } 
  File myFile = SD.open("estufa.txt");
 
  if (myFile) { 
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
  }
  else {
    Serial.println("Falha na inicialização do SD Card.");
    return;
  } 
  myFile.close();
}
void apagar(){
  if (SD.begin()) {
    Serial.println("SD Card pronto para uso.");
  }
  else {
    Serial.println("Falha na inicialização do SD Card.");
    return;
  }
  SD.remove("estufa.txt");
  File myFile = SD.open("estufa.txt");
  if (!myFile) { 
    Serial.println("Arquivo Apagado com sucesso!");
  }
  myFile.close();
}


void salvar_sd(){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Gravando");
    lcd.setCursor(0,1);
    lcd.print("Arquivo");
    float RU;
    float PO;
    float temp_s_0 = 0;
    float temp_u_1 = 0;
    byte n = 10;
    for (int i = 0; i < n; i++){
      do{
        Temp.requestTemperatures();    
      } while (isnan(Temp.getTempCByIndex(0)) || Temp.getTempCByIndex(0) < -10 || Temp.getTempCByIndex(0) > 60 ||
             isnan(Temp.getTempCByIndex(1)) || Temp.getTempCByIndex(1) < -10 || Temp.getTempCByIndex(1) > 60 
            );
      temp_s_0 += Temp.getTempCByIndex(0);
      temp_u_1 += Temp.getTempCByIndex(1);
      delay(1000);
    }
    temp_s_0 /= n;
    temp_u_1 /= n; 
    do{
      RU = PSIC(temp_s_0, temp_u_1, 101.325);
      PO = dewPoint(temp_s_0,RU);
    } while(isnan(RU) || RU < 0 || RU > 200 ||
            isnan(PO));
    DateTime data_hora = rtc.now();
    UMD(temp_s_0,RU,PO);
    if (SD.begin()) {
      Serial.println("SD Card pronto para uso."); 
    }
    else {
      Serial.println("Falha na inicialização do SD Card.");
      return;
    } 
    File myFile = SD.open("estufa.txt", FILE_WRITE); 
    if (myFile) { 
      myFile.print(data_hora.day());
      myFile.print("/");
      myFile.print(data_hora.month());
      myFile.print("/");
      myFile.print(data_hora.year());
      myFile.print(";");
      myFile.print(data_hora.hour());
      myFile.print(":");
      myFile.print(data_hora.minute());
      myFile.print(":");
      myFile.print(data_hora.second());
      myFile.print(";");
      myFile.print(RU,2);
      myFile.print(";");
      myFile.print(temp_s_0,2);
      myFile.print(";");
      myFile.print(temp_u_1,2);
      myFile.print(";");
      myFile.print(PO,2);
      myFile.print(";");
      myFile.println(liga_desliga);
      Serial.println("Salvando dados");
    }
    else {     
      Serial.println("Erro ao Abrir Arquivo .txt");
    }
    myFile.close();
    
    sim_nao = 0;            
    start = 5;
    controle  = 0;
    time_inicio = millis();
    
}


void temp_umidade(){
  float RU;
  float PO;
  float temp_s_0 = 0;
  float temp_u_1 = 0;
    
  do{
    Temp.requestTemperatures();    
    temp_s_0 = Temp.getTempCByIndex(0);
    temp_u_1 = Temp.getTempCByIndex(1);
    RU = PSIC(temp_s_0, temp_u_1, 101.325);
    PO = dewPoint(temp_s_0,RU);
  } while (isnan(Temp.getTempCByIndex(0)) || Temp.getTempCByIndex(0) < -10 || Temp.getTempCByIndex(0) > 60 ||
           isnan(Temp.getTempCByIndex(1)) || Temp.getTempCByIndex(1) < -10 || Temp.getTempCByIndex(1) > 60 ||
           isnan(RU) || RU < 0 || RU > 200 ||
           isnan(PO)
          );
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("T/PO");
    lcd.print((char)223);
    lcd.print("C:");
    lcd.print(temp_s_0,1);
    lcd.print("/");
    lcd.print(PO,1);
    lcd.setCursor(0,1);
    lcd.print("Umidade: ");
    lcd.print(RU,1);
    lcd.print("%");
}

void data_hora(){
  DateTime data_hora = rtc.now();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Data: ");
  lcd.print(data_hora.day());
  lcd.print("/");
  lcd.print(data_hora.month());
  lcd.print("/");
  lcd.print(data_hora.year());
  lcd.setCursor(0,1);
  lcd.print(data_hora.hour());
  lcd.print(":");
  lcd.print(data_hora.minute());
  lcd.print("    Umid: ");
  lcd.print(liga_desliga);
}

void escolha_serial(int dados_serial ){
  switch (dados_serial){
      case 'L':
        Serial.println("Lendo Arquivo");
        ler();
        break;
      case 'A':
        Serial.println("Apagando");
        apagar();
        break;
      case 'S':
        salvar_sd();
        break;
        
      case 'R' :
        Serial.print("Valor do RU_MAX é: ");
        Serial.println(RU_MAX);
      break;

//      case 'H':
//        Serial.println("L: Ler dados do microSSD.");
//        Serial.println("A: Apagar dados do microSSD.");
//        Serial.println("S: Salvar dados no microSSD.");
//        Serial.println("R: Ler o valor do RU_MAX");
//      break;
      default:
        Serial.println("Comando Inválido");
        break;
    }
}

void setup()
{
  // Inicializando 
  Serial.begin(9600); //INICIALIZA A SERIAL
  Temp.begin(); // Sensor de temperatura
  Temp.setResolution(insideThermometer, 9);
  pinMode(10, OUTPUT); 
  pinMode(8, OUTPUT);  // Porta do relé
  digitalWrite(8, HIGH); // Ativando relé
  lcd.init();
  lcd.setBacklight(HIGH);
    
  if (! rtc.begin()) { 
    Serial.println("DS1307 não encontrado"); 
    //while(1); 
  }
  if (rtc.isrunning()) { 
    Serial.println("DS1307 rodando!");
    // rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); //CAPTURA A DATA E HORA EM QUE O SKETCH É COMPILADO
    // rtc.adjust(DateTime(2018, 7, 5, 1, 1, 1)); //(ANO), (MÊS), (DIA), (HORA), (MINUTOS), (SEGUNDOS)
  }
  inicio();
}
 
void loop()
{
  if(Serial.available() > 0){ 
      int dados_serial = Serial.read(); // Serial.read();
      if (dados_serial != -1 && dados_serial != 10){
        Serial.println(dados_serial);
        time_inicio = millis();
        sim_nao = 0;
       escolha_serial(dados_serial);
      }
      
  }
  
    if ( controle > start*60){ // 1 min = 60000
      salvar_sd();    
    
    //delay(5000);
    }
    else if ( millis() - time_inicio >= 0 && millis() - time_inicio <= 7000){
      if (! sim_nao){
        temp_umidade();
        sim_nao = 1;
      }
    }
    else if ( millis() - time_inicio > 7000 && millis() - time_inicio <= 10000){
      if (sim_nao){
        data_hora();
        sim_nao = 0;
      }
    }
    else if (millis() - time_inicio > 10000){
      time_inicio = millis();
      controle +=1;
    }
  
  
}

  