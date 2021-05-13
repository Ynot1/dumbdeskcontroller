//#Correct board type for the esp-01 is "generic esp8266"
//# Programmer AVRISP mk II
#define USING_AXTLS
#include <time.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecureAxTLS.h>
using namespace axTLS;
#define STACODE "amzn1.ask.account.AFHAAQFF43O6XBUSKHQ537XX66QPLCHNUASFIJV6DS4R3Q2NFCMQDCXQLNKFRTM2RQ7SDI6U5RNR6IRXPUYDR4GEYKDJBV6XZKWOT4ZLLG3XRTAJBFLQHEGQZJDKX5YGTWWMJJ64XTHX7ASX4UZ6GFNJQICQI3UTIOQJUQCXSGXTFE4MULS6S7UJVRXAXVDPWQXICV6NKFB4QHQ" // see www.virtualbuttons.com for more information

//Version History
// mods to add PIR Sensor instead of ultrasonic to detect person & use the ultrasonic to detect desk height instead of hall effect

// adding visual reminder that its time to sit or stand

// Tweaks for WNO - changing Wifi node to cellphone (or Kguest creds)
//different PIR sensor used, but this didnt require code change as inverted sense using hardware (otherwise it wouldnt boot)
// setting desk heights to operate above credenza - better reflective surface than carpet
// adding smoothing of desk up/down state
// last worked on  28/9 1600:00
// adding making smoothing of desk up/down state adjustable from setup & display of average distance readings
//adding making desk height settings adjustable from setup
// last worked on  30/9 1600:00
// adding making person smoothing settings adjustable from setup
// last worked on  30/9 1800:00
// adding beeper and button
// last worked on  1/10 0900
// adding button read
// last worked on  1/10 1100
// combining codebases for home&work
// last worked on  5/10 0900
// correcting standing display at work
// last worked on  6/10 1700
// adding motor drive logic and diagnostic port
// last worked on  7/10 1800
// making led respond to DeskHeightDownStopLimit & DeskHeightUpStopLimit thresholds, and making them adjustable from setup
// last worked on  8/10 0700
// chnaging work desk default stop limits, and relay logic. relay 1 on, relay 2 off = down, inverse = up. Both on or both off is stop
// last worked on  10/10 0940
// add rate of movement detection, and cuttoff motors if ever to slow.
// last worked on  14/10 1600
// improving speed detection, do difference on 5 sec intervals (adustable from web)Make up and down speeds setable (from web)
// last worked on  28/10 1400

// adding ability to 'skip' a sit/stand prompt by clicking on the button that says to sit or stand
// last worked on  19/-1/2021 1830

//improving stability without wifi in work mode
//last worked on  12/5/21

// PARITALLY DONE ?? Needed smoothing capacitor fitted to 3v3 rail of processor need to reduce sensitivity of PIR sensor , it reports always present - Still temp sensitive... works ok at night.
// DONE need to connect diag port physically
// DONE need to provide nudge values for motor movement logic
// DONE need to connect push button and piezo buzzer
// DONE need to dervice motor moving logic 
// DONE (removed) either remove or implement Nudge
// fix DST Offset
// DONE add desk height offset so displayed height is actual desk height
// DONE add current height measurement to sit/stand buttons
// DONE rate of movement detection, and cuttoff motors if ever too slow.
// DONE need to quantify desk speed and enter into code when motor fitted.
// DONE might need to remove the abs bits if it strips floats to ints...
// be nice to be able to enter float values from the web page, rather than lots of up/down buttons
// https://randomnerdtutorials.com/esp32-esp8266-input-data-html-form/
// DONE can it survive a restart without the wifi network being available?
// MAYBE DONE Its possible for the desk to be trggered to move without the underspeed or web page movingup/down icons being active. This happened twice in late November
// ^^ was that relay off commands being missed by the board? doubled up on all relay on/off commands
// DONE add up and down buttons, and web pages that trigger this when they are visted.
//mount controller properly (both home and work)
// Can controller sit on credensa at work?
// intergrate other prox sensors ,at least on work one


 
byte CurrentSittingMinutesAlarmThreshold = 120;
byte CurrentStandingMinutesAlarmThreshold = 80;

bool PushButtonState = false; // Current State of the Local Push 
bool PrevPushButtonState;

bool LocationWork = HIGH; //  HIGH = work, LOW = Home;
bool DeskMovingUp = 0;
bool DeskMovingDown = 0;

