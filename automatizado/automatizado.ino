

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
 * REFERENCIAS:
 * https://www.usinainfo.com.br/blog/projeto-arduino-sd-card-leitura-e-escrita-de-dados-no-cartao-micro-sd/
 * https://blogmasterwalkershop.com.br/arduino/como-usar-com-arduino-modulo-bluetooth-hc-05-hc-06
 * https://www.filipeflop.com/blog/tutorial-modulo-bluetooth-com-arduino/
 * https://www.arduinoportugal.pt/usar-memoria-eeprom-arduino/
 * https://www.paulotrentin.com.br/programacao/dicas/lendo-uma-string-com-arduino-via-serial/
 * 
 *    
 * >>>COLINHA:
 * 
 *    EEPROM.length(); //Retorna o tamanho da EEPROM
 *    EEPROM.write(endereço_bytes, valor_bytes);
 *    byte valor = EEPROM.read(endereço_bytes);
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
 
//#include "temperature.h"            //Inclusão de biblioteca: Para o calculo do Ponto de orvalho. 
#include <LiquidCrystal_I2C.h>      //Inclusão de biblioteca: Para usar o LCD.
#include "RTClib.h"                 //Inclusão de biblioteca: Para poder usar o relógio.
#include <OneWire.h>                //Inclusão de biblioteca: Para poder usar os sensores de temperatura DS18B20.
#include <DallasTemperature.h>      //Inclusão de biblioteca: Para poder usar os sensores de temperatura DS18B20.
#include <SD.h>                     //Inclusão de biblioteca: Para usar o microSSD.
#include <SPI.h>                    //Inclusão de biblioteca: Para usar o microSSD.   
// #include <EEPROM.h>     


OneWire oneWire(4); // Termometro no pino quatro
DallasTemperature Temp(&oneWire);
DeviceAddress insideThermometer;
LiquidCrystal_I2C lcd(0x27,16,2); //Inicializa o display no endereco 0x27
RTC_DS1307 rtc; //OBJETO DO TIPO RTC_DS1307 Para Relógio

// const PROGMEM byte endereco = 0;
const PROGMEM byte RU_MAX = 94; // RU_MAX; // Umidade Relativa máxima de trabalho
bool sim_nao;
char liga_desliga;
int start; //minutos
unsigned long time_inicio;
unsigned long time_salvar;
float TsTuRu[3] = {0,0,0};
int DMAHMS[6] = {0,0,0,0,0,0};


////////////// FUNÇÔES //////////////

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

void ler_DMAHMS(){
  DateTime data_hora = rtc.now();
  DMAHMS[0] = data_hora.day();
  DMAHMS[1] = data_hora.month();
  DMAHMS[2] = data_hora.year();
  DMAHMS[3] = data_hora.hour();
  DMAHMS[4] = data_hora.minute();
  DMAHMS[5] = data_hora.second();
}

void ler_TsTuRu(){
/*
  Devido a um problema de leitura dos sensores com o arduino uno, tive que fazer um melhor de 3.
  A temperatura é lida 3 vezes e guardada. Depois
  Ainda falta implementar um algoritmo para 
*/

  TsTuRu[0] = 0;
  TsTuRu[1] = 0;
  float T0[3]= {0,0,0};
  float T1[3] = {0,0,0};
  bool controle = false;

  do{

    for (int i = 0; i < 3; i++){
      do{
        Temp.requestTemperatures();   
          delay(800);
  
      } while (isnan(Temp.getTempCByIndex(0)) || Temp.getTempCByIndex(0) < 0 || Temp.getTempCByIndex(0) > 60 ||
              isnan(Temp.getTempCByIndex(1)) || Temp.getTempCByIndex(1) < 0 || Temp.getTempCByIndex(1) > 60 ||
              Temp.getTempCByIndex(0) > Temp.getTempCByIndex(1)
            );
      T0[i] = Temp.getTempCByIndex(0);
      T1[i] = Temp.getTempCByIndex(1);
    }

    if (T0[0] == T0[1]){
      controle = true;
    }
    else if (T0[0] == T0[2]){
      controle = true;
    }
    else if (T0[1] == T0[2]){
      T0[0] = T0[1];
      controle = true;
    }

    if (T1[0] == T1[1]){
      controle = controle && true;
    }
    else if (T1[0] == T1[2]){
      controle = controle && true;
    }
    else if (T1[1] == T1[2]){
      T1[0] = T1[1];
      controle = true;
    }
    else{
      controle = false;
    }
    if (controle){
      TsTuRu[0] = T0[0];
      TsTuRu[1] = T1[0];
      TsTuRu[2] = PSIC(TsTuRu[0], TsTuRu[1], 101.325);
    }

  } 
  while(isnan(TsTuRu[2]) || TsTuRu[2] < 50 || TsTuRu[2] > 100 || !controle);
    

  // do{
  //   TsTuRu[2] = PSIC(TsTuRu[0], TsTuRu[1], 101.325);

  // } 
  // while(isnan(TsTuRu[2]) || TsTuRu[2] < 50 || TsTuRu[2] > 100 );
}

