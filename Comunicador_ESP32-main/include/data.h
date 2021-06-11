//GLOBAL VAR
int scanTime = 1;
const int CUTOFF = -50;
int best = CUTOFF;
long lastEeconnectAttemp = 0;
String formattedDate;
String nearby_alert;
String msg;
//WIFI
const char* ssid = "3R_TECHNICAL";
const char* password = "z1k61dqk";
//MQTT
const char* mqtt_server = "suigeneris.ml";
const char* mqtt_user = "indoor_client";
const char* mqtt_password = "Realtime_1";
int mqtt_port = 1883;
const char* topic = "indoor";
const char* topic_led = "unoled";

//const char* ... es un char array
