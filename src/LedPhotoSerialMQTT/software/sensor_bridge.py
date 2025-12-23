import serial
import time
import random
from paho.mqtt.client import Client

SERIAL_PORT = 'COM8'
BAUD_RATE = 9600
MQTT_BROKER = "broker.emqx.io"
LUMINOSITY_TOPIC = "laboratory/greenhouse/luminosity"
CLIENT_ID = f'SENSOR_{random.randint(1000, 9999)}'

try:
    serial_conn = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
    time.sleep(2)
    print(f"Подключено к Sensor MCU на {SERIAL_PORT}")
except Exception as e:
    print(f"Ошибка подключения к Serial: {e}")
    exit()


def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Подключено к MQTT Broker!")
    else:
        print(f"Ошибка подключения, код: {rc}")


mqtt_client = Client(client_id=CLIENT_ID)
mqtt_client.on_connect = on_connect

mqtt_client.connect(MQTT_BROKER)
mqtt_client.loop_start()

try:
    while True:
        serial_conn.write(b'p')

        if serial_conn.in_waiting > 0:
            line = serial_conn.readline().decode('utf-8').strip()
            if "PHOTO_VALUE:" in line:
                try:
                    value_str = line.split(":")[1]
                    mqtt_client.publish(LUMINOSITY_TOPIC, value_str, qos=1)
                    print(f"[Sensor PC] Прочитано: {value_str} -> Опубликовано")
                except IndexError:
                    pass

        time.sleep(2)

except KeyboardInterrupt:
    print("Остановка...")
    serial_conn.close()
    mqtt_client.disconnect()
    mqtt_client.loop_stop()