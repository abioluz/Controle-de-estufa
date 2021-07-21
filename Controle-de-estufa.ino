




//https://blogmasterwalkershop.com.br/arduino/como-usar-com-arduino-modulo-real-time-clock-rtc-ds1307
//https://blogmasterwalkershop.com.br/arduino/como-usar-com-arduino-modulo-sensor-de-umidade-e-temperatura-dht11
//https://circuitdigest.com/microcontroller-projects/interfacing-mq135-gas-sensor-with-arduino-to-measure-co2-levels-in-ppm
//https://davidegironi.blogspot.com/2014/01/cheap-co2-meter-using-mq135-sensor-with.html#.YPQu7nVKiV4


#include <Wire.h> //INCLUSÃO DA BIBLIOTECA
#include "RTClib.h" //INCLUSÃO DA BIBLIOTECA
#include <dht.h>



RTC_DS1307 rtc; //OBJETO DO TIPO RTC_DS1307
dht DHT;
 
void setup () {
  pinMode(13, OUTPUT);
  pinMode(12, OUTPUT);

  Serial.begin(9600); //INICIALIZA A SERIAL
  if (! rtc.begin()) { // SE O RTC NÃO FOR INICIALIZADO, FAZ
    Serial.println("DS1307 não encontrado"); //IMPRIME O TEXTO NO MONITOR SERIAL
    while(1); //SEMPRE ENTRE NO LOOP
  }
  if (! rtc.isrunning()) { //SE RTC NÃO ESTIVER SENDO EXECUTADO, FAZ
    Serial.println("DS1307 rodando!"); //IMPRIME O TEXTO NO MONITOR SERIAL
    //REMOVA O COMENTÁRIO DE UMA DAS LINHAS ABAIXO PARA INSERIR AS INFORMAÇÕES ATUALIZADAS EM SEU RTC
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); //CAPTURA A DATA E HORA EM QUE O SKETCH É COMPILADO

  }

   Serial.begin(9600);
   Serial.println("DHTxx test!");

  
  delay(100); //INTERVALO DE 100 MILISSEGUNDOS
}
 
void loop () {
    print_info();
    luz(15,30);
    umidade_temperatura(85.00, 10.00, 20.00, 2);
    delay(1000); //INTERVALO DE 1 SEGUNDO

    
}
void umidade_temperatura(float umidade, float margem_umidade, float temperatura, float margem_temperatura){
  
  DHT.read11(2);
  if (DHT.humidity  <= umidade-margem_umidade && DHT.humidity <= umidade){
      Serial.println("Liga umidade");
      porta_on_of(12, 1);
  
  }
  else{
    Serial.println("Desliga umidade");
    porta_on_of(12, 0);
    }
  
  //DHT.humidity
  //DHT.temperature
  
  
}

void luz(int inicio,int fim){
  DateTime horas = rtc.now();
  if (horas.second() >= inicio && horas.second() <= fim){
      Serial.println("Liga");
      porta_on_of(13, 1);
  }
  else {
      Serial.println("Desliga");
      porta_on_of(13, 0);
  }
}

void porta_on_of(int porta, int on_of){
  
  if (on_of == 1){
    digitalWrite(porta, HIGH);
  
  }
  else {
    digitalWrite(porta, LOW);
  }
}

void print_info(){
  DateTime now = rtc.now(); //CHAMADA DE FUNÇÃO
    Serial.print("Data: "); //IMPRIME O TEXTO NO MONITOR SERIAL
    Serial.print(now.day(), DEC); //IMPRIME NO MONITOR SERIAL O DIA
    Serial.print('/'); //IMPRIME O CARACTERE NO MONITOR SERIAL
    Serial.print(now.month(), DEC); //IMPRIME NO MONITOR SERIAL O MÊS
    Serial.print('/'); //IMPRIME O CARACTERE NO MONITOR SERIAL
    Serial.print(now.year(), DEC); //IMPRIME NO MONITOR SERIAL O ANO
    Serial.print(" / Dia: "); //IMPRIME O TEXTO NA SERIAL
    Serial.print(now.dayOfTheWeek()); //IMPRIME NO MONITOR SERIAL O DIA
    Serial.print(" / Horas: "); //IMPRIME O TEXTO NA SERIAL
    Serial.print(now.hour(), DEC); //IMPRIME NO MONITOR SERIAL A HORA
    Serial.print(':'); //IMPRIME O CARACTERE NO MONITOR SERIAL
    Serial.print(now.minute(), DEC); //IMPRIME NO MONITOR SERIAL OS MINUTOS
    Serial.print(':'); //IMPRIME O CARACTERE NO MONITOR SERIAL
    Serial.print(now.second(), DEC); //IMPRIME NO MONITOR SERIAL OS SEGUNDOS
    Serial.println(); //QUEBRA DE LINHA NA SERIAL

    
  DHT.read11(2); //LÊ AS INFORMAÇÕES DO SENSOR
  Serial.print("Umidade: "); //IMPRIME O TEXTO NA SERIAL
  Serial.print(DHT.humidity); //IMPRIME NA SERIAL O VALOR DE UMIDADE MEDIDO
  Serial.print("%"); //ESCREVE O TEXTO EM SEGUIDA
  Serial.print(" / Temperatura: "); //IMPRIME O TEXTO NA SERIAL
  Serial.print(DHT.temperature, 0); //IMPRIME NA SERIAL O VALOR DE UMIDADE MEDIDO E REMOVE A PARTE DECIMAL
  Serial.println("*C"); //IMPRIME O TEXTO NA SERIAL
}
