
/*
  Controle de estufa

  Biblioteca temperature.h é bibliteca padrão do arduino
   Temperature
   by Rob Tillaart versão 0.2.4
   Usado para calcular o ponto de orvalho da estufa.

   Bibliteca LiquidCrystal_I2C é uma biblioteca padrão do arduino
   LiquidCrystal I2C
   by Marco Schwartz Versão 1.1.2

   DHTStable é uma biblioteca padrão do arduino
   DHTStable
   by Rob Tillaart Versão 1.0.0


   REFERENCIAS:
   https://www.usinainfo.com.br/blog/projeto-arduino-sd-card-leitura-e-escrita-de-dados-no-cartao-micro-sd/
   https://blogmasterwalkershop.com.br/arduino/como-usar-com-arduino-modulo-bluetooth-hc-05-hc-06
   https://www.filipeflop.com/blog/tutorial-modulo-bluetooth-com-arduino/
   https://www.arduinoportugal.pt/usar-memoria-eeprom-arduino/
   https://www.paulotrentin.com.br/programacao/dicas/lendo-uma-string-com-arduino-via-serial/


   >>>COLINHA:

      EEPROM.length(); //Retorna o tamanho da EEPROM
      EEPROM.write(endereço_bytes, valor_bytes);
      byte valor = EEPROM.read(endereço_bytes);

      p é o ponteiro então *p é o valor guardado no endereço
      x é uma variável então &x é o endereço da variável

      int x = 5;
      int *p; // declarando ponteiro
      p = &x; // pasando o endereço do x para o ponteiro
      resultado = *p; // passando o valor do endereço salvo no ponteiro p
      Serial.print(resultado); // será mostrado o valor 5

   Exemplos de uso de ponteiros

  void ponteiro (float *vetor) {                // Aqui é declarado um ponteiro. A função não prescisa ser float
  vetor[0] = 1;                                // Não se coloca o * na frente da variável vetor.
  vetor[1] = 2;
  vetor[2] = 3;
  vetor[3] = vetor[0] + vetor[1] + vetor[2];   // As alterações serão feitas na variável que está fora deste escopo.
  }

  float *umafuncao() {       // Como vamos retornar, a função deve ser float e ela deve ser um
  static float vetor[2];   // É necessário o static na frente da declaração da variável para dar certo.
  vetor[0] = random(300);  // as alterações são feitas dentro do escopo da função
  vetor[1] = random(300);
  return vetor;             // Ela irá retornar o ponteiro do vetor para fora da função.
                            // Aqui pode ocorre algo chato.  A variável pode se perder (eu acho), pois está dentro da função
                            // Pode ser necessário realocar a memória com o malloc()
  }


  void setup() {
  Serial.begin(9600);
  float Po_vetor[4];
  ponteiro(PO_vetor); // O vetor já é um endereço de ponteiro enão não prescisa do & na frente.
                      // O resultado é que ele altera o valor do Po_vetor dentro da função.
                      // é necessário, pois de outra forma dá erro.



  float *ponteiro_funcao;               // A segunda forma e mais elegante
  ponteiro_funcao = umafuncao();        // Desta forma fica mais organica, fluida.

  Serial.println(ponteiro_funcao[0]);   // A ponteiro_funcao se comporta como o vetor estabelecido dentro da função.
  Serial.println(ponteiro_funcao[1]);
  }

*/

#include <LiquidCrystal_I2C.h>      //Inclusão de biblioteca: Para usar o LCD.
#include "RTClib.h"                 //Inclusão de biblioteca: Para poder usar o relógio.
#include <OneWire.h>                //Inclusão de biblioteca: Para poder usar os sensores de temperatura DS18B20.
#include <DallasTemperature.h>      //Inclusão de biblioteca: Para poder usar os sensores de temperatura DS18B20.
#include <SD.h>                     //Inclusão de biblioteca: Para usar o microSSD.
#include <SPI.h>                    //Inclusão de biblioteca: Para usar o microSSD.   


