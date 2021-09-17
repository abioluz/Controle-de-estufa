

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

int RU_MAX = 95; // Umidade Relativa máxima de trabalho
bool sim_nao = 0;
char liga_desliga = 'L';
int start = 20; //minutos
unsigned long time_inicio;
unsigned long time_inicio_gravacao;
// unsigned long time_fim;

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
  lcd.clear();
  time_inicio = millis();
  time_inicio_gravacao = millis();

  if (! rtc.begin()) { 
    Serial.println("DS1307 não encontrado"); 
    while(1); 
  }
  if (rtc.isrunning()) { 
    Serial.println("DS1307 rodando!");
    // rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); //CAPTURA A DATA E HORA EM QUE O SKETCH É COMPILADO
    // rtc.adjust(DateTime(2018, 7, 5, 1, 1, 1)); //(ANO), (MÊS), (DIA), (HORA), (MINUTOS), (SEGUNDOS)
  }
}

 
void loop()
{
  int dados_serial = Serial.read();
  DateTime data_hora = rtc.now();



/*   float RU;
  float PO;
  do{
    Temp.requestTemperatures();
     RU = PSIC(Temp.getTempCByIndex(0), Temp.getTempCByIndex(1), 101.325);
     PO = dewPoint(Temp.getTempCByIndex(0),RU);

  } while (isnan(Temp.getTempCByIndex(0)) || Temp.getTempCByIndex(0) < -10 || Temp.getTempCByIndex(0) > 60 ||
           isnan(Temp.getTempCByIndex(1)) || Temp.getTempCByIndex(1) < -10 || Temp.getTempCByIndex(1) > 60 ||
           isnan(RU) ||  RU < 0 || RU > 150 ||
           isnan(PO) 
          ); */
  
  float RU;
  float PO;
  float temp_s_0 = 0;
  float temp_u_1 = 0;
  for (int i = 0; i < 5; i++){

    do{
      Temp.requestTemperatures();

    } while (isnan(Temp.getTempCByIndex(0)) || Temp.getTempCByIndex(0) < -10 || Temp.getTempCByIndex(0) > 60 ||
            isnan(Temp.getTempCByIndex(1)) || Temp.getTempCByIndex(1) < -10 || Temp.getTempCByIndex(1) > 60 
            );

    temp_s_0 += Temp.getTempCByIndex(0);
    temp_u_1 += Temp.getTempCByIndex(1);

  }
  temp_s_0 /= 5;
  temp_u_1 /= 5; 

  RU = PSIC(temp_s_0, temp_u_1, 101.325);
  PO = dewPoint(temp_s_0,RU);
  

  if ( millis() - time_inicio >= 0 && millis() - time_inicio <= 4000){
    if (sim_nao){
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
      sim_nao = 1;
    }
  }
  else if ( millis() - time_inicio > 4000 && millis() - time_inicio <= 8000){
    if (sim_nao){
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
      sim_nao = 0;
    }
  }
  else if (millis() - time_inicio > 8000){
    time_inicio = millis();

  }


  if ( millis() - time_inicio_gravacao > start*60000 ||dados_serial == 'S' ){ // 1 min = 60000
    UMD(Temp.getTempCByIndex(0),RU,PO);
    time_inicio_gravacao = millis();
    start = 5;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Gravando");
    lcd.setCursor(0,0);
    lcd.print("Arquivo");

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
    delay(5000);
    time_inicio = millis();
    sim_nao = 0;            
  }

  if (dados_serial != -1 && dados_serial != 10){
    Serial.println(dados_serial);
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
        
        break;

      default:
        Serial.println("Comando Inválido");
        break;
    }
  }
  
}

void UMD(float TEMP, float RU, float PO){
  if (PO >= TEMP){
    Serial.println("Desliga");
    digitalWrite(8, LOW);
    if (liga_desliga != 'D'){
      liga_desliga = 'D'; 
    }
  }
  else{
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
}


/* // Printar no LCD os dados
void LCD (String Primeira, String Segunda){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(Primeira);
  lcd.setCursor(0,1);
  lcd.print(Segunda); 
} */

/*
void escrever(String texto){
  
  if (SD.begin()) {
    Serial.println("SD Card pronto para uso."); 
  }
  else {
    Serial.println("Falha na inicialização do SD Card.");
    return;
  } 
  File myFile = SD.open("estufa.txt", FILE_WRITE); 
  if (myFile) { 
    myFile.println(texto); 
    Serial.println("Salvando dados");
    Serial.println(texto);
  }
  else {     
    Serial.println("Erro ao Abrir Arquivo .txt");
  }
  myFile.close();
}
*/

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


  
