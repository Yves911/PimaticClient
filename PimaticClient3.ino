  /*
 *  This sketch sends data (gas + electricity) via a port on a local network
 *
 */

#include <ESP8266WiFi.h>
#include <WiFiClient.h>

const char* ssid     = "*******************";
const char* password = "*******************";

// The IP address of your Pimatic installation
const char* Pimatic_host = "192.168.1.11";

// A port - be carefull here it's not the pimatic port but a port that is not currently in use on your pimatic server
// we will send the data from the ESP8266 to this particular port
const int httpPort = 8888;

void setup() {
  Serial.begin(115200);
  delay(10);

  // Flash the LED on pin 12 for a short time
  
  pinMode(12, OUTPUT);
  digitalWrite(12, HIGH);
  delay(900);
  digitalWrite(12, LOW); 
  
  wifi_connect;
  
  // Attach Interrupts
  // pin 14: a hall sensor model OH44E
  // pin 5: a light sensor model TSL257-LF
  attachInterrupt(14, gas, RISING);
  attachInterrupt(5, elec, FALLING);
}

volatile long gas_tick = 0;
volatile long elec_tick = 0;
long gas_realtime = 0;
long elec_realtime = 0;
volatile long start_g = 0;
volatile long elapsed_g = 0;
volatile long start_e = 0;
volatile long elapsed_e = 3600000;
long bckelapsed_e = 0;
String data_gas = "G,";
String data_elec = "E,";
String old_data_gas = "G,";
String old_data_elec = "E,";
WiFiClient client;

void loop() {
  // This loop will run forever
  delay(2000);

  // Prepare the "gas" message, something like G,7,188,19116 where
  // G is the type of data
  // 7 is the curent # of ticks (interrupts) (ie # of times the magnet came close to the hall sensor)
  // 188 is the curent consumption 
  // 19116 is the duration between 2 ticks

  data_gas = "G,";
  data_gas += gas_tick;
  data_gas += ",";
  if (elapsed_g > 0) 
  {
    gas_realtime=3600000/elapsed_g;
  } 
  else 
  {
    gas_realtime=0;
  }
  data_gas += gas_realtime;
  data_gas += ",";
  data_gas += elapsed_g;
    
 
  // Open (if not done already) the connection to the pimatic server on a specific port
  if (!client.connected())
  {
    //Serial.println("connected false");
    if (!client.connect(Pimatic_host, httpPort)) {
      Serial.println("connection to pimatic failed");
      delay(1500);
      // Reconnect to wifi if not connected anymore
      if (WiFi.status() != WL_CONNECTED) {
        Serial.println("connection to wireless failed");
        wifi_connect;
        }
      return;
    }
  }
  

  if (data_elec != old_data_elec) {client.println(data_elec);}
  old_data_elec = data_elec;

  if (data_gas != old_data_gas) {client.println(data_gas);}
  //if ((millis() - start_g) > 200000)
  if ((((millis() - start_g) > (elapsed_g *2))) or (((millis() - start_g) > 60000)))
  {
    gas_realtime=0;
    elapsed_g=0;
  }  

  old_data_gas = data_gas;
}

void gas()
{
  ++gas_tick;
  elapsed_g=millis()-start_g;
  start_g=millis();
  //Serial.println("GasTick");
}

void elec()
{
  elapsed_e=millis()-start_e;
  if (elapsed_e > 300) 
  {
    ++elec_tick;
    start_e=millis();
//    Serial.print("ElecTick:");
//    Serial.println(elapsed_e);

  // Prepare the "elec" message, something like E,740,487,7391 where
  // E is the type of data
  // 740 is the curent # of ticks (interrupts) (ie # of times the led flashes)
  // 487 is the curent consumption (Wh)
  // 7391 is the duration between 2 ticks

    data_elec = "E,";
    data_elec += elec_tick;
    data_elec += ",";
    elec_realtime=3600000/elapsed_e;
    data_elec += elec_realtime;
    data_elec += ",";
    data_elec += elapsed_e;
//    Serial.println(data_elec);   
    delayMicroseconds(250000);
  }
//  else
//  {
//    Serial.print("RejectedTick:");
//    Serial.println(elapsed_e);
//  }
}

void wifi_connect()
{
  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
}