OneWire oneWire(4); // Termometro no pino quatro
DallasTemperature Temp(&oneWire);
DeviceAddress insideThermometer;
LiquidCrystal_I2C lcd(0x27, 16, 2); //Inicializa o display no endereco 0x27
RTC_DS1307 rtc; //OBJETO DO TIPO RTC_DS1307 Para Relógio

#define HUMIDADE_MINIMA 50 //50 // humidade minima para leitura
const PROGMEM byte RU_MAX = 95; // Umidade Relativa máxima de trabalho
const PROGMEM byte RU_MIN = 86; // Umidade Relativa minima de trabalho
// bool sim_nao;
char liga_desliga;
int start;
unsigned long time_inicio;
unsigned long time_salvar;


////////////// FUNÇÔES //////////////

// Psicrometro usando dois termometros
float PSIC(float bulbo_seco, float bulbo_umido, float pressao_kPA, char Tipo = 'N') {
  float A;
  if (Tipo == 'F') {
    A = 0.000667; //Com Ventilação forçada
  }
  else {
    A = 0.0008;  //Sem Ventilação forçada
  }
  float Es1 = 0.6108 * pow(10, ((7.5 * bulbo_seco) / (237.3 + bulbo_seco)));
  float Es2 = 0.6108 * pow(10, ((7.5 * bulbo_umido) / (237.3 + bulbo_umido)));
  float Ea = Es2 - A * pressao_kPA * (bulbo_seco - bulbo_umido);
  float UR = Ea * 100 / Es1;
  return UR;
}

int *ler_DMAHMS() {
  static int DMAHMS[6];
  DateTime data_hora = rtc.now();
  DMAHMS[0] = data_hora.day();
  DMAHMS[1] = data_hora.month();
  DMAHMS[2] = data_hora.year();
  DMAHMS[3] = data_hora.hour();
  DMAHMS[4] = data_hora.minute();
  DMAHMS[5] = data_hora.second();
  return DMAHMS;
}

float *ler_TsTuRu() {
  /*
    Devido a um problema de leitura dos sensores com o arduino uno, tive que fazer um melhor de 3.
    A temperatura é lida 3 vezes e guardada. Depois é verificado se há pelo menos
    2 valores correspondentes, se sim então este valor é usado.

    https://cta.if.ufrgs.br/projects/suporte-cta/wiki/DS18B20
  */
  static float TsTuRu[3];
  float T0[3] = {0, 0, 0};
  float T1[3] = {0, 0, 0};
  bool controle = false;
  int contador = 0;

  do {

    digitalWrite(5, HIGH);

    for (int i = 0; i < 3; i++) {
      do {

        contador++;

        if (contador == 30) {
          Serial.println("ERRO TEEMPERATURA");
          lcd.clear();
          lcd.print("ERRO DE LEITURA");
          lcd.setCursor(0, 1);
          lcd.print("DE TEMERATURA ");
          digitalWrite(8, HIGH);
          liga_desliga = 'L';
          digitalWrite(5, LOW);
          delay(30000);
          lcd.clear();
          lcd.print("REINICIANDO");
          lcd.setCursor(0, 1);
          lcd.print("LEITURA");
          digitalWrite(5, HIGH);
          delay(1000);
//          contador = 0;
        }
        else if (contador >= 60){
          TsTuRu[0] = NAN;
          TsTuRu[1]= NAN;
          TsTuRu[2] = NAN;
          return TsTuRu;
        }

        delay(1000);
        Temp.requestTemperatures();
//       Serial.println(Temp.getTempCByIndex(0));
//       Serial.println(Temp.getTempCByIndex(1));
      

      } while (Temp.getTempCByIndex(0) < 0 || Temp.getTempCByIndex(0) > 60 ||
               Temp.getTempCByIndex(1) < 0 || Temp.getTempCByIndex(1) > 60 // ||
               //              Temp.getTempCByIndex(0) < Temp.getTempCByIndex(1)
              );
      T0[i] = Temp.getTempCByIndex(0);
      T1[i] = Temp.getTempCByIndex(1);
      
    }
    digitalWrite(5, LOW);
    

    if (T0[0] == T0[1]) {
      controle = true;
    }
    else if (T0[0] == T0[2]) {
      controle = true;
    }
    else if (T0[1] == T0[2]) {
      T0[0] = T0[1];
      controle = true;
    }

    if (T1[0] == T1[1]) {
      controle = controle && true;
    }
    else if (T1[0] == T1[2]) {
      controle = controle && true;
    }
    else if (T1[1] == T1[2]) {
      T1[0] = T1[1];
      controle = true;
    }
    else {
      controle = false;
    }
    if (controle) {
      TsTuRu[0] = T0[0];
      TsTuRu[1] = T1[0];
      TsTuRu[2] = PSIC(TsTuRu[0], TsTuRu[1], 101.325);
      TsTuRu[2] = constrain(TsTuRu[2], 0, 100);
    }

  }
  while (!controle  ||TsTuRu[2] < HUMIDADE_MINIMA || TsTuRu[2] > 100 || isnan(TsTuRu[2])
        );
  return TsTuRu;
}


