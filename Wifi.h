//*****************************************************************************************
//**                              Crystal lamp project                                   **
//*****************************************************************************************
//This file belongs to the Crystal lamp project and is intended to work together with the other project files, not standalone!
//This file contains the function necessary to connect to the Wifi

#include <ESP8266WiFi.h>        //Include the ESP8266 Wi-Fi library

class Wifi {
  private:
    //Define the Wifi Credentials
    String home_ssid;                         //SSID of the Home Wi-Fi network
    String home_password;                     //Password of the Wi-Fi network
    String hotspot_ssid;                      //SSID of the Lamp Hotspot
    String hotspot_password;                  //Password of the Lamp Hotspot
		String HostName = "Crystal Lamp";         //Hostname shown in Wifi
		
		boolean DHCPoverwrite = false;
		IPAddress homenetworkIP;                  //Homenetwork IP in case DHCP overide above is true
		IPAddress homenetworkGatewayIP;           //The router=gateway IP in case DHCP overide above is true
		IPAddress homenetworkSubnet;              //Subnet mask in case DHCP overide above is true
		
		IPAddress hotspotIP;                      //The hotspot IP
		IPAddress hotspotSubnet;                  //The hotspot subnet mask
	 
		uint8_t ConnectionMode = 0;               //Connection Mode 0: Connect to Home Wifi (if available, if not Hotspot), 1: Create an Hotspot, 2: Create a Crystal lamp Network
		uint8_t ConnectionTrys = 2;               //Number of trys to establish a connection before a Hotspot is started
    IPAddress IP;                             //The current IP Address
    boolean homenetwork = false;              //Homenetwork avaiable & connected
    boolean hotspot = false;                  //Hotspot activated

  public:  
    //Public functions
    Wifi(void);                                  //Constructor
    void ConnectWifi(void);                      //Connect Wifi
		String getHomeSSID(void);                    //Request for Home SSID
		String getHomeIP(void);
		String getRouterIP(void);
		void saveHomeCredentials(String, String, String, String);    //Save Home SSID and PW in EEP
		String getHotspotSSID(void);                 //Request for Hotspot SSID
		String getHotspotIP(void);                   //Request for Hotspot IP
    void saveHotspotCredentials(String, String, String); //Save Hotspot SSID and PW in EEP
		uint8_t getConnectionMode(void);             //Request for current connection mode
		void setConnectionMode(uint8_t);             //Set new connection mode
		boolean getHomenetworkConnected(void);       //Connected to Homenetwork?
		boolean getHotspotActivated(void);           //Hotspot Activated?
		void saveWifi(void);                         //save settings (atucally only the connection mode) to EEP

} Wifi;

Wifi::Wifi(void):
	//You can overwrite your homenetwors router's DHCP with the data below in order to make the lamp IP static, but be sure you know what you are doing!
	homenetworkIP(192,168,2,200), 
  homenetworkGatewayIP(192,168,2,1),
  homenetworkSubnet(255,255,255,0),
  hotspotIP(192,168,4,1),
	hotspotSubnet(255,255,255,0)
{    //Nothing to do here
}

