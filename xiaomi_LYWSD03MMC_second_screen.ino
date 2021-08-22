// SECOND SCREEN FOR XIAOMI THEMPERATURE LYWSD03MMC
// BASED ON LILYGO TTGO T5 2.13 E/PAPER
// golos 2021


#include <BLEDevice.h>
#include <SimpleTimer.h>
#include <GxEPD.h>
#include <GxGDEH0213B73/GxGDEH0213B73.h>  // 2.13" b/w newer panel
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>

#define LYWSD03MMC_ADDR "a4:c1:38:1b:2c:66" // firm original


GxIO_Class io(SPI, /*CS=*/ 5, /*DC=*/ 17, /*RST=*/ 16); // arbitrary selection of 8, 9 selected for default of GxEPD_Class
GxEPD_Class display(io, /*RST=*/ 16, /*BUSY=*/ 4); // default selection of (9), 7
 

BLEClient* pClient;
 

static BLEAddress htSensorAddress(LYWSD03MMC_ADDR);

bool connectionSuccessful = false;

// The remote service we wish to connect to.
 static BLEUUID serviceUUID("ebe0ccb0-7a0a-4b0c-8a1a-6ff2997da3a6");

// The characteristic of the remote service we are interested in.
 static BLEUUID    charUUID("ebe0ccc1-7a0a-4b0c-8a1a-6ff2997da3a6");

class MyClientCallback : public BLEClientCallbacks {
    void onConnect(BLEClient* pclient) {
      Serial.println("Connected");
      connectionSuccessful = true;
    
    }

    void onDisconnect(BLEClient* pclient) {
      Serial.println("Disconnected");
      if (!connectionSuccessful) {
        Serial.println("RESTART");
        ESP.restart();
      }
    }
};

static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
  float temp;
  float humi;
  Serial.print("Notify callback for characteristic ");
  Serial.println(pBLERemoteCharacteristic->getUUID().toString().c_str());
  temp = (pData[0] | (pData[1] << 8)) * 0.01; //little endian 
  humi = pData[2];
  Serial.printf("temp = %.1f : humidity = %.1f \n", temp, humi);
   // delay(3000);
  Serial.println("mostral en pantalla");
  display.setRotation(1);
  display.setTextColor(GxEPD_BLACK);
//  display.setFont(&FreeMonoOblique12pt7b);
  display.fillScreen(GxEPD_WHITE);
  display.setTextSize(9);
  display.setCursor(0, 0);
  display.print(temp, 1);
  
  display.setTextSize(5);
  display.setCursor(25, 80);
  display.print(humi, 0);
  display.update();
  pClient->disconnect();
}

void registerNotification() {

  // Obtain a reference to the service we are after in the remote BLE server.
  BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
  if (pRemoteService == nullptr) {
    Serial.print("Failed to find our service UUID: ");
    Serial.println(serviceUUID.toString().c_str());
    pClient->disconnect();
  }
  Serial.println(" - Found our service");

  // Obtain a reference to the characteristic in the service of the remote BLE server.
  BLERemoteCharacteristic* pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
  if (pRemoteCharacteristic == nullptr) {
    Serial.print("Failed to find our characteristic UUID: ");
    Serial.println(charUUID.toString().c_str());
    pClient->disconnect();
  }
  Serial.println(" - Found our characteristic");
  pRemoteCharacteristic->registerForNotify(notifyCallback);
}

void createBleClientWithCallbacks() {
  pClient = BLEDevice::createClient();
  pClient->setClientCallbacks(new MyClientCallback());
}

void connectSensor() {
  pClient->connect(htSensorAddress);
  
  
}


void setup() {
  Serial.begin(115200);
  Serial.println("Starting MJ client...");
     display.init();
  delay(500);

  BLEDevice::init("ESP32");

}

void loop() {
   
   Serial.println("Buscando");
   // BLEDevice::init("ESP32");
  createBleClientWithCallbacks();
  delay(500);
  Serial.println("RconnectSensor");
  connectSensor();
  // restart if bt not on range
   if (!connectionSuccessful) {
        Serial.println("RESTART");
        ESP.restart();
      }
  Serial.println("Registernotification");
  registerNotification();

  delay(300000);
  connectionSuccessful = false;
    
}
