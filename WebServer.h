//*****************************************************************************************
//**                              Crystal cyrstal lamp project                           **
//*****************************************************************************************
//This file belongs to the Smart crystal lamp project and is intended to work together with the other project files, not standalone!
//
//This file contains the function necessary to operate the file system and the webservers for HTTP, SSDP and mDNS
//For those who might wonder: In contrast to most other modules I did not implement this as class, because this caused trouble with all the server
//objects used here constantly.

#include <ESP8266WebServer.h>   //Include WebServer library
#include <LittleFS.h>           //Include LittleFS library
#include <ESP8266SSDP.h>        //Include SSDP library
#include <ESP8266mDNS.h>        //Include mDNS responder for Home Wifi (nor supported by Andriod :-()

//Some definitions
#define WEBSERVER_PORT 80

//We create our webserver objects
ESP8266WebServer WebServer(WEBSERVER_PORT);  

//Global variables
String mDNS_Name = "Crystal";                //This is the local webpage without .local, e.g. "Crystal.local" (mDNS Service)
String SSDP_Name = "Smart Crystal Lamp";     //The basic information shown in Windows Network search (SSDP service)
String SSDP_Model_Name = "Smart Crystal Lamp";
String SSDP_Model_Version = "1.0";
String SSDP_Manufacturer_Name = "Smart Crystal Lamp Project";

         
//Our internal little helper functions
String getContentType(String filename);    //Helping function to convert the file extension to the MIME type
String formatBytes(size_t bytes);          //Helping function to calculate datasize format
bool handleFileRead(String path);          //Send the right file to the client (if it exists)

//The handle function for server requests
void handleSetControl();                   //Handle function for commands from Webinterface
void handleGetControl();                   //Handle function for states to Webinterface
void handleSetOptions();                   //Handle function for set options request
void handleGetOptions();                   //Handle function for get options request
void handleSetWifi();                      //Handle function for commands from Webinterface
void handleGetWifi();                      //Handle function for states to Webinterface

//This function starts up the WebServer
void startWebServer(void){
  //Start the Flash File System (LittleFS)
  LittleFS.begin();                                        
  WriteSerial.Write(String("Flash File System (LittleFS) started, listing content:\n")); 
	                          
	//List the files (only suppports one level of folders!)
	Dir dir = LittleFS.openDir("/");    
	while (dir.next()) {
		if(dir.isDirectory()){
		WriteSerial.Write(String(dir.fileName())+String("\\")+String("\n"));
			Dir fold = LittleFS.openDir(dir.fileName());
			while(fold.next()){
				if(fold.isFile()) WriteSerial.Write(String("   ")+String(fold.fileName())+String(", ")+String(formatBytes(fold.fileSize()))+String("\n"));
			}
			
		} else if (dir.isFile()) WriteSerial.Write(String(dir.fileName())+String(", ")+String(formatBytes(dir.fileSize()))+String("\n"));
	}
	WriteSerial.Write("\n");
	
  //Start the HTTP Webserver
  WebServer.begin();                                      
  WebServer.on("/setcontrol", HTTP_POST, handleSetControl);            //Handle function for settings from control tab
  WebServer.on("/getcontrol", HTTP_POST, handleGetControl);            //Handle function for feedbackt to control page
  WebServer.on("/setoptions", HTTP_POST, handleSetOptions);            //Handle function for settings from options tab
  WebServer.on("/getoptions", HTTP_POST, handleGetOptions);            //Handle function for feedbackt to options page
  WebServer.on("/setwifi", HTTP_POST, handleSetWifi);                  //Handle function for settings from wifi tab
  WebServer.on("/getwifi", HTTP_POST, handleGetWifi);                  //Handle function for feedbackt to wifi page

  WebServer.on("/description.xml", HTTP_GET, []() {              //Handle function for SSDP request
    SSDP.schema(WebServer.client());
  });
  
  //This is actually our main function handling the file system
  WebServer.onNotFound([]() {                              // If the client requests any URI, that is not specified explicitly above...
    if (!handleFileRead(WebServer.uri()))                  // ...we check if it exists in the file system ...
      WebServer.send(404, "text/plain", "404: Not Found"); // ...if not we complain with 404 (Not Found) error
  });  
  WriteSerial.Write(String("Webserver started!\n\n")); 

  //Start "Simple Service Discovery Protocol (SSDP) setup...
  SSDP.setSchemaURL("description.xml");
  SSDP.setHTTPPort(WEBSERVER_PORT);
  SSDP.setName(SSDP_Name);
  SSDP.setSerialNumber(ESP.getChipId());
  SSDP.setURL("index.html");
  SSDP.setModelName(SSDP_Model_Name);
  SSDP.setModelNumber(SSDP_Model_Version);
  SSDP.setModelURL("https://github.com/danielschlingmeier/smartcrystallamp");
  SSDP.setManufacturer(SSDP_Manufacturer_Name);
  SSDP.setManufacturerURL("");
  SSDP.setDeviceType("upnp:rootdevice");
	
  SSDP.begin();  //..and start up the whole thing
  
  WriteSerial.Write(String("Simple Service Discovery Protocol (SSDP) started!\n\n"));
	
  //Start the mDNS Server
	if (!MDNS.begin(mDNS_Name)) WriteSerial.Write("Error setting up MDNS responder!\n");
  else WriteSerial.Write("mDNS responder started!\n\n");

}

