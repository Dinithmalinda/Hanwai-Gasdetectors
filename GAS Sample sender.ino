#include <ArduinoJson.h>
#include <PubSubClient.h>

//USE ESP8266 to generate random values of gas levels and forword to mqtt server
//use ESP32 to read ModBus Actual data and forword to mqtt server

const String GasList[]={"ERR","CH4","NH3","H2S","CO","O2","H2","C2H6","C2H4","C2H2","C3H8","C3H6","C4H10",
    "C4H8","C4H6","LIGHTOIL","HEAVY OIL","GASOLIN","DIESEL","KEROSIN","CH3OH","C2H50H"
    ,"(CH3)2CHOH","HCHO","C3H7CHO","C3H6O","CH3COC2H5","BENZINE","TOLUENE","XYLENE","STRYRENE","PHENOL"
    ,"ETHER","DIMETHYLETHER","PETROLIUMETHYL","DIMETHYLLAMINE","TRYMETHYLAMINE","FORMAIDE","TETRAHYDROFURAN"
    ,"ETHYLACETATE","CHLORINATETOLUNENE","EPOXY ETHANE","O3","SO2","NO2","NO"
    ,"HCL","HCN","CO2","CL2","FL"};
const String Unit[]={"ERR","%LEL","%VOL","PPM"};
const String SystemState[]={"Warmup","Normal","Cal Error","Sensorfault","preAlarm","LowAlarm","HiAlarm","ComFault","Exceed F.S","Need Calibrate"};

byte address=0;
bool dataready=false;
bool recieved=false;

int dataarray[20][5];

#if defined(ESP32)
  #include <WiFi.h>
  #include <esp32ModbusRTU.h>
  esp32ModbusRTU modbus(&Serial2, 4);  // use Serial1 and pin 16 as RTS
  void Init_modbus(){
    Serial2.begin(9600);  // Modbus connection

    modbus.onData([](uint8_t serverAddress, esp32Modbus::FunctionCode fc, uint16_t address, uint8_t* data, size_t length) {
    Serial.printf("id 0x%02x fc 0x%02x len %u: 0x", serverAddress, fc, length);
    for (size_t i = 0; i < length; ++i) {
      Serial.printf("%02x", data[i]);
      
    }
    for(byte ii=0;ii<5;ii++){
   

  dataarray[serverAddress-1][ii] = 0;
  dataarray[serverAddress-1][ii]|= ((int)data[ii*2]) << 8;
  dataarray[serverAddress-1][ii] |= ((int)data[ii*2+1]) ;
  Serial.println(String(dataarray[serverAddress-1][ii]));

    }

    recieved=true;
  });
  modbus.onError([](esp32Modbus::Error error) {
    Serial.printf("error: 0x%02x\n\n", static_cast<uint8_t>(error));
    recieved=true;
  });
  modbus.begin();
}

#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif

void generatrRandom(byte address){
  const byte gasname[]={12,5,48,2,4,49,47,3,12,12,12,5,48,2,4,49,47,3,12,12};
  const byte gastype[]={1,2,3,3,3,3,3,3,1,1,1,2,3,3,3,3,3,3,2,2};

int randomnumber=random(0,100);
dataarray[address-1][0]=gasname[address-1];
dataarray[address-1][1]=randomnumber;
dataarray[address-1][2]=gastype[address-1];
dataarray[address-1][3]=0;
dataarray[address-1][4]=randomnumber/10;
String out="";
for(byte tt=0;tt<5;tt++){out+=String(dataarray[address-1][tt])+",";}

Serial.print("data="+out);
}



// #define WIFI_AP "OLAX_MFi_786C"
// #define WIFI_PASSWORD "81118954"
//  #define WIFI_AP "NBL"
//  #define WIFI_PASSWORD "1234567890N@ble!1234567890"
#define WIFI_AP "IOTTEST"
#define WIFI_PASSWORD "Aspirine"



WiFiClient wifiClient;

PubSubClient client(wifiClient);

void reconnect() {
  byte tt=0,bb=0;
  // Loop until we're reconnected
  while (!client.connected()) {
    delay(100);tt++;
    if(tt>10)return;
    if ( WiFi.status() != WL_CONNECTED) {
    
      WiFi.begin(WIFI_AP, WIFI_PASSWORD);
      while (WiFi.status() != WL_CONNECTED) {
        bb++;
        if(bb>10)return;
        delay(500);
        Serial.print(".");
      }
      Serial.println("Connected to AP");
    }
    Serial.print("Connecting to MQTTServer");
    // Attempt to connect (clientId, username, password)
    if ( client.connect("D101","D101","D101") ) {
      Serial.println( "[DONE]" );
      // Subscribing to receive RPC requests
      client.subscribe("D101/SUB");
      // Sending current GPIO status
      Serial.println("Sending current GPIO status ...");
      
      //client.publish("D101/PUB","{\"testdata\":\"DONE\"}");

      
      
    } else {
      Serial.print( "[FAILED] [ rc = " );
      Serial.print( client.state() );
      Serial.println( " : retrying in 5 seconds]" );
      // Wait 5 seconds before retrying
      delay( 5000 );
    }
  }
}


