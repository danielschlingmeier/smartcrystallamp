//*****************************************************************************************
//**                              Crystal lamp project                                   **
//*****************************************************************************************
//This file belongs to the Crystal lamp project and is intended to work together with the other project files, not standalone!

//This file contains the function necessary to manage the configuration data in the EEPROM
#include <EEPROM.h>       //Include the EEPROM library

class EEP {
  private:
    uint16_t maxsize = 240;     //The maximum size of the EEPROM content in byte, need to be bigger than the last address below :)
    byte MemMap[28] = {0, 33, 66, 99, 132, 133, 134, 135, 136, 137, 138, 140, 142, 144, 145, 146, 147, 148, 149, 150, 152, 154, 156, 158, 179, 200, 221, 237};   //This array contains the start addresses in th EEPROM
    byte maxlength = 32;
    //Index (starting with 0!)
    //0: home_ssid [32+1],             0
    //1: home_password [32+1],         33
    //2: local_ssid [32+1],            66
    //3: local_password [32+1],        99
		//4: Connection_Mode [1],          132
    //5: Power LED deactivation [1],   133
    //6: Warning LED deactivation [1], 134
    //7: Radar sensor connected [1],   135
    //8: Soft Mode [1],                136
		//9: Motion Mode [1],              137
		//10: Off Time [2],                138
		//11: On Time [2],                 140
		//12: Sleep Time [2],              142
		//13: Current LED Mode [1],        144
		//14: Current Effect [1],          145
		//15: Current Power [1],           146
		//16: Current Color RED [1],       147
		//17: Current Color GREEN [1],     148
		//18: Current Color BLUE [1],      149
		//19: Soft time [2],               150		
		//20: Fullvolatge [2]              152
		//21: Endvolatge [2]               154		
		//22: Capacity [2]                 156				
		//23: Homenetwork IP [21]          158
		//24: Homenetwork router IP [21]   179
		//25: Hotspot IP [21]              200
		//26: Blank EEP check string [16]  221
    //27: Endstop sign (no data starting here!), 237
  public:
    EEP(void);
    void Start(void);
    void End(void);
    void Commit(void);
    void Reset(void);
    boolean WriteValue16(byte, uint16_t);
    uint16_t ReadValue16(byte);
    boolean WriteValue8(byte, uint8_t);
    uint8_t ReadValue8(byte);
    boolean WriteString(byte, String);
    String ReadString(byte);
		void WriteEEPSerial(void);
}EEP;

EEP::EEP(void){
}

void EEP::Start(void){   
  EEPROM.begin(maxsize);    //We activate EEP access
	if(ReadString(26) != "3LHgb25wfDpkdFL"){     //When the check string is not what we expect the EEP has not been written (first start), so we Reset() it	
		WriteSerial.Write("EEP ROM has not been written before.\n");
		Reset();
	}
}

void EEP::End(void){
  EEPROM.end();   
}

//Function to restore all standard values
void EEP::Commit(void){
  EEPROM.commit();
}

//Function to restore all standard values
void EEP::Reset(void){
  WriteSerial.Write("Resetting EEPROM to default values (change via options page)...");
  WriteString(0, String(""));                   //Home Wifi Credentials
  WriteString(1, String(""));
  WriteString(2, String("CrystalNet"));         //Hotspot Credentials
  WriteString(3, String(""));  
  WriteValue8(4, 0);                            //Connection mode: Standard Home Connect
  WriteValue8(5, 0);                            //Power LED deactivation: No
  WriteValue8(6, 0);                            //Warning LED deactivation: No
  WriteValue8(7, 1);                            //Radar sensor connected: Yes 
  WriteValue8(8, 1);                            //Soft mode: Activated 
  WriteValue8(9, 0);                            //Motion mode: Off
  WriteValue16(10, 10);                         //Off time: 10s
  WriteValue16(11, 10);                         //On time: 10s
  WriteValue16(12, 600);                        //Sleep time: 600s / 10 min
  WriteValue8(13, 0);                           //LED Mode(start default): Off 
  WriteValue8(14, 1);                           //LED Effect (start default): Color
  WriteValue8(15, 128);                         //Effect Power (start default): 128 (50%) (start default)
  WriteValue8(16, 255);                         //Color RED: 255 (full red, start default)
  WriteValue8(17, 0);                           //Color GREEN: 0 
  WriteValue8(18, 0);                           //Color BLUE: 0 
  WriteValue16(19, 5);                          //Soft time: 5s
	WriteValue16(20, 4200);                       //Full battery voltage: 4200 mV
	WriteValue16(21, 3000);                       //End battery voltage: 3000 mV
	WriteValue16(22, 2770);                       //Capacity: 2770 mAh
	WriteString(23, "");                          //Homenetwork IP is not net, thus DHCP is used
	WriteString(24, "");                          //Homenetwork Gateway IP is not net, thus DHCP is used
	WriteString(25, "192.168.4.1");               //Hotspot Standard IP
	WriteString(26, "3LHgb25wfDpkdFL");           //A random string used to check if the EEP has been written before. If not, standard values are taken
  Commit();                                     //Save to EEP
	WriteSerial.Write("done!\n");
  WriteEEPSerial();                             //Give a serial output 
	
}

