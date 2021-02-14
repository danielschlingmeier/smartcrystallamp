//*****************************************************************************************
//**                              Crystal lamp project                                   **
//*****************************************************************************************
//This file belongs to the Crystal lamp project and is intended to work together with the other project files, not standalone!
//This file contains the JS functions for the communication between the Control tab of the Control Webpage and the lamp


var timer = setInterval(getControl, updatetime);   //We create a class and an event handler for the cyclic update
colorPicker.on('input:end', setColor);             //Event handler

function getControl() {
  xhttp = new XMLHttpRequest();
  xhttp.open("POST", "getcontrol", true);
  xhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
  xhttp.timeout = timeouttime;
  xhttp.send();	
  
  //Timeout handler
  xhttp.ontimeout = function () {
    if(consoleoutput) console.error("Connection to lamp lost, no answer within " + xhttp.timeout + " ms");
    setConnectionStatus(false);
		
  };
  
  //Ready state change handler
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {                                    //We clear the info message
		setConnectionStatus(true);
	  var answer = JSON.parse(this.responseText);          //Get the response text and run it through the JSON parser creating an object to access our answer

	  
		//We update the control page
		if(answer.mode=="0"){  	                                            //Effect Mode buttons update
		  document.getElementById("offButton").className = "active";
		  document.getElementById("colorButton").className = "";
		  document.getElementById("effectButton").className = "";
			document.getElementById("motionButton").className = "";
	  } else if (answer.mode=="1"){		  
		  document.getElementById("offButton").className = "";
		  document.getElementById("colorButton").className = "active";
		  document.getElementById("effectButton").className = "";		 
			document.getElementById("motionButton").className = "";			
	  } else if (answer.mode=="2"){
		  document.getElementById("offButton").className = "";
		  document.getElementById("colorButton").className = "";
		  document.getElementById("effectButton").className = "active";	 			
			document.getElementById("motionButton").className = ""; 
	  }	else if (answer.mode=="3"){
			document.getElementById("offButton").className = "";
		  document.getElementById("colorButton").className = "";
		  document.getElementById("effectButton").className = "";	 			
			document.getElementById("motionButton").className = "active"; 		
		}			
		
		colorPicker.color.rgb = { r: answer.r, g: answer.g, b: answer.b };         //Colorpicker
		
		if(answer.effect==255) {document.getElementById("effectdropdownbutton").innerHTML = "None";}  //Effect drop down menu
	  else {document.getElementById("effectdropdownbutton").innerHTML = effects[answer.effect];}
		
	  if(answer.batterysensconnected > 0){
		document.getElementById("OptionsVoltage").innerHTML = answer.voltage + " mV";     //Voltage display
		document.getElementById("OptionsCurrent").innerHTML = answer.current + " mA";     //Current display
		document.getElementById("OptionsSOCmAh").innerHTML = answer.SOCmAh + " mAh"       //SOC displays
		document.getElementById("OptionsSOCpercent").innerHTML = "/ "+ answer.SOCpercent  + " %";  
		document.getElementById("ControlSOC").innerHTML = answer.SOCpercent + "%";
    if(answer.SOCpercent >= 20) document.getElementById("ControlSOC").style.backgroundColor = "#248f24";
		else if ((answer.SOCpercent < 20)&&(answer.SOCpercent >= 5)) document.getElementById("ControlSOC").style.backgroundColor = "#FFC300";
		else if (answer.SOCpercent < 5) document.getElementById("ControlSOC").style.backgroundColor = "#b30000";
		
	  } else {
	  document.getElementById("OptionsVoltage").innerHTML = "n/a";     //Voltage display
		document.getElementById("OptionsCurrent").innerHTML = "n/a";       //Current display
		document.getElementById("OptionsSOCmAh").innerHTML = "n/a";        //SOC displays
		document.getElementById("OptionsSOCpercent").innerHTML = "";
		document.getElementById("ControlSOC").innerHTML = "n/a"; 
		document.getElementById("ControlSOC").style.backgroundColor = "#2F4F4F";
		}
	  
		if(answer.radarsensconnected > 0){
			document.getElementById("motionbutton").style.visibility = "visible";
			if(answer.motion > 0){
				document.getElementById("motionbutton").style.backgroundColor = "#FFC300";	         //Motion button update
				document.getElementById("motionbutton").style.color = "black";
			} else {
				document.getElementById("motionbutton").style.backgroundColor = "#2F4F4F";
				document.getElementById("motionbutton").style.color = "#F0F8FF";
			} 
		} else {
			document.getElementById("motionbutton").style.visibility = "hidden";
		}
		}
	};
}

//Function to transmit the LED mode request
function setMode(mode) {
  var xhttp;
  xhttp=new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      if(mode=="0"){
		  document.getElementById("offButton").className = "active";
		  document.getElementById("colorButton").className = "";
		  document.getElementById("effectButton").className = "";
		  document.getElementById("motionButton").className = "";			
			} else if (mode=="1"){		  
		  document.getElementById("offButton").className = "";
		  document.getElementById("colorButton").className = "active";
		  document.getElementById("effectButton").className = "";		
		  document.getElementById("motionButton").className = "";		  	  
			} else if (mode=="2"){
		  document.getElementById("offButton").className = "";
		  document.getElementById("colorButton").className = "";
		  document.getElementById("effectButton").className = "active";	  
			document.getElementById("motionButton").className = "";		
			} else if (mode=="3"){
			document.getElementById("offButton").className = "";
		  document.getElementById("colorButton").className = "";
		  document.getElementById("effectButton").className = "";	  
			document.getElementById("motionButton").className = "active";	
		  }
    }
  };
  xhttp.open("POST", "setcontrol", true);
  xhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
  xhttp.send("mode="+ mode);
}

function setColor(color, changes) {
  var rgbcolor = colorPicker.color.rgb;
	var value = colorPicker.color.value;
  xhttp=new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
    }
  };
  xhttp.open("POST", "setcontrol", true);
  xhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
  xhttp.send("r=" +rgbcolor.r +"&g=" +rgbcolor.g +"&b="+rgbcolor.b+"&power="+value);	
}

//Function to transmit the effect request
function setEffect(effect){
  var xhttp;
  xhttp=new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
	  document.getElementById("effectdropdownbutton").innerHTML = effects[effect];
	  document.getElementsByClassName("effectdropdowncontent")[0].classList.remove("show");
	  if(consoleoutput) console.log("Set effect request confirmed by lamp");
	}
  };
  xhttp.open("POST", "setcontrol", true);
  xhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
  xhttp.send("effect="+ effect); 
  if(consoleoutput) console.log("Set effect request send to lamp");
}

//Function to transmit the save request, which will command the ESP8266 to store the settings in the EEP
function SaveDefault(){
  var xhttp;
  xhttp=new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      if(consoleoutput) console.log("Default control save request confirmed by lamp");
	}
  };
  xhttp.open("POST", "setcontrol", true);
  xhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
  xhttp.send("save=1"); 
  if(consoleoutput) console.log("Default control save request send to lamp");
}