//Helper function for content type declaration on the file system
String getContentType(String filename){
  if (filename.endsWith(".htm"))      return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css"))  return "text/css";
  else if(filename.endsWith(".js"))   return "application/javascript";
  else if(filename.endsWith(".png"))  return "image/png";
  else if(filename.endsWith(".gif"))  return "image/gif";
  else if(filename.endsWith(".jpg"))  return "image/jpeg";
  else if(filename.endsWith(".ico"))  return "image/x-icon";
  else if(filename.endsWith(".xml"))  return "text/xml";
  else if(filename.endsWith(".pdf"))  return "application/x-pdf";
  else if(filename.endsWith(".zip"))  return "application/x-zip";
  else if(filename.endsWith(".gz"))   return "application/x-gzip";
  return "text/plain";  
}

//Helper function for file size listing on the file system
String formatBytes(size_t bytes) {
  if (bytes < 1024)                        return String(bytes) + "B";
  else if (bytes < (1024 * 1024))          return String(bytes / 1024.0) + "KB";
  else if (bytes < (1024 * 1024 * 1024))   return String(bytes / 1024.0 / 1024.0) + "MB";
  else                                     return String(bytes / 1024.0 / 1024.0 / 1024.0) + "GB";
}

void handleSetControl(){
  //This function handles all commands send by the control webpage. It checks for known requests, but not always for a valid content 
  //(unless data type overlfows e.g. when converting int to boolean. ThesetXYZ() functions have to check for validity
  
  boolean Known_Request = false;      //Temporary variable to check if at least one valid request has been received.

  if(WebServer.hasArg("mode")){ //Change mode
		if((LED.getLedMode()==3)&&(WebServer.arg("mode").toInt()!=3)) LED.endMotionMode();
    LED.setLedMode(WebServer.arg("mode").toInt());
    WriteSerial.Write(String("LED mode ") + String(LED.getLedMode()) + String(" set via Controlpage\n"));
    WebServer.send(200, "text/plain", "200: New LED mode set!"); 
    Known_Request = true; 
  }

  if(WebServer.hasArg("effect")){ //Change effect 
    LED.setEffect(WebServer.arg("effect").toInt());
    WriteSerial.Write(String("LED effect: ") + String(LED.getEffect()) + String(" set via Controlpage\n"));
    WebServer.send(200, "text/plain", "200: New LED effect set!"); 
    Known_Request = true;  
  }  

  if(WebServer.hasArg("r")&&WebServer.hasArg("g")&&WebServer.hasArg("b")){ //Change color
    LED.setColor(CRGB(WebServer.arg("r").toInt(),WebServer.arg("g").toInt(), WebServer.arg("b").toInt()));
    WriteSerial.Write(String("Color: R: ") + WebServer.arg("r") + String(" G: ") + WebServer.arg("g") + String(" B: ") + WebServer.arg("b") + String(" set via Controlpage\n"));
    WebServer.send(200, "text/plain", "200: New color set!");  
    Known_Request = true; 
  }
  
  if(WebServer.hasArg("power")){ //Changge power
    LED.setPower(WebServer.arg("power").toInt());
    WriteSerial.Write(String("Effect power: ") + String(LED.getPower()) + String(" set via Controlpage\n"));
    WebServer.send(200, "text/plain", "200: New effect power set!");    
    Known_Request = true; 
  }
	
	if(WebServer.hasArg("save")){ //EPP save request
	  LED.saveDefault();          //Save LED Options
    WriteSerial.Write(String("LED defaults save requested via Controlpage\n"));
    WebServer.send(200, "text/plain", "200: New LED defaults saved!");
    Known_Request = true;     
  }
	
  if(!Known_Request){
    WriteSerial.Write("Unknown command request from Controlpage");  
    WebServer.send(400, "text/plain", "400: Unkown command request");
  }
}

