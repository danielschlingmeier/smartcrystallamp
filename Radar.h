//*****************************************************************************************
//**                              Smart crystal lamp project                             **
//*****************************************************************************************
//This file belongs to the Smart crystal lamp project and is intended to work together with the other project files, not standalone!
//This file contains the functions to operate the RCWL 0516 radar motion sensor

#define RADAR_PIN D6               //Define Radar pin

class Radar {
  private: 
	//Please note, that some default values are overwritten with the EEP stored values in the Start() function!
    boolean Radar_connected = true;     //Defines if a radar sensor is installed or not (optional)
    uint32_t Last_Call_Time = 0;        //The last time when the Radar Run function was called, used to control the calls per second properly
		uint8_t CallsPerSecond = 5;         //Radar update calls per second, used to control the computation load
    uint32_t Last_Sensor_On_Time = 0;   //Time stamp of last rising edge
		uint32_t Last_Sensor_Off_Time = 0;  //Time stamp of last falling edge
    uint8_t Motion_Mode = 0;            //Motion Mode, 0: None, 1: Off for a time after motion, 2: On for a time after motion, 3: Sleep after time without motion
    uint16_t Off_Time = 10;             //Corresponding times in [s], overwritten by EEP
    uint16_t On_Time = 10;
    uint16_t Sleep_Time = 300;
    boolean ActualMotion = 0;           //Indicates if there is currently motion
		boolean PreviousMotion = 0;         //Indicates if there has been motion in the previous call, necessary for rising and falling edge detection

  public:
    Radar(void);                  //Constructor
		void Start(void);             //Start function needs to be called once in setup()
    void Run(void);               //The run function to be called in the loop()
    boolean isMotion(void);       //Check if there is motion
    uint16_t getOffTime(void);    //Function to set and get the important parameters
    void setOffTime(uint16_t);
    uint16_t getOnTime(void);
    void setOnTime(uint16_t);
    uint16_t getSleepTime(void);
    void setSleepTime(uint16_t);
    uint8_t getMotionMode(void);
    void setMotionMode(uint8_t);
    boolean getRadarConnected(void);
		void saveOptions(void);       //Function to save the options to EEP when the options tab is left
	
} Radar;

Radar::Radar(void){
	pinMode(RADAR_PIN, INPUT);      //Set radar pipn to input
}

void Radar::Start(void){
	//Read the stored start-up default values from the EEP
	Radar_connected = EEP.ReadValue8(7);
	Motion_Mode = EEP.ReadValue8(9);
	Off_Time = EEP.ReadValue16(10);
	On_Time = EEP.ReadValue16(11);
	Sleep_Time = EEP.ReadValue16(12);
	WriteSerial.Write(String("Radar started.\n\n"));  
}


void Radar::Run(void){
  if(Radar_connected&&(millis()>Last_Call_Time+(uint32_t)1000/(uint32_t)CallsPerSecond)){  //If the filter call period passed the function is executed, otherwise not     
    //Check if the radar pin is high
    if(digitalRead(RADAR_PIN)) ActualMotion = true;
    else ActualMotion = false;
		
		//If there is motion now and wasn't before, we detect a rising edge and other way around and set the time stamp
		if(ActualMotion&&!PreviousMotion){  
			Last_Sensor_On_Time = millis();
			PreviousMotion = true;
		} else if (!ActualMotion&&PreviousMotion){
			Last_Sensor_Off_Time = millis();
			PreviousMotion = false;
		}
		
		//When Motion mode is switched on, we give feedback to the LED function via setActive() of the lamp should be on or off at the moment
		if(LED.getLedMode()==3){
			switch(Motion_Mode){
				case 0: //Motion Mode 0 (None) means we do nothing
					LED.setActive(0);
					break;
				case 1: //Motion Mode 1 (Off after motion) means we switch off for a certain time after last sensor on
					if(ActualMotion||((millis()-Last_Sensor_Off_Time)<=Off_Time*1000)) LED.setActive(0);
					else LED.setActive(1);
					break;
				case 2: //Motion Mode 2 (On after motion) means we switch on for a certain time after last sensor on
					if(ActualMotion||((millis()-Last_Sensor_Off_Time)<=On_Time*1000)) LED.setActive(1);
					else LED.setActive(0);
					break;		
				case 3:  //Motion Mod 3 (Sleep after no motion) means we switch the lamp off after a certain time after last motion
					if(ActualMotion||((millis()-Last_Sensor_Off_Time)<=Sleep_Time*1000)){
						LED.setActive(1);
					} else {
						LED.endMotionMode();
						LED.setLedMode(0);   //..and switch the lamp off afterwards
					}
					break;
			}
		} else LED.setActive(0);

		
    Last_Call_Time = millis();  //Save the new call time
  }
}


boolean Radar::isMotion(void){
  if(digitalRead(RADAR_PIN)) ActualMotion = true;
  else ActualMotion = false;
  return(ActualMotion);
}

uint16_t Radar::getOffTime(void){
  return(Off_Time);
}

void Radar::setOffTime(uint16_t input){
  Off_Time = input;
}

uint16_t Radar::getOnTime(void){
  return(On_Time);
}
 
void Radar::setOnTime(uint16_t input){
  On_Time = input;
}

uint16_t Radar::getSleepTime(void){
  return(Sleep_Time);
}

void Radar::setSleepTime(uint16_t input){
  Sleep_Time = input;  
}

uint8_t Radar::getMotionMode(void){
  return(Motion_Mode);  
}

void Radar::setMotionMode(uint8_t input){
  Motion_Mode = input;
}

boolean Radar::getRadarConnected(void){
  return(Radar_connected);
}

void Radar::saveOptions(void){
	EEP.WriteValue8(7, Radar_connected);
	EEP.WriteValue8(9, Motion_Mode);
	EEP.WriteValue16(10, Off_Time);
	EEP.WriteValue16(11, On_Time);
	EEP.WriteValue16(12, Sleep_Time);
	EEP.Commit();
  WriteSerial.Write(String("Radar default values saved to EEP\n\n"));   	
}	
