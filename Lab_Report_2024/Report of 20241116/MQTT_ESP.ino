#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>  // Thư viện LCD I2C

// Địa chỉ I2C của màn hình LCD, thông thường là 0x27 hoặc 0x3F
LiquidCrystal_I2C lcd(0x27, 16, 2); // Địa chỉ I2C, màn hình 16 cột và 2 dòng
const int ledPin = 4; 
bool ledStatus= false;
// Thông tin mạng Wi-Fi
const char* ssid = "P203";       // Thay bằng tên Wi-Fi của bạn
const char* password = "anhbaprovip";      // Thay bằng mật khẩu Wi-Fi của bạn

// Thông tin MQTT broker
const char* mqtt_server = "192.168.1.22";   // Địa chỉ IP của máy tính chạy broker
const int mqtt_port = 1883;                  // Cổng MQTT (thường là 1883)

// Tạo WiFi và MQTT client
WiFiClient espClient;
PubSubClient client(espClient);


//------------------------------------------------------------------------------
void setup_wifi() {  // kết nối với wifi 
  Serial.println("Connecting to Wi-Fi...");
  lcd.setCursor(0, 0);
  lcd.print("Connecting..");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print("..");
    lcd.print(".");
  }
  Serial.println("\nWiFi connected!");
  lcd.setCursor(0, 0);
  lcd.print("Wifi connected");
}
//-----------------------------------------------------------------------------------
void connect_to_mqtt() {  // kết nối với broker MQTT
  while (!client.connected()) {
    Serial.print("Connecting to MQTT Broker...");
   if (client.connect("ESP32_Client")) {
      Serial.println("connected!");
      client.subscribe("MQTT_ESP32/Publish");
      client.subscribe("MQTT_ESP32/DHT11_tmp"); // topic dữ liệu cảm biến
      client.subscribe("MQTT_ESP32/DHT11_hmd");
      client.subscribe("MQTT_ESP32/ButtonCommand"); // Topic dữ liệu nút nhấn
    } else {
      Serial.print("failed, rc=");
      Serial.println(client.state());
      delay(1000);
    }
  }
}
//-------------------------------------------------------------------------------------
void mycallBack(char * topic, byte * payload, unsigned int lenght){

  String message = "";
   for(int i=0; i<lenght  ; i ++){
    message += (char)payload[i];
   }
if (String(topic) == "MQTT_ESP32/DHT11_tmp") {
  Serial.println(message);
  lcd.setCursor(0, 0);
  lcd.print(message);
} 
else if (String(topic) == "MQTT_ESP32/DHT11_hmd") {
  Serial.println(message);
  lcd.setCursor(0, 1);
  lcd.print(message);
}
else if(String(topic) == "MQTT_ESP32/ButtonCommand"){
    if(message == "1"){
      Serial.println("Status of the LED : ON");
      ledStatus = true;
      String s = "ON";
      client.publish("MQTT_ESP32/Publish", s.c_str());
    }
    else{
      Serial.println("Status of the LED : OFF");
      ledStatus = false;
      String s = "OFF";
      client.publish("MQTT_ESP32/Publish", s.c_str());
    }
    
  }


}


//----------------------------------------------------------------------------------------------
void setup() {
  pinMode(ledPin,OUTPUT);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(mycallBack);
  Wire.begin(21,22);
  lcd.begin(16, 2);
  lcd.backlight(); // Bật đèn nền
}


//----------------------------------------------------------------------------------
void loop() {
  if (!client.connected()) {
    connect_to_mqtt();
  }
  client.loop();
if(ledStatus == true){
  digitalWrite(ledPin, HIGH);

}
else {
  digitalWrite(ledPin,LOW);
}
}
