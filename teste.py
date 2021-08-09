import datetime

import serial
import time

def umidificadores(umidade,limite_interior, limite_superior):
	if umidade < limite_interior:
		#print("ligar_umidificadores")
		ser.write(COMANDOS["ligar_umidificadores"])
	elif umidade >= limite_superior:
		ser.write(COMANDOS["desligar_umidificadores"])
		#print("desligar_umidificadores")

def luz(hora_ligar, hora_desligar=None):
	hora = time.localtime()
	hora = datetime.time(hora.tm_hour,hora.tm_min)
	#print(hora)
	if hora >= hora_ligar and hora < hora_desligar:
		ser.write(COMANDOS["ligar_luz"])
		#print("ligar_luz")
	else:
		ser.write(COMANDOS["desligar_luz"])
		#print("desligar_luz")

def timer(minutos):
	minutos = minutos*60
	global TIMER_CONTROLE
	hora = time.time()
	if abs(TIMER_CONTROLE - hora) <= minutos:
		ser.write(COMANDOS["desligar_esxaustor_saida"])
		print("desligar_esxaustor_saida",abs(TIMER_CONTROLE - hora))
	elif abs(TIMER_CONTROLE - hora) <= minutos*2:
		ser.write(COMANDOS["ligar_esxaustor_saida"])
		print("ligar_esxaustor_saida",abs(TIMER_CONTROLE - hora))
	else:
		TIMER_CONTROLE = time.time()
		print("reset")


PORTA = "/dev/ttyUSB0"
VELOCIDADE = 9600
COMANDOS = {"ligar_esxaustor_saida" : b"LES",
			"desligar_esxaustor_saida" : b"DES",
			"ligar_luz" : b"LL",
			"desligar_luz" : b"DL",
			"ligar_umidificadores" : b"LU",
			"desligar_umidificadores" : b"DU",
			}

TIMER_CONTROLE = time.time()

# Declarando a porta serial
with serial.Serial(PORTA, VELOCIDADE) as ser:
	while True:
		dados = str(ser.readline())
		dados = dados[2:-5]
		lista = dados.split(";")
		try:
			umidade = float(lista[0])
			temperatura = float(lista[1])
			print(f'Umidade: {lista[0]}\nTemperatura: {lista[1]}')
			umidificadores(umidade, 85.00, 95.00)
			luz(datetime.time(8,0),datetime.time(14,49))
			timer(1)
		except:
			pass


# enviar comandos para o arduino por serial
# ser.write (b'comando_arduino_texto')