void ler_temp_hora(){
  ler_TsTuRu();
  ler_DMAHMS();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(DMAHMS[0]);
  lcd.print("/");
  lcd.print(DMAHMS[1]);
  lcd.print("/");
  lcd.print(DMAHMS[2]);
  lcd.print(" ");
  lcd.print(DMAHMS[3]);
  lcd.print(":");
  lcd.print(DMAHMS[4]);
  lcd.setCursor(0,1);
  lcd.print(TsTuRu[0],1);
  lcd.print((char)223);
  lcd.print("C ");
  lcd.print(TsTuRu[2],1);
  lcd.print("% ");
  lcd.print(liga_desliga);
}

void UMD(float TEMP, float RU){
  if (RU <= RU_MAX){
//    Serial.println("Liga");
    liga_desliga = 'L';
    digitalWrite(8, HIGH);
  }
  else{
//    Serial.println("Desliga2");
    liga_desliga = 'D';
    digitalWrite(8, LOW);
  }
}

void ler_apagar(char n = 'L'){
  if (SD.begin()) {
//    Serial.println("SD Card pronto para uso."); 
  }
  else {
//    Serial.println("Falha na inicialização do SD Card.");
    return;
  } 

  if (n=='L'){  
    File myFile = SD.open("estufa.txt");
  
    if (myFile) { 
      while (myFile.available()) {
        Serial.write(myFile.read());
      }
    }
    else {
//      Serial.println("Falha na inicialização do SD Card.");
      return;
    }
    myFile.close();
  }  
  else if (n=='A'){
    SD.remove("estufa.txt");
    File myFile = SD.open("estufa.txt");
    if (!myFile) { 
     Serial.println("Arquivo Apagado com sucesso!");
    }
    myFile.close();
  }
  
}

// void apagar(){
//   if (SD.begin()) {
// //    Serial.println("SD Card pronto para uso.");
//   }
//   else {
// //    Serial.println("Falha na inicialização do SD Card.");
//     return;
//   }
//   SD.remove("estufa.txt");
//   File myFile = SD.open("estufa.txt");
//   if (!myFile) { 
// //    Serial.println("Arquivo Apagado com sucesso!");
//   }
//   myFile.close();

// }

void salvar(bool n = true){

  ler_TsTuRu();
  ler_DMAHMS();

  
  if (n){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("GRAVANDO");
    UMD(TsTuRu[0],TsTuRu[2]);
  }


  if (SD.begin()) {
//    Serial.println("SD Card pronto para uso."); 
  }
  else {
//    Serial.println("Falha na inicialização do SD Card.");
    return;
  } 
  File myFile = SD.open("estufa.txt", FILE_WRITE); 
  if (myFile) { 
    myFile.print(DMAHMS[0]);
    myFile.print("/");
    myFile.print(DMAHMS[1]);
    myFile.print("/");
    myFile.print(DMAHMS[2]);
    myFile.print(";");
    myFile.print(DMAHMS[3]);
    myFile.print(":");
    myFile.print(DMAHMS[4]);
    myFile.print(":");
    myFile.print(DMAHMS[5]);
    myFile.print(";");
    myFile.print(TsTuRu[2],2);
    myFile.print(";");
    myFile.print(TsTuRu[0],2);
    myFile.print(";");
    myFile.print(TsTuRu[1],2);
    myFile.print(";");
    if (liga_desliga == 'S'){
      myFile.println("START");
    }
    else{
      myFile.println(liga_desliga);
    }
//    Serial.println("Salvando dados");
  }
  else {     
//    Serial.println("Erro ao Abrir Arquivo .txt");
  }
  myFile.close(); 
}