boolean EEP::WriteValue16(byte startidx, uint16_t value){
  //When the size of value (of 16) exceeds the reserved space in the Memory Map, we stop
  if(sizeof(value)>(MemMap[startidx+1]-MemMap[startidx])) {  //If the value length exceeds the maximum reserved memory, we stop!
    WriteSerial.Write(String("EEP: Input value size of ")+ String(sizeof(value)) + String(" exceeds maximum reserved memory in the Memory Map of ") + String(MemMap[startidx+1]-MemMap[startidx])+String("\n"));  
    return(false);       
  } else {   //If everything is fine so far, we save the string to the EEP
    EEPROM.put(MemMap[startidx], value);
    return(true);
  }   
}

uint16_t EEP::ReadValue16(byte startidx){
  uint16_t value = 0;
  if(sizeof(value)>(MemMap[startidx+1]-MemMap[startidx])) {  //If the value length exceeds the maximum reserved memory, we stop!
    WriteSerial.Write(String("EEP: Value size of ")+ String(sizeof(value)) + String(" exceeds maximum reserved memory in the Memory Map of ") + String(MemMap[startidx+1]-MemMap[startidx])+String("\n"));  
    return(0);       
  } else {   //If everything is fine so far, we save the string to the EEP
    EEPROM.get(MemMap[startidx], value); 
    return(value);
  }  
}

boolean EEP::WriteValue8(byte startidx, uint8_t value){
  //When the size of value (of 16) exceeds the reserved space in the Memory Map, we stop
  if(sizeof(value)>(MemMap[startidx+1]-MemMap[startidx])) {  //If the value length exceeds the maximum reserved memory, we stop!
    WriteSerial.Write(String("EEP: Input value size of ")+ String(sizeof(value)) + String(" exceeds maximum reserved memory in the Memory Map of ") + String(MemMap[startidx+1]-MemMap[startidx])+String("\n"));  
    return(false);       
  } else {   //If everything is fine so far, we save the string to the EEP
    EEPROM.put(MemMap[startidx], value);
    return(true);
  }   
}

uint8_t EEP::ReadValue8(byte startidx){
  uint8_t value;
  if(sizeof(value)>(MemMap[startidx+1]-MemMap[startidx])) {  //If the value length exceeds the maximum reserved memory, we stop!
    WriteSerial.Write(String("EEP: Value size of ")+ String(sizeof(value)) + String(" exceeds maximum reserved memory in the Memory Map of ") + String(MemMap[startidx+1]-MemMap[startidx])+String("\n"));  
    return(false);       
  } else {   //If everything is fine so far, we save the string to the EEP
    EEPROM.get(MemMap[startidx], value);
    return(value);
  }  
}

boolean EEP::WriteString(byte startidx, String input){
  //When the string length exceeds the maximum allowed length (defined above in this class), we stop
  if(input.length()>maxlength) {
    WriteSerial.Write(String("EEP: Input string length of '") + input + String("' is ") + String(input.length()) + String(" and exceeds maximum allowed length of ") + String(maxlength)+String("\n"));
    return(false);
  }

  //When the string lenth exceeds the reserved space in the Memory Map, we stop
   if(input.length()>=(MemMap[startidx+1]-MemMap[startidx])) {  //If the string length exceeds the maximum reserved memory, we stop!
    WriteSerial.Write(String("EEP: Input string length of '") + input + String("' is ") + String(input.length()+1) + String(" (incl. null terminator!) and exceeds maximum reserved memory in the Memory Map of ") + String(MemMap[startidx+1]-MemMap[startidx])+String("\n"));  
    return(false);       
   }
   
  //If everything is fine so far, we save the string to the EEP
  char charinput[maxlength+1];                     //We create a buffer char array (and add one space for the null terminator)
  input.toCharArray(charinput, maxlength+1);       //Convert the string to a char array
  for (int i = 0; i <= strlen(charinput) ;i++){    //And write 
    EEPROM.write(MemMap[startidx] + i, charinput[i]);    
  } 
  return(true);
}