void Wifi::ConnectWifi(void) {
  //Step 1: At first we read the access data from the EEPROM
  home_ssid = EEP.ReadString(0);
  home_password = EEP.ReadString(1);
	if(EEP.ReadString(23)==""||EEP.ReadString(24)==""){   //When no homenetwork IP or homenetwork outer IP is stored, be use DHCP...
		DHCPoverwrite = false;		
	} else {                                             	//if not, we overwrite the standard values above.
	  DHCPoverwrite = true;	
		homenetworkIP.fromString(EEP.ReadString(23));
		homenetworkGatewayIP.fromString(EEP.ReadString(24));
	}
	
  hotspot_ssid = EEP.ReadString(2);
  hotspot_password = EEP.ReadString(3);
	hotspotIP.fromString(EEP.ReadString(25));
  ConnectionMode = EEP.ReadValue8(4);

  WriteSerial.Write(String("\nLoading Wifi Access data...\n"));
  WriteSerial.Write(String("Home SSID: ")+home_ssid+String("\n")); 
  WriteSerial.Write(String("Home Password: "));  
  WriteSerial.WritePW(home_password);
  WriteSerial.Write(String("\n"));  
  WriteSerial.Write(String("Hotspot SSID: ")+hotspot_ssid+String("\n"));   
  WriteSerial.Write(String("Hotspot Password: ")); 
  WriteSerial.WritePW(String(hotspot_password));
  WriteSerial.Write(String("\n"));  

  //Step 2: Some basic Wifi configurations
	WiFi.hostname(HostName);
	
  //Step 3: Scan for existing Wifi networks
  WriteSerial.Write(String("\nStarting network scan..."));
  int n = WiFi.scanNetworks();
  WriteSerial.Write(String("finished. ")+String(n)+String(" networks found.\n"));

	//List of found networks
	for (int i = 0; i < n; ++i) {
		WriteSerial.Write(String(i+1)+String(": ")+String(WiFi.SSID(i))+String(" (")+String(WiFi.RSSI(i))+String(")")+String((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*")+String("\n"));
		delay(100);
	}
	WriteSerial.Write(String("\n"));


  //Setp 4: We go through the found wifi networks and search for our home network 
  for (int i = 0; i < n; ++i) {
    if (String(WiFi.SSID(i)) == home_ssid) {
      homenetwork = true;
      WriteSerial.Write(String("Home Wifi-Network (")+home_ssid+String(") found!\n"));
    } 
		if (String(WiFi.SSID(i)) == hotspot_ssid) {
      hotspot = true;
      WriteSerial.Write(String("Crystal lamp Hotspot (")+String(hotspot_ssid)+String(") found!\n"));
    }
  }
  
  if (!homenetwork) WriteSerial.Write(String("Home Wifi-Network (")+home_ssid+String(") not found!\n"));
  if (!hotspot) WriteSerial.Write(String("Local crystal lamp Wifi-Network (")+String(hotspot_ssid)+String(") not found!\n"));
	
  //Step 5: Connect a Wifi Network or create a Hotspot
	WriteSerial.Write(String("Connection Mode '")+String(ConnectionMode)+String("' has been set.\n"));
  if (ConnectionMode==0&&homenetwork) {                             //If connection mode 0 is set and the home wifi network is present, we connect  
    WiFi.mode(WIFI_STA);                                            //We switch to station mode
		
		if(DHCPoverwrite) {                                             //When DHCP overwrite is true, we set IP, gateway and subnet mask manually
				WiFi.config(homenetworkIP, homenetworkGatewayIP, homenetworkSubnet);
		}
		
		for(int i=0; i < ConnectionTrys; i++){       
      WriteSerial.Write(String("Connecting to Home Wifi-Network, try number ") +String(i+1)+String(" "));		//Start to connect to the home wifi network
			WiFi.begin(home_ssid, home_password);           
			
			while (WiFi.status() != WL_CONNECTED) {                        // Wait for the Wi-Fi to connect
				WriteSerial.Write(String("."));
				delay(500);
				if (WiFi.status() == WL_CONNECT_FAILED){ 
					break;
				}
			}
			
			if (WiFi.status() == WL_CONNECTED) {
				IP = WiFi.localIP();   
				WriteSerial.Write(String("connected!\nIP Address: ")+String(IP.toString())+String("\n\n"));
				break;                                                       //End the connection try for-loop 
			} else if (WiFi.status() == WL_CONNECT_FAILED)  {
				homenetwork = false;
				WriteSerial.Write(String("failed!"));
			}
		}
    
  } else if (ConnectionMode==1||!homenetwork) {
    WriteSerial.Write(String("Starting Crystal Lamp Hotspot: ") + hotspot_ssid);  
    WiFi.mode(WIFI_AP);                                     //We switch to soft access point mode
		WiFi.softAPConfig(hotspotIP, hotspotIP, hotspotSubnet); //We configure the soft access point...
    hotspot = WiFi.softAP(hotspot_ssid, hotspot_password);  //..and start it
    delay(500);                                             //Wait a moment

    if (hotspot) {
      IP = WiFi.softAPIP();   
      WriteSerial.Write(String("Lamp Hotspot started\nAccess point IP address: ")+String(IP.toString())+String("\n\n"));
    } else {
      WriteSerial.Write(String("Failed to start Lamp Hotspot, please reset!\n\n"));
    }
  } 
	
	//Note: Connection Mode 2 logic is not yet implemented
}

String Wifi::getHomeSSID(void){
	return(String(home_ssid));	
}

String Wifi::getHomeIP(void){
	if(DHCPoverwrite) return(homenetworkIP.toString());
	else return("");
}

String Wifi::getRouterIP(void){
	if(DHCPoverwrite) return(homenetworkGatewayIP.toString());
	else return("");
}

void Wifi::saveHomeCredentials(String newhomessid, String newhomepassword, String newhomeip, String newrouterip){
	boolean changes = false;                                   //Variable to note if we changes something, controlling the restart below
	if((newhomessid==home_ssid) && (newhomepassword=="")) {            //If the home ssid is still the old one and password hasn't been set, we do nothing!
	} else if ((newhomessid==home_ssid) && (newhomepassword!="")){     //If the hotspot ssid is still the old, but a new password has been set, we change it
		home_password = newhomepassword;
		EEP.WriteString(1, newhomepassword);
		changes = true;
  }	else if (newhomessid!=home_ssid){                         //If a new ssid has been set, we change ssid and password
		home_password = newhomepassword;
    EEP.WriteString(1, newhomepassword);
	  home_ssid = newhomessid;           
		EEP.WriteString(0, newhomessid);  
		changes = true;    
	}
	
	IPAddress newhomeIP, newrouterIP;         //We convert the newhome and router ip strings to objects
	newhomeIP.fromString(newhomeip); 
	newrouterIP.fromString(newrouterip);
	
	if(newhomeip==""&&DHCPoverwrite) {   //When no new IP has been transmitted, but there was one before, we switch off the DHCP overwrite and erase the EEP 
		DHCPoverwrite	= false;
		EEP.WriteString(23, newhomeip);   
		EEP.WriteString(24, newrouterip);
		changes = true;  
	} else if ((newhomeip!=""&&newrouterip!="")&&((newhomeIP!=homenetworkIP)||(newrouterIP!=homenetworkGatewayIP))){   //If home and gateway IP are given and not equal to the actual ones
		DHCPoverwrite	= true;
    homenetworkIP = newhomeIP;
		homenetworkGatewayIP = newrouterIP;
		EEP.WriteString(23, newhomeip);   
		EEP.WriteString(24, newrouterip);
		changes = true;
	} 
	
	if(changes) EEP.Commit(); //As an exception, the wifi credentials are directly stored in EEP on submit, when something has changed.
}


String Wifi::getHotspotSSID(void){
	return(String(hotspot_ssid));
}

String Wifi::getHotspotIP(void){
	return(hotspotIP.toString());
}

void Wifi::saveHotspotCredentials(String newhotspotssid, String newhotspotpassword, String newhotspotip){
  boolean changes = false;                                   //Variable to note if we changes something, controlling the restart below
	if((newhotspotssid==hotspot_ssid) && (newhotspotpassword=="")) {         //If the hotspot ssid is still the old one and password hasn't been set, we do nothing!
	} else if ((newhotspotssid==hotspot_ssid) && (newhotspotpassword!="")){  //If the hotspot ssid is still the old, but a new password has been set, we change it
		hotspot_password = newhotspotpassword;
		EEP.WriteString(3, newhotspotpassword);
		changes = true;
  }	else if (newhotspotssid!=hotspot_ssid){                         //If a new ssid has been set, we change ssid and password
		hotspot_password = newhotspotpassword;
    EEP.WriteString(3, newhotspotpassword);
	  hotspot_ssid = newhotspotssid;           
		EEP.WriteString(2, newhotspotssid);  
		changes = true;    
	}
	
	IPAddress newhotspotIP;                 //We convert the newip string to an object
	newhotspotIP.fromString(newhotspotip); 
	
	if (newhotspotip!=""&&newhotspotIP!=hotspotIP){  //When a new non-empty IP has been transmitted
    hotspotIP = newhotspotIP;
		EEP.WriteString(25, newhotspotip);
		changes = true;
	} 
	
	if(changes) EEP.Commit(); //As an exception, the wifi credentials are directly stored in EEP on submit, when something has changed.
}

uint8_t Wifi::getConnectionMode(void){
	return(ConnectionMode);
}

//Set new connection mode
void Wifi::setConnectionMode(uint8_t input){
		ConnectionMode = input;
}	

boolean Wifi::getHomenetworkConnected(void){  //Connected to Homenetwork?
	return(homenetwork);
}
     
boolean Wifi::getHotspotActivated(void){           //Hotspot Activated?
	return(hotspot);
}           

//Save settings (atucally only the connection mode) to EEP
void Wifi::saveWifi(void){      
	EEP.WriteValue8(4, ConnectionMode);  
  EEP.Commit(); //As an exception, the wifi credentials are directly stored in EEP on submit
}