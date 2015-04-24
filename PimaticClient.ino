/*
 *  This sketch sends data (gas + electricity) via a port on a local network
 *
 */

#include <ESP8266WiFi.h>
#include <WiFiClient.h>

const char* ssid     = "-------------";
const char* password = "-------------";

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
  
  // Attach Interrupts
  // pin 14: a hall sensor model OH44E
  // pin 5: a light sensor model TSL257-LF
  attachInterrupt(14, gas, RISING);
  attachInterrupt(5, elec, FALLING);
}

long gas_tick = 0;
long elec_tick = 0;
long gas_realtime = 0;
long elec_realtime = 0;
long finished_g = 0;
long start_g = 0;
long elapsed_g = 0;
long finished_e = 0;
long start_e = 0;
long elapsed_e = 0;
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
  data_gas += gas_realtime;
  data_gas += ",";
  data_gas += elapsed_g;
  
  // Prepare the "gas" message, something like E,740,487,7391 where
  // E is the type of data
  // 740 is the curent # of ticks (interrupts) (ie # of times the led flashes)
  // 487 is the curent consumption (Wh)
  // 7391 is the duration between 2 ticks
  
  data_elec = "E,";
  data_elec += elec_tick;
  data_elec += ",";
  data_elec += elec_realtime;
  data_elec += ",";
  bckelapsed_e = elapsed_e;
  data_elec += elapsed_e;
 
  // Open (if not done already) the connection to the pimatic server on a specific port
  if (!client.connected())
  {
    Serial.println("connected false");
    if (!client.connect(Pimatic_host, httpPort)) {
      Serial.println("connection failed");
      delay(1500);
      return;
    }
  }

  if (bckelapsed_e > 251) 
  {
    if ((data_elec != old_data_elec) and (!data_elec.endsWith(",0"))) {client.println(data_elec);}
  }
  else if (bckelapsed_e == 250) 
  {
    --elec_tick;
  }

  if (data_gas != old_data_gas) {client.println(data_gas);}
  //if ((millis() - start_g) > 200000)
  if ((millis() - start_g) > (elapsed_g *2))
  {
    gas_realtime=0;
    elapsed_g=0;
  }
  
  old_data_elec = data_elec;
  old_data_gas = data_gas;
}

void gas()
{
  ++gas_tick;
  finished_g=millis();
  elapsed_g=finished_g-start_g;
  start_g=millis();
  gas_realtime=3600000/elapsed_g;
  //Serial.println("GasTick");
}

void elec()
{
  ++elec_tick;
  finished_e=millis();
  elapsed_e=finished_e-start_e;
  start_e=millis();
  elec_realtime=3600000/elapsed_e;
  //Serial.println("ElecTick");
  delayMicroseconds(250000);
}