void setup() {
  delay(100);  
  Serial.begin(115200);
  client.setServer("122.255.9.5", 1883 );
  delay(1000);
  reconnect();

  Serial.begin(115200);  // Serial output
#if defined(ESP32)
  Init_modbus()
#endif

  address=0;
}
//////////////////////sample dummy data//////////////////////////////////////////////////
// char* data_packet = "{\"sensorData\":[{\"addr\":1,\"name\":\"C4H10\",\"val\":51,\"unit\":\"%LEL\",\"status\":\"HighAlarm\"}," 
//                                     "{\"addr\":2,\"name\":\"O2\",\"val\":0.09,\"unit\":\"PPM\",\"status\":\"Sensorerror\"},"
//                                     "{\"addr\":3,\"name\":\"CO2\",\"val\":460.0,\"unit\":\"PPM\",\"status\":\"Sensorerror\"},"
//                                     "{\"addr\":4,\"name\":\"NH3\",\"val\":2.0,\"unit\":\"PPM\",\"status\":\"Sensorerror\"},"
//                                     "{\"addr\":5,\"name\":\"CO\",\"val\":1.0,\"unit\":\"PPM\",\"status\":\"Preheat\"},"
//                                     "{\"addr\":6,\"name\":\"Cl2\",\"val\":0.01,\"unit\":\"PPM\",\"status\":\"Preheat\"},"
//                                     "{\"addr\":7,\"name\":\"HCN\",\"val\":1.0,\"unit\":\"PPM\",\"status\":\"Preheat\"},"
//                                     "{\"addr\":8,\"name\":\"H2S\",\"val\":0.0,\"unit\":\"PPM\",\"status\":\"Preheat\"},"
//                                     "{\"addr\":9,\"name\":\"C4H10\",\"val\":0.10,\"unit\":\"%LEL\",\"status\":\"LowAlarm\"},"
//                                     "{\"addr\":10,\"name\":\"C4H10\",\"val\":0.10,\"unit\":\"%LEL\",\"status\":\"Preheat\"},"
//                                     "{\"addr\":11,\"name\":\"C4H10\",\"val\":0.10,\"unit\":\"%LEL\",\"status\":\"Preheat\"},"
//                                     "{\"addr\":12,\"name\":\"O2\",\"val\":0.90,\"unit\":\"PPM\",\"status\":\"Preheat\"},"
//                                     "{\"addr\":13,\"name\":\"CO2\",\"val\":430.0,\"unit\":\"PPM\",\"status\":\"Preheat\"},"
//                                     "{\"addr\":14,\"name\":\"NH3\",\"val\":0.20,\"unit\":\"PPM\",\"status\":\"Preheat\"},"
//                                     "{\"addr\":15,\"name\":\"CO\",\"val\":0.10,\"unit\":\"PPM\",\"status\":\"Preheat\"},"
//                                     "{\"addr\":16,\"name\":\"Cl2\",\"val\":0.0,\"unit\":\"PPM\",\"status\":\"Preheat\"},"
//                                     "{\"addr\":17,\"name\":\"HCN\",\"val\":0.10,\"unit\":\"PPM\",\"status\":\"Preheat\"},"
//                                     "{\"addr\":18,\"name\":\"H2S\",\"val\":0.20,\"unit\":\"PPM\",\"status\":\"Preheat\"},"
//                                     "{\"addr\":19,\"name\":\"C4H10\",\"val\":0.10,\"unit\":\"%LEL\",\"status\":\"Preheat\"},"
//                                     "{\"addr\":20,\"name\":\"C4H10\",\"val\":0.10,\"unit\":\"%LEL\",\"status\":\"Preheat\"}]}";
//////////////////////sample dummy data//////////////////////////////////////////////////

char data_packet[2048];

void makeJasonArray(){
 StaticJsonDocument<2048> doc;
  JsonArray data = doc.createNestedArray("sensorData");
 for(byte rr=0;rr<20;rr++){
   StaticJsonDocument<100> data1;
   if(dataarray[rr][0]>49)dataarray[rr][0]=0;
   if(dataarray[rr][2]>3)dataarray[rr][2]=0;
   if(dataarray[rr][4]>10)dataarray[rr][4]=0;
   data1["addr"] = rr+1;
   data1["name"] = GasList[dataarray[rr][0]];
   data1["val"] = dataarray[rr][1];
   data1["unit"] = Unit[dataarray[rr][2]];
   data1["status"] = SystemState[dataarray[rr][4]];
   data.add(data1);
 }
serializeJson(doc, data_packet);
dataready=true;
}

uint32_t lastMillis = 0;
void loop(){
if ((millis() - lastMillis > 1000)) {lastMillis = millis();
    address++;
    if(address>20){address=1;makeJasonArray();}
    Serial.print("ModbusADD-"+String(address)+" ");

#if defined(ESP32)
    modbus.readHoldingRegisters(address,0x1f5,5); 
#elif defined(ESP8266)
    generatrRandom(address);
#endif
    recieved=false;
}
if (!client.connected()) {reconnect();}
if(dataready){client.publish("CICTGAS",data_packet);
              dataready=false;
              Serial.println("");
              Serial.println( data_packet );
              }  
client.loop();


// Serial.println( data_packet );
// if (!client.connected()) {reconnect();}
//   client.publish("CICTGAS",data_packet);    
//   client.loop();
//   delay(5000);
}


