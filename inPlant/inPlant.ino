
/*
inPlant - Domestic Plant Monitoring System

*/

//Telegram Library Includes ---------------------------------//
#include <ArduinoJson.h> //JSON Library required to program the interaction with the bot
#include <TelegramBot.h> // Telegram Bot Library reqired to interact with Telegram
#include <TelegramKeyboard.h> // Keyboard library - to create the custom keyboard

// include SPI and Wifi101 libraries
#include <SPI.h>
#include <WiFi101.h>

//added for Temboo ------------------------------------------//
#include <Temboo.h>
#include "TembooAccount.h" // Contains Temboo account information
//---------------------------------------------------------------//

//Timer Libraries -----------------------------------------------//
//required to run functions cohesively without delay() method
#include <Event.h>
#include <Timer.h>


//inPlant data.SparkFun info
//url: https://data.sparkfun.com/streams/XGy68EqOXRhZXarLpWRO
//publicKey: XGy68EqOXRhZXarLpWRO
//privateKey: 1JYwjNyVg9uPv8xXq1Ao

// Wifi Vars ---------------------------------------------------//
char ssid[] = "***";     // network SSID
char pass[] = "***";        // network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status

// Telegram Creds ---------------------------------------------//
const char BotToken[] = "***";



// Temboo Vars ------------------------------------------------//
int numRuns = 1;   // Execution count, so this doesn't run forever
int maxRuns = 150;   // Maximum number of times the Choreo should be executed
String temp;   // Temp var to store choreo temperature info.

// Specify Client vars ----------------------------------------//
WiFiClient temSpark; //Declare connection variable for WifiClient - Temboo and Sparkfun
WiFiSSLClient client; //Declare connection variable for WifiSSLClient - Telegram
TelegramBot bot (BotToken, client); //Bot Credentials
TelegramKeyboard keyboard_one;

//Timer -------------------------------------------------------//
Timer t; //Timer variable


//Set-up ---------------------------------------------------------------------------//

void setup() {
Serial.begin(9600); // initialize serial communication at 9600 bits per second:
delay(4000); // Wait 4 secs until the serial console is connected

  //Comment out of code for mains power deployment.
  //Wifi Library
  //Initialize serial and wait for port to open:

/*
  while (!Serial) {
  ;
  //wait for serial port to connect. Needed for native USB port only
  }

*/

// check for the presence of the shield:
if (WiFi.status() == WL_NO_SHIELD) {
 //Serial.println("WiFi shield not present");
// don't continue:
while (true);
  }

  // attempt to connect to Wifi network:
while ( status != WL_CONNECTED) {
  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(ssid);
  // Connect to WPA/WPA2 network:
  status = WiFi.begin(ssid, pass);

  // wait 10 seconds for connection:
  delay(10000);
}

  // you're connected now, so print out the data:
  Serial.print("You're connected to the network");
  Serial.println();

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.println(rssi);
  Serial.println();

  //Telegram Keyboard
   // define Telegram keyboard row's
  const char* row_one[] = {"How's my plant doing?"};
  const char* row_two[] = {"Temp", "Soil", "Light"};
  keyboard_one.addRow(row_one, 1);
  keyboard_one.addRow(row_two, 3);
  // startBot
  bot.begin();

  //Get totals once only before Counter ----------------------------------------//
  senseInRoom();
  tembooTemp();
  sendReading();

  //Timer ------------------------------------------------------------//
  int tickSense = t.every(200,senseInRoom); //sense every .2 of a second
  int tickTemboo = t.every(7200000,tembooTemp); //check for temperature every 2 hours
  int tickSend = t.every(7200000,sendReading); // send info to sparkfun every 2 hours
  int tickTelegram = t.every(200,telegramBot); // run telegram every .2 of a second

}


void loop() {

  t.update(); //run the timer that contains the sketch functions

}

// Sensing in the room ----------------------------------------------------------/

void senseInRoom(){

int soilSensor = analogRead(A0); //store the data for soil moisture in a variable
int lightSensor = analogRead(A3);  //store the data for light in a variable

// print to serial the reading of the soilSensor
Serial.print("Soil Moisture Reading: ");
Serial.println(soilSensor);


// if the moisture level is less than 100 then print: "I'm Thirsty"
// or print: "I'm Cool."
if (soilSensor < 750) {
  Serial.println ("Soil Moisture Status: I'm thirsty, water me!");
  Serial.println();
  } else {
    Serial.println ("Soil Moisture Status: Everything is cool!");
    Serial.println();
      }


//print to serial the reading of the lightSensor
Serial.print("Light Sensor Reading: ");
Serial.println(lightSensor);
Serial.println();

}


