from pymata4 import pymata4
import time


PINO_DHT = 2
PINO_TESTE = 12

POLL_TIME = 5  # number of seconds between polls

# Callback data indices
CB_PIN_MODE = 0
CB_PIN = 1
CB_VALUE = 2
CB_TIME = 3

class Singleton:
	__instance = None


class Controle(object):
	def __new__(cls, *args, **kwargs):
		if not hasattr(cls, 'instance'):
			cls.instance = super(Controle, cls).__new__(cls)
			cls.arduino = pymata4.Pymata4()
			cls.ligar_sensores(cls)
		return cls.instance

	def __init__(self):
		pass
		# self.pin_dht = pin_dht
		# self.pin_rele_luz = pin_rele_luz
		# self.pin_rele_umidade = pin_rele_umidade
		# self.pin_rele_exaustor_saida = pin_rele_exaustor_saida
		# self.pin_rele_exaustor_entrada = pin_rele_exaustor_entrada

	def dht_leitura(self):
		leitura = self.arduino.dht_read(PINO_DHT)
		return leitura

	def ligar_sensores(self):
		self.arduino.set_pin_mode_dht(PINO_DHT, sensor_type=11)
		time.sleep(3)
		self.arduino.set_pin_mode_digital_output(PINO_TESTE)
		self.arduino.digital_write(PINO_TESTE,1)




# board = pymata4.Pymata4()
#
# board.set_pin_mode_dht(2, sensor_type=11)
# time.sleep(3)
# valor = board.dht_read(2)
# tlist = time.localtime(valor[2])
arduino = Controle()