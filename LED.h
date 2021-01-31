//*****************************************************************************************
//**                              Crystal lamp project                                   **
//*****************************************************************************************
//This file belongs to the Crystal lamp project and is intended to work together with the other project files, not standalone!

//This files contains the functions to create the color and fire effects on the WS2812B LED Strip

#define NUM_LEDS 64                      //We define the total number of leds (has to be NUM_STRIPS x NUM_LEDS_STRIPS)  
#define NUM_STRIPS 8                     //We define the number of LED strips
#define NUM_LEDS_STRIP 8                 //We define the number of LEDs per strip
#define LED_PIN 0                        //Defines RX pin, why ever this is No. 0
#define LED_CALLS_PER_SECOND 50          //LED Update Calls per Second (=FPS!)
#define MAX_COLOR_PALETTE_SIZE 64        //The maximum number of color per palette
#define FASTLED_ESP8266_D1_PIN_ORDER     //We set the D1 Pin Order, BEFORE we include the fastled library!

#include <FastLED.h>                     //Include the FastLED library
#include "ColorPalettes.h"               //We include our color palettes and color interpolation function

class LED {
  private:
    //Please note, that the default values here are overwritten by the ones from the EEP, see Start()!
		//Common variables
		uint8_t NumStrips = NUM_STRIPS;                        //We have to overload our defines above into variables in order to calculate with them
		uint8_t NumLedsStrip = NUM_LEDS_STRIP;
		CRGB leds[NUM_LEDS];                                   //We create our object to store the color information we want to write to the leds
    uint8_t CallsPerSecond = LED_CALLS_PER_SECOND;         //How often will FireStrip excute its calculations, regardless of the actual calling frequency in the loop()  
    uint32_t LastCall = 0;                                 //The time when the FireStrip function was called last time, used to control the frames per second properly
    uint8_t Mode = 255;                                    //Mode, 1: Off, 2: Constant Color, 3: Effects, 4: Motion, 255: initial
    uint8_t Effect = 255;                                  //Effect, 0: Normal Fire, 1: Color Fire, 2 Pacifica,..., 255: initial
    uint8_t Power = 100;                                   //Effect power is used as input for the effect, can e.g. be brightness or whatever the effect needs
		boolean Active = 0;                                    //Support variable 
		uint8_t ColorPalette[MAX_COLOR_PALETTE_SIZE];
		uint8_t ColorPaletteSize = 0;
		
		const uint8_t LedTable[8][8]{  		//LED position table (its not so easy to write a clever function for this instead of a table, but feel free! I failed :)
			{ 7, 6, 5, 4, 3, 2, 1, 0},
			{ 8, 9,10,11,12,13,14,15},
			{23,22,21,20,19,18,17,16},
			{24,25,26,27,28,29,30,31},
			{39,38,37,36,35,34,33,32},
			{40,41,42,43,44,45,46,47},
			{55,54,53,52,51,50,49,48},
		  {56,57,58,59,60,61,62,63}
		};
		
	  //Parameters belonging to the constant color effect
    CRGB Color = 0x0000FF;      //initial color blue
		boolean Soft_Mode = true;   //Activates soft start function for single color mode
    uint16_t Soft_Time = 1;     //Soft start time in s (when soft soft mode is active) controls the time to reach full or zero brightness in Single color mode

    //Parameters belonging to the 2D fire effect
		uint8_t Energy[NUM_STRIPS][NUM_LEDS_STRIP] = {{0}};    //Array for the current energy level of each LED
		struct FireParameters{
			uint8_t Cooling = 55;             //Cooling Coefficient, Default 55, suggested range 20-100 
			uint8_t Sparking = 120;           //Sparking Probability, Default 120, suggested range 50-200.  
			uint8_t minSparkPower = 80;       //Sparking Power min and max values (when sparking occurs)
			uint8_t maxSparkPower = 255;	
			uint8_t minPower = 55;            //min and max Effect Power
			uint8_t maxPower = 255;
			uint8_t Bottom = 3;               //Defines how may cells are considered to be bottom aka heatable
		} FireParams;
		
		//Parameters belonging to the pacifica effect

		
    //Internal little helper functions
		void LoadPalette(uint8_t[], uint8_t);   //Loads a gradient color palette from ColorPalettes.h into the used ColorPalette
		void setAllLeds(CRGB);                  //Writes all Leds to the given color
		CRGB InterpolateColor(uint8_t[], uint8_t, uint8_t);   //Interpolates the resulting color with the given gradient color palette
		
    //Effect Functions
		void Fire2D(void);              //Function to create the fire effect
		void Pacifica(void);            //Function to create the Pacifica Effect
		void PacificaOneLayer(CRGBPalette16& p, uint16_t , uint16_t , uint8_t , uint16_t);