const String access_code = STACODE;
/*
  The evolving requirements and bug list section.

  make everything important definable and stored in eeprom

  time servers, DST offset value, timezone
  ultrasonic distance measurements
  ssid & password (generate its own wifi on virgin birth or no login after fixed period)
  Sit/Stand notification web site
  alexa api keys
  secure cert
  number of prompts
  Sit Stand Absent & ghost colors
  AlexaNotificationThreshold
  CurrentSittingMinutesAlarmThreshold = 60;
  CurrentStandingMinutesAlarmThreshold = 45;
  
  NOT Repeatable RTC functions are messed up since adding tracking of Absent time. Time runs fast.
  DONE Make current absent time in days, hrs , mins and seconds

  Make it long beep only once, then short pips as reminders. That way, the loop coun wont get fucked up if it gets into a sate where it wants to change desk
  when no one is around.

  add a resync RTC button?

  DONE change absent time to days, hrs, mins

  Done The amount of absent time is starting to creep up. Add this to the histogram?

   Can it beep when a user defined threshold for sitting / standing time is crossed ? A prompt to sit or Stand?
      DONE A diffrernt beep to sit from stand?
      DONE how about an acending beep for stand, and a decending one for sit?
      DONE different thresholds for max sitting and max standing
      write these sit/stand alarm threshold values to eeprom and read them back on restart
      DONE can you have enable/disable buttons for each alarm ?
      DONE replace lst hours times with consecutive sitting and standing times, and drive the beeper from that.
      WONT DO Can the alarm be louder?
      DONE Is an Alexa notification for sit/stand alarms viable?
          allow definition of virtual button code at runtime?
          back up relevant settings in eeprom

      Can the prompts to site/stand be added to the daily graph somehow?

    PARITALLY DONE Can it collect the real time from the internet?
        DONE Intergarte the SNTP time to be used
        DONE Correct to NZST somehow
        find a local timesource
        allow definition of time source at runtime
        allow DST and Timezone to be set at runtime
        DONE resync the clock for graphing/timing etc every hour - its free running after setup at present
        DST Offset isnt being honored at clock set

   DONE make more room on page - put the sitting/standing and present/absent & refresh buttons side by side
   DONE make new CSS objects that could form a graph bar and place on weekly page -not workable soultion though
   DONE make html objects that can be manipulated in size
   make working time / sitting time in a defined box
   is .genericrectangle used?
   PARTIALLY DONE make a function to display rectangles as a graph from an array of data
     The big string of &nbsp is ugly, and doesnt render well on mobiles. fix this.
      DONE 'grey out' or change color of previous days sit/stand histogram at midnight.
      That would look beeter as a stacked graph
      https://www.homeandlearn.co.uk/WD/wds6p2.html

   The big bunch of 'ifs' around the controls is ugly, a case statement would be way nicer. just remove the old graph..

   DONE add ultrasonic and hall effect sensors to detect person present and desk postion

   optimise range sensor and add delays to make detections stable
     DONE add timers or smoothing around person present detection and map to personPresent
     Arrange for different thresholds in sitting or standing mode ?
     Replace ultrasonic with microwave detector
   WONT DO install run/load switch and reset button, to allow in situ programming via diagnostic port.

   DONE remove sit button, make current state bigger on web page

   DONE add last few distance readings to web page to aid in disgnosing why it misbehaves occasinally

   DONE ? runs of zeros received from distance sensors from time to time - why ? stops the whole void loop...



     DONE try looking for two or more consectutive detections of present or absent before changing presence modes
         this works really well. An improvement might be to look for x valid reradings in y samples, rather than sequential

     can the min distanace for the back of a pushed back chair be sorted? might not be worth it...
   DONE Fix total time worked calcs - or log present time instead of summing sitting & standing
   DONE Check acuracy of the timer for accumulating times. might be fast...

   DONE optimise hall effect sensor and mount magnet to desk.
   DONE intergate sensors state to web page

   Non functional changes
     tidy indents up. What would Adrian and Dylan say?
     add rest of tests around Debugmessages conditions.
     document what the debug message format is
       ~ = measurement
       > counting up or terminal count of personpresent, < counting down
     remove unnessasry variables
     remove unused code blocks
     make the 3 time counting routines a function
     standardise Camel casing on variables
     restructure both CSS and HTML so the tags are more clearly indented
     make the checks for buttons pushed a case statement
    WallDetectedStateCountThreshold
    PersonDetectedStateCountThreshold


    

   DONE Make total worked time correct - minutes and seconds can sum to > 60. Probably easier to count period person is present
   DONE put Desk up / down counts on same line of web page

     figure out how to detect a working day
     add functinality that assumes a day starts after approaching the desk after a ~5 hr break, and if I am there for > 15mins, start accumalating daily stats
     similarly, when absent for > ~5 hrs, thats the day end
     or, I could get time from an internet timeserver and assume a 0700 daily start (mon-fri)
     PARTIALLY DONE could i display contents of the working/sitting time array as a bar graph? Hourly on daily page, daily on weekly page

     figure out how to detect a working week, add weekly stats on its own page
     have Archive stats on its own page

   arrange for eeprom write of all data (daily/weekly and Archive cumulative standing and sitting hours) every hour -
     put time since last eeprom write write on web page
     arrange to read in eeprom stored values on restart
     eeprom write of cumulative standing and sitting hours as end of working day is reached
     24 hrs * 7 days * 52 weeks is 8736 writes per year
     100000 (min read/write cycles b4 failure) / 8736 is 11.4 years so;
     figure out how to surpress eprom writes if no data changed.



   PARITALLY DONE add up/down desk cycle counter
   - add to Archive and daily/weekly totals on web page



   PARITALLY DONE add up/down desk cycle counter
   - add Archive and daily/weekly totals on web page

   BUGS
     It wont boot in desk sitting mode. Scope to flip the sense of the hall effect sensor over ? or will that just move the problem to wont boot when standing?
     the back of a chair is detected as a person in sitting mode.

     time keeping bug

     twice the standing time threshold has been passed overnight, and the rise/fall buzzer has sounded constantly. The buzzer takes ages
     so time appeared to run v slow , losing many hours with respect to real time. 
     however, muting the buzzr pushes the system into super fast mode, where time runs superfast with respect to real time
     i think this is because millis gets so far ahead of prevmillis, that loop counting gets screwed till it catches up. in this state, a loop takes only 431 milis
     but that passes the simple millis > previousmillis +1000  check.
     a simple reboot doesnt fix this - dont understand why not....

    system hangs if it cant sync SNTP time at start. Ideally that routine would timeout and continue with the existing time if it doesnt succeed in a minute or so
    


   OPTIONS
   DONE ignore values from fluffy objects (only use real reflections) and require X readings sequentailly before changing person present states
   CONSIDER are the echo values the same night and day? i suspect a temperature variation is affecting the ultrasonic sensor

*/
const char* VirtualButtonsHost = "api.virtualbuttons.com";
const int httpsPort = 443;
// Root certificate used by VirtualButtons.com is Defined in the "VirtualButtonsCACert" tab.
extern const unsigned char caCert[] PROGMEM;
extern const unsigned int caCertLen;

char* RunTimessid = "TonysIphone";
char* RunTimepassword = "Zxcvbnml";

//const char* RunTimessid = "KGuest";
//const char* RunTimepassword = "rmH472NTB832VhY8";

WiFiServer server(80); // internally facing web page is http.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored  "-Wdeprecated-declarations"
WiFiClientSecure client;
#pragma GCC diagnostic pop

String header; // Variable to store the HTTP request for the local webserver

int PageMode = 0; // 0 = Daily, 1 = ProxSensor, 2 = unused, 3= Setup

String SitStandState = "Sitting";
unsigned long StandCounter = 0;
unsigned long SitCounter = 0;

unsigned long currentTime = millis();

unsigned long previousTime = 0;
unsigned long previousMillis = 0;

byte standingseconds ;
byte standingminutes ;
byte standinghours ;
long standingtotalseconds;
unsigned long dailystandingpercentage;

byte sittingseconds ;
byte sittingminutes ;
byte sittinghours ;
long sittingtotalseconds;
unsigned long dailysittingpercentage;

byte workingseconds ;
byte workingminutes ;
byte workinghours ;
long workingtotalseconds;


byte currentseconds = 0 ;
byte currentminutes = 55;
byte currenthours = 12 ;

byte RectHeight = 110;
byte RectWidth = 10;
String RectColor = "Black";
byte percentagegraphscalingfactor = 1;

String SittingColor = "DeepSkyBlue";
String GhostSittingColor = "Azure";

String StandingColor = "SteelBlue";
String GhostStandingColor = "AliceBlue";

String AbsentColor = "MOCCASIN";
String GhostAbsentColor = "CORNSILK";

String StandUpColor = "FireBrick";
String SitDownColor = "Crimson";

String DeskMovingUpColor = "OrangeRed";
String DeskMovingDownColor = "DarkOrange";

//String WorkingColor = "DarkBlue";
//String GhostWorkingColor = "ROYALBLUE";

const long timeoutTime = 2000; // Define web page timeout time in milliseconds (example: 2000ms = 2s)

//int PersonPresentSmoothingCount = 0;
byte distancereportcounter = 0;
//byte PersonPresentSmoothingCountThreshold = 60;// It takes this many valid distance readings to infer a person has left the workdesk
byte WallDetectedStateCount = 0;
byte PersonDetectedStateCount = 0;
byte WallDetectedStateCountThreshold = 3;
byte PersonDetectedStateCountThreshold = 1;
int PersonPresentCounter = 0;
int PersonPresentTerminalCount = 360; //60 equates to 20 sec timer. Is the loop running quicker than 1 sec loops ?
int PersonPresentDownCount = 1; // The PIR detector is set to really short timer in hardware. Possible to change it in some modules but not the SR602 i am usinmg now/ This software timwer allows retriggering within the window to produce a stable ppresence detection. 
int PersonPresentUpCount = 10; // x times faster to detect a person leaving compared to arriving if the DownCount is set to 1


