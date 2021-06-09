/*
 * Program to operate ESP32 in client mode and use fitness band as proximity switch
 * Program by: Aswinth Raj B
 * Dated: 31-10-2018
 * Website: www.circuitdigest.com 
 * Reference: https://github.com/nkolban/esp32-snippets 
 * //NOTE: The My_BLE_Address, serviceUUID and charUUID should be changed based on the BLe server you are using 
 */

#include <BLEDevice.h> //Header file for BLE 

static BLEUUID serviceUUID(BLEUUID((uint16_t)0xFE86)); //Service UUID of fitnessband obtained through nRF connect application 
static BLEUUID    charUUID(BLEUUID((uint16_t)0xFE03)); //Characteristic  UUID of fitnessband obtained through nRF connect application 
String My_BLE_Address = "34:12:f9:03:1f:15"; //Hardware Bluetooth MAC of my fitnessband, will vary for every band obtained through nRF connect application 
static BLERemoteCharacteristic* pRemoteCharacteristic;

BLEScan* pBLEScan; //Name the scanning device as pBLEScan
BLEScanResults foundDevices;

static BLEAddress *Server_BLE_Address;
String Scaned_BLE_Address;

//#define LED_BUILTIN 13

boolean paired = false; //boolean variable to togge light

bool connectToserver (BLEAddress pAddress){
    
    BLEClient*  pClient  = BLEDevice::createClient();
    Serial.println(" - Created client");

    // Connect to the BLE Server.
    pClient->connect(pAddress);
    Serial.println(" - Connected to fitnessband");

    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService != nullptr)
    {
      Serial.println(" - Found our service");
      return true;
    }
    else
    return false;

    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
    if (pRemoteCharacteristic != nullptr)
      Serial.println(" - Found our characteristic");

      return true;
}

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks 
{
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      Serial.printf("Scan Result: %s \n", advertisedDevice.toString().c_str());
      Server_BLE_Address = new BLEAddress(advertisedDevice.getAddress());
      
      Scaned_BLE_Address = Server_BLE_Address->toString().c_str();
      
    }
};

void setup() {
    Serial.begin(115200); //Start serial monitor 
    Serial.println("ESP32 BLE Server program"); //Intro message 

    BLEDevice::init("");
    pBLEScan = BLEDevice::getScan(); //create new scan
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks()); //Call the class that is defined above 
    pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
    pBLEScan->setInterval(100); // set Scan interval
    pBLEScan->setWindow(99);  // less or equal setInterval value

    pinMode (LED_BUILTIN,OUTPUT); //Declare the in-built LED pin as output 
}

void loop() {

  foundDevices = pBLEScan->start(1); //Scan for 3 seconds to find the Fitness band 

  while (foundDevices.getCount() >= 1)
  {
    if (Scaned_BLE_Address == My_BLE_Address && paired == false)
    {
      Serial.println("Found Device :-)... connecting to Server as client");
       if (connectToserver(*Server_BLE_Address))
      {
      paired = true;
      Serial.println("********************LED turned ON************************");
      digitalWrite (LED_BUILTIN,HIGH);
      break;
      }
      else
      {
      Serial.println("Pairing failed");
      break;
      }
    }
    
    if (Scaned_BLE_Address == My_BLE_Address && paired == true)
    {
      Serial.println("Our device went out of range");
      paired = false;
      Serial.println("********************LED OOOFFFFF************************");
      digitalWrite (LED_BUILTIN,LOW);
      ESP.restart();
      break;
    }
    else
    {
    Serial.println("We have some other BLe device in range");
    break;
    }
  } 
  pBLEScan->clearResults();
}
