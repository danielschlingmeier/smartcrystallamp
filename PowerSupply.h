//*****************************************************************************************
//**                              Crystal lamp project                                   **
//*****************************************************************************************
//This file belongs to the crystal project and is intended to work together with the other project files, not standalone.
//
//This file contains the class for the battery management, which means the Li-Ion Battery voltage & current monitor, 
//state of charge (SOC) calculation and also the logic for the power and error LEDs

#define WARNING_PIN D3                  //Define the pin to indicate low battery or other problems
#define POWER_PIN D0                  //Define the pin to power on
#define NUM_BATTERY 3                 //Define the number of 18650 batteries
#define POWER_CALLS_PER_SECOND 2
#include <Adafruit_INA260.h>          //We include the adafruit ina 260 libraray necessary to operate the battery monitor IC via I2C Bus

class PowerSupply {
  private:
    //General control variables
    uint32_t Last_Call_Time = 0;            //The last time when the PowerSupply Run function was called, used to control the calls per second properly         
		uint8_t CallsPerSecond = 2;             //Calls per second for execution regardless how often the Run function is called   
		boolean INA260_Connected = false;       //Indicates if a INA260 battery monitor is installed or not
    boolean Deactivate_Power_LED = false;   //Indicates if the power LED has been deactivated, can be annoying, overwritten by constructor!
    boolean Deactivate_Warning_LED = false; //Indicates if the warning LED has been deactivated, overwritten by constructor!
   
	  //Battery properties (standard values here overwritten by EEP reading in start())
		uint16_t FullVoltage = 4200;      //Voltage for 100% SOC
		uint16_t EmptyVoltage = 3000;		  //Voltage for 0% SOC
		uint16_t Capacity = 2770;         //Capacity between full and empty voltage
		uint16_t Resistance = 100;        //Resistance
		//Note: The real inner resistance of the cells is around 20mOhm, however this value reflects the overall resistance of the PCB based on measurements 
		
		/*These are the coefficients of the 4th grade polynom C_nom = c4*U_nom^4+c3+U_nom^3+c2*U_nom^2+(1-c2-c2-c4)*U_nom to approximate the 
		normalized SOC curve of a Sony US18650VTC6 cell as reference, see battery curve Excel sheet for the parameters*/
		float c4 = -2.6538;   
		float c3 = +3.5621;
		float c2 = -0.3663;
		//c1 = (1-c2-c3-c4) in order to fit C_nom = 1 for U_nom = 1
		
		//Lamp shutdown thresholds for current and voltage and corresponding warning flags
		uint16_t minmaxThreshold = 100;         //Min/Maximum in relation to Full & Empty voltage in mV, to shutdown the lamp
    uint16_t MaxCurrent = 1800;             //Maximum allowed current in mA, reaching this current will stop the lamp (overcurrent / short circuit) 
    boolean WrnBatteryOverloaded = false;   //Indicates an overloaded battery error, leds shall stop operation		
		boolean WrnBatteryEmpty = false;        //Indicates an empty battery error, leds shall stop operation
    boolean WrnOvercurrent = false;         //Indicates an overcurrent error (not good at all!), leds shall stop operation
    
    //Actual values
    float ActualCurrent = 0;             //Actual values for current and voltage
    float ActualVoltage = 0; 
    float ActualSOCmAh = 0;              //Actual SOC in mAh
    float ActualSOCpercent = 0;          //Actual SOC in %
    
    //Private classes
    Adafruit_INA260 ina260;                 //We define our class to access the current and voltage data read via I2C from our INA260 battery monitor

    //Private functions  
    void UpdateValues(void);                  //Private function to update current and voltage values from the INA260
		float calculateSOC(float, float); 
        