bool PersonDetectedState = false;  // True or HIGH when someone is detected
bool PersonPresentState = false;  // True or HIGH when someone is present - this is a buffered version of PersonDetectedState
bool PrevPersonDetectedState = false;
bool PIRSensorState = false;
bool PrevPIRSensorState = false;
bool DeskPostitionState = false; // UP is logic HIGH
bool PrevDeskPostitionState = false;
bool PrevPersonPresentState = false;
bool InvalidDistanceReading = false;
//bool WallDetectedState = false;
bool AlarmMute = true;
bool StackedGraph = true;

unsigned long UpDownCounter = 0;
int WeeklyUpDownCounter = 0;

int DailyUpCounter = 0;
int DailyDownCounter = 0;

float LastReading ;
float PenultimateReading ;
float SubPenultimateReading ;

int LastValidReading ;
int PenultimateValidReading ;
int SubPenultimateValidReading ;

//const int MaxValidDistance = 275;

boolean wifiConnected = false;
bool DebugMessages = true;

float duration, distance; // Duration used to calculate distance

// Assign variables to GPIO pins

const int GPIO2 = 2;  // GPIO2 pin - on board LED of some of the ESP-01 variants.
const int PIRSensorPin = 2;  //
const int echoPin = 3;  // 3 is the RX pin
const int trigPin = 1; // 1 is the TX Pin
const int PiezoPin = 0;  // GPIO0
const int LEDPin = 0;  // GPIO0
const int PushButtonPin = 0;  // GPIO0

//int DailySitStandArray[50] ; // 24 hrs, plus sit/stand for each index [0] is the sitting minutes for the first hour (00-01) two unused slots...
int DailySitStandArray[75] ; // 24 hrs, plus sit/stand/absent for each index [0] is the sitting minutes for the first hour (00-01) 1, is standing, 3 is absent
byte DailySitStandArrayIndex = 0;
byte CurrentHourSittingMinutes = 0;
byte CurrentHourStandingMinutes = 0;
int CurrentHourAbsentMinutes = 0;
byte CurrentSittingMinutes = 0;
byte CurrentStandingMinutes = 0;
int CurrentAbsentMinutes = 0;
byte CurrentAbsentHours = 0;
byte CurrentAbsentDays = 0;
byte AbsentSeconds = 0;

//byte CurrentSittingMinutesAlarmThreshold = 60;
//byte CurrentStandingMinutesAlarmThreshold = 45;



int AlexaNotifications = 0;
int AlexaNotificationThreshold = 0;
int DSTOffset = 1;

int StackedIndex = 0;
byte WebPageMode = 2; // 1 = Setup, 2 = Runtime

String Newssid = "defaultssid";
String Newpassword = "defaultpw";

byte rel1ON[] = {0xA0, 0x01, 0x01, 0xA2};  //Hex command to send to serial for open relay 1 - Moves Desk UP
byte rel1OFF[] = {0xA0, 0x01, 0x00, 0xA1}; //Hex command to send to serial for close relay 1
byte rel2ON[] = {0xA0, 0x02, 0x01, 0xA3};  //Hex command to send to serial for open relay 2 - Moves Desk DOWN
byte rel2OFF[] = {0xA0, 0x02, 0x00, 0xA2}; //Hex command to send to serial for close relay 2

int DeskPostitionSmoothingHighCount =0;
int DeskPostitionSmoothingHighTerminalCount = 10;
int DeskPostitionSmoothingLowCount =0;
int DeskPostitionSmoothingLowTerminalCount = 10;
int AverageValidReading = 0;

int DeskHeightDownLowerLimit = 2;
int DeskHeightDownUpperLimit = 6;
int DeskHeightUpLowerLimit = 34;
int DeskHeightUpUpperLimit = 42;
int DeskHeightOffset = 1;
int DeskHeightDownStopLimit = 4;
int DeskHeightUpStopLimit = 40;

float DeskSpeed = 0 ; //  mm/sec
//Movemt will be stopped if DeskSpeed falls below DeskSpeedCutOffRate threshold for DeskSpeedCutOffInterval while DeskSpeedCutOffEnabled is 1 . 
//DeskSpeedIntegrationInterval sets the period between meausements and speed checks

float DeskSpeedCutOffRate = 0.3; // desk moves any slower than this, and movement will be stopped in case its jammed
bool DeskSpeedCutOffEnabled = 0; // defaults to off for this dumbdeskmode implemenation
int DeskSpeedCutOffCounter = 0;
int DeskSpeedIntegrationInterval = 20; // seconds?
int DeskSpeedIntegrationCounter = 0;
float LastDeskPosition = 0; //last desk postion measurement


/*  WIRING DETAILS
    CPU TXD connects to the ultrasonic Trigger pin , (and to the external diagnotic port for debug log.)
    CPU RXD connects to the ultrasonic Echo pin
    Ultrasonic transducer used to calculate desk height - so it faces DOWN - 
    Front facing  PIR sensor connected to GPIO2, with 5k pullup resistor to 3.3v
    piezo sounder connected to GPIO0 (via 100ohm resistor to GND) & 7k pullup to 3.3v & a switch to Gnd as well....

*/

//boolean connectWifi();

