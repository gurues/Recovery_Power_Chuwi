*/

Recovery Power miniPC Chuwi

Mediante un servomotor realizamos un accionamiento mec,anico que permite arrancar el miniPC tras recuperar la energía

gurues@2022

*/

#include <Arduino.h>
#include <Servo.h>
#include <ESP8266WiFi.h>
#include <Ticker.h>             // Temporizador funciones asincronas
#include <AsyncMqttClient.h>    // MQTT Libreria asincrona basada en eventos
#include <ArduinoOTA.h>         // Actualización por OTA
#include <ESP8266mDNS.h>        // Actualización por OTA
#include <WiFiUdp.h>            // Actualización por OTA

//Descomenta para usar DEBUG por puerto serie
//#define ___DEBUG___

// Datos red WIFI 
const char* ssid = "XXXXXXXXXXXXX"; //Nombre de la red WiFi a la que nos vamos a conectar
const char* password = "XXXXXXXXXXXXX"; //Contraseña de la red WiFi a la que nos vamos a conectar

IPAddress ip(xxx, xxx, x, x); // Modificar en platformio.ini si se cambia la IP
IPAddress gateway(192,168,1,1);
IPAddress subnet (255,255,255,0);

// Configuración MQTT
#define MQTT_HOST IPAddress(xxx, xxx, x, x) // host del Broker MQTT
#define MQTT_PORT 1883
const char* user = "xxxxxx";  // user mqtt
const char* pass = "xxxxxx";  // pass mqtt user

const char* topicEstado = "Casa/Autopower/Estado";
const char* topicControl = "Casa/Autopower/Control";
static const char* topicTestamento = "Casa/Autopower/Testamento";

// Objeto MQTT
AsyncMqttClient mqttClient;
uint16_t keepAlive = 30; // 1200 -> 20 minutos tiempo de conexión abierta

// CObjetos controladores de eventos WIFI
WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;

// Programadores de eventos Ticker
Ticker mqttReconnectTimer, wifiReconnectTimer;

const int pin = 5;
Servo myservo;  
int pos_init = 80;        //  Posición inicial del servo
int pos_action = 130;     //  Posiciónactuación del servo
bool reinicia = false;    // Variable de control 
bool reset = false;       // Variable de control 

//    Funciones del Broker MQTT
//***********************************************************************************************************************

//Función para conectarse al Broker MQTT
void connectToMqtt() {
  
#ifdef ___DEBUG___
  Serial.println("********** connectToMqtt ************");
  Serial.println("Conectando Auto Power Chuwi al Broker MQTT...");
#endif

  mqttClient.connect();
}

// Evento producido cuando se conecta al Broker
void onMqttConnect(bool sessionPresent) {

  #ifdef ___DEBUG___
    Serial.println("********** onMqttConnect ************");
    Serial.println("Connected to MQTT.");
    Serial.print("Session present: ");
    Serial.println(sessionPresent);
  #endif

  uint16_t packetIdSub = mqttClient.subscribe(topicControl, 2);
  mqttClient.setWill(topicTestamento, 2, false, "Recovery Power Chuwi offline");
  mqttClient.publish(topicEstado, 0, false, "Recovery Power Chuwi online");
  
  #ifdef ___DEBUG___
    Serial.print("Suscrito a topicControl QoS 2, packetId: ");
    Serial.println(packetIdSub);
  #endif
}

//Evento cuando se desconecta del Broker
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {

#ifdef ___DEBUG___
  Serial.println("********** onMqttDisconnect ************");
  Serial.println("Auto Power Chuwi Desconectado del MQTT.....");
#endif

  if (WiFi.isConnected()) {
    connectToMqtt(); 
  }
}

//Gestión de Mensajes Suscritos MQTT ***************************************************************************************************
void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {

  String Str_topic = String(topic);
  String Str_payload;
  for (size_t i = 0; i < len; i++) {
    Str_payload+=(char)payload[i];
  }
 
  #ifdef ___DEBUG___
    Serial.println("********** onMqttMessage ************");
    Serial.print("Message llegado [");
    Serial.print(Str_topic);
    Serial.print("]= "); Serial.println(Str_payload);
    Serial.print("  qos: ");  Serial.println(properties.qos);
    Serial.print("  dup: ");  Serial.println(properties.dup);
    Serial.print("  retain: ");  Serial.println(properties.retain);
  #endif

  
  if (Str_topic==(String)topicControl){

    // Se reinicia Wemos NodeRed
    if (Str_payload == "reset") {
      reset =true;
      mqttClient.publish(topicEstado, 0, false, "reset");
    } 

    if (Str_payload == "reinicia") {
      reinicia = true;
      mqttClient.publish(topicEstado, 0, false, "reinicia");
    }


  }
}

// Conectando a la WiFi network
void setup_wifi() {

    delay(10);
  #ifdef ___DEBUG___
    Serial.println();
    Serial.print("Conectado a ");
    Serial.println(ssid);
  #endif
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    WiFi.config(ip,gateway,subnet);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      #ifdef ___DEBUG___
        Serial.print(".");
      #endif
    }
    randomSeed(micros());
  #ifdef ___DEBUG___
    Serial.println("");
    Serial.println("WiFi conectado");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println("");
  #endif
}

void onWifiConnect(const WiFiEventStationModeGotIP& event) {
  #ifdef ___DEBUG___
    Serial.println("Connected to Wi-Fi.");
  #endif
  mqttReconnectTimer.once(2, connectToMqtt);
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
  #ifdef ___DEBUG___
    Serial.println("Disconnected from Wi-Fi.");
  #endif
  mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
  wifiReconnectTimer.once(2, setup_wifi);
}


void setup() {

  #ifdef ___DEBUG___
    Serial.begin(115200);
  #endif

  delay(30000); //retardp de 30 seg para disponer de tensión en el enchufe del PC                                                    

  // Configuración y actuación inicial del servo
  myservo.attach(pin, 500, 2450);
  myservo.write(pos_init);     
  delay(3000);
  myservo.write(pos_action);  
  delay(1000);
  myservo.write(pos_init);   

  // Inicializo manipuladores/controladores eventos WIFI
  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);
 
  //Configuración MQTT
  mqttClient.setKeepAlive(keepAlive);
  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.setCredentials(user, pass);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);

  //Configuración y conecta a WIFI
  setup_wifi();

  // Inicializo OTA
  ArduinoOTA.setHostname("on_chuwi"); // Hostname OTA
  ArduinoOTA.begin();

}

void loop() {

  ArduinoOTA.handle();    // Actualización código por OTA

  if (reinicia){
    myservo.write(pos_init); 
    delay(1000);
    myservo.write(pos_action);  
    delay(5000);
    myservo.write(pos_init); 
    reinicia = false;
  }
  
  if(reset){
    delay(2000);
    ESP.restart();
  }

}