void handleGetControl(){    //Function to send the variables for the control tab content back to the controlpage, when requested
  //At first we create an answer string in JSON format
  String s = String("{\"mode\":"+ String(LED.getLedMode())+
                    ",\"effect\":"+ String(LED.getEffect())+
                    ",\"power\":"+ String(LED.getPower())+
										",\"batterysensconnected\":"+ String(PowerSupply.getINAconnected())+   
                    ",\"radarsensconnected\":"+String(Radar.getRadarConnected())+   										
                    ",\"voltage\":"+String(PowerSupply.getVoltage())+
                    ",\"current\":" +String(PowerSupply.getCurrent())+
										",\"SOCmAh\":"+String(PowerSupply.getSOCmAh())+
                    ",\"SOCpercent\":"+String(PowerSupply.getSOCpercent())+   		
                    ",\"motion\":"+String(Radar.isMotion())+               
                    ",\"r\":"+ String(LED.getColor().r)+
                    ",\"g\":"+ String(LED.getColor().g)+
                    ",\"b\":"+ String(LED.getColor().b)+                   
                    "}"); 
  WebServer.send(200, "text/plain", s);  //...and send it back to the client
}

void handleSetOptions(){
  //This function handles the options changes send by the control webpage. It checks for known requests, but not always for a valid content 
  //(unless data type overlfows e.g. when converting int to boolean. ThesetXYZ() functions have to check for validity
  
  boolean Known_Request = false;      //Temporary variable to check if a valid request has been received.

  if(WebServer.hasArg("powerleddeact")){ //Power led deactivation request
    PowerSupply.DeactivatePowerLed(WebServer.arg("powerleddeact").toInt() > 0);
    WriteSerial.Write(String("Deactivate Power LED: ") + String(PowerSupply.getPowerLedDeactivated()) + String(" set via Controlpage\n"));
    WebServer.send(200, "text/plain", "200: New power led deactivation set!");    
    Known_Request = true; 
  }
  
  if(WebServer.hasArg("warningleddeact")){ //Warning led deactivation request
    PowerSupply.DeactivateWarningLed(WebServer.arg("warningleddeact").toInt() > 0);
    WriteSerial.Write(String("Deactivate Warning LED: ") + String(PowerSupply.getWarningLedDeactivated()) + String(" set via Controlpage\n"));
    WebServer.send(200, "text/plain", "200: New warning led deactivation set!");  
    Known_Request = true;   
  }

  if(WebServer.hasArg("motionmode")){ //Motion mode request
    Radar.setMotionMode(WebServer.arg("motionmode").toInt());
    WriteSerial.Write(String("Motion mode: ") + String(Radar.getMotionMode()) + String(" set via Controlpage\n"));
    WebServer.send(200, "text/plain", "200: New motion mode set!"); 
    Known_Request = true;    
  }

  if(WebServer.hasArg("offtime")){ //Off time request
    Radar.setOffTime(WebServer.arg("offtime").toInt());
    WriteSerial.Write(String("Off time: ") + String(Radar.getOffTime()) + String(" set via Controlpage\n"));
    WebServer.send(200, "text/plain", "200: New off time set!");   
    Known_Request = true;  
  }

  if(WebServer.hasArg("ontime")){ //On time request
    Radar.setOnTime(WebServer.arg("ontime").toInt());
    WriteSerial.Write(String("On time: ") + String(Radar.getOnTime()) + String(" set via Controlpage\n"));
    WebServer.send(200, "text/plain", "200: New on time set!");    
    Known_Request = true; 
  }

  if(WebServer.hasArg("sleeptime")){ //Sleep time request
    Radar.setSleepTime(WebServer.arg("sleeptime").toInt());
    WriteSerial.Write(String("Sleep time: ") + String(Radar.getSleepTime()) + String(" set via Controlpage\n"));
    WebServer.send(200, "text/plain", "200: New sleep time set!"); 
    Known_Request = true;    
  }

  if(WebServer.hasArg("softmode")){ //Soft mode request
    LED.setSoftMode(WebServer.arg("softmode").toInt() > 0);
    WriteSerial.Write(String("Mode mode: ") + String(LED.getSoftMode()) + String(" set via Controlpage\n"));
    WebServer.send(200, "text/plain", "200: New soft mode set!");
    Known_Request = true;     
  }

  if(WebServer.hasArg("softtime")){ //Soft time request
    LED.setSoftTime(WebServer.arg("softtime").toInt());
    WriteSerial.Write(String("Soft time: ") + String(LED.getSoftTime()) + String(" set via Controlpage\n"));
    WebServer.send(200, "text/plain", "200: New soft time set!"); 
    Known_Request = true;    
  }
	
	if(WebServer.hasArg("fullvoltage")){ //Full voltage request
    PowerSupply.setFullVoltage(WebServer.arg("fullvoltage").toInt());
    WriteSerial.Write(String("Full battery voltage: ") + String(PowerSupply.getFullVoltage()) + String(" set via Controlpage\n"));
    WebServer.send(200, "text/plain", "200: New full battery voltage set!"); 
    Known_Request = true;    
  }
	
		if(WebServer.hasArg("emptyvoltage")){ //Empty voltage request
    PowerSupply.setEmptyVoltage(WebServer.arg("emptyvoltage").toInt());
    WriteSerial.Write(String("Empty battery voltage: ") + String(PowerSupply.getEmptyVoltage()) + String(" set via Controlpage\n"));
    WebServer.send(200, "text/plain", "200: New empty battery voltage set!"); 
    Known_Request = true;    
  }
	
		if(WebServer.hasArg("capacity")){ //Capacity request
    PowerSupply.setCapacity(WebServer.arg("capacity").toInt());
    WriteSerial.Write(String("Battery capacity: ") + String(PowerSupply.getCapacity()) + String(" set via Controlpage\n"));
    WebServer.send(200, "text/plain", "200: New battery capacity!"); 
    Known_Request = true;    
  }
  
  if(WebServer.hasArg("save")){ //EPP save request
		LED.saveOptions();          //Save LED Options
	  PowerSupply.saveOptions();  //Save Power Supply Options
    Radar.saveOptions();        //Save Radar / Motion Sensor Options
    WriteSerial.Write(String("Options save requested via Controlpage\n"));
    WebServer.send(200, "text/plain", "200: New options saved!");
    Known_Request = true;     
  }
	
	if(WebServer.hasArg("reset")){ //Reset request
		WriteSerial.Write(String("Reset requested via Controlpage\n"));
		WebServer.send(200, "text/plain", "200: Reset confirmed!");  	
		WriteSerial.Write(String("EEP Reset, lamp restart triggered!\n"));
		delay(500);
		EEP.Reset();
		ESP.restart();
		Known_Request = true; 
  }
	
  if(WebServer.hasArg("restart")){ //Reset request
		WriteSerial.Write(String("Restart requested via Controlpage\n"));
		WebServer.send(200, "text/plain", "200: Restart confirmed!");    
		WriteSerial.Write(String("Lamp restart!\n"));
		delay(500);
		ESP.restart();
		Known_Request = true; 
  }
  
  if(!Known_Request){
    WriteSerial.Write("Unknown set option request send by Controlpage");  
    WebServer.send(400, "text/plain", "400: Unkown set option request");
  }
}


