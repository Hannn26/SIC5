#Import library yang dibutuhkan
from flask import Flask, jsonify
import paho.mqtt.client as mqtt
#flask untuk membuat API
#jsonify untuk parsing data json
#paho.mqtt.client untuk mengoneksikan ke mqtt

#buat instance dari flask
app = Flask(__name__)

# Settings untuk mqtt
MQTT_BROKER = "test.mosquitto.org"
MQTT_PORT = 1883
MQTT_KEEPALIVE_INTERVAL = 60
TOPIC_TEMPERATURE = "/home/sensor/data/temperature"
TOPIC_HUMIDITY = "/home/sensor/data/humidity"

# Buat variable global untuk meyimpan data dari temperature dan kelembapan
temperature = None
humidity = None

# Membuat fungsi untuk mengoneksikan ke mqtt
def on_connect(client, userdata, flags, rc):
    print(f"Connected to MQTT Broker with result code {rc}")
    client.subscribe(TOPIC_TEMPERATURE)
    client.subscribe(TOPIC_HUMIDITY)

# Membuat fungsi yang men-decode pesan dari mqtt
def on_message(client, userdata, msg):
    global temperature, humidity
    if msg.topic == TOPIC_TEMPERATURE:
        temperature = float(msg.payload.decode())
        print(f"Temperature updated: {temperature}")
    elif msg.topic == TOPIC_HUMIDITY:
        humidity = float(msg.payload.decode())
        print(f"Humidity updated: {humidity}")

# Menginisialisasikan mqtt client
mqtt_client = mqtt.Client()
mqtt_client.on_connect = on_connect
mqtt_client.on_message = on_message
mqtt_client.connect(MQTT_BROKER, MQTT_PORT, MQTT_KEEPALIVE_INTERVAL)
mqtt_client.loop_start()

#Membuat route untuk halaman default
@app.route('/')
def index():
    return "MQTT to HTTP API Server"

#Membuat route untuk data temperatur dan menginisialisasikan dengan method get pada API
@app.route('/data/temperature', methods=['GET'])
def get_temperature():
    if temperature is not None:
        return jsonify({"temperature": temperature})
    else:
        return jsonify({"error": "No temperature data available"}), 404

#Membuat route untuk data kelembapan dan menginisialisasikan dengan method get pada API
@app.route('/data/humidity', methods=['GET'])
def get_humidity():
    if humidity is not None:
        return jsonify({"humidity": humidity})
    else:
        return jsonify({"error": "No humidity data available"}), 404

#Menjalankan API dengan port 8000
if __name__ == '__main__':
    app.run(host='0.0.0.0',port=8000)