String zero(int numero) {
  if (numero >= 0 && numero < 10) {
    return "0" + String(numero);
  }
  else {
    return String(numero);
  }
}


void UMD_min_max(float RU) {
  if (RU <= RU_MIN || isnan(RU)) {
    liga_desliga = 'L';
    digitalWrite(8, HIGH);
  }
  else if (RU >= RU_MAX) {
    liga_desliga = 'D';
    digitalWrite(8, LOW);
  }
}

void ler_temp_hora() {
  float *TsTuRu;
  int *DMAHMS;
  TsTuRu = ler_TsTuRu();
  DMAHMS = ler_DMAHMS();

  UMD_min_max(TsTuRu[2]);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(zero(DMAHMS[0]));
  lcd.setCursor(2, 0);
  lcd.print("/");
  lcd.print(zero(DMAHMS[1]));
  lcd.setCursor(5, 0);
  lcd.print("/");
  lcd.print(DMAHMS[2]);
  lcd.setCursor(11, 0);
  lcd.print(zero(DMAHMS[3]));
  lcd.setCursor(13, 0);
  lcd.print(":");
  lcd.print(zero(DMAHMS[4]));
  lcd.setCursor(0, 1);
  lcd.print(TsTuRu[0], 2);
  lcd.print((char)223);
  lcd.print("C ");
  lcd.setCursor(8, 1);
  lcd.print(TsTuRu[2], 1);
  lcd.print("% ");
  lcd.setCursor(15, 1);
  lcd.print(liga_desliga);
}



void UMD(float RU) {
  if (RU <= RU_MAX || isnan(RU)) {
    liga_desliga = 'L';
    digitalWrite(8, HIGH);
  }
  else {
    liga_desliga = 'D';
    digitalWrite(8, LOW);
  }
}


void SSD(char n = 'S') {
  //Serial.print("N = ");
 // Serial.println(n);
  if (SD.begin()) {
//        Serial.println("SD Card pronto para uso.");
  }
  else {
        Serial.println("Falha na inicialização do SD Card.");
    return;
  }

  if (n == 'S' || n == 'I') {
    float *TsTuRu;
    int *DMAHMS;
    TsTuRu = ler_TsTuRu();
    DMAHMS = ler_DMAHMS();
    
    if (n == 'S') {
      lcd.clear();
      lcd.setCursor(3, 0);
      lcd.print("GRAVANDO");
//      UMD(TsTuRu[2]);
//      UMD_min_max(TsTuRu[2]);
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
      myFile.print(TsTuRu[0], 2);
      myFile.print(";");
      myFile.print(TsTuRu[1], 2);
      myFile.print(";");
      myFile.print(TsTuRu[2], 2);
      myFile.print(";");
      if (liga_desliga == 'S') {
        myFile.println("START");
      }
      else {
        myFile.println(liga_desliga);
      }
//          Serial.println("Salvando dados");
    }
    else {
          Serial.println("Erro ao Abrir Arquivo .txt");
    }
    myFile.close();
  }
  else if (n == 'L') {
    File myFile = SD.open("estufa.txt");

    if (myFile) {
      while (myFile.available()) {
        Serial.write(myFile.read());
      }
    }
    else {

          Serial.println("Erro ao Ler Arquivo .txt");
    }      
    
    myFile.close();

  }
  else if (n == 'A') {
    SD.remove("estufa.txt");
    File myFile = SD.open("estufa.txt");
    if (!myFile) {
      Serial.println("Arquivo Apagado com sucesso!");
    }
    myFile.close();
  }

}

