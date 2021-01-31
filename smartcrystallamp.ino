//*****************************************************************************************
//**                              Crystal lamp project                                   **
//*****************************************************************************************
//This is the main file of the Crystal lamp project

#include "WriteSerial.h"    //Include the Serial write function 
#include "EEPROM.h"         //Include the class to access the EEPROM configuration
#include "PowerSupply.h"    //Include power supply
#include "LED.h"            //Include the functions to operate the led strip
#include "Wifi.h"           //Include our Wifi Communication
#include "Radar.h"          //Include the radar functions
#include "WebServer.h"      //Include the file system and webserver

void setup() {
  //Pretty straight forward: We start the modules (in the right order! :-))   
  WriteSerial.Start(); //The very first thing we do is switching on the serial connection (if activated, but that is handled in the class internally)
  EEP.Start();         //We start the EEP Access (no EEP Access before this point!)
  PowerSupply.Start(); //We switch on the power supply and check for the battery sensor
  Radar.Start();       //Start the Radar functions
  LED.Start();         //Start the LED functions
  Wifi.ConnectWifi();  //Connect to our Wifi Home Network or open a Hotspot   
  startWebServer();    //Start the Webservers
}

void loop() {
  PowerSupply.Run();              //Power supply run function  
	Radar.Run();                    //Check for motion
  LED.Run();                      //LED Strip run function
 
	
	MDNS.update();                               //Listen for mDNS request (crystal.local)
  WebServer.handleClient();                    // Listen for HTTP requests from clients   
}
