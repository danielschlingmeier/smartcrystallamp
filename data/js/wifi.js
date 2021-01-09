//*****************************************************************************************
//**                              Crystal lamp project                                   **
//*****************************************************************************************
//This file belongs to the Crystal lamp project and is intended to work together with the other project files, not standalone!
//This file contains the JS functions for the communication between the Wi-Fi tab of the Control Webpage and the lamp

function getWifi(){
  //We generate our request to send over the options
  xhttp = new XMLHttpRequest();                                                //Create the xhttp object
  xhttp.open("POST", "getwifi", true);                                         //Configure the message type "POST" and "getoptions"
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
	  setConnectionStatus(false);                                  
	  var answer = JSON.parse(this.responseText);          //Get the response text and run it through the JSON parser creating an object to access our answer
	  
	  //We update the wifi  page
	  document.getElementById("home_ssid").value = answer.homessid;
	  document.getElementById("home_password").value = "";
		document.getElementById("home_ip").value = answer.homeip;  
		document.getElementById("router_ip").value = answer.routerip;  
	  
	  document.getElementById("hotspot_ssid").value = answer.hotspotssid;  
	  document.getElementById("hotspot_password").value = "";	
		document.getElementById("hotspot_ip").value = answer.hotspotip;  
	  
	  if(answer.connectionmode == 0) document.getElementById("homeconnect").checked = true;      //Connection mode radio button
	  else if(answer.connectionmode == 1) document.getElementById("hotspotconnect").checked = true;
    }
  };	
}

function submithomewifi(){
	//We check our IPs for a valid entry, otherwise we ignore the input
	var homeip, routerip;
	var pattern = new RegExp("^(?:[0-9]{1,3}\.){3}[0-9]{1,3}$");
	if(pattern.test(document.getElementById("home_ip").value)) homeip=document.getElementById("home_ip").value;
	else {
		ip = "";
	  document.getElementById("home_ip").value = "";  //Clear the field to show the user, that we don't intend to accept the input
	}
  if(pattern.test(document.getElementById("router_ip").value)) routerip=document.getElementById("router_ip").value;
	else {
		ip = "";
	  document.getElementById("router_ip").value = "";  //Clear the field to show the user, that we don't intend to accept the input
	}
	
  var xhttp = new XMLHttpRequest();	
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      if(consoleoutput) console.log("New Home Wifi settings confirmed by lamp");
			document.getElementById("home_password").value = "";
      locksubmits();
			getWifi();
	}
  };
  xhttp.open("POST", "setwifi", true);
  xhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
  xhttp.send("homessid=" + document.getElementById("home_ssid").value + "&homepassword=" + document.getElementById("home_password").value + "&homeip=" + homeip+ "&routerip=" + routerip); 
  if(consoleoutput) console.log("New Home Wifi settings send to lamp");	
	
}

function submithotspotwifi(){
  //We check our IP for a valid entry, otherwise we ignore the input
	var ip;
	var pattern = new RegExp("^(?:[0-9]{1,3}\.){3}[0-9]{1,3}$");
	if(pattern.test(document.getElementById("hotspot_ip").value)) ip=document.getElementById("hotspot_ip").value;
	else {
		ip = "";
		document.getElementById("hotspot_ip").value = "";  //Clear the field to show the user, that we don't intend to accept the input
	}
	
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      if(consoleoutput) console.log("New Hotspot wifi settings confirmed by lamp");
			document.getElementById("hotspot_password").value = "";
			locksubmits();
			getWifi();
	}
  };
  xhttp.open("POST", "setwifi", true);
  xhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
  xhttp.send("hotspotssid=" + document.getElementById("hotspot_ssid").value + "&hotspotpassword=" + document.getElementById("hotspot_password").value + "&hotspotip=" + ip); 
  if(consoleoutput) console.log("New Hotspotocal wifi settings send to lamp");	
}

//Function to transmit the connection mode request
function setConnectionMode(){
  var connectionmode;
  if(document.getElementById("homeconnect").checked) motionmode = 0;
  else if(document.getElementById("hotspotconnect").checked) motionmode = 1;
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
	  if(consoleoutput) console.log("Connection mode request (" + connectionmode + ") confirmed by lamp");
	}
  };
  xhttp.open("POST", "setwifi", true);
  xhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
  xhttp.send("connectionmode="+ motionmode); 
  if(consoleoutput) console.log("Connection mode request (" + connectionmode + ") send to lamp");
}

//Function to transmit the save request, which will command the ESP8266 to store the settings in the EEP
function setSaveWifi(){
  var xhttp;
  xhttp=new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      if(consoleoutput) console.log("Wifi save request confirmed by lamp");
	}
  };
  xhttp.open("POST", "setwifi", true);
  xhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
  xhttp.send("save=1"); 
  if(consoleoutput) console.log("Wifi save request send to lamp");
}

function togglesubmitlock(selector){
  if(selector==1) document.getElementById("homesubmit").disabled = !document.getElementById("homeunlock").checked;
  if(selector==2) document.getElementById("hotspotsubmit").disabled = !document.getElementById("hotspotunlock").checked; 
  if(selector==3) document.getElementById("resetsubmit").disabled = !document.getElementById("resetunlock").checked; 
}

function locksubmits(){
  document.getElementById("homeunlock").checked = false;	
  document.getElementById("homesubmit").disabled = true;
  document.getElementById("hotspotunlock").checked = false;
  document.getElementById("hotspotsubmit").disabled = true;	
  document.getElementById("resetunlock").checked = false;
  document.getElementById("resetsubmit").disabled = true;
}