  public:
    LED(void);                    //Constructor
    void Start(void);             //Init function, sets up the LEDs and loads parameters from EEP
    void Run(void);
	
	  //Interface functions for interaction with Web Interface
    void setLedMode(uint8_t);     //Set new mode, 1: Off, 2: Constant Color, 3: Effects, 255: initial
    uint8_t getLedMode(void);	    //Get the current mode (self explaining)
    void setEffect(uint8_t);      //Set new effect, 0: Normal Fire, 1: Color Fire, 2 Pacifica,..., 255: initial
    uint8_t getEffect(void);	    
    void setColor(CRGB);          //set the color in mode 2 and for effects using the colorpicker
    CRGB getColor(void);          
    void setPower(uint8_t);       //sets the effect power (with equals the brightness of the set color, but we can use it for effect intensity
	  uint8_t getPower(void);       
    void setSoftMode(boolean);    //actitvates the soft transition in mode 2
    boolean getSoftMode(void);    
    uint16_t getSoftTime(void);
    void setSoftTime(uint16_t);   
		void setActive(boolean);
		
		//EEP save functions
    void saveOptions(void);       //Function to write the current LED options to EEP
		void saveDefault(void);       //Function to write the current control settings (color, power, mode, effect etc) to EEP as default for next start
} LED;

//Constructor
LED::LED(void){
}

void LED::Start(void){
  FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);    //We start the FastLED library

  //Read the stored start-up default values from the EEP
  setLedMode(EEP.ReadValue8(13));          
  setEffect(EEP.ReadValue8(14));        
  setPower(EEP.ReadValue8(15));  
  setSoftMode(EEP.ReadValue8(8));  
  setSoftTime(EEP.ReadValue16(19));
	setColor(CRGB(EEP.ReadValue8(16), EEP.ReadValue8(17), EEP.ReadValue8(18)));
	
  WriteSerial.Write(String("FastLED started.\n\n"));  
}

void LED::Run(void){
  if(millis()>LastCall+(uint32_t)1000/(uint32_t)CallsPerSecond){   //If more than 1000ms / Calls per second has passed, execute, otherwise skip  
	
	/*IMPOARTNT: When there is a battery problem (mostly empty), we switch off the LED. This is also important to prevent overload of the USB, when connected, because without
	external supply the LEDs draw the power from the USB. This can cause "strange" behaviour when the USB is protected and switches off, otherwise also hardware damage!*/
	if(PowerSupply.isWarning()) setLedMode(0);  
	
	//We call the effects function that require periodic calls, you can add more effect here easily, just pick the next number and call your own function
	if(Mode==2){                           
		if(Effect==0||Effect==1) Fire2D();
		if(Effect==2) Pacifica();
	}
	
	if(Mode==3){
		if(Active){
			
			
		}
			
	}
	

	FastLED.show();     //Run FastLed
  LastCall=millis();  //Safe a new time stamp  
  }  
}

//What ever has to happend, when the mode is changed is done here
void LED::setLedMode(uint8_t NewMode){
	if(PowerSupply.isWarning()) NewMode = 0;
		if(NewMode != Mode){
			Mode = NewMode;
			switch(Mode){       //We have to change some stuff, when the mode is changed.
				case 0:           //Switch all LEDs off
					setAllLeds(CRGB::Black);
					break;
				case 1:           //Set constant color
					setAllLeds(Color);
					break;
				case 2:         
					if(Effect == 1){	   //When Color Fire is selected, we set the inner color of the color fire palette
						ColorPalette[5] = Color.r;
						ColorPalette[6] = Color.g;
						ColorPalette[7] = Color.b;
					}
					break;
				case 3:
				  
				  break;
			}
		}
}

uint8_t LED::getLedMode(void){
  return(Mode);
}


void LED::setEffect(uint8_t NewEffect){
  if(NewEffect != Effect){
		Effect = NewEffect;      //We set the new effect
    switch(Effect){          //And load the corresponding Color Palettes from FastLed library or ColorPalettes.h, if necessary
      case 0:  //Normal fire
				LoadPalette(NormalFire, sizeof(NormalFire));
        break;
      case 1:  //Color fire
				LoadPalette(ColorFire, sizeof(ColorFire));
				setColor(Color);   //We recall setColor() here in order to overwrite the loaded ColorFire Palette with the selected color
        break;  
      case 2:   //Witching
  
        break;
      case 3:   //Magic
  
        break;          
    }
  }
}

uint8_t LED::getEffect(void){
  return(Effect); 
}

