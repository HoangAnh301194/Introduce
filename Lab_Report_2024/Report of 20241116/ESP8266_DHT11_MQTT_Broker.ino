#include<ESP8266WiFi.h>
#include<PubSubClient.h>
#include<DHT11.h>
DHT11 dht11(2);  // GPIO2 = D4 in esp8266
const int buttonPin = 4; // Chân kết nối nút nhấn
int pressCount = 0;      // Biến đếm số lần nhấn nút

void IRAM_ATTR countPressing() {
  static unsigned long lastInterruptTime = 0; // Thời gian xảy ra ngắt lần cuối
  unsigned long interruptTime = millis();    // Thời gian hiện tại

  // Nếu thời gian giữa hai lần ngắt lớn hơn ngưỡng debounce
  if (interruptTime - lastInterruptTime > 50) { 
    pressCount++; // Tăng biến đếm
  }
  lastInterruptTime = interruptTime; // Cập nhật thời gian ngắt cuối
}


const char* ssid = "P203";
const char * pw = "anhbaprovip";

const char* mqtt_ip_server = "192.168.1.22";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client (espClient);

void setup_wifi(){
  Serial.println("connecting to Wifi-----");
  WiFi.begin(ssid, pw);
  while(WiFi.status() != WL_CONNECTED){
    delay(100);
    Serial.print("--");
  }
  Serial.println("\n Wifi connected :3 !!!!");
  }

void connect_to_MQTT (){
  while(!client.connected()){
    Serial.print("connecting to MQTT Broker");
    if(client.connect("ESP8266_Client")){
      Serial.println("Connected!!! :>");
      client.subscribe("MQTT_ESP32/Publish");
      client.subscribe("MQTT_ESP32/DHT11_tmp");
      client.subscribe("MQTT_ESP32/ButtonCommand");
      client.subscribe("MQTT_ESP32/DHT11_hmd");
    }
    else {
      Serial.print("failed, rc=");
      Serial.println(client.state());
    }

  }
}
void setup() {
  pinMode(buttonPin,INPUT_PULLUP);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_ip_server, mqtt_port);
  attachInterrupt(digitalPinToInterrupt(buttonPin), countPressing , FALLING);
}

void loop() {
  if (!client.connected()) {
    connect_to_MQTT();
  }
  client.loop();
  int t = 0;
  int h = 0;
  int result = dht11.readTemperatureHumidity(t,h);
  if(result == 0){
        Serial.print("Temperature: ");
        Serial.print(t);
        Serial.print(" °C\tHumidity: ");
        Serial.print(h);
        Serial.println(" %");
        String tmp = "Temperature: " + String(t) + "oC";
        String hmd = "Humidity: " + String(h) + "%";
        client.publish("MQTT_ESP32/DHT11_tmp", tmp.c_str());
        client.publish("MQTT_ESP32/DHT11_hmd", hmd.c_str());
  }
  if(pressCount >= 2 ) pressCount = 0;
  Serial.println(pressCount);
  client.publish("MQTT_ESP32/ButtonCommand",String(pressCount).c_str());

}
