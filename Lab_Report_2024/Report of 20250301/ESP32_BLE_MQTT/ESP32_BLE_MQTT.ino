    #include <WiFi.h>
    #include <WebServer.h>
    #include <BLEDevice.h>
    #include <BLEUtils.h>
    #include <BLEServer.h>
    #include <BLE2902.h> 
    #include <ArduinoJson.h>
    #include <PubSubClient.h>
    // khởi tạo đối tượng websever lắng nghe trên cổng 80 
    // ESP sẽ hoạt động như một máy chủ 
    WebServer server(80);
    String ssid, password, topic , message ; // biến lưu ssid và pass wifi khi nhập từ giao diện web 
    unsigned long timeUpdate = millis();
    BLEServer *pServer;
    BLEService *pBattery; // Khai báo biến toàn cục

    // Cấu hình MQTT
    char *mqtt_broker   = "192.168.1.5";   
    char *mqtt_username = "";
    char *mqtt_password = "";
    int   mqtt_port     = 1883;

    //tạo đối tượng WifiClient để kết nối tới wifi và giao tiếp với broker MQTT 
    WiFiClient espClient;
    PubSubClient client(espClient); // tạo đối tượng MQTT client  với espClient để sử dụng các hàm trong thư viện PubSubClientPubSubClient
    String client_id = "esp32-client";
    bool _BLEClientConnected = false; // biến kiểm tra xem đã kết nối được tới WebClient chưa 

    #define BatteryService BLEUUID((uint16_t)0x181F)  // định nghĩa mã UUID cho dịch vụ BLE 

    // định nghĩa mã UUID cho đặc tính dịch vụ 
    BLECharacteristic BatteryLevelCharacteristic(BLEUUID((uint16_t)0x2A18), BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY);// với các thuộc tính như đọc , ghi giá trị của đặc tính và cho phép đặc tính gửi thông báo 
    // bộ mô tả (descriptor) cung cấp thông tin mô tả đặc tính để người dùng khác có thể hiểu rõ hơn về đặc tính 
    BLEDescriptor BatteryLevelDescriptor(BLEUUID((uint16_t)0x2901));
    

    // khởi tạo đối tượng MyseverCallbacks kế thừa từ lớp BLESevercallBacks
    // với 2 phương thức onconnect bà onDisconnectonDisconnect
    class MyServerCallbacks : public BLEServerCallbacks {
      // phương thức onConnect sẽ cập nhật giá trị cho biến toàn cục _BLEClientConnected = TRUE khi có web ble kết nối tới ESP 
      void onConnect(BLEServer* pServer) { _BLEClientConnected = true; }
      // phương thức onDisconnnect sẽ cập nhật lại giá trị FALSE khi Web ble ngắt kết nối 
      void onDisconnect(BLEServer* pServer) { _BLEClientConnected = false; }
    };
    //Định nghĩa UUID cho characteristic nhận dữ liệu ( ESP ) 
    #define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"


        // hàm publish tin nhắn lên broker với topic và message 
    // retained == true : giữ lại tin nhắn trên Broker , false : không giữ tin nhắn, chỉ gửi tin nhắn tại thời điểm publish 
    void publishMessage(const char* topic, String payload, bool retained) {
      if (client.publish(topic, payload.c_str(), retained)) {
        Serial.println("Message published [" + String(topic) + "]: " + payload);
      }
    }

        void MQTT_Connect() { // hàm kết nối MQTT 
      if (!client.connected()) { 
        client.setServer(mqtt_broker, mqtt_port);
        Serial.print("Connecting to MQTT...");
        if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
          Serial.println("Connected!");
        } else {
          Serial.print("Failed, rc=");
          Serial.println(client.state());// hàm in ra trạng thái lỗi khi kết nối MQTT 
          delay(2000);
        }
      }
    }
    //Lớp MyCallbacks kế thừa từ lớp BLECharacteristicCallbacks có sẵn trong thư viện BLE
    class MyCallbacks: public BLECharacteristicCallbacks {
      // thuộc tính onWrite được gọi mỗi khi một client viết dữ liệu lên characteristic mà callback này được gắn vào.
      void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = std::string(pCharacteristic->getValue().c_str());
        if (rxValue.length() > 0) {
          Serial.print("Received Value: ");
          Serial.println(rxValue.c_str());
        }

      // Phân tách dữ liệu thành SSID, Password, Topic, Message
      String receivedData = String(rxValue.c_str());
      receivedData.trim();
      size_t delimiterPos = receivedData.indexOf(':');
      
      if (delimiterPos != -1) {
        String check = receivedData.substring(0, delimiterPos);
        
        if (check == "MQTT") {
          String mqttData = receivedData.substring(delimiterPos + 1);
          size_t delimiterPos2 = mqttData.indexOf(';');
          if (delimiterPos2 != -1) {
            topic = mqttData.substring(0, delimiterPos2);
            message = mqttData.substring(delimiterPos2 + 1);
          } else {
            topic = mqttData;
            message = "";
          }

          // Publish lên MQTT ngay sau khi nhận được dữ liệu
          if (client.connected()) {
            publishMessage(topic.c_str(), message, false);
          } else {
            Serial.println("MQTT not connected, trying to reconnect...");
            MQTT_Connect();
          }
        }
      }
    }
    };
    // hàm khởi tại server BLE 
    void InitBLE() {
      BLEDevice::init("BLE_Wifi");// khởi tạo BLE với tên thiết bị là BLE_Wifi
      pServer = BLEDevice::createServer(); // tạo server để các web ble có thể kết nối tới esp
      pServer->setCallbacks(new MyServerCallbacks()); // hàm setcallback để liên tục xứ lí các sự kiện kết nối và ngắt kết nối BLE 
      pBattery = pServer->createService(BatteryService);// tạo một dịch vụ BLE với định danh UUID đã được define là 0x181f

      pBattery->addCharacteristic(&BatteryLevelCharacteristic); // tạo đặc tính BLE để đọc ,ghi, nhận thông tin 
      BatteryLevelDescriptor.setValue("Percentage 0 - 100"); // mô tả đặc tính để khi khi người dử dụng đặc tính dịch vụ có thể nắm bắt  
      BatteryLevelCharacteristic.addDescriptor(&BatteryLevelDescriptor);
      BatteryLevelCharacteristic.addDescriptor(new BLE2902()); // cho phép bật thông báo để gửi dữ liệu tự động khi giá trị thay đổi 

      BLECharacteristic *pWriteCharacteristic = pBattery->createCharacteristic(
        CHARACTERISTIC_UUID_RX, BLECharacteristic::PROPERTY_WRITE
      ); // tạo đặc tính nhận dữ liệu từ web BLE với quyền ghi Write 
      pWriteCharacteristic->setCallbacks(new MyCallbacks()); // xử lí dữ liệu mỗi khi thiết bị gửi thông tin qua BLE 

      pServer->getAdvertising()->addServiceUUID(BatteryService); // bạt mode quảng bá BLE để các thiết bị SCAN như máy tính , điện thoại có thể quét 
      pBattery->start();
      pServer->getAdvertising()->start();
    }



    void setup() {
      Serial.begin(115200);
      InitBLE();
      Serial.println("BLE Initialized");

      // Nhận dữ liệu WiFi từ WebBLE
      while (ssid.isEmpty() || password.isEmpty()) {
        receiData(ssid, password, topic , message );
        delay(500);
      }


      WiFi.mode(WIFI_STA); // bạt wifi với chế độ như một thết bị khách kết nối với bộ định tuyến 
      WiFi.begin(ssid, password); // kết nối wifi với thông tin wifi được nhận tùư web ble 

      int timeout = 20;
      while (WiFi.status() != WL_CONNECTED && timeout > 0) {
        Serial.print(".");
        delay(1000);
        timeout--;
      }

      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi Connected!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP()); // in ra địa chỉ IP của esp được bộ định tuyến cấp phát 
      } else {
        Serial.println("\nWiFi Connection Failed!");
      }

      MQTT_Connect(); // kết nối tới MQTT Broker 
      server.begin(); // kích hoạt server , bắt đầu lắng nghe kết nối từ các web ble tới esp và xư xử lí yêu cầu 
    }

    void receiData (String& ssid, String& password, String& topic, String& message ){
      BLECharacteristic *pWriteCharacteristic = pBattery->getCharacteristic(CHARACTERISTIC_UUID_RX);
      if (pWriteCharacteristic == nullptr) {
        Serial.println("Error: RX Characteristic not found!");
        return;
    }
      std::string data = std::string(pWriteCharacteristic->getValue().c_str()); // Lấy dữ liệu từ đúng RX UUID
      String receivedData = String(data.c_str());
      receivedData.trim(); // Loại bỏ khoảng trắng thừa
      
      size_t delimiterPos = receivedData.indexOf(':');
      if(delimiterPos != -1 ){
        String check = receivedData.substring(0,delimiterPos);
        if(check == "MQTT"){
            String mqttData = receivedData.substring(delimiterPos +1 );
            size_t delimiterPos2 = mqttData.indexOf(';');
            if(delimiterPos2 != -1 ){
              topic = mqttData.substring(0,delimiterPos2);
              message = mqttData.substring (delimiterPos2+1);
            }
            else {
              // tức là có mỗi topic 
              topic = mqttData;
              message = "";
            }
            Serial.print("Publish to ");
            Serial.print (topic + " : Message : ");
            Serial.println(message);
        }
        if (check == "WIFI"){
            String WifiData = receivedData.substring(delimiterPos +1 );
            size_t delimiterPos2 = WifiData.indexOf(';');
            if(delimiterPos2 != -1 ){
              ssid = WifiData.substring(0,delimiterPos2);
              password = WifiData.substring (delimiterPos2+1);
            }
            else {
              // tức là có mỗi topic 
              ssid = WifiData;
              password = "";
            }
            Serial.println("Wifi data: ");
            Serial.println("ssid: "+ ssid);
            Serial.println("Password: "+ password);
        }
      }


    }

    void loop() {
      client.loop();
      if (!client.connected()) { // nếu chưa kết nối được với broker thì liên tục kết nối lại
        MQTT_Connect();
      }

      if (millis() - timeUpdate > 3000) { // cứ mỗi 3 giây , esp sẽ gửi 100 tin nhắn đếm từ 1 đến 100 lên broker 
        static int n = 100;
        for (int i = 0; i <= n; i++){
          DynamicJsonDocument doc(1024);
          doc["n"] = i;
          char mqtt_message[128];
          serializeJson(doc, mqtt_message);

          if (client.connected()) {
            publishMessage("No", mqtt_message, true);
            delay(1000);
          }
        }
        timeUpdate = millis();
      }
    }