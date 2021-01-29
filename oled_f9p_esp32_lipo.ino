
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// #include <Fonts/FreeSerifBold12pt7b.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

/*
* OLEd to ESP32
   1 Vss   gnd
   2 Vdd   3v3  
   3 sclk  D18
   4 sda   D23
   5 /rst  D4
   6 sao   D2
   7 nc    D15

   I2C --> ESP32
   SCL  D22
   SDA  D21
   
 * D8         7 CS
 * D15        6 DC
 * D04        5 RES
 * D23        4 SDA
 * D05        18 SCLK
/* arduino uno Declaration for SSD1306 display connected using software SPI (default case):

#define OLED_MOSI   11
#define OLED_CLK   13
#define OLED_DC   9
#define OLED_CS    10
#define OLED_RESET 8
*/
// esp32
#define OLED_MOSI   23
#define OLED_CLK   18
#define OLED_DC   2
#define OLED_CS    15
#define OLED_RESET 4

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,
  OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

//lipo starts
#include <SparkFunBQ27441.h>

// Set BATTERY_CAPACITY to the design capacity of your battery.
const unsigned int BATTERY_CAPACITY = 2800; // e.g. 850mAh battery



//F9P starts
#include <Wire.h> //Needed for I2C to GPS

#include "SparkFun_Ublox_Arduino_Library.h" //http://librarymanager/All#SparkFun_Ublox_GPS
SFE_UBLOX_GPS myGPS;

long lastTime = 0; //Simple local timer. Limits amount if I2C traffic to Ublox module.


// F9P ends

void set_oled() {
  Serial.begin(9600);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC)) {
  Serial.println(F("SSD1306 allocation failed"));
  for(;;); // Don't proceed, loop forever
  }


  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000); // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();

  // Draw a single pixel in white
  display.drawPixel(10, 10, WHITE);

// display.setFont(&FreeSerifBold12pt7b);

  // Show the display buffer on the screen. You MUST call display() after
  // drawing commands to make them visible on screen!
  display.display();
  delay(2000);
 }
 
void set_gnss()
  {
   
//F9P definitions
  Wire.begin();
  Wire.setClock(400000); //Optional. Increase I2C clock speed to 400kHz.
  myGPS.begin(Wire);
  if (myGPS.begin() == false) //Connect to the Ublox module using Wire port
  {
    Serial.println(F("Ublox GPS not detected at default I2C address. Please check wiring. Freezing."));
  //   lcd.clear();
  //   lcd.setCursor(0, 1);
  Serial.print(F("No GPS detected"));
    while (1);
  }
 
}

void draw(void) {
  display.clearDisplay();

  long longitude = myGPS.getLongitude();
  long latitude = myGPS.getLatitude();
  long altitudeMSL = myGPS.getAltitudeMSL();
  byte SIV = myGPS.getSIV();
  long accuracy = myGPS.getPositionAccuracy();
  long altitude = myGPS.getAltitude();
  int pDOP = myGPS.getPDOP();
  
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.print("Long: ");
  display.print(longitude/10000000.0,8);


 //  display.setTextColor(BLACK, WHITE); // Draw 'inverse' text
  display.setCursor(0,10);             // Start at top-left corner
  display.print("Lat : ");
  display.println(latitude/10000000.0,8);

    
  display.setTextColor(WHITE); // Draw 'inverse' text
  display.setCursor(0,20);             // Start at top-left corner
  display.print("Alt : ");
  display.print(altitudeMSL/1000.0,3);

  display.setCursor(0,30);             // Start at top-left corner
  display.print("Alt : ");
  display.print(altitude/1000.0,3);
  
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(WHITE); // Draw 'inverse' text
  display.setCursor(0,40);             // Start at top-left corner
  display.print("Sat : ");
  display.print(SIV);

 //display.setCursor(60,40);             // Start at top-left corner
 // display.print("pDOP: ");
 // display.print(pDOP);

  display.setCursor(0,50);             // Start at top-left corner
  display.print("acc.: ");
  display.print(accuracy/1000.0,3);

byte fixType = myGPS.getFixType();
    display.setCursor(60,40); 
  //  display.print("GNSS:");
    
         if(fixType == 0) display.print("No fix");
    else if(fixType == 1) display.print("Dead reckoning");
    else if(fixType == 2) display.print("2D");
    else if(fixType == 3) display.print("3D");
    else if(fixType == 4) display.print("GNSS+Dead reckoning");

byte RTK = myGPS.getCarrierSolutionType();
    display.setCursor(80,40); 
    if (RTK == 1)  display.print(" Flt"); //Serial.print(F("High precision float fix!"))
    if (RTK == 2)  display.print(" RTK"); //Serial.print(F("High precision fix!"))

// Read battery stats from the BQ27441-G1A
  unsigned int volts = lipo.voltage(); // Read battery voltage (mV)
  display.setCursor(90,30);             // Start at top-left corner
  display.print("V:");
  display.print(volts/1000.0,2);
 
  display.display();
  
 delay(100);
/*
  display.setTextSize(2);             // Draw 2X-scale text
  display.setTextColor(WHITE);
  display.print(F("0x")); display.println(0xDEADBEEF, HEX);

*/
    }
 

void setup() {
  delay (2500);
  set_gnss ();
  set_oled ();
}
  
void loop(void)
{
//Query module only every 2.5 second. Doing it more often will just cause I2C traffic.
  if (millis() - lastTime > 1000)
  {
    lastTime = millis(); //Update the timer
      draw();
     }
}

 
