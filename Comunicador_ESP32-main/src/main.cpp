#include <WiFi.h>
#include <data.h>
#include <WiFiUdp.h>
#include <Arduino.h>
#include <BLEScan.h>
#include <NTPClient.h>
#include <BLEDevice.h>
#include <PubSubClient.h>
#include <BLEAdvertisedDevice.h>

#define ONBOARD_LED 2
//MQTT
WiFiClient espClient;
PubSubClient client(espClient);
//TIME
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
//BLE
BLEScan* scan;
BLEAdvertisedDevice advert;

//WIFI
void wifi_config(){
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  int counter = 0;
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
    counter++;
    if(counter > 10){
      ESP.restart();
    }
  }
  Serial.print("IP Network: ");
  Serial.println(WiFi.localIP());
}

//MQTT reconnect
bool reconnect(){
  client.setServer(mqtt_server, mqtt_port);  //Iniciamos el servidor MQTT
  if(client.connect("usuario_prueba", mqtt_user, mqtt_password)){ //Colocamos las credenciales
    client.subscribe(topic);
    client.subscribe(topic_led);
    Serial.println("Conectados por MQTT");
    return true;
  }else{
    Serial.println("No estamos conectados por MQTTT");
    return false;
  }
}

//MQTT
void check_mqtt_connection(){
  if (WiFi.status() != WL_CONNECTED){
    Serial.println("No estamos conectados a la red");
    delay(5000);
    ESP.restart();
  }

  if(!client.connected()){
    long now = millis();
    if(now - lastEeconnectAttemp > 5000){
      lastEeconnectAttemp = millis();
      if(reconnect()){
        lastEeconnectAttemp = 0;
      }
    }
  }
  else{
    client.loop();
  }
}

//LED ON/OFF
void callback(String topic_3, byte* message, unsigned int length){
  for(int i=0; i < length; i++){
    nearby_alert += (char)message[i];
  }

  if(topic_3 == topic_led){
    if(nearby_alert == "on"){
      Serial.println("\nEncendiendo led");
      digitalWrite(ONBOARD_LED, HIGH);
    }
    else if(nearby_alert == "off"){
      Serial.println("\nApagando led");
      digitalWrite(ONBOARD_LED, LOW);
    }
  }
  nearby_alert = "";
}

//SETUP
void setup(){
  Serial.begin(921600);
  pinMode(ONBOARD_LED, OUTPUT);
  BLEDevice::init("");  //Iniciamos el BLE
  wifi_config();
  client.setCallback(callback);
  timeClient.begin();
  timeClient.setTimeOffset(7200); //3600 o 7200
}

//LOOP
void loop(){
  check_mqtt_connection();

  scan = BLEDevice::getScan();  //Comenzamos con el escaneo de dispositivos
  scan->setActiveScan(true);
  BLEScanResults results = scan->start(scanTime, false);  //Obtenemos un resultaod inicial

  for(int i=0;i<results.getCount();i++) {
    while(!timeClient.update()){
      timeClient.forceUpdate();
    }
    formattedDate = timeClient.getFormattedDate();
    advert = results.getDevice(i);
    String g_mac = WiFi.macAddress();
    int d_rssi = advert.getRSSI();  //Tomamos los valores que necesitamos enviar
    String bleName = advert.getName().c_str();
    String d_mac = advert.getAddress().toString().c_str();
    String msg = String("{\"timestamp\":") + String("\"") + String(formattedDate) + String("\"") +
                        String(",\"type\":\"device\",\"g_mac\":") + String("\"") + String(g_mac) + String("\"") +
                        String(",\"mac\":") + String("\"") + String(d_mac) + String("\"") +  String(",\"rssi\":") +
                        int(d_rssi) + String(",\"bleName\":") + String("\"") + String(bleName) + String("\"") + String("}");

    Serial.println(msg);
    client.publish(topic, msg.c_str()); //c_str() -> cambia el String a un char array (const char*)
  }
}
