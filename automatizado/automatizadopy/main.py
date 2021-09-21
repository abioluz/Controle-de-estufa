from machine import Pin
import time

led = Pin(25,Pin.OUT)

led.value(1)

# while True:
#     led.toggle()

#     time.sleep(1)