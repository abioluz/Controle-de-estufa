

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
 
#include "temperature.h"            //Inclusão de biblioteca: Para o calculo do Ponto de orvalho. 
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
byte RU_MAX = 94; // RU_MAX; // Umidade Relativa máxima de trabalho
bool sim_nao = 0;
char liga_desliga = 'L';
int start = 20; //minutos
unsigned long time_inicio;
int controle = 0;
// float TsTuRuPo[4] = {0,0,0,0};


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

String ler_serial(){
  String texto = "";
  char leitura_serial;
  
  while(Serial.available() > 0) {
    leitura_serial = Serial.read();
    if (leitura_serial != '\n'){
      texto.concat(leitura_serial);
    }
    delay(10);
  }
  return texto;
}

void ler_temp(int n=1){

  int *TsTuRuPo = ler_TsTuRuPo(n);
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("T/PO");
  lcd.print((char)223);
  lcd.print("C:");
  lcd.print(TsTuRuPo[0],1);
  lcd.print("/");
  lcd.print(TsTuRuPo[3],1);
  lcd.setCursor(0,1);
  lcd.print("Umidade: ");
  lcd.print(TsTuRuPo[2],1);
  lcd.print("%");
}

float *ler_TsTuRuPo(int n=1){
  float TsTuRuPo = {0,0,0,0};

  for (int i = 0; i < n; i++){
    do{
      Temp.requestTemperatures();    
    } while (isnan(Temp.getTempCByIndex(0)) || Temp.getTempCByIndex(0) < -10 || Temp.getTempCByIndex(0) > 60 ||
            isnan(Temp.getTempCByIndex(1)) || Temp.getTempCByIndex(1) < -10 || Temp.getTempCByIndex(1) > 60 
          );
    TsTuRuPo[0] += Temp.getTempCByIndex(0);
    TsTuRuPo[1] += Temp.getTempCByIndex(1);
    delay(500);
  }
  TsTuRuPo[0] /= n;
  TsTuRuPo[1] /= n; 
  do{
    TsTuRuPo[2] = PSIC(TsTuRuPo[0], TsTuRuPo[1], 101.325);
    TsTuRuPo[3] = dewPoint(TsTuRuPo[0],TsTuRuPo[2]);
  } while(isnan(TsTuRuPo[2]) || TsTuRuPo[2] < 0 || TsTuRuPo[2] > 200 ||
          isnan(TsTuRuPo[3])
          );
return TsTuRuPo;
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

/* void set_RU(){
  if (RU_MAX == 0 || RU_MAX > 100){
    EEPROM.write(0, 94); 
  }
} */

void inicio(){

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

void ler_DataHora(){
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

void salvar(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Gravando");
  lcd.setCursor(0,1);
  lcd.print("Arquivo");
  
  float *TsTuRuPo = ler_TsTuRuPo(20);

  DateTime data_hora = rtc.now();

  UMD(TsTuRuPo[0],TsTuRuPo[2],TsTuRuPo[3]);

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
    myFile.print(TsTuRuPo[2],2);
    myFile.print(";");
    myFile.print(TsTuRuPo[0],2);
    myFile.print(";");
    myFile.print(TsTuRuPo[1],2);
    myFile.print(";");
    myFile.print(TsTuRuPo[3],2);
    myFile.print(";");
    myFile.println(liga_desliga);
    Serial.println("Salvando dados");
  }
  else {     
    Serial.println("Erro ao Abrir Arquivo .txt");
  }
  myFile.close(); 
}

void escolha_serial(String dados_serial){
  if (dados_serial == "LER"){
    Serial.println("Lendo Arquivo");
    ler();
  }
  else if (dados_serial == "APAGAR"){
    Serial.println("Apagando");
    apagar();
  }
  else if (dados_serial == "SALVAR"){
    salvar();
  }
  else if (dados_serial == "TEMP"){
    
    for (int i = 0; i < 30; i++){
      ler_temp(10);
      delay(1000);
    }
  }
  else if (dados_serial == "DATA"){
    for (int i = 0; i < 30; i++){
      ler_DataHora();
      delay(1000);
    }
  }
  /* else if (dados_serial == "SET_RU_MAX"){
    Serial.print("RU_MAX atual: ");
    Serial.println(RU_MAX);
    Serial.println("Digite a Umidade máxima de trabalho.");
    Serial.println("Valores iguais a 0 e maiores a 100 serão convertidos para 94");
    do {
      dados_serial = ler_serial();
    }
    while (dados_serial == "");

    EEPROM.write(0, dados_serial.toInt());
    set_RU();
    Serial.print("RU_MAX atualizado: ");
    Serial.println(RU_MAX);
  } */
  else if (dados_serial == "RU_MAX"){
    Serial.print("RU_MAX: ");
    Serial.println(RU_MAX);
  }
  else if (dados_serial == "HELP"){
    Serial.println("LER: Para ler os dados salvos no microSD e mostra no serial.");
    Serial.println("APAGAR: Apaga todos os dados salvos no microSD.");
    Serial.println("SALVAR: Salva os dados no microSD.");
    Serial.println("TEMP: Lê a PO, umidade e a média de 15 temperaturas, mostrando no LCD por no minimo 30 segundos.");
    Serial.println("DATA: Lê a data e hora e se os umidificadores estão ligados, mostrando no LCD por no minimo 30 segundos.");
    // Serial.println("SET_RU_MAX: Para setar a umidade máxima de trabalho com valores entre 1 a 100. Valores fora deste intervalo serão setados como 94%.");
    Serial.println("RU_MAX: Mostra no serial o valor da umidade máxima de trabalho.");
  }
  else {
    Serial.println("Comando Inválido");
    Serial.println("Digite HELP para mais informações \n");
  }
}


////////////// PROGRAMA //////////////


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
  // set_RU();
  inicio();

  if (! rtc.begin()) { 
    Serial.println("DS1307 não encontrado"); 
    //while(1); 
  }
  if (rtc.isrunning()) { 
    Serial.println("DS1307 rodando!");
    // rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); //CAPTURA A DATA E HORA EM QUE O SKETCH É COMPILADO
    // rtc.adjust(DateTime(2018, 7, 5, 1, 1, 1)); //(ANO), (MÊS), (DIA), (HORA), (MINUTOS), (SEGUNDOS)
  }
  
}
 
void loop()
{
  
  if(Serial.available() > 0){ 
    escolha_serial(ler_serial()); // Serial.read();
    time_inicio = millis();
    sim_nao = 0;
  }
  else {
    if ( millis() - time_inicio >= 0 && millis() - time_inicio <= 7000){
      if (! sim_nao){
        ler_temp();
        sim_nao = 1;
      }
    }
    else if ( millis() - time_inicio > 7000 && millis() - time_inicio <= 10000){
      if (sim_nao){
        ler_DataHora();
        sim_nao = 0;
      }
    }
    else if (millis() - time_inicio > 10000){
      time_inicio = millis();
      controle +=1;

    }

    if ( controle > start*60){ // 1 min = 60000
      salvar();
      //delay(5000);
      sim_nao = 0;            
      start = 5;
      controle  = 0;
      time_inicio = millis();
    }
  }
}