void LED::setColor(CRGB NewColor){
  Color=NewColor;          //Set the Color for the Color Mode
	if(Mode==1) setAllLeds(Color);  //If in Color Mode we directly write to the Leds.
	if(Effect == 1){	       //When Color Fire is selected, we set the inner color of the color fire palette aswell
		ColorPalette[5] = Color.r;
		ColorPalette[6] = Color.g;
		ColorPalette[7] = Color.b;
	}
}


CRGB LED::getColor(void){
  return(Color);
}

void LED::setPower(uint8_t input){
  Power = constrain(input,0,100);
}

uint8_t LED::getPower(void){
  return(Power); 
}

void LED::setSoftMode(boolean input){
  Soft_Mode = input;
}

boolean LED::getSoftMode(void){
  return(Soft_Mode);  
}

void LED::setSoftTime(uint16_t input){
  Soft_Time = input;
}

uint16_t LED::getSoftTime(void){
  return(Soft_Time);
}

void LED::saveOptions(void){
  //Save the current options in the EEP
  EEP.WriteValue8(8, Soft_Mode);  
  EEP.WriteValue16(19, Soft_Time);
  EEP.Commit();
  WriteSerial.Write(String("LED options values saved to EEP\n\n"));  
}

void LED::saveDefault(void){
  //Save the current values as start-up default in the EEP
	//Please note, that the power doesn't need to be stored, because it is included in the RGB
  EEP.WriteValue8(13, Mode);          
  EEP.WriteValue8(14, Effect);        
  EEP.WriteValue8(15, Power);  
  EEP.WriteValue8(16, Color.r);
  EEP.WriteValue8(17, Color.g);  
  EEP.WriteValue8(18, Color.b); 
  EEP.Commit();
  WriteSerial.Write(String("LED start default values saved to EEP\n\n"));  	
}

void LED::LoadPalette(uint8_t NewPalette[], uint8_t NewPaletteSize){
	ColorPaletteSize = NewPaletteSize;
	for(int i = 0;i<MAX_COLOR_PALETTE_SIZE;i++){ //For safety we empty the array completely
		ColorPalette[i] = 0;	
	}
	
	for(int i = 0;i<NewPaletteSize;i++){   //And fill it with the new Palette
		ColorPalette[i] = NewPalette[i];
	}
}

void LED::setAllLeds(CRGB LedColor){
	for(uint8_t i=0;i<(NumStrips*NumLedsStrip); i++) {  
		leds[i] = LedColor;
	}  
}

CRGB LED::InterpolateColor(uint8_t palette[], uint8_t palette_size, uint8_t value){
  CRGB result;
  for(uint8_t i=0; i < (palette_size/4); i++){
    if (palette[i*4] > value){     
      result.r=(uint8_t)((palette[i*4+1]-palette[(i-1)*4+1])/(float)(palette[i*4]-palette[(i-1)*4])*(float)(value-palette[(i-1)*4])+(float)palette[(i-1)*4+1]);
      result.g=(uint8_t)((palette[i*4+2]-palette[(i-1)*4+2])/(float)(palette[i*4]-palette[(i-1)*4])*(float)(value-palette[(i-1)*4])+(float)palette[(i-1)*4+2]);
      result.b=(uint8_t)((palette[i*4+3]-palette[(i-1)*4+3])/(float)(palette[i*4]-palette[(i-1)*4])*(float)(value-palette[(i-1)*4])+(float)palette[(i-1)*4+3]);  
      return(result);
    }
  }
}

void LED::Fire2D(void){
	//This function implements the Fire2012 withPalette example from the FastLED library, adapted for an nxn led array but as 2D effect
	
  // Step 1.  Cool down every cell
  for(uint8_t i=0;i<NumLedsStrip; i++) {
		uint8_t cooling = random8(0, ((FireParams.Cooling * 10) / NumStrips*NumLedsStrip) + 2);
		for(uint8_t j=0;j<NumStrips;j++){   
			Energy[j][i] = qsub8(Energy[j][i], cooling);
    }
  }
	
	// Step 2.  Heat from each cell drifts 'up' and diffuses a little
	for(uint8_t i= NumLedsStrip - 1; i >= 2; i--) {
		for(uint8_t j=0;j<NumStrips;j++){ 
			Energy[j][i] = (Energy[j][i - 1] + Energy[j][i - 2] + Energy[j][i - 2] ) / 3;
    }
	}
	  
  // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
	if(random8() < FireParams.Sparking) {
			uint8_t i = random8(FireParams.Bottom);
			uint8_t sparking = random8((uint8_t)((float)FireParams.minSparkPower/100*(float)Power),(uint8_t)((float)FireParams.maxSparkPower/100*(float)Power));
	  for(uint8_t j=0;j<NumStrips;j++){ 
			Energy[j][i] = qadd8(Energy[j][i],sparking);
		}
	}
	
	// Step 4.  Map from heat cells to LED colors
  for( uint8_t i = 0; i < NumLedsStrip; i++) {
	  for(uint8_t j=0;j<NumStrips;j++){ 
      leds[LedTable[j][i]] = InterpolateColor(ColorPalette, ColorPaletteSize, Energy[j][i]);
    }
  }
	
	//Step 5. We adjust the brightness according to the power level (with a bit of scaling otherwise it becomes to dark at low power)
	FastLED.setBrightness(constrain(FireParams.minPower+(FireParams.maxPower-FireParams.minPower)/100*Power, 0, FireParams.maxPower));
}



