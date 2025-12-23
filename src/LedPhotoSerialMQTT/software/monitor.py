import time
import random
from paho.mqtt.client import Client, MQTTMessage
from paho.mqtt.enums import CallbackAPIVersion

MQTT_BROKER = "broker.emqx.io"
TOPIC_PATTERN = "laboratory/greenhouse/#"
CLIENT_ID = f'MONITOR_{random.randint(1000, 9999)}'

def on_connect(client, userdata, flags, rc, properties):
    print(f"Монитор подключен с кодом: {rc}")
    client.subscribe(TOPIC_PATTERN)
    print(f"Мониторинг топиков: {TOPIC_PATTERN}")


def on_message(client, userdata, message: MQTTMessage):
    timestamp = time.strftime("%H:%M:%S", time.localtime())
    topic_name = message.topic
    payload_str = message.payload.decode("utf-8")
    print(f"[{timestamp}] ТОПИК: {topic_name:<35} | СООБЩЕНИЕ: {payload_str}")


mqtt_client = Client(callback_api_version=CallbackAPIVersion.VERSION2, client_id=CLIENT_ID)
mqtt_client.on_connect = on_connect
mqtt_client.on_message = on_message

mqtt_client.connect(MQTT_BROKER)

try:
    print("--- СИСТЕМА МОНИТОРИНГА ЗАПУЩЕНА ---")
    mqtt_client.loop_forever()
except KeyboardInterrupt:
    print("Монитор остановлен")
    mqtt_client.disconnect()