#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

/************* WiFi Access Point **************/
#define WLAN_SSID       "**"
#define WLAN_PASS       "**"
/**********************************************/

/***************** MQTT server ****************/
#define MQTT_SERVER        "192.168.1.32"
#define MQTT_SERVERPORT    1883   // use 8883 for SSL
#define MQTT_USERNAME      ""
#define MQTT_KEY           ""
const String MQTT_TOPIC =  "/feeds/sensor";
WiFiClient espClient;
PubSubClient client(espClient);


/**********************************************/


/***************** NTP client ****************/
#define NTP_OFFSET   0 // 60 * 60      // In seconds
#define NTP_INTERVAL 60 * 1000    // In miliseconds
#define NTP_ADDRESS  "europe.pool.ntp.org"
#define LEAP_YEAR(Y)     ( (Y>0) && !(Y%4) && ( (Y%100) || !(Y%400) ) )
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);
/**********************************************/

void connectWifi(){
  // Connect to WiFi access point.
  Serial.print("\n");
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP address: "); Serial.println(WiFi.localIP());
  Serial.println(WiFi.macAddress());
}

void connectMqtt(){
  client.setServer(MQTT_SERVER, MQTT_SERVERPORT);
  client.setCallback(callback);
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";
    clientId += WiFi.macAddress();
    Serial.print(clientId);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe("/inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void publishMqtt(String subTopic, String payload){
  char attributes[100];
  char topic[100];
  String topicName=MQTT_TOPIC+subTopic;
  payload.toCharArray( attributes, 100 );
  topicName.toCharArray(topic,100);
  client.publish(topic, attributes);
//  Serial.print("publish to");
  client.subscribe("/inTopic");
//  Serial.print("subscribe to /inTopic");
}

void connectNtp(){
//WiFiUDP ntpUDP;
//NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);
  timeClient.begin();
}

String getDate() {
  timeClient.update();
  unsigned long rawTime = timeClient.getEpochTime() / 86400L;  // in days
  unsigned long days = 0, year = 1970;
  uint8_t month;
  static const uint8_t monthDays[]={31,28,31,30,31,30,31,31,30,31,30,31};
  while((days += (LEAP_YEAR(year) ? 366 : 365)) <= rawTime)
    year++;
  rawTime -= days - (LEAP_YEAR(year) ? 366 : 365); // now it is days in this year, starting at 0
  days=0;
  for (month=0; month<12; month++) {
    uint8_t monthLength;
    if (month==1) { // february
      monthLength = LEAP_YEAR(year) ? 29 : 28;
    } else {
      monthLength = monthDays[month];
    }
    if (rawTime < monthLength) break;
    rawTime -= monthLength;
  }
  String monthStr = ++month < 10 ? "0" + String(month) : String(month); // jan is month 1  
  String dayStr = ++rawTime < 10 ? "0" + String(rawTime) : String(rawTime); // day of month  
  return String(year) + "-" + monthStr + "-" + dayStr ;
}

String getTime(){
  timeClient.update();
  return timeClient.getFormattedTime();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";
    clientId += WiFi.macAddress();
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe("/inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void checkMQTT(){
  client.loop();
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}