void LED::Pacifica()
{
  // Increment the four "color index start" counters, one for each wave layer.
  // Each is incremented at a different speed, and the speeds vary over time.
  static uint16_t sCIStart1, sCIStart2, sCIStart3, sCIStart4;
  static uint32_t sLastms = 0;
  uint32_t ms = GET_MILLIS();
  uint32_t deltams = ms - sLastms;
  sLastms = ms;
  uint16_t speedfactor1 = beatsin16(3, 179, 269);
  uint16_t speedfactor2 = beatsin16(4, 179, 269);
  uint32_t deltams1 = (deltams * speedfactor1) / 256;
  uint32_t deltams2 = (deltams * speedfactor2) / 256;
  uint32_t deltams21 = (deltams1 + deltams2) / 2;
  sCIStart1 += (deltams1 * beatsin88(1011,10,13));
  sCIStart2 -= (deltams21 * beatsin88(777,8,11));
  sCIStart3 -= (deltams1 * beatsin88(501,5,7));
  sCIStart4 -= (deltams2 * beatsin88(257,4,6));

  // Clear out the LED array to a dim background blue-green
  fill_solid( leds, NUM_LEDS, CRGB( 2, 6, 10));

  // Render each of four layers, with different scales and speeds, that vary over time
  PacificaOneLayer( pacifica_palette_1, sCIStart1, beatsin16( 3, 11 * 256, 14 * 256), beatsin8( 10, 70, 130), 0-beat16( 301) );
  PacificaOneLayer( pacifica_palette_2, sCIStart2, beatsin16( 4,  6 * 256,  9 * 256), beatsin8( 17, 40,  80), beat16( 401) );
  PacificaOneLayer( pacifica_palette_3, sCIStart3, 6 * 256, beatsin8( 9, 10,38), 0-beat16(503));
  PacificaOneLayer( pacifica_palette_3, sCIStart4, 5 * 256, beatsin8( 8, 10,28), beat16(601));

  // Add brighter 'whitecaps' where the waves lines up more
  uint8_t basethreshold = beatsin8( 9, 55, 65);
  uint8_t wave = beat8( 7 );
  
  for( uint16_t i = 0; i < NUM_LEDS; i++) {
    uint8_t threshold = scale8( sin8( wave), 20) + basethreshold;
    wave += 7;
    uint8_t l = leds[i].getAverageLight();
    if( l > threshold) {
      uint8_t overage = l - threshold;
      uint8_t overage2 = qadd8( overage, overage);
      leds[i] += CRGB( overage, overage2, qadd8( overage2, overage2));
    }
  }
	
	// Deepen the blues and greens a bit
	for( uint16_t i = 0; i < NUM_LEDS; i++) {
    leds[i].blue = scale8( leds[i].blue,  145); 
    leds[i].green= scale8( leds[i].green, 200); 
    leds[i] |= CRGB( 2, 5, 7);
  }
}

// Add one layer of waves into the led array
void LED::PacificaOneLayer( CRGBPalette16& p, uint16_t cistart, uint16_t wavescale, uint8_t bri, uint16_t ioff)
{
  uint16_t ci = cistart;
  uint16_t waveangle = ioff;
  uint16_t wavescale_half = (wavescale / 2) + 20;
  for( uint16_t i = 0; i < NUM_LEDS; i++) {
    waveangle += 250;
    uint16_t s16 = sin16( waveangle ) + 32768;
    uint16_t cs = scale16( s16 , wavescale_half ) + wavescale_half;
    ci += cs;
    uint16_t sindex16 = sin16( ci) + 32768;
    uint8_t sindex8 = scale16( sindex16, 240);
    CRGB c = ColorFromPalette( p, sindex8, bri, LINEARBLEND);
    leds[i] += c;
  }
}

void LED::setActive(boolean newActive){
	Active = newActive;
}