  public:
    PowerSupply(void);                           //Constructor
    void Start(void);                            //Switch on poswer supply
    void Run(void);                              //Function to be called in the loop()
    boolean isWarning(void);                     //Function to check for the max values, is automatically called in Run, but can also be called separately
    uint16_t getVoltage(void);       //Functions to get the actual values
    uint16_t getCurrent(void);
    uint16_t getSOCmAh(void);  
    uint16_t getSOCpercent(void);  
    void DeactivatePowerLed(boolean);
    boolean getPowerLedDeactivated(void);
    void DeactivateWarningLed(boolean);
    boolean getWarningLedDeactivated(void);
    boolean getINAconnected(void);
		void setFullVoltage(uint16_t);
		uint16_t getFullVoltage(void);
		void setEmptyVoltage(uint16_t);
		uint16_t getEmptyVoltage(void);
		void setCapacity(uint16_t);
		uint16_t getCapacity(void);			
		void saveOptions(void); 
	
} PowerSupply;

PowerSupply::PowerSupply(void){
  pinMode(POWER_PIN, OUTPUT);       //Set the power pin as output
  pinMode(WARNING_PIN, OUTPUT);       //Set warning pin as output 
}

void PowerSupply::Start(void){
  //We start the INA260 battery monitor
  WriteSerial.Write(String("Connecting INA26 battery monitor via I2C..."));
  INA260_Connected = ina260.begin();
  if(INA260_Connected) WriteSerial.Write(String("connected!\n\n"));
  else WriteSerial.Write(String("not found! Voltage and current monitoring deactivated.\n"));
  
  //We calculate the initial SOC based in the open circuit voltage (OCV) when no current is drawn at startup
  if(INA260_Connected) {
    ina260.setAveragingCount(INA260_COUNT_512);   //We set the average filter to 521 values (*1,1ms per value = 563,2ms filter filter time)    
    delay(600);                                   //We have wait a moment until the signals have stabilized and then continue
    UpdateValues();                               //Get our first set of values from the sensor
    
  }
  
  //Read the stored start-up default values from the EEP
  Deactivate_Power_LED = EEP.ReadValue8(5);
  Deactivate_Warning_LED = EEP.ReadValue8(6);
	FullVoltage = EEP.ReadValue16(20);
	EmptyVoltage = EEP.ReadValue16(21);	
	Capacity = EEP.ReadValue16(22);
  
  if(!Deactivate_Power_LED) digitalWrite(POWER_PIN, HIGH);  //We switch the power led on, but only if not deactivated  
	
	WriteSerial.Write(String("Power supply started: "));  
	if(INA260_Connected) WriteSerial.Write(String("Battery Voltage: ")+String(ActualVoltage)+String(" mV (SOC: ")+String(ActualSOCmAh)+String(" mAh / ")+String(ActualSOCpercent)+String("%) \n\n"));
	else WriteSerial.Write(String("\n\n"));
}

void PowerSupply::Run(void){
  if(INA260_Connected&&(millis()>Last_Call_Time+(uint32_t)1000/(uint32_t)CallsPerSecond)){  //If the call period passed the function is executed, otherwise not (otherwise we overload the loop!)
    UpdateValues();           //Fetch new sensor values and calculate new SOC                           
    isWarning();              //Check for battery warnings
		Last_Call_Time=millis();  //Save the new call time
  }
}  


boolean PowerSupply::isWarning(void){  
  if(INA260_Connected&&ActualCurrent > MaxCurrent) {
    if(!WrnOvercurrent) WriteSerial.Write(String("Maximum allowed battery current (")+String(MaxCurrent)+String("mA) exeeced! LEDs switched off!\n\n")); //We complain, but only about first errors
    WrnOvercurrent = true;
  } else {
    WrnOvercurrent = false;  
  }

  if(INA260_Connected&&ActualVoltage > (FullVoltage + minmaxThreshold)) {
    if(!WrnBatteryOverloaded) WriteSerial.Write(String("Maximum allowed battery voltage (")+String(FullVoltage + minmaxThreshold)+String(" mV) exeeced, battery overloaded! LEDs switched off!\n\n")); //We complain, but only about first errors
    WrnBatteryOverloaded = true;
  } else {
    WrnBatteryOverloaded = false;  
  }

  if(INA260_Connected&&ActualVoltage < (EmptyVoltage - minmaxThreshold)) {
    if(!WrnBatteryEmpty) WriteSerial.Write(String("Minimum allowed battery voltage (")+String(EmptyVoltage - minmaxThreshold)+String(" mV) exeeced, battery empty! LEDs switched off!\n\n")); //We complain, but only about first errors
    WrnBatteryEmpty = true;
  } else {
    WrnBatteryEmpty = false;  
  }

  if(WrnOvercurrent||WrnBatteryOverloaded||WrnBatteryEmpty){
    if(!Deactivate_Warning_LED) digitalWrite(WARNING_PIN, HIGH);     
    return(true);
  } else {
    digitalWrite(WARNING_PIN, LOW);     
    return(false);
  }
}