void inicio() {

  liga_desliga = 'S';
  start = 15;

  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("INICIALIZANDO!");

  SSD('I');

  lcd.setCursor(0, 1);
  for (byte i = 0; i < 16; i++) {
    lcd.print(".");
    delay(100);
  }
  
  liga_desliga = 'L';
  ler_temp_hora();
  time_inicio = millis();
  time_salvar = millis();
  
}



void escolha_serial(char dados_serial) {
//  Serial.println(dados_serial);
  if (dados_serial == 'L') {
    SSD('L');
  }
  else if (dados_serial == 'A') {
    SSD('A');
    
  }
  else if (dados_serial == 'S') {
    SSD();
  }
  else if (dados_serial == 'U') {
    digitalWrite(8, HIGH);
    liga_desliga = 'L';
  }
  else if (dados_serial == 'D') {
    digitalWrite(8, LOW);
    liga_desliga = 'D';
  }
  else if (dados_serial == 'R') {
    inicio();
  }
  else if (dados_serial == 'T') {
    start = 1;
  }

  else if (dados_serial == 'H') {
    Serial.println("L=LER, A=APAGAR, S=SALVAR, U=LIGAR UMIDIFICADOR, D=DESLIGAR, R=REINICIAR, T=DIMINUIR TEMPO START,");

  }
  else {
    // Serial.println("Comando Inválido. Digite H");
  }
}

////////////// PROGRAMA //////////////


void setup() {
  // Inicializando
  Serial.begin(9600); //INICIALIZA A SERIAL

  lcd.init();
  lcd.clear();
  lcd.setBacklight(HIGH);

  pinMode(10, OUTPUT);
  pinMode(8, OUTPUT);  // Porta do relé
  digitalWrite(8, HIGH); // Ativando relé
  pinMode(5, OUTPUT); // Alimentação dos Sensores
  digitalWrite(5, HIGH); // Ligar os sensores

  Temp.begin(); // Sensor de temperatura
  for (int i = 0; i < 2; i++) {
    if (Temp.getAddress(insideThermometer, i))
    {
      Temp.setResolution(insideThermometer, 12);
    }
  }


  if (! rtc.begin()) {
//    Serial.println("DS1307 não encontrado");
    //while(1);
  }
  if (rtc.isrunning()) {
//    Serial.println("DS1307 rodando!");
    // rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); //CAPTURA A DATA E HORA EM QUE O SKETCH É COMPILADO
    // rtc.adjust(DateTime(2018, 7, 5, 1, 1, 1)); //(ANO), (MÊS), (DIA), (HORA), (MINUTOS), (SEGUNDOS)
  }
//     rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); //CAPTURA A DATA E HORA EM QUE O SKETCH É COMPILADO

  inicio();

}

void loop() {

  if (Serial.available() > 0) {
    char leitura_serial = Serial.read();
    if (leitura_serial != 10 && leitura_serial != -1) {
//      Serial.println(leitura_serial);
      escolha_serial(leitura_serial);
    }
  }

  if ( millis() - time_salvar > start * 60000) { // 1 min = 60000
    SSD('S');
    start = 5;
    time_salvar  = millis();
    time_inicio = millis();
    return;
  }

  else if (millis() - time_inicio > 10000) {
    ler_temp_hora();
    time_inicio = millis();
  }

}
