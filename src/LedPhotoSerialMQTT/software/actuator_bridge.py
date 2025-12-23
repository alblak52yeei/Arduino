import serial
import time
import random
from paho.mqtt.client import Client, MQTTMessage
from paho.mqtt.enums import CallbackAPIVersion

SERIAL_PORT = 'COM4'
BAUD_RATE = 9600
MQTT_BROKER = "broker.emqx.io"
LUMINOSITY_TOPIC = "laboratory/greenhouse/luminosity"
STATUS_TOPIC = "laboratory/greenhouse/light_status"
CLIENT_ID = f'ACTUATOR_{random.randint(1000, 9999)}'

LOW_LIGHT_THRESHOLD = 500

try:
    serial_conn = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
    time.sleep(2)
    print(f"Подключено к Actuator MCU на {SERIAL_PORT}")
except Exception as e:
    print(f"Ошибка подключения к Serial: {e}")
    exit()


def on_connect(client, userdata, flags, rc, properties):
    if rc == 0:
        print("Подключено к MQTT Broker")
        client.subscribe(LUMINOSITY_TOPIC, qos=1)
        print(f"Подписка на {LUMINOSITY_TOPIC}")
    else:
        print(f"Ошибка подключения, код: {rc}")


def on_message(client, userdata, message: MQTTMessage):
    try:
        payload_str = message.payload.decode("utf-8")
        light_level = int(payload_str)
        print(f"[Actuator PC] Получена освещенность: {light_level}")

        cmd = ''
        if light_level < LOW_LIGHT_THRESHOLD:
            cmd = 'u'
        else:
            cmd = 'd'

        serial_conn.write(cmd.encode())

        time.sleep(0.1)
        if serial_conn.in_waiting > 0:
            response_line = serial_conn.readline().decode('utf-8').strip()
            print(f"[Actuator PC] Ответ MCU: {response_line}")

            client.publish(STATUS_TOPIC, response_line, qos=1)

    except ValueError:
        print("Получены некорректные данные")


mqtt_client = Client(callback_api_version=CallbackAPIVersion.VERSION2, client_id=CLIENT_ID)
mqtt_client.on_connect = on_connect
mqtt_client.on_message = on_message

mqtt_client.connect(MQTT_BROKER)

try:
    mqtt_client.loop_forever()
except KeyboardInterrupt:
    print("Остановка...")
    serial_conn.close()
    mqtt_client.disconnect()