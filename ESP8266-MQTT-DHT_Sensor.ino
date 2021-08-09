#include "DHTesp.h"
DHTesp dht;

#include <timer.h>
auto dhtTimer = timer_create_default(); // create a timer with default settings

void setup() {
  Serial.begin(115200);
  connectWifi();
  connectMqtt();
  connectNtp();
  dht.setup(2, DHTesp::DHT22); // Connect DHT sensor to D4
  dhtTimer.every(10000, takeMesurements);
}

void loop() {
  checkMQTT();
  dhtTimer.tick(); // tick the timer
}

bool takeMesurements(void *){
  String date= getDate();
  String time= getTime();
  float temperature=dht.getTemperature();
  float humidity=dht.getHumidity();
  Serial.println((String)"Date (CET) : "+date);
  Serial.println((String)"Time (CET) : "+time);
  Serial.println((String)"Temperature : "+temperature);
  Serial.println((String)"Humidity : "+humidity);
  publishMqtt("/DHT_Sensor", (String)"{\"date\": \""+date+"\", \"time\": \""+time+"\", \"temperature\": \""+temperature+"\", \"humidity\": \""+humidity+"\"}");
  return true;
}
