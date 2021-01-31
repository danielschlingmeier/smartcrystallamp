//*****************************************************************************************
//**                              Crystal lamp project                                   **
//*****************************************************************************************
//This file belongs to the Crystal lamp project and is intended to work together with the other project files, not standalone!
//
//This file contains the JS functions for the communication between the Options tab of the Control Webpage and the lamp

//Function call to request the settings from the lamp when switching to the Options tab
function getOptions(){
  //We generate our request for the options
  xhttp = new XMLHttpRequest();                                                //Create the xhttp object
  xhttp.open("POST", "getoptions", true);                                      //Configure the message type "POST" and "getoptions"
  xhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded'); //Configure the content type
  xhttp.timeout = timeouttime;                                                 //Set the standard timeout 
  xhttp.send();	                                                               //And finally send everything to the lamp     
  
  //Timeout handler  
  xhttp.ontimeout = function () {                           
    if(consoleoutput) console.error("Connection to lamp lost, no answer within " + xhttp.timeout + " ms");
		setConnectionStatus(false);
  };
  
  //Ready state change handler
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
		setConnectionStatus(true);
	  var answer = JSON.parse(this.responseText);          //Get the response text and run it through the JSON parser creating an object to access our answer
		
	  //We update the option page
	  if(answer.powerleddeact == 0) document.getElementById("PowerLeddeact").checked = false;      //Power LED disable checkbox
	  else document.getElementById("PowerLeddeact").checked = true;	  
	  
	  if(answer.warningleddeact == 0) document.getElementById("WarningLeddeact").checked = false;  //Warning LED disable checkbox
      else document.getElementById("WarningLeddeact").checked = true;
	  
      if(answer.radarsensconnected == 0) {
		  document.getElementById("radarSensConnected").checked = false;  //Radar sensor connected checkbox
		  document.getElementById("mmNone").disabled = true;      //Motion mode radio button
      document.getElementById("mmOff").disabled = true;
      document.getElementById("mmOn").disabled = true; 
	    document.getElementById("mmSleep").disabled = true; 
		  document.getElementById("OffTime").disabled = true;                  //Off time [s]
	    document.getElementById("OnTime").disabled = true;              //On time [s]
	    document.getElementById("SleepTime").disabled = true;            //Sleep time [s]	  
	  }
      else {
		  document.getElementById("radarSensConnected").checked = true;
		  document.getElementById("mmNone").disabled = false;      //Motion mode radio button
      document.getElementById("mmOff").disabled = false;  
      document.getElementById("mmOn").disabled = false;  
	    document.getElementById("mmSleep").disabled = false;  
		  document.getElementById("OffTime").disabled = false;               //Off time [s]
	    document.getElementById("OnTime").disabled = false;         //On time [s]
	    document.getElementById("SleepTime").disabled = false;   //Sleep time [s]	  
	  }
	  
	  if(answer.batterysensconnected == 0) document.getElementById("batterySensConnected").checked = false;  //Battery sensor connected checkbox
	  else document.getElementById("batterySensConnected").checked = true;
	  
	  if(answer.radradarsensconnected > 0){
		if(answer.motionmode == 0) document.getElementById("mmNone").checked = true;      //Motion mode radio button
		else if(answer.motionmode == 1) document.getElementById("mmOff").checked = true;
		else if(answer.motionmode == 2) document.getElementById("mmOn").checked = true;	  
		else if(answer.motionmode == 3) document.getElementById("mmSleep").checked = true;
	  } else document.getElementById("mmNone").checked = true; 
	  
	  document.getElementById("OffTime").value = answer.offtime;                       //Off time [s]
	  document.getElementById("OnTime").value = answer.ontime;                         //On time [s]
	  document.getElementById("SleepTime").value = answer.sleeptime;                   //Sleep time [s]
	  
	  if(answer.softmode == 0) document.getElementById("SoftMode").checked = false;            //Soft start/stop checkbox
	  else document.getElementById("SoftMode").checked = true;	 	  
	  
	  document.getElementById("SoftTime").value = answer.softtime;                     //Soft time [s]
		
		document.getElementById("FullVoltage").value = answer.fullvoltage;               //Battery properties
		document.getElementById("EmptyVoltage").value = answer.emptyvoltage; 
		document.getElementById("Capacity").value = answer.capacity; 
  	
	  if(answer.batterysensconnected > 0){
		document.getElementById("OptionsVoltage").innerHTML = answer.voltage + " mV";     //Voltage display
		document.getElementById("OptionsCurrent").innerHTML = answer.current + " mA";     //Current display
		document.getElementById("OptionsSOCmAh").innerHTML = answer.SOCmAh + " mAh" 
		document.getElementById("OptionsSOCpercent").innerHTML = "/ "+ answer.SOCpercent  + " %";  
	  } else {
	    document.getElementById("OptionsVoltage").innerHTML = "n/a";     //Voltage display
		document.getElementById("OptionsCurrent").innerHTML = "n/a";     //Current display
		document.getElementById("OptionsSOCmAh").innerHTML = "n/a";
		document.getElementById("OptionsSOCpercent").innerHTML = "";	  
	  }
    }
  };	
}

