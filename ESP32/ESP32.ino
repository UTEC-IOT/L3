// Incluir librerias
#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// Definicion del pin y el tipo de pin del sensor
#define DHTPIN 4
#define DHTTYPE DHT11

// Inicializacion del pin del sensor
DHT dht(DHTPIN, DHTTYPE);

// Definicion de variables para realizar la conexion Wi-Fi
const char* ssid = "InternetWL";
const char* password = "CsM7Jtc7PydEkA7m65UR";

const char* mqtt_broker = "192.168.1.24";
const char* topic = "test/topic";
const int mqtt_port = 1883;

// Definicion del cliente esp
WiFiClient espClient;
PubSubClient client(espClient);

// Funcion de callback para manejar mensajes MQTT entrantes
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message: ");

  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }

  Serial.println();
  Serial.println(" - - - - - - - - - - - -");
}

void setup() {
  Serial.begin(115200);

  // Conexion Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
 
  Serial.println("Connected to the WiFi network");

  // Inicializacion del sensor DHT
  dht.begin();

  // Conexion al broker MQTT
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);

  // Esperar a que se establezca la conexion con el broker MQTT
  while (!client.connected()) {
    String client_id = "esp8266-client-";
    client_id += String(WiFi.macAddress());
    
    Serial.printf("The client %s connects to mosquitto mqtt broker\n", client_id.c_str());
    
    if (client.connect(client_id.c_str())) {
      Serial.println("Public emqx mqtt broker connected");
    } else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }

  // Publicar un mensaje en el topic MQTT al establecer la conexion
  client.publish(topic, "Hello From ESP32!");
  client.subscribe(topic);
}

void loop() {
  delay(2000);
  
  // Lectura del sensor DHT
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float f = dht.readTemperature(true);

  // Verificar si la lectura del sensor fue exitosa
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // Crear un mensaje con los datos del sensor
  String message = "Temperature: " + String(t) + " C, Humidity: " + String(h) + "%";
  // Publicar el mensaje en el topic MQTT
  client.publish(topic, message.c_str());
  
  // Manejo de mensajes MQTT
  client.loop();
}
