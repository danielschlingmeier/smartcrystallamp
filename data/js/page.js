//*****************************************************************************************
//**                              Crystal lamp project                                   **
//*****************************************************************************************
//This file belongs to the Crystal lamp project and is intended to work together with the other project files, not standalone!
//
//This is the main script, which controls the behaviour of the Control Webpage

//Come global variables
var effects = new Array("Fire", "Color Fire", "Pacifica", "Magic");  //We create an array to store the effect names (maybe for automatic listing later...)
var updatetime = 2000;                    //Update interval for control page update between Webpage and Lamp
var timeouttime = 1500;                   //Timeout interval for lamp response
var currenttab = "control";               //Stores the currently opened tab
var consoleoutput = true;                 //Console output enable

var colorPicker = new iro.ColorPicker('#picker',{  //We create a color picker object
	width: 320,
	borderWidth: 2,
	borderColor: "#F0F8FF",
	wheelLightness: false,
	layoutDirection: "horizontal"
});  


//Function to activate the selected tabs (control, options or Wi-Fi)
function openTab(tabNum) {
  var i, tabcontent, tablinks;  
  tabcontent = document.getElementsByClassName("tabcontent"); //Hide all tab contents
  for (i = 0; i < tabcontent.length; i++) {
    tabcontent[i].style.display = "none";
  }  
  tablinks = document.getElementsByClassName("tablinks");     //Deactivate all buttons
  for (i = 0; i < tablinks.length; i++) {
    tablinks[i].className = tablinks[i].className.replace(" active", "");
  }
  
  if(tabNum == 1){
	getControl();                                                         //Update control page right away (prevents delays with cyclic update)
  document.getElementById("Control").style.display = "block";           //Display the selected tab content  
	document.getElementById("controlButton").className += " active";      //Set the corresponding button to active
	if(currenttab == "options"){                                          //When coming from options or wifi tab, this is also the indication to save the settings
	  setSaveOptions();
	} else if(currenttab == "wifi"){
	  setSaveWifi(); 
	}
	currenttab = "control"; 	
	if(consoleoutput) console.log("Switched to " + currenttab + " tab");  //Send console message
  }
    
  if(tabNum == 2){
	getOptions();                                                         //We request the lamp to send the options and update the tab accordingly before showing it
    document.getElementById("Options").style.display = "block";           //Display the selected tab content  
	document.getElementById("optionsButton").className += " active";      //Set the corresponding button to active
    if(currenttab == "wifi"){                                             //When coming from wifi tab, this is also the indication to save the settings
	  setSaveWifi(); 
	}
    currenttab = "options"; 
	  if(consoleoutput) console.log("Switched to " + currenttab + " tab");  //Send console message
  }
  
  if(tabNum == 3){
	locksubmits();                                                         //When entering the wifi page, the submit buttons are locked (again) to prevent unintended submit
	getWifi();                                                             //We request the lamp to send the wifi options and update the tab accordingly before showing it
    document.getElementById("Wifi").style.display = "block";               //Display the selected tab content  
	document.getElementById("wifiButton").className += " active";          //Set the corresponding button to active
    if(currenttab == "options"){                                           //When coming from options tab, this is also the indication to save the settings
	  setSaveOptions();
	}
	currenttab = "wifi"; 
	if(consoleoutput) console.log("Switched to " + currenttab + " tab");   //Send console message
  }
}

function showMenu() {
  document.getElementsByClassName("dropdown-content")[0].style.display = "block";    
}

function showEffectMenu() {
  document.getElementsByClassName("effectdropdowncontent")[0].classList.toggle("show");
}

window.onclick = function(event) {
  if (!event.target.matches('.effectdropdownbutton')) {
    document.getElementsByClassName("effectdropdowncontent")[0].classList.remove("show");
  }
}

function setConnectionStatus(status){
	if (!status){
		document.getElementById("connectionbutton").innerHTML = "Lost";
		document.getElementById("connectionbutton").style.backgroundColor = "#b30000";
		
  } else {
		document.getElementById("connectionbutton").innerHTML = "Online";
		document.getElementById("connectionbutton").style.backgroundColor = "#248f24";
	}
}