void inicio(){
    digitalWrite(8, HIGH); // Ativando relé
    sim_nao = 1;
    liga_desliga = 'S';
    start = 15;

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("INICIALIZANDO!");

    salvar(false);
    
    lcd.setCursor(0,1);
    for (byte i=0; i < 16; i++){
      lcd.print(".");
      delay(100);
    }
    liga_desliga = 'L';
    time_inicio = millis();
    time_salvar = millis();
}

void escolha_serial(int dados_serial){
  if (dados_serial == 'L'){
//    Serial.println("Lendo Arquivo");
    ler_apagar();
  }
  else if (dados_serial == 'A'){
//    Serial.println("Apagando");
    ler_apagar('A');
    ler_apagar();
  }
  else if (dados_serial == 'S'){
    salvar();
  }
  else if (dados_serial == 'U'){
    digitalWrite(8, HIGH);
  }
  else if (dados_serial == 'D'){
    digitalWrite(8, LOW);
  }
  else if (dados_serial == 'R'){
    inicio();
  }
  else if (dados_serial == 'T'){
    start = 1;
  }
  
   else if (dados_serial == 'H'){
    Serial.println("L=LER, A=APAGAR, S=SALVAR, U=LIGAR UMIDIFICADOR, D=DESLIGAR, R=REINICIAR, T=DIMINUIR TEMPO START");
  //   Serial.println("LER: Para ler os dados salvos no microSD e mostra no serial.");
  //   Serial.println("APAGAR: Apaga todos os dados salvos no microSD.");
  //   Serial.println("SALVAR: Salva os dados no microSD.");
  //   Serial.println("TEMP: Lê a PO, umidade e a média de 15 temperaturas, mostrando no LCD por no minimo 30 segundos.");
  //   Serial.println("DATA: Lê a data e hora e se os umidificadores estão ligados, mostrando no LCD por no minimo 30 segundos.");
  //   // Serial.println("SET_RU_MAX: Para setar a umidade máxima de trabalho com valores entre 1 a 100. Valores fora deste intervalo serão setados como 94%.");
  //   Serial.println("RU_MAX: Mostra no serial o valor da umidade máxima de trabalho.");
   }
  else {
    Serial.println("Comando Inválido. Digite H");
  }
}


////////////// PROGRAMA //////////////


void setup()
{
  // Inicializando 
  Serial.begin(9600); //INICIALIZA A SERIAL
  Temp.begin(); // Sensor de temperatura
  Temp.setResolution(insideThermometer, 12);
  pinMode(10, OUTPUT); 
  pinMode(8, OUTPUT);  // Porta do relé
  lcd.init();
  lcd.clear();
  lcd.setBacklight(HIGH);

  // set_RU();
  
  if (! rtc.begin()) { 
    // Serial.println("DS1307 não encontrado"); 
    //while(1); 
  }
  if (rtc.isrunning()) { 
    // Serial.println("DS1307 rodando!");
    // rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); //CAPTURA A DATA E HORA EM QUE O SKETCH É COMPILADO
    // rtc.adjust(DateTime(2018, 7, 5, 1, 1, 1)); //(ANO), (MÊS), (DIA), (HORA), (MINUTOS), (SEGUNDOS)
  }
  inicio();

}
 
void loop()
{
  
  if(Serial.available() > 0){ 
    int leitura_serial = Serial.read();
    if (leitura_serial != 10 && leitura_serial != -1 ){
      escolha_serial(leitura_serial);
//      time_inicio = millis();
//      sim_nao = 1;
    }
  }
  
  if ( millis() - time_salvar > start*60000){ // 1 min = 60000
    salvar();
    sim_nao = 0;            
    start = 5;
    time_salvar  = millis();
    time_inicio = millis();
    return;
  }
  else if ( millis() - time_inicio >= 0 && millis() - time_inicio <= 10000){
    if (sim_nao){
      ler_temp_hora();
      sim_nao = 0;
    }
  }
  else if (millis() - time_inicio > 10000){
    time_inicio = millis();
    sim_nao = 1;

  }

}