//Function to transmit the power led deactivation request
function setPowerLeddeact(){
  var deactivation;
  if(document.getElementById("PowerLeddeact").checked) deactivation = 1;
  else deactivation = 0;
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
	  if(consoleoutput) console.log("Power led deactivation request ("+ deactivation + ") confirmed by lamp");
	}
  };
  xhttp.open("POST", "setoptions", true);
  xhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
  xhttp.send("powerleddeact="+ deactivation); 
  if(consoleoutput) console.log("Power led deactivation request (" + deactivation + ") send to lamp");
}

//Function to transmit the warning led deactivation request
function setWarningLeddeact(){
  var deactivation;
  if(document.getElementById("WarningLeddeact").checked) deactivation = 1;
  else deactivation = 0;
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
	  if(consoleoutput) console.log("Warning led deactivation request ("+ deactivation + ") confirmed by lamp");
	}
  };
  xhttp.open("POST", "setoptions", true);
  xhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
  xhttp.send("warningleddeact="+ deactivation); 
  if(consoleoutput) console.log("Warning led deactivation request (" + deactivation + ") send to lamp");
}

//Function to transmit the motion mode request
function setMotionMode(){
  var motionmode;
  if(document.getElementById("mmNone").checked) motionmode = 0;
  else if(document.getElementById("mmOff").checked) motionmode = 1;
  else if(document.getElementById("mmOn").checked) motionmode = 2;  
  else if(document.getElementById("mmSleep").checked) motionmode = 3;  
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
	  if(consoleoutput) console.log("Motion mode request (" + motionmode + ") confirmed by lamp");
	}
  };
  xhttp.open("POST", "setoptions", true);
  xhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
  xhttp.send("motionmode="+ motionmode); 
  if(consoleoutput) console.log("Motion mode request (" + motionmode + ") send to lamp");
}

//Function to transmit the off time request
function setOffTime(){
  var offtime;
  offtime = document.getElementById("OffTime").value;
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
	  if(consoleoutput) console.log("Off time request (" + offtime + ") confirmed by lamp");
	}
  };
  xhttp.open("POST", "setoptions", true);
  xhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
  xhttp.send("offtime="+ offtime); 
  if(consoleoutput) console.log("Off time request (" + offtime + ") send to lamp");
}

//Function to transmit the on time request
function setOnTime(){
  var ontime;
  ontime = document.getElementById("OnTime").value;
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
	  if(consoleoutput) console.log("On time request (" + ontime + ") confirmed by lamp");
	}
  };
  xhttp.open("POST", "setoptions", true);
  xhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
  xhttp.send("ontime="+ ontime); 
  if(consoleoutput) console.log("On time request (" + ontime + ") send to lamp");
}