void handleGetOptions(){    //Function to send the variables for the options tab content back to the controlpage, when requested
  //At first we create an answer string in JSON format
  String s = String("{\"voltage\":" +String(PowerSupply.getVoltage())+
                    ",\"current\":" +String(PowerSupply.getCurrent())+
										",\"SOCmAh\":"+String(PowerSupply.getSOCmAh())+
                    ",\"SOCpercent\":"+String(PowerSupply.getSOCpercent())+
                    ",\"powerleddeact\":"+ String(PowerSupply.getPowerLedDeactivated())+
                    ",\"warningleddeact\":"+ String(PowerSupply.getWarningLedDeactivated())+  
                    ",\"radarsensconnected\":"+String(Radar.getRadarConnected())+    					
                    ",\"batterysensconnected\":"+ String(PowerSupply.getINAconnected())+       
                    ",\"motionmode\":"+String(Radar.getMotionMode())+
                    ",\"offtime\":"+String(Radar.getOffTime())+
                    ",\"ontime\":"+String(Radar.getOnTime())+
                    ",\"sleeptime\":"+String(Radar.getSleepTime())+
                    ",\"softmode\":"+ String(LED.getSoftMode())+
                    ",\"softtime\":"+ String(LED.getSoftTime())+  
                    ",\"fullvoltage\":"+ String(PowerSupply.getFullVoltage())+     
                    ",\"emptyvoltage\":"+ String(PowerSupply.getEmptyVoltage())+     
                    ",\"capacity\":"+ String(PowerSupply.getCapacity())+     										
                    "}"); 
  WebServer.send(200, "text/plain", s);  //...and send it back to the client
}

