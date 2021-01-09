//*****************************************************************************************
//**                              Crystal lamp project                                   **
//*****************************************************************************************
//This file belongs to the Crystal lamp project and is intended to work together with the other project files, not standalone!
//This file contains the definitions of the color gradient maps and the color interpolation function
//Can define a color gradient map from using n colors between index 0 and 255, index 0 and 255 HAVE to be used at minimum 
//Format: [index_1 0...255, Red_1 0..255, Green_1 0..255, Blue_1 0..255, ... to index_n]

//These functions use an indexed color palette, where the colors are not equidistant, therefore the CRGBPalette16 is not used
uint8_t NormalFire[20] = {0, 0, 0, 0, 51, 255, 0, 0, 153, 255, 255, 0, 242, 255, 255, 255, 255, 0, 0, 255};  
//0: black, 51: red, 153: bright yellow, 242: full white, 255: light blue for high energy sparks

uint8_t ColorFire[16] = {0, 0, 0, 0, 200, 0, 255, 0, 242, 255, 255, 255, 255, 0, 0, 255};
//0: black, 200: green, 242: full white, 255: light blue for high energy sparks

//For Pacifica Effect
CRGBPalette16 pacifica_palette_1 = 
    { 0x000507, 0x000409, 0x00030B, 0x00030D, 0x000210, 0x000212, 0x000114, 0x000117, 
      0x000019, 0x00001C, 0x000026, 0x000031, 0x00003B, 0x000046, 0x14554B, 0x28AA50 };
CRGBPalette16 pacifica_palette_2 = 
    { 0x000507, 0x000409, 0x00030B, 0x00030D, 0x000210, 0x000212, 0x000114, 0x000117, 
      0x000019, 0x00001C, 0x000026, 0x000031, 0x00003B, 0x000046, 0x0C5F52, 0x19BE5F };
CRGBPalette16 pacifica_palette_3 = 
    { 0x000208, 0x00030E, 0x000514, 0x00061A, 0x000820, 0x000927, 0x000B2D, 0x000C33, 
      0x000E39, 0x001040, 0x001450, 0x001860, 0x001C70, 0x002080, 0x1040BF, 0x2060FF };



