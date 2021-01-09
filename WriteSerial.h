//*****************************************************************************************
//**                              Crystal lamp project                                   **
//*****************************************************************************************
//This file belongs to the Crystal lamp project and is intended to work together with the other project files, not standalone!

class WriteSerial {
  private:
  public: 
    WriteSerial(void);
    void Start(void);
    void Write(String);
    void WritePW(String);
    boolean SerialActive = 1;
    boolean ShowPasswords = 0;   
    
} WriteSerial;

WriteSerial::WriteSerial(void){
}

void WriteSerial::Start(){
  if(SerialActive){
    Serial.begin(115200); 
    Write(String("\nSerial connection started\n\n"));  
  }
  else Write(String("\nSerial connection deactivated!\n\n"));
}

void WriteSerial::Write(String s){
  if(SerialActive) Serial.print(s);
}

void WriteSerial::WritePW(String s){
  if(SerialActive){
    if(ShowPasswords){
      Serial.print(s);  
    }
    else{
      Serial.print("*********");
    }
  }
}


 