//Send Readings to data.Sparkfun ----------------------------------------------------------------/
void sendReading() {

int soilSensor = analogRead(A0); //store the data for soil moisture in a variable
int lightSensor = analogRead(A3); //store the data for light in a variable

    temSpark.connect("data.sparkfun.com", 80);

    if (temSpark.connected()) {
        Serial.println("connected to data.sparkfun");
        Serial.println();
        // Make a HTTP request:
        temSpark.print("GET /input/XGy68EqOXRhZXarLpWRO?private_key=1JYwjNyVg9uPv8xXq1Ao&light=");
        temSpark.print(lightSensor);
        temSpark.print("&soilmoisture=");
        temSpark.print(soilSensor);
        temSpark.print("&temp=");
        temSpark.print(temp);
        temSpark.println(" HTTP/1.1");
        temSpark.println("Host: data.sparkfun.com");
        temSpark.println("Connection: close");
        temSpark.println();
        temSpark.stop();
  } else {
        // if you didn't get a connection to the server:
        Serial.println("connection failed");
  }
}

//---------- Temboo ------------------------------------------------------//
void tembooTemp(){

 if (numRuns <= maxRuns) {
    Serial.println("Running GetTemperature - Run #" + String(numRuns++));

    TembooChoreo GetTemperatureChoreo(temSpark);

    // Invoke the Temboo client
    GetTemperatureChoreo.begin();

    // Set Temboo account credentials
    GetTemperatureChoreo.setAccountName(TEMBOO_ACCOUNT);
    GetTemperatureChoreo.setAppKeyName(TEMBOO_APP_KEY_NAME);
    GetTemperatureChoreo.setAppKey(TEMBOO_APP_KEY);

    // Set Choreo inputs
    String AddressValue = "147 Lothian Road, Edinburgh";
    GetTemperatureChoreo.addInput("Address", AddressValue);
    String UnitsValue = "c";
    GetTemperatureChoreo.addInput("Units", UnitsValue);

    // Identify the Choreo to run
    GetTemperatureChoreo.setChoreo("/Library/Yahoo/Weather/GetTemperature");

    // Run the Choreo; when results are available, print them to serial
    GetTemperatureChoreo.run();

    while(GetTemperatureChoreo.available()) {
      // read the name of the next output item
      String name = GetTemperatureChoreo.readStringUntil('\x1F');
      name.trim(); // use "trim" to get rid of extra text

      // read the value of the next output item
      String data = GetTemperatureChoreo.readStringUntil('\x1E');
      data.trim(); // use "trim" to get rid of extra text

      if (name == "Temperature") {
        temp = data.toInt(); // convert the data from type String to type int
        Serial.println(temp);
      }
    }
    GetTemperatureChoreo.close();
  }

  Serial.println("\nWaiting...\n");

}

void telegramBot(){

int soilSensor = analogRead(A0);  //store the data for soil moisture in a variable
int lightSensor = analogRead(A3); //store the data for light in a variable


message m = bot.getUpdates(); // Read new messages

String temperature = "Current temp: "; // Temp String
String thisString = String(temp); // Change the TEMP to String for Output
String soilString = String(soilSensor); //Change the soil moisture to string for Output
String lightString = String(lightSensor); //Change the light readings to string for Output



 if (m.text.equals("Temp") || m.text.equals("/Temp")){
    Serial.println(m.text);
    bot.sendMessage(m.chat_id, temperature + thisString + "°C"); //if user types TEMP them send them the temp
  } else if(m.text.equals("Soil") || m.text.equals("/Soil")){
       bot.sendMessage(m.chat_id, "Current Soil: " + soilString + "."); //if user types SOIL send them the soil
      } else if (m.text.equals("Light") || m.text.equals("/Light")){
       bot.sendMessage(m.chat_id, "Current Light: " + lightString + "."); //If user types LIGHT send them the light
          } else if (m.text.equals("How's my plant doing?") || m.text.equals("/How")){
              bot.sendMessage(m.chat_id, "Current Light: " + lightString + "." + "\n"
                                         + "Current Soil: " + soilString + "." + "\n"
                                         +  temperature + thisString + "°C"
                                         );  //If user types How's My Plant Doing? send them all the output.
            } else{
              //if the text is not one of the key phrases send this message
               bot.sendMessage(m.chat_id, "...I don't recognise this command, please use keywords: \n /Temp \n /Soil \n /Light \n /How's\ my\ plant\ doing?",keyboard_one);
               Serial.println("no new message"); //if nothing is happening print no new message to the serial monitor

              }

//  Library doesn't allow the ardunio to initiate a message from the bot to a user. It can only react to user input.
  if (soilSensor < 1000){
     bot.sendMessage("@testAccount","Current Soil: " + soilString + "." + "\n" + "Water Me!");
    }



 client.stop(); // stop the connection after each run.


}
