
#include "dht.h" //INCLUSÃO DE BIBLIOTECA

// ler comando enviado pelo python
// char situacao = Serial.read();



const int pinoDHT11 = 2; //PINO ANALÓGICO UTILIZADO PELO DHT11

dht DHT; //VARIÁVEL DO TIPO DHT

void setup(){
  Serial.begin(9600); //INICIALIZA A SERIAL
}

void loop(){
  DHT.read11(pinoDHT11); //LÊ AS INFORMAÇÕES DO SENSOR
  Serial.println(String(DHT.humidity)+";"+ String(DHT.temperature)); //IMPRIME NA SERIAL O VALOR DE UMIDADE MEDIDO
  delay(2000); //INTERVALO DE 2 SEGUNDOS * NÃO DIMINUIR ESSE VALOR
}