//Function to transmit the sleep time request
function setSleepTime(){
  var sleeptime;
  sleeptime = document.getElementById("SleepTime").value;
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
	  if(consoleoutput) console.log("Sleep time request (" + sleeptime + ") confirmed by lamp");
	}
  };
  xhttp.open("POST", "setoptions", true);
  xhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
  xhttp.send("sleeptime="+ sleeptime); 
  if(consoleoutput) console.log("Sleep time request (" + sleeptime + ") send to lamp");
}

//Function to transmit the soft mode request
function setSoftMode(){
  var softmode;
  if(document.getElementById("SoftMode").checked) softmode = 1;
  else softmode = 0;
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
	  if(consoleoutput) console.log("Soft mode request (" + softmode + ") confirmed by lamp");
	}
  };
  xhttp.open("POST", "setoptions", true);
  xhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
  xhttp.send("softmode="+ softmode); 
  if(consoleoutput) console.log("Soft mode request (" + softmode + ") send to lamp");
}


//Function to transmit the soft time request
function setSoftTime(){
  var softtime;
  softtime = document.getElementById("SoftTime").value;
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
	  if(consoleoutput) console.log("Soft time request (" + softtime + ") confirmed by lamp");
	}
  };
  xhttp.open("POST", "setoptions", true);
  xhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
  xhttp.send("softtime="+ softtime); 
  if(consoleoutput) console.log("Soft time request (" + softtime + ") send to lamp");
}

//Function to transmit the full battery voltage
function setFullVoltage(){
	var fullvoltage = document.getElementById("FullVoltage").value;
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
	  if(consoleoutput) console.log("Full battery voltage request (" + fullvoltage + ") confirmed by lamp");
	}
  };
  xhttp.open("POST", "setoptions", true);
  xhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
  xhttp.send("fullvoltage="+ fullvoltage); 
  if(consoleoutput) console.log("Full battery voltage request (" + fullvoltage + ") send to lamp");
}

//Function to transmit the end or empty battery voltage
function setEmptyVoltage(){
	var emptyvoltage = document.getElementById("EmptyVoltage").value;
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
	  if(consoleoutput) console.log("Empty battery voltage request (" + emptyvoltage + ") confirmed by lamp");
	}
  };
  xhttp.open("POST", "setoptions", true);
  xhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
  xhttp.send("emptyvoltage="+ emptyvoltage); 
  if(consoleoutput) console.log("Empty battery voltage request (" + emptyvoltage + ") send to lamp");
}

//Function to transmit the battery capacity voltage
function setCapacity(){
	var capacity = document.getElementById("Capacity").value;
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
	  if(consoleoutput) console.log("Battery capacity request (" + capacity + ") confirmed by lamp");
	}
  };
  xhttp.open("POST", "setoptions", true);
  xhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
  xhttp.send("capacity="+ capacity); 
  if(consoleoutput) console.log("Battery capacity request (" + capacity + ") send to lamp");
}

//Function to transmit the save request, which will command the ESP8266 to store the settings in the EEP
function setSaveOptions(){
  var xhttp;
  xhttp=new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      if(consoleoutput) console.log("Options save request confirmed by lamp");
	}
  };
  xhttp.open("POST", "setoptions", true);
  xhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
  xhttp.send("save=1"); 
  if(consoleoutput) console.log("Options save request send to lamp");
}

function submitReset(){
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
    if(consoleoutput) console.log("Reset request confirmed by lamp"); 
		location.reload();
	}
  };
  xhttp.open("POST", "setoptions", true);
  xhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
  xhttp.send("reset=1"); 
  if(consoleoutput) console.log("Reset request send to lamp");	
	locksubmits();
}

function submitRestart(){
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
    if(consoleoutput) console.log("Restart request confirmed by lamp");
		location.reload();
	}
  };
  xhttp.open("POST", "setoptions", true);
  xhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
  xhttp.send("restart=1"); 
  if(consoleoutput) console.log("Restart request send to lamp");	
	
}