void setup()
{


  pinMode(GPIO2, OUTPUT); // it changes to an input later...

  Serial.begin(115200);


  Serial.println("void setup starting ");
  Serial.println("Booting...");
  delay(2000);
  Serial.println("flashing LED on GPIO2...");
  //flash fast a few times to indicate CPU is booting
  digitalWrite(GPIO2, LOW);
  delay(100);
  digitalWrite(GPIO2, HIGH);
  delay(100);
  digitalWrite(GPIO2, LOW);
  delay(100);
  digitalWrite(GPIO2, HIGH);
  delay(100);
  digitalWrite(GPIO2, LOW);
  delay(100);
  digitalWrite(GPIO2, HIGH);

  Serial.println("Delaying a bit...");
  delay(2000);

 
  Serial.println("Setting up for WORK");
      CurrentSittingMinutesAlarmThreshold = 120;
      //CurrentSittingMinutesAlarmThreshold = 2;
      CurrentStandingMinutesAlarmThreshold = 80;

      //CurrentSittingMinutesAlarmThreshold = 1;
      //CurrentStandingMinutesAlarmThreshold = 1;
      
      DeskPostitionSmoothingHighTerminalCount = 5;
      
      DeskPostitionSmoothingLowTerminalCount = 5;
      
      
      DeskHeightDownLowerLimit = 2;
      DeskHeightDownStopLimit =4 ;
      DeskHeightDownUpperLimit = 6;
      DeskHeightUpLowerLimit = 32;
      DeskHeightUpStopLimit = 38;
      DeskHeightUpUpperLimit = 42;
      DeskHeightOffset = 1000;
      
      
      DeskHeightOffset = 710; // down is  ? real  vs 2 measured .up is 1050 real vs 380 measured or 670
      
      AlexaNotificationThreshold = 0;
      PersonPresentTerminalCount = 360;
      RunTimessid = "TonysIphone";
      RunTimepassword = "Zxcvbnml";
  

 

    Serial.println("Making RX pin into an Digital INPUT"); // used to receive ultrasonic pulse legnth to infer if someone is present
  pinMode(echoPin, FUNCTION_3);
  pinMode(echoPin, INPUT); // The ultrasonic detector uses timing pulse length to indicate distance, not serial.

  Serial.println("Making GPIO pin 2 into an Digital INPUT"); // used to detect PIR sensor state for Person detector
  pinMode(PIRSensorPin, FUNCTION_3);
  pinMode(PIRSensorPin, INPUT); //

  Serial.println("Making GPIO pin 0 into an Digital OUPUT"); // used to drive the piezo beeper or LED
  pinMode(PiezoPin, OUTPUT); //
/*
 * 
 *  
  
  //dummy up some site stand absent data on restart, can remove this when eeprom writing/reading is working

  DailySitStandArray[0] = 20; // 00 hr sitting
  DailySitStandArray[1] = 20; // 00 hr Standing
  DailySitStandArray[2] = 20; // 00 hr absent
  
   DailySitStandArray[3] = 2; // 01 hr sitting
  DailySitStandArray[4] = 56; // 01 hr Standing
  DailySitStandArray[5] = 2; // 01 hr absent
  
  DailySitStandArray[6] = 60; // 02 hr sitting
  DailySitStandArray[7] = 0; // 02 hr Standing
  DailySitStandArray[8] = 0; // 02 hr absent
  // 91011 121314 151617
  DailySitStandArray[18] = 30; // 6th hr sitting
  DailySitStandArray[19] = 20; // 6th hr standing
  DailySitStandArray[20] = 10; // 6th hr absent
  // 212223 242526 272829 303132 333435
  DailySitStandArray[36] = 20; // 12th hr sitting
  DailySitStandArray[37] = 30; // 12th hr standing
  DailySitStandArray[38] = 10; // 12th hr absent
  //394041 424344 454647 494950
  DailySitStandArray[54] = 5; // 18th hr sitting
  DailySitStandArray[55] = 40; // 18th hr standing
  DailySitStandArray[56] = 15; // 18th hr absent
  // 575859 606162 636465 666768
  DailySitStandArray[69] = 40; // 23th hr sitting
  DailySitStandArray[70] = 10; // 23th hr standing
  DailySitStandArray[71] = 10; // 23th hr absent
*/
    Serial.println("End of - Void Setup "); 
} // void setup