void handleSetWifi(){
  /*This function handles the wifi options changes send by the control webpage. It checks for known requests, but not always for a valid content 
  (unless data type overlfows e.g. when converting int to boolean. ThesetXYZ() functions have to check for validity*/
  
  boolean Known_Request = false;      //Temporary variable to check if at least one valid request has been received.

  if(WebServer.hasArg("homessid")&&WebServer.hasArg("homepassword")&&WebServer.hasArg("homeip")&&WebServer.hasArg("routerip")){ //Power led deactivation request
	  Wifi.saveHomeCredentials(WebServer.arg("homessid"), WebServer.arg("homepassword"), WebServer.arg("homeip"), WebServer.arg("routerip"));
    WriteSerial.Write(String("New Home Network SSID ") + String(Wifi.getHomeSSID()) + String(" and Passwort ***** (hidden) set via Controlpage\n"));
    WebServer.send(200, "text/plain", "200: New Home Network Wifi Credentials set!");    
    Known_Request = true; 
  }
 
  if(WebServer.hasArg("hotspotssid")&&WebServer.hasArg("hotspotpassword")&&WebServer.hasArg("hotspotip")){ //Power led deactivation request
		Wifi.saveHotspotCredentials(WebServer.arg("hotspotssid"), WebServer.arg("hotspotpassword"), WebServer.arg("hotspotip"));
    WriteSerial.Write(String("New Hotspot SSID '") + String(Wifi.getHotspotSSID()) + String("' Passwort '*****'and Hotspot IP '")+String(Wifi.getHotspotIP()) + String("' set via Controlpage\n"));
    WebServer.send(200, "text/plain", "200: New Hotspot SSID and password set!");    
    Known_Request = true; 
  }
  
  if(WebServer.hasArg("connectionmode")){ //Connection mode request
    Wifi.setConnectionMode(WebServer.arg("connectionmode").toInt());
    WriteSerial.Write(String("New Connection mode ") + String(Wifi.getConnectionMode())+ String(" set via Controlpage\n"));
    WebServer.send(200, "text/plain", "200: New Connection mode set!");    
    Known_Request = true; 
  }
  
  if(WebServer.hasArg("save")){ //Save settings to EEP
    Wifi.saveWifi();
    WriteSerial.Write(String("Wifi settings save requested via Controlpage\n"));
    WebServer.send(200, "text/plain", "200: New Wifi settings saved!");    
    Known_Request = true; 
  }
  
  if(!Known_Request){
    WriteSerial.Write("Unknown set option request send by Controlpage");  
    WebServer.send(400, "text/plain", "400: Unkown set option request");
  }
}
  
  void handleGetWifi(){    //Function to send the variables for the options tab content back to the controlpage, when requested
  //At first we create an answer string in JSON format
  String s = String("{\"homessid\":\"" + Wifi.getHomeSSID() + "\"" +
	                  ",\"homeip\":\""+ Wifi.getHomeIP() + "\"" +
										",\"routerip\":\""+ Wifi.getRouterIP() + "\"" +
                    ",\"hotspotssid\":\""+ Wifi.getHotspotSSID() + "\"" +
										",\"hotspotip\":\""+ Wifi.getHotspotIP() + "\"" +
										",\"connectionmode\":\"" + String(Wifi.getConnectionMode())+ "\"" +
                    "}"); 
  WebServer.send(200, "text/plain", s);  //...and send it back to the client
  }


//This function is executed when files shall be fetched from the file system, if they exist
bool handleFileRead(String path){ 
  Serial.println("handleFileRead: " + path);
  if(path.endsWith("/")) path += "index.html";           // If a folder is requested, send the index file
  String contentType = getContentType(path);             // Get the MIME type
  String pathWithGz = path + ".gz";
  if(LittleFS.exists(pathWithGz) || LittleFS.exists(path)){  // If the file exists, either as a compressed archive, or normal
    if(LittleFS.exists(pathWithGz))                          // If there's a compressed version available
      path += ".gz";                                         // Use the compressed version
    File file = LittleFS.open(path, "r");                    // Open the file
    size_t sent = WebServer.streamFile(file, contentType);   // Send it to the client
    file.close();                                            // Close the file again
    Serial.println(String("\tSent file: ") + path);
    return true;
  }
  Serial.println(String("\tFile Not Found: ") + path);
  return false;                                          // If the file doesn't exist, return false
}