uint16_t PowerSupply::getVoltage(void){
  return((uint16_t)ActualVoltage);  
}

uint16_t PowerSupply::getCurrent(void){
  return((uint16_t)ActualCurrent);   
}

uint16_t PowerSupply::getSOCmAh(void){
  return((uint16_t)ActualSOCmAh);   
}

uint16_t PowerSupply::getSOCpercent(void){
  return((uint16_t)ActualSOCpercent);
}

void PowerSupply::DeactivatePowerLed(boolean input){
  Deactivate_Power_LED = input;
  if(Deactivate_Power_LED) digitalWrite(POWER_PIN, LOW);
  else digitalWrite(POWER_PIN, HIGH);
}
    
boolean PowerSupply::getPowerLedDeactivated(void){
  return(Deactivate_Power_LED);      
}

void PowerSupply::DeactivateWarningLed(boolean input){
  Deactivate_Warning_LED = input; 
  isWarning();  //We have to call the isWarning function once here to affect the LED
}

boolean PowerSupply::getWarningLedDeactivated(void){
  return(Deactivate_Warning_LED);
}

boolean PowerSupply::getINAconnected(void){
  return(INA260_Connected);
}

void PowerSupply::setFullVoltage(uint16_t input){
	FullVoltage = input;	
}

uint16_t PowerSupply::getFullVoltage(void){
	return(FullVoltage);
}

void PowerSupply::setEmptyVoltage(uint16_t input){
	EmptyVoltage = input;
}

uint16_t PowerSupply::getEmptyVoltage(void){
	return(EmptyVoltage);
}
		
void PowerSupply::setCapacity(uint16_t input){
	Capacity = input;
}
		
uint16_t PowerSupply::getCapacity(void){
	return(Capacity);
}	

void PowerSupply::saveOptions(void){
	EEP.WriteValue8(5, Deactivate_Power_LED);
	EEP.WriteValue8(6, Deactivate_Warning_LED);
	EEP.WriteValue16(20, FullVoltage);
	EEP.WriteValue16(21, EmptyVoltage);	
	EEP.WriteValue16(22, Capacity);
	EEP.Commit();
  WriteSerial.Write(String("Power suppply default values saved to EEP\n\n"));   
}	

void PowerSupply::UpdateValues(void){
  //Current and voltage can become negative at very low values, which causes an overflow in the uint16_t coonversion, that we want to prevent
	if(INA260_Connected){
		ActualCurrent = max(ina260.readCurrent(), (float)0.0); 
		ActualVoltage = max(ina260.readBusVoltage()/(float)NUM_BATTERY, (float)0.0);  
    ActualSOCmAh = calculateSOC(ActualVoltage, ActualCurrent);
		ActualSOCpercent = (uint16_t)((float)ActualSOCmAh*100.0/(float)Capacity);
	} else {
		ActualCurrent = 0;
		ActualVoltage = 0;
		ActualSOCmAh = 0;
		ActualSOCpercent = 0;
	}
}

float PowerSupply::calculateSOC(float voltage, float current){
	/*We constrain the current to positive values and compensate the current depending voltage drop for 
	an approximative open cell voltage (OCV), for safety this is contrained between min and max voltage  */
	float OCV_voltage = constrain(voltage + (float)Resistance*max(current, (float)0.0)/(float)1000.0, (float)EmptyVoltage, (float)FullVoltage);
	float U_OCV_nom = (OCV_voltage - (float)EmptyVoltage)/(float)(FullVoltage-EmptyVoltage);  //We calculate the normalized cell voltage
	return(Capacity*(c4*pow(U_OCV_nom, 4)+c3*pow(U_OCV_nom, 3)+c2*pow(U_OCV_nom, 2)+(1-c2-c3-c4)*U_OCV_nom));   //And use our approximation function to calculate the capacity
	
}