String EEP::ReadString(byte startidx){
  char charoutput[maxlength+1];                //We create a buffer char array (and add one space for the null terminator)
  for (int i = 0; i < (MemMap[startidx+1]-MemMap[startidx]); i++){
    if(EEPROM.read(MemMap[startidx] + i) == NULL) {
      charoutput[i] = char(EEPROM.read(MemMap[startidx] + i));
      break;
    }
    else {
      charoutput[i] = char(EEPROM.read(MemMap[startidx] + i));
    }
  }
  return(String(charoutput));
}

void EEP::WriteEEPSerial(void){
	WriteSerial.Write(String("***EEP Content***\n"));
	WriteSerial.Write(String("Wifi Parameters:\n"));
	WriteSerial.Write(String("  Homenetwork:\n"));
	WriteSerial.Write(String("    SSID: ") + ReadString(0)+ String("\n"));
	WriteSerial.Write(String("    Password: ") + ReadString(1)+ String("\n")); 
	WriteSerial.Write(String("    IP: ") + ReadString(23)+ String("\n"));
	WriteSerial.Write(String("    Gateway IP: ") + ReadString(24)+ String("\n"));
	WriteSerial.Write(String("  Hotspot:\n"));
	WriteSerial.Write(String("    SSID: ") + ReadString(2)+ String("\n")); 
	WriteSerial.Write(String("    Password: ") + ReadString(3)+ String("\n")); 
	WriteSerial.Write(String("    IP: ") + ReadString(25)+ String("\n"));
	WriteSerial.Write(String("  Connection Mode: ") + ReadValue8(4)+ String("\n\n"));
	
	WriteSerial.Write(String("Status LED deactivation:\n"));
	WriteSerial.Write(String("  Power LED: ") + ReadValue8(5)+ String("\n"));
	WriteSerial.Write(String("  Warning LED: ") + ReadValue8(6)+ String("\n\n"));
	
	WriteSerial.Write(String("Radar sensor connected: ") + ReadValue8(7)+ String("\n\n"));
	
	WriteSerial.Write(String("Soft Mode: ") + ReadValue8(8)+ String("\n"));
	WriteSerial.Write(String("  Soft time: ") + ReadValue16(19)+ String("[s]\n\n"));
	WriteSerial.Write(String("Motion Mode: ") + ReadValue8(9)+ String("\n"));
	WriteSerial.Write(String("  Off time: ") + ReadValue16(10)+ String("[s]\n"));
	WriteSerial.Write(String("  On Time: ") + ReadValue16(11)+ String("[s]\n"));
	WriteSerial.Write(String("  Sleep Time: ") + ReadValue16(12)+ String("[s]\n\n"));
	
	WriteSerial.Write(String("Lamp start defaults:\n"));
	WriteSerial.Write(String("  LED Mode: ") + ReadValue8(13)+ String("\n"));
	WriteSerial.Write(String("  LED Effect: ") + ReadValue8(14)+ String("\n"));
	WriteSerial.Write(String("  Effect Power: ") + ReadValue8(15)+ String("\n"));
  WriteSerial.Write(String("  Color: R ") + ReadValue8(16)+ String("G ")+ ReadValue8(17)+ String("B ")+ ReadValue8(18) +String("\n\n"));
	
	WriteSerial.Write(String("Battery parameters:\n"));
  WriteSerial.Write(String("  Full voltage: ") + ReadValue16(20)+ String("\n"));
	WriteSerial.Write(String("  Empty voltage: ") + ReadValue16(21)+ String("\n"));
	WriteSerial.Write(String("  Capacity: ") + ReadValue16(22)+ String("\n"));	
	WriteSerial.Write(String("***EEP Content end!***\n\n"));
}