void loop()
{

 // Serial.println("void loop starting ");

  //Serial.println("GPIO0 pin into an Digital INPUT"); // used to read push button state
  pinMode(PushButtonPin, FUNCTION_3);
  pinMode(PushButtonPin, INPUT); 

  PushButtonState = digitalRead(PushButtonPin);

  delay(10);  
  if (PushButtonState == LOW) {
      if (PrevPushButtonState == HIGH){ 
        Serial.println("Push Button Just pushed");

            //if (LocationWork == HIGH) { // Work use 

              if (DeskMovingUp == HIGH) {
                  // STOP ALL MOTORS!!!
                  
                  // Turn off UP Relay
                  delay(10);  
                  Serial.write(rel1OFF, sizeof(rel1OFF));
                  delay(10);
                  Serial.println("Operator Turning Relay#1 Off ...");
                  
                  //Serial.write(rel1OFF, sizeof(rel1OFF));
                  
                  // Turn off DOWN Relay
                  delay(10);  
                  Serial.write(rel2OFF, sizeof(rel2OFF));
                  delay(10);
                  Serial.println("Operator Turning Relay#2 Off ...");
                  //Serial.write(rel2OFF, sizeof(rel2OFF));

                  // Turn off UP Relay
                  delay(10);  
                  Serial.write(rel1OFF, sizeof(rel1OFF));
                  delay(10); 
                  Serial.println("Operator Turning Relay#1 Off ...");
                   //Serial.write(rel1OFF, sizeof(rel1OFF));
                  
                  // Turn off DOWN Relay
                  delay(10);  
                  Serial.write(rel2OFF, sizeof(rel2OFF));
                  delay(10);
                  Serial.println("Operator Turning Relay#2 Off ...");
                  //Serial.write(rel1OFF, sizeof(rel1OFF));
                  
                  DeskMovingUp = LOW;
                  DeskMovingDown = LOW;
              }
              else if (DeskMovingDown == HIGH) {                
                  // STOP ALL MOTORS!!!
                                              
                  // Turn off UP Relay
                  delay(10);  
                  Serial.write(rel1OFF, sizeof(rel1OFF));
                  delay(10); 
                  Serial.println("Operator Turning Relay#1 Off ...");
                   //Serial.write(rel1OFF, sizeof(rel1OFF));
                  
                  // Turn off DOWN Relay
                  delay(10);  
                  Serial.write(rel2OFF, sizeof(rel2OFF));
                  delay(10);
                  Serial.println("Operator Turning Relay#2 Off ...");
                  //Serial.write(rel1OFF, sizeof(rel1OFF));

                  // Turn off UP Relay
                  delay(10);  
                  Serial.write(rel1OFF, sizeof(rel1OFF));
                  delay(10); 
                  Serial.println("Operator Turning Relay#1 Off ...");
                   //Serial.write(rel1OFF, sizeof(rel1OFF));
                  
                  // Turn off DOWN Relay
                  delay(10);  
                  Serial.write(rel2OFF, sizeof(rel2OFF));
                  delay(10);
                  Serial.println("Operator Turning Relay#2 Off ...");
                  //Serial.write(rel1OFF, sizeof(rel1OFF));
                  
                  DeskMovingUp = LOW;
                  DeskMovingDown = LOW;
              }
              
              else if (SitStandState == "Standing" ){ //&& DeskMovingDown == LOW
                  // relay 1 on, relay 2 off = down, inverse = up. Both on or both off is stop
                  // Move desk down   
                  
                  DeskMovingUp = LOW;
                  DeskMovingDown = HIGH; 
                  
                  // Turn on #1 Relay
                  delay(10);
                  Serial.write(rel1ON, sizeof(rel1ON));
                  delay(10);
                  Serial.println("Operator Turning Relay#1 On ...");
                                    
                  // Turn off #2 Relay
                  delay(10);
                  Serial.write(rel2OFF, sizeof(rel2OFF)); 
                  delay(10);
                  Serial.println("Operator Turning Relay#2 Off ...");

                  // Turn on #1 Relay
                  delay(10);
                  Serial.write(rel1ON, sizeof(rel1ON));
                  delay(10);
                  Serial.println("Operator Turning Relay#1 On ...");
                                    
                  // Turn off #2 Relay
                  delay(10);
                  Serial.write(rel2OFF, sizeof(rel2OFF)); 
                  delay(10);
                  Serial.println("Operator Turning Relay#2 Off ...");
                  
                  
                  DeskMovingUp = LOW;
                  DeskMovingDown = HIGH;       
                  DeskSpeedCutOffCounter = 0; // dump the counter, desk isnt jammed 
                  DeskSpeedIntegrationCounter = 0; // make sure you get a complete clean count as desks starts to move
                  
                
              } // if standing 
            

            
             else if (SitStandState == "Sitting" ){ //&& DeskMovingUp == LOW
                  // Move desk up
                  // relay 1 on, relay 2 off = down, inverse = up. Both on or both off is stop
                  // Move desk down   
                 
                  DeskMovingUp = HIGH;
                  DeskMovingDown = LOW;

                   // Turn on #2 Relay
                  delay(10);
                  Serial.write(rel2ON, sizeof(rel2ON));
                  delay(10);
                  Serial.println("Operator Turning Relay#2 On ...");
                                    
                  // Turn off #1 Relay
                  delay(10);
                  Serial.write(rel1OFF, sizeof(rel1OFF)); 
                  delay(10);
                  Serial.println("Operator Turning Relay#1 Off ...");
                  
                   // Turn on #2 Relay
                  delay(10);
                  Serial.write(rel2ON, sizeof(rel2ON));
                  delay(10);
                  Serial.println("Operator Turning Relay#2 On ...");
                                    
                  // Turn off #1 Relay
                  delay(10);
                  Serial.write(rel1OFF, sizeof(rel1OFF)); 
                  delay(10);
                  Serial.println("Operator Turning Relay#1 Off ...");
                  
                  DeskMovingUp = HIGH;
                  DeskMovingDown = LOW; 
                  DeskSpeedCutOffCounter = 0; // dump the counter, desk isnt jammed 
                  DeskSpeedIntegrationCounter = 0; // make sure you get a complete clean count as desks starts to move
           
              
              }  // if sitting

           //} // if @ work
      } // if button == HIGH
   } // if prev button == LOW  


// check if desk has arrived at the desired height or jammed

/*
 *    DeskHeightDownLowerLimit = 2;
 *    DeskHeightDownStopLimit = 4;
      DeskHeightDownUpperLimit = 6;
      
      DeskHeightUpLowerLimit = 34;
      DeskHeightUpStopLimit = 40;
      DeskHeightUpUpperLimit = 42;
      DeskHeightOffset = 1;
 */

//if (LocationWork == HIGH) { // Work use 

              if (DeskMovingUp == HIGH) {
                  if (AverageValidReading >= DeskHeightUpStopLimit) {
                      
                      // Turn off both Relay
                      delay(10);
                      Serial.write(rel1OFF, sizeof(rel1OFF));
                      delay(10);
                       
                      Serial.write(rel2OFF, sizeof(rel2OFF));
                      delay(10);
                      Serial.println("Desk has arrived at the UP point Turning Relays Off ...");

                      // Turn off both Relay
                      delay(20);
                      Serial.write(rel2OFF, sizeof(rel2OFF));
                      delay(10);
                       
                      Serial.write(rel1OFF, sizeof(rel1OFF));
                      delay(10);
                      Serial.println("Desk has arrived at the UP point Turning Relays Off ...");
                      
                      DeskMovingUp = LOW;
                      
                      
                   } // if desk has arrived at the required height

 

               }// desk moving up

              if (DeskMovingDown == HIGH) {
                  if (AverageValidReading <= DeskHeightDownStopLimit) {
                     
                      
                      // Turn off both Relay
                      delay(10);
                      Serial.write(rel1OFF, sizeof(rel1OFF));
                      delay(10);
                      Serial.write(rel2OFF, sizeof(rel2OFF));
                      delay(10);
                      Serial.println("Desk has arrived at the Down point Turning Relays Off ...");

                      delay(20);
                      Serial.write(rel2OFF, sizeof(rel2OFF));
                      delay(10);
                       
                      Serial.write(rel1OFF, sizeof(rel1OFF));
                      delay(10);
                      Serial.println("Desk has arrived at the Down point Turning Relays Off ...");
                      
                      DeskMovingDown = LOW;
                   
                   } // if desk has arrived at the required height

                   

               } // desk moving down
               
//} // if at work   
  
      
  if (PushButtonState == HIGH) {
       if (PrevPushButtonState == LOW){ 
        Serial.println("Push Button Just released");
        
      }
  }      

   PrevPushButtonState = PushButtonState;   // remember prev state for next pass



   
  //Serial.println("Making GPIO pin 0 back into an Digital OUPUT"); // used to drive the piezo beeper or LED
  pinMode(PiezoPin, OUTPUT);


  PIRSensorState = digitalRead(PIRSensorPin);
  delay(100);
  

if (PIRSensorState == HIGH) {
   PersonPresentCounter = PersonPresentCounter + PersonPresentUpCount;
   if (PersonPresentCounter >= PersonPresentTerminalCount) {
      PersonPresentState = true;
      PersonPresentCounter = PersonPresentTerminalCount;
   }
    
}

if (PIRSensorState == LOW) {
   PersonPresentCounter = PersonPresentCounter - PersonPresentDownCount;
   if (PersonPresentCounter <= 0) {
      PersonPresentState = false;
      PersonPresentCounter = 0;
   }
   
}

//Serial.print (PersonPresentCounter);

  //PersonPresentState = !PIRSensorState; // allowing some timing/buffereing/contact bounce to happen here if needed...

  delay(100);
  if (PersonPresentState == true) {
    if (PrevPersonPresentState == false) {
      Serial.println("Person has just arrived");
    }
  }

  if (PersonPresentState == false) {
    if (PrevPersonPresentState == true) {
      Serial.println("Person has just left");
    }
  }

  PrevPersonPresentState = PersonPresentState; // edge detection of Person Present state

  

  measuredistance(); // take a ping
  //logdistance(); // log the result

  SubPenultimateReading = PenultimateReading; // rotate last 3 readings (displays on ProxSensor page)
  PenultimateReading = LastReading;
  LastReading = distance;

  // calculate desk speed
/*
float DeskSpeed = 0 ; //  mm/sec
//Movemt will be stopped if DeskSpeed falls below DeskSpeedCutOffRate threshold for DeskSpeedIntegrationInterval while DeskSpeedCutOffEnabled is 1 . 
//DeskSpeedIntegrationInterval sets the period between meausements and speed checks

int DeskSpeedCutOffRate = 2; // desk moves any slower than this, and movement will be stopped in case its jammed

bool DeskSpeedCutOffEnabled = 0; // defaults to off
int DeskSpeedCutOffCounter = 0;
int DeskSpeedIntegrationInterval = 5; // seconds?
int DeskSpeedIntegrationCounter = 0;
float LastDeskPosition = 0; //last desk postion measurement
*/

DeskSpeedIntegrationCounter = DeskSpeedIntegrationCounter +1;

if (DeskSpeedIntegrationCounter >= DeskSpeedIntegrationInterval) {
    DeskSpeedIntegrationCounter = 0; // Dump counter for next time
    
    DeskSpeed = (LastDeskPosition - LastReading) ; // 
    //DeskSpeed = -0.93; // this was a typical up speed reading
    Serial.print("DeskSpeed is ");
    Serial.print(String  (DeskSpeed));
         if (DeskMovingUp == HIGH) {
      // this is required as the abs function converts a float to an int as well as correcting the sign.
      //Moving up results in negative values of DeskSpeed
      //with int counter at 10 we get 1.41, 1.44, 1.43 going down
      //with int counter at 10 we get -0.93, -0.53, -1.39, -0.96 going up 
      // propose .5 as a speed limit?
      DeskSpeed = (DeskSpeed * -1);
      Serial.print("Corrected DeskSpeed is ");
      Serial.print(String  (DeskSpeed));
     }

    LastDeskPosition = LastReading ; // store for next pass



     if (DeskSpeedCutOffEnabled == 1 && ((DeskMovingUp == HIGH) || (DeskMovingDown == HIGH)))  {
        if ( DeskSpeed <  DeskSpeedCutOffRate) { // Check - is desk underspeed ? abs allows this to work on both up and down directions

                // Turn off both Relays
                delay(10);
                Serial.write(rel1OFF, sizeof(rel1OFF));
                delay(10);
                 
                Serial.write(rel2OFF, sizeof(rel2OFF));
                delay(10);
                Serial.println("Desk has JAMMED!! Turning Relays Off ...");

               // Turn off both Relays again
                delay(20);
                Serial.write(rel2OFF, sizeof(rel2OFF));
                delay(10);
                 
                Serial.write(rel1OFF, sizeof(rel1OFF));
                delay(10);
                Serial.println("Desk has JAMMED!! Turning Relays Off ...");

                DeskMovingUp = LOW;
                DeskMovingDown = LOW;
                
          
        } // is desk underspeed ?
      
     } // if speedcuttoff enabled and desk is moving

}



InvalidDistanceReading = true; // validity will be checked in if statements below each pass..
  
  //switch (distance) {
 if  (distance <= DeskHeightDownLowerLimit-1  ) {
  
    //case 0 ... DeskHeightDownLowerLimit-1: // desk is impossibly low 0-6
    
       InvalidDistanceReading = true;       
       DeskPostitionSmoothingLowCount = DeskPostitionSmoothingLowCount -1;
       DeskPostitionSmoothingHighCount = DeskPostitionSmoothingHighCount -1;
       
       if (DeskPostitionSmoothingLowCount <= 0) {
            DeskPostitionSmoothingLowCount =0;
       }
       if (DeskPostitionSmoothingHighCount <= 0) {
            DeskPostitionSmoothingHighCount =0;
       }
 }     
      
      
 else if ( (distance >= DeskHeightDownLowerLimit) && (distance <= DeskHeightDownUpperLimit ) ) {
 
    //case DeskHeightDownLowerLimit ... DeskHeightDownUpperLimit: // Desk Down 6-12
       InvalidDistanceReading = false; 

       DeskPostitionSmoothingLowCount = DeskPostitionSmoothingLowCount +1;
          if (DeskPostitionSmoothingLowCount >= DeskPostitionSmoothingLowTerminalCount) {
            DeskPostitionState = LOW;
            DeskPostitionSmoothingHighCount = 0;
          }
         if (DeskPostitionSmoothingLowCount >= 500) {
            DeskPostitionSmoothingLowCount = 500;
            
          }
 }            
 else if ( (distance >= DeskHeightDownUpperLimit+1) && (distance <= DeskHeightUpLowerLimit-1 ) ) {     

    //case DeskHeightDownUpperLimit+1 ... DeskHeightUpLowerLimit-1: // Desk Halfway 13-40

      InvalidDistanceReading = true;
       DeskPostitionSmoothingLowCount = DeskPostitionSmoothingLowCount -1;
       DeskPostitionSmoothingHighCount = DeskPostitionSmoothingHighCount -1;
       
       if (DeskPostitionSmoothingLowCount <= 0) {
            DeskPostitionSmoothingLowCount =0;
       }
       if (DeskPostitionSmoothingHighCount <= 0) {
            DeskPostitionSmoothingHighCount =0;
       }
 }
     
else if ( (distance >= DeskHeightUpLowerLimit) && (distance <= DeskHeightUpUpperLimit ) ) { 
    //case DeskHeightUpLowerLimit ... DeskHeightUpUpperLimit: // Desk Up 41-50
      
      InvalidDistanceReading = false;
     
      DeskPostitionSmoothingHighCount = DeskPostitionSmoothingHighCount +1;
          if (DeskPostitionSmoothingHighCount >= DeskPostitionSmoothingHighTerminalCount) {
            DeskPostitionState = HIGH;
            DeskPostitionSmoothingLowCount = 0;
          }
          if (DeskPostitionSmoothingHighCount >= 500) {
            DeskPostitionSmoothingHighCount = 500;
            
          }
}          
      
else if  (distance >= DeskHeightUpUpperLimit+1)  { 
    //case DeskHeightUpUpperLimit+1 ... 3000: // Desk is impossibly high

       DeskPostitionSmoothingLowCount = DeskPostitionSmoothingLowCount -1;
       DeskPostitionSmoothingHighCount = DeskPostitionSmoothingHighCount -1;
       
       if (DeskPostitionSmoothingLowCount <= 0) {
            DeskPostitionSmoothingLowCount =0;
       }
       if (DeskPostitionSmoothingHighCount <= 0) {
            DeskPostitionSmoothingHighCount =0;
       }
      InvalidDistanceReading = true;


}// if distance is above or below several threshholds
 

if (InvalidDistanceReading == false) {
  SubPenultimateValidReading = PenultimateValidReading; // rotate last 3 valid readings (displays on ProxSensor page and used in motor control)
  PenultimateValidReading = LastValidReading;
  LastValidReading = distance;

  AverageValidReading = (LastValidReading + PenultimateValidReading + SubPenultimateValidReading)/3;

}

  
 


if (DeskPostitionState == true) {
    if (PrevDeskPostitionState == false) {
      Serial.println("Desk has just gone up (Logic HIGH)");
      SitStandState = "Standing";
      DailyUpCounter = DailyUpCounter + 1;
      CurrentSittingMinutes = 0;
      AlexaNotifications = 0;
      CurrentAbsentMinutes = 0;
      CurrentAbsentHours = 0;
      CurrentAbsentDays = 0;
    }
  }

  if (DeskPostitionState == false) {
    if (PrevDeskPostitionState == true) {
      Serial.println("Desk has just gone down (Logic LOW)");
      SitStandState = "Sitting";
      DailyDownCounter = DailyDownCounter + 1;
      CurrentStandingMinutes = 0;
      AlexaNotifications = 0;
      CurrentAbsentMinutes = 0;
      CurrentAbsentHours = 0;
      CurrentAbsentDays = 0;
    }
  }

  PrevDeskPostitionState = DeskPostitionState; // edge detection of Desk Position state
     

  //Check if standing or sitting times are exceeded
  if (PersonPresentState == true){    // should stop sounder going off late at night and screwing up loop timing  
 


//if (LocationWork == HIGH) { // Work use 
      if (SitStandState == "Standing"){              
       if (CurrentStandingMinutes >= CurrentStandingMinutesAlarmThreshold) {
        //Serial.println("Turning LED On ...");
       digitalWrite(LEDPin, LOW);
       } else {
        //Serial.println("Turning LED Off ...");  
        digitalWrite(LEDPin, HIGH);
        }  
      }
     if (SitStandState == "Sitting"){ 
      if (CurrentSittingMinutes >= CurrentSittingMinutesAlarmThreshold) {
      //Serial.println("Turning LED On ...");
       digitalWrite(LEDPin, LOW);
       } else {
        //Serial.println("Turning LED Off ...");  
        digitalWrite(LEDPin, HIGH);
        }  
    }
//} // if LocationWork = HIGH     



    
  } // if person present
  
   //noTone (PiezoPin);

 

      // Count Standing / Sitting Time
      if (millis() >= (previousMillis)) {
        //Serial.print(" millis = ");
        //Serial.print(String (millis())); // takes about 1300 millis in dev, 431 in prod.
        previousMillis = previousMillis + 1000;
        //Serial.print(" prevmillis = ");
        //Serial.print(String (previousMillis));
        // should be here every second....
        if (PersonPresentState == true) {
          //counttime(workinghours,workingminutes,workingseconds); This function needs to return multiple values, doesnt work yet..
          workingseconds = workingseconds + 1;
          workingtotalseconds = workingtotalseconds + 1;
          if (workingseconds == 60)
          {
            workingseconds = 0;
            workingminutes = workingminutes + 1;
          }
          if (workingminutes == 60)
          {
            workingminutes = 0;
            workinghours = workinghours + 1;
          }
          if (SitStandState == "Standing") {
            //Serial.print ("Accumulating Standing Time");
            standingseconds = standingseconds + 1;
            standingtotalseconds = standingtotalseconds + 1;
            if (standingseconds == 60)
            {
              standingseconds = 0;
              standingminutes = standingminutes + 1;
              CurrentHourStandingMinutes = CurrentHourStandingMinutes + 1;
              CurrentStandingMinutes = CurrentStandingMinutes + 1;
            }
            if (standingminutes == 60)
            {
              standingminutes = 0;
              standinghours = standinghours + 1;
            }
            /*
              if (standinghours == 24)
              {
              standinghours = 0;
              }
              if (standinghours < 10)
              {
              //Serial.print("0");
              }
              //Serial.print (standinghours, DEC);
              //Serial.print ("h");
              if (standingminutes < 10)
              {
              //Serial.print("0");
              }
              //Serial.print (standingminutes,DEC);
              //Serial.print ("m");
              if (standingseconds < 10)
              {
              //Serial.print("0");
              }
              //Serial.print(standingseconds,DEC);
              //Serial.println("s");
            */
          } else {
            //Serial.print ("Accumulating Sitting Time");
            sittingseconds = sittingseconds + 1;
            sittingtotalseconds = sittingtotalseconds + 1;
            if (sittingseconds == 60)
            {
              sittingseconds = 0;
              sittingminutes = sittingminutes + 1;
              CurrentHourSittingMinutes = CurrentHourSittingMinutes + 1;
              CurrentSittingMinutes = CurrentSittingMinutes + 1;
            }
            if (sittingminutes == 60)
            {
              sittingminutes = 0;
              sittinghours = sittinghours + 1;
            }
            /*
              if (sittinghours == 24)
              {
              sittinghours = 0;
              }
              if (sittinghours < 10)
              {
              //Serial.print("0");
              }
              //Serial.print (sittinghours, DEC);
              //Serial.print ("h");
              if (sittingminutes < 10)
              {
              // Serial.print("0");
              }
              //Serial.print (sittingminutes,DEC);
              //Serial.print ("m");
              if (sittingseconds < 10)
              {
              // Serial.print("0");
              }
              //Serial.print(sittingseconds,DEC);
              //Serial.println("s");
            */
          }




          //calculate percentages - this calc will cause  run time crashes without the if's in place. Divide by zero...
          // val1 = (val2 / 255.0) * 100; // does this get around using a byte to hold the ansqwer?
          if (workingtotalseconds > 0) {
            dailystandingpercentage = round((standingtotalseconds * 100) / workingtotalseconds);
          }
          if (workingtotalseconds > 0) {
            dailysittingpercentage = round((sittingtotalseconds * 100) / workingtotalseconds);
          }


        } else { // is PersonPresentState
          // here if personAbsent
          // increment absent minutes
          AbsentSeconds = AbsentSeconds + 1;
          if (AbsentSeconds == 60)
          {
            AbsentSeconds = 0;
            CurrentAbsentMinutes = CurrentAbsentMinutes + 1;
            CurrentHourAbsentMinutes = CurrentHourAbsentMinutes + 1;
          }
          if (CurrentAbsentMinutes == 60)
          {
            CurrentAbsentMinutes = 0;
            CurrentAbsentHours = CurrentAbsentHours + 1;

          }
          if (CurrentAbsentHours == 24)
          {
            CurrentAbsentHours = 0;
            CurrentAbsentDays = CurrentAbsentDays + 1;
          }
        }




        // Maintain RTC and do archive functions

        currentseconds = currentseconds + 1;
        if (currentseconds == 60)
        {
          currentseconds = 0;
          currentminutes = currentminutes + 1;
          //Serial.println("another minute has passed");
        }
        if (currentminutes == 60)
        {
          /*
            // Put current hours worth of standing and sittting minutes into the array

            DailySitStandArrayIndex = (currenthours * 2); // work out the array index - 2 values stored per hour
            DailySitStandArray[DailySitStandArrayIndex] = CurrentHourSittingMinutes;
            Serial.print("Writing Sitting mins to Array ");
            Serial.println(DailySitStandArrayIndex);
            Serial.println(CurrentHourSittingMinutes);
            DailySitStandArray[(DailySitStandArrayIndex + 1)] = CurrentHourStandingMinutes;
            Serial.print("Writing Standing mins to Array ");
            Serial.println(DailySitStandArrayIndex + 1);
            Serial.println(CurrentHourStandingMinutes);
          */

         
          CurrentHourStandingMinutes = 0;
          CurrentHourSittingMinutes = 0;
          CurrentHourAbsentMinutes = 0;

          currentminutes = 0;
          currenthours = currenthours + 1;
          //resync the clock every hour

        }
        if (currenthours == 24)
        {
          currentseconds = 0;
          currentminutes = 0;
          currenthours = 0;
         // SetTime (); // resync the clock

          // Dump daily stats
          standingseconds = 0;
          standingminutes = 0;
          standinghours = 0;
          standingtotalseconds = 0;

          sittingseconds = 0;
          sittingminutes = 0;
          sittinghours = 0;
          sittingtotalseconds = 0;

          workingseconds = 0;
          workingminutes = 0;
          workinghours = 0;
          workingtotalseconds = 0;

          DailyUpCounter = 0;
          DailyDownCounter = 0;

          //  could extend this to days/week if required
          // do archive here?
          // test for day start / end here?
        }

      } // end 1 second
      
    } // end void loop


    // connect to wifi – returns true if successful or false if not
    boolean connectWifi() {
      boolean state = true;
      int i = 0;

      WiFi.mode(WIFI_STA);
      WiFi.begin(RunTimessid, RunTimepassword);
      Serial.println("");
      Serial.println("Connecting to WiFi Network");

      Serial.println(RunTimessid);

      // Wait for connection
      Serial.print("Connecting ...");
      while (WiFi.status() != WL_CONNECTED) {
        delay(2000);
        Serial.print(".");
        if (i > 10) {
          state = false;
          break;
        }
        i++;
      }

      if (state) {
        Serial.println("");
        Serial.print("Connected to ");
        Serial.println(RunTimessid);
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
      }
      else {
        Serial.println("");
        Serial.println("Connection failed. Bugger");
      }

      return state;
    }

    void measuredistance() {
      delay(100); //pause to stabilise

      /* The following trigPin/echoPin cycle is used to determine the
        distance of the nearest object by bouncing soundwaves off of it.
        The ESP-01 running on a USB adaptor gets confused easily by power supply noise
        with this code. Beware!
        especally important to run the ultrasonic sensor on its own PSU - it gets
        very septic running on the USB power as well. A grunty capacitor across
        its PSU pins helps as well.
        YMMV
      */
      Serial.print("~"); // this will trigger the ultrasonic to take a measurement. The ~ character chosen as its 1 bit set in a 8 bit word

      duration = pulseIn(echoPin, HIGH);
      //Calculate the distance (in cm) based on the speed of sound.
      distance = duration / 58.2;

    }

    void logdistance() {
      if (distancereportcounter < 12) {
        Serial.print (distance, DEC);
        Serial.print("-");
        Serial.print ("W");
        Serial.print(WallDetectedStateCount);
        Serial.print ("P");
        Serial.print(PersonDetectedStateCount);
        Serial.print (" ");
        //Serial.print (distancereportcounter);
        distancereportcounter = distancereportcounter + 1;
      } else {
        distancereportcounter = 0;
        Serial.println  (distance, DEC);
        Serial.print("-");
        Serial.print ("W");
        Serial.print(WallDetectedStateCount);
        Serial.print ("P");
        Serial.print(PersonDetectedStateCount);
        Serial.print (" ");
      }
    }
    /*
      void PrintRectangle(int RectWidth,int RectHeight,String RectColor) {

      client.print("<div id=\"genericrectangle\" style=\"display: inline-block; width:"+ String(RectWidth) +"px; height:"+ String(RectHeight) +"px; background-color:"+ (RectColor)+"\"></div>");
      }
    */


    int counttime(int hours, int minutes, int seconds) { // This function needs to return multiple values, doesnt work yet..

      seconds = seconds + 1;
      if (seconds == 60)
      {
        seconds = 0;
        minutes = minutes + 1;
      }
      if (minutes == 60)
      {
        minutes = 0;
        hours = hours + 1;
      }

    }



    void myTone(byte pin, uint16_t frequency, uint16_t duration) // not used in current implementation , could be removed?
    { // input parameters: Arduino pin number, frequency in Hz, duration in milliseconds
      unsigned long startTime = millis();
      unsigned long halfPeriod = 1000000L / frequency / 2;
      //pinMode(pin,OUTPUT);
      while (millis() - startTime < duration)
      {
        //Serial.print("-");
        digitalWrite(pin, HIGH);
        delayMicroseconds(halfPeriod);
        digitalWrite(pin, LOW);
        delayMicroseconds(halfPeriod);
      }
      //pinMode(pin,INPUT);
    }

    void NotifyAlexa () {
      // Connect to remote server
      Serial.print("connecting to ");
      Serial.println(VirtualButtonsHost);
      if (!client.connect(VirtualButtonsHost, httpsPort)) {
        Serial.println("connection failed");
        return;
      }

      // Verify validity of server's certificate
      if (client.verifyCertChain(VirtualButtonsHost)) {
        Serial.println("Server certificate verified");
      } else {
        Serial.println("ERROR: certificate verification failed!");
        return;
      }

      String url = "/v1?virtualButton=1&accessCode=" + access_code;
      Serial.print("requesting URL: ");
      Serial.println(url);

      client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                   "Host: " + VirtualButtonsHost + "\r\n" +
                   "User-Agent: BuildFailureDetectorESP8266\r\n" +
                   "Connection: close\r\n\r\n");

      Serial.println("request sent");
      while (client.connected()) {
        String line = client.readStringUntil('\n');
        if (line == "\r") {
          Serial.println("headers received");
          break;
        }
      }
      /*
        String line = client.readStringUntil('\n');
        Serial.println("reply was:");
        Serial.println("==========");
        Serial.println(line);
        Serial.println("==========");
        Serial.println();
      */

    }

    void SetTime () {
      // Synchronize time using SNTP. This is necessary to verify that
      // the TLS certificates offered by the server are currently valid. Also used by the graph plotting routines
      Serial.println("Setting time using SNTP");
      configTime(-13 * 3600, DSTOffset, "pool.ntp.org", "time.nist.gov"); // set localtimezone, DST Offset, timeservers, timeservers...


      time_t now = time(nullptr);
      //while (now < 8 * 3600 * 2) { // would take 8 hrs to fall thru. Thats a long time....
      while (now < 100) {
        delay(500);
        Serial.print(".");
        Serial.print(String (now));
        now = time(nullptr);
      }
      Serial.println("");


      struct tm * timeinfo; //http://www.cplusplus.com/reference/ctime/tm/
      time(&now);
      timeinfo = localtime(&now);
      Serial.println(timeinfo->tm_hour);
      Serial.println(timeinfo->tm_min);
      Serial.println(timeinfo->tm_sec);
      currentseconds = timeinfo->tm_sec ;
      currentminutes = timeinfo->tm_min;
      currenthours = timeinfo->tm_hour ;

    }

    void PlotSSABlob() {

      int SubIndex = (DailySitStandArrayIndex % 3 );
      Serial.print("DailySitStandArrayIndex is ");
      Serial.println(DailySitStandArrayIndex);

      Serial.println("SubIndex is ");
      Serial.print(String(SubIndex));


      if (SubIndex == 0) {
        // here if its sitting minute value (divisable by 3 with 0 remainder)
        Serial.println("Plotting sitting ");

        // check if plotting today or yesterday , add "ghost" to colorname if the index is > current hrs
        if ((DailySitStandArrayIndex / 3) < currenthours) {
          RectColor = SittingColor;
        } else {
          RectColor = GhostSittingColor;
        }
      }

      if (SubIndex == 1) {
        // here if standing reading (
        Serial.println("Plotting standing ");

        // check if plotting today or yesterday , add "ghost" to colorname if the index is > current hrs
        if ((DailySitStandArrayIndex / 3) < currenthours) {
          RectColor = StandingColor;
        } else {
          RectColor = GhostStandingColor;
        }

      }

      if (SubIndex == 2) {
        // here if absent reading
        Serial.println("Plotting absent  ");

        // check if plotting today or yesterday , add "ghost" to colorname if the index is > current hrs
        if ((DailySitStandArrayIndex / 3) < currenthours) {
          RectColor = AbsentColor;
        } else {
          RectColor = GhostAbsentColor;
        }
      }


      RectHeight = DailySitStandArray[DailySitStandArrayIndex];
      client.print("<div id=\"genericrectangle\" style=\"display: inline-block; width:" + String(RectWidth) + "px; height:" + String(RectHeight) + "px; background-color:" + (RectColor) + "\"></div>");

    }

    
void SaveSSIDFunction()
{
  Serial.println("savessid ran!");
  Serial.println(String(Newssid));
  Serial.println(String(Newpassword));

 // client.println("<input value=\"\" id=\"Newssid\" placeholder=\"SSID\"> <input type=\"password\" value=\"\" id=\"Newpassword\" placeholder=\"password\">");
};
 

