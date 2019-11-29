/*In version 1.5
  goals:
  xrecognize codes better by storing the immediate result in a variable (see IR_getHex)
  -any of the number buttons can be used to set the return time
*/

//==============================================================================
// LIBRARIES
#include <Servo.h>
#include <LiquidCrystal_I2C.h>
#include <DS3231.h>
#include <IRremote.h>


// OBJECTS
Servo myservo;  // create servo object to control a servo; see later for use myservo.attach(9)
LiquidCrystal_I2C lcd(0x27, 20, 4); // set the LCD address to 0x27 for a 16 chars and 2 line display module
DS3231  rtc(SDA, SCL); // Init the DS3231 RTC using the hardware interface
Time  t; // Init a Time-data structure for RTC.. (or is this an object?)
const int RECV_PIN = 7;     // IR receive pin # 7
IRrecv irrecv(RECV_PIN);
decode_results results;

//USER SETTING VARIABLES
//These variables are for initialization only! Use setHour, schTemp, or schPeriod
int wakeHour = 7;
int leaveHour = 15;
int returnHour = 22;
int bedHour = 24;

bool autoMode = false; // run the schedule or keep it at hold temp?
int setTemp = 74;   //variable holding the current target temperature (either manual or scheduled); set here to 72 by default
int wakeTemp = 74;
int leaveTemp = 45;
int returnTemp = 72;
int bedTemp = 60;

//OTHER VARIABLES
long unsigned int IRCode; // stored IRcode
int timeOutLen = 10000; //length of time in seconds before LCD screen goes to main menu
unsigned long dispRefreshRate = 1000; //this is how often I want the lcd to update. set to 1 second to align with second updates
bool printLCDNow; // a state-holder dummy variable that's set to 1 upon user input. Allows an immediate refresh of LCD from printLCD()
int readTemp; // stores the thermometer temperature
int hour;
int min;
int sec;
int maxServo = 180; // programmer-coded variable for how far fartherst position clockwise servo should travel
int minServo = 20;  // the user-coded variable for how far counter-clockwise the servo should travel
bool heaterState = 1; //the last SET heater-state from code.
bool scheduleMode; //a boolean variable that toggles between running the program schedule or sticking with the manual input.
String schPeriod[] = {"Wake", "Leave", "Return", "Bed"}; //to do later, can this be made a constant string?
int setHour[] = {wakeHour, leaveHour, returnHour, bedHour};
int schTemp[] = {wakeTemp, leaveTemp, returnTemp, bedTemp};
int cursorPos = 0; // sets tme & temp as initial display

unsigned long keyValue = 0;
unsigned long lastPress = 0;
unsigned long debounce = 0;
unsigned long currentMillis = 0;
unsigned long lastPrintLCD = 0;  // time (in ms) at which the LCD was last updated
int trigger = 0; // dummy counter variable

// ==============================================================================
//FUNCTIONS

void setHeater(bool x) { //boolean x value is whether heater is set to on or off
  if (x == true && heaterState != true) {
    myservo.write(maxServo);
    /*delay(300);
      myservo.write(maxServo - 7); //the upper limit for this servo minus 10 for not twitching
      delay(300);
      myservo.write(maxServo - 3); //the upper limit for this servo minus 10 for not twitching
      delay(300);
    */
    heaterState = true; // sets gobal variable heaterState to reflect most recent change: on
  }
  if (x == false && heaterState != false) {
    myservo.write(minServo); //the servo's out position corresponding with the heater's minimum setting
    /*delay(300);
      myservo.write(minServo + 5); //the servo's out position corresponding with the heater's minimum setting
      delay(300);
      myservo.write(minServo + 1); //the upper limit for this servo minus 10 for not twitching
    */
    heaterState = false; // sets gobal variable heaterState to reflect most recent change: off
  }
} //end setHeater


void updateTimeVars() {

  t = rtc.getTime(); // updates variable t with formatted time data
  hour = t.hour;
  min = t.min;
  sec = t.sec;
} // end updateTimeVars

void upButton() {
  printLCDNow = true; // switches a state-holder variable to true so the LCD updates immediately (not after 1 second)
  lastPrintLCD = currentMillis;
  lastPress = currentMillis;
  if (cursorPos > 0) //ensures doesn't go below zero
    cursorPos--;
  Serial.print("\nup ");
  Serial.print(cursorPos);
} //endUpbutton

void downButton() {
  printLCDNow = true;
  lastPrintLCD = currentMillis;
  lastPress = currentMillis; //the global variable for detecting user interaction. set to currentMillis at the end of use inputs
  if (cursorPos < 8) //ensures cursorPos doesn't go past more positions than there are menus to display
    cursorPos++;
  Serial.print("\ndown ");
  Serial.print(cursorPos);
} //end downButton

void leftButton () {
  printLCDNow = true;
  lastPrintLCD = currentMillis;
  lastPress = currentMillis;
  if (cursorPos == 0)            //checks to see if it is at the home screen
    autoMode = false;           //if it is, disables the scheduler and runs temperature hold
  if (cursorPos != 0)           //checks to see if it is at the home screen
    autoMode = true;            //if it is, disables the scheduler and runs temperature hold
  switch (cursorPos) {
    case 0: //when cusor is at the home screen
      //do later: set schedule/manual variable to manual mode; set-up if statement to run manual/runSchedule
      setTemp--;
      break;
    case 1:
      setHour[0]--;
      break;
    case 2:
      schTemp[0]--;
      break;
    case 3:
      setHour[1]--;
      break;
    case 4:
      schTemp[1]--;
      break;
    case 5:
      setHour[2]--;
      break;
    case 6:
      schTemp[2]--;
      break;
    case 7:
      setHour[3]--;
      break;
    case 8:
      bedTemp--;
      schTemp[3]--;
      break;
  } //end switch


} //end left button

void rightButton () {
  printLCDNow = true;
  lastPrintLCD = currentMillis;
  lastPress = currentMillis;
  if (cursorPos == 0)            //checks to see if it is at the home screen
    autoMode = false;           //if it is, disables the scheduler and runs temperature hold
  if (cursorPos != 0)           //checks to see if it is at the home screen
    autoMode = true;            //if it is, disables the scheduler and runs temperature hold

  switch (cursorPos) {
    case 0: //when cusor is at the home screen
      //do later: set schedule/manual variable to manual mode; set-up if statement to run manual/runSchedule
      setTemp++;
      break;
    case 1:
      setHour[0]++;
      break;
    case 2:
      schTemp[0]++;
      break;
    case 3:
      setHour[1]++;
      break;
    case 4:
      schTemp[1]++;
      break;
    case 5:
      setHour[2]++;
      break;
    case 6:
      schTemp[2]++;
      break;
    case 7:
      setHour[3]++;
      break;
    case 8:
      bedTemp++;
      schTemp[3]++;
      break;
  } //end switch


} //end right button


void posTracker() { // checks the cursor position and displays the corresponding menu
  //setHour = {wakeHour, leaveHour, returnHour, bedHour}; //these variables declared at beginning and updated with button press; updated here too
  //schTemp = {wakeTemp, leaveTemp, returnTemp, bedTemp};
  if (currentMillis - lastPress < timeOutLen) {  // has more than 5 seconds gone buy without a button press? then show user placed cursor
    switch (cursorPos) {
      case 0:
        //Serial.print("cursorPos = 0");    //despite the else if below, this one gets shown because the user can scroll back to this "Main display": time & temp/set temp
        printLCDHome();
        //allow manual override from this location
        break;
      case 1:                           // each case checks the cursor position, i.e. the root variable which is passed as an argument into other display functions
        //Serial.print("cursorPos = 1");
        printLCDSet(0);       // 0 = wake   // two rows of printLCDSet(x) per cursor position
        break;
      case 2:
        //Serial.print("cursorPos = 2");
        printLCDSet(0);
        break;
      case 3:
        //Serial.print("cursorPos = 3");
        printLCDSet(1); // 1 = leave
        break;
      case 4:
        // Serial.print("cursorPos = 4");
        highlight(cursorPos);
        printLCDSet(1);
        break;
      case 5:
        //Serial.print("cursorPos = 5");
        highlight(cursorPos);
        printLCDSet(2); //  2 return
        break;
      case 6:
        //Serial.print("cursorPos = 6");
        highlight(cursorPos);
        printLCDSet(2);
        break;
      case 7:
        //Serial.print("cursorPos = 7");
        highlight(cursorPos);
        printLCDSet(3); // 3 = bed
        break;
      case 8:
        //Serial.print("cursorPos = 8");
        highlight(cursorPos);
        printLCDSet(3);
        break;
      case 9:
        //Serial.print("cursorPos = 9");
        highlight(cursorPos);
        printLCDSet(4); // 4 = bed
        break;
      case 10:
        // Serial.print("cursorPos = 10");
        highlight(cursorPos);
        printLCDSet(4);
        break;
    } //end switch
  } //end if
  else if (currentMillis - lastPress >= timeOutLen) {
    cursorPos = 0;
    printLCDHome();
  } //end else if
} // end posTracker()


void highlight(int x) {
  if (x % 2 == 1) {// remainder is odd, goes on 1st line
    lcd.setCursor(7, 0);
    lcd.print("<<");
    lcd.setCursor(14, 0);
    lcd.print(">>");
  }

  if (x % 2 == 0) {// if remainder of 2 into x (i.e., cursorPos) is even, then cursor is on 2nd line
    lcd.setCursor(7, 1);
    lcd.print("<<");
    lcd.setCursor(14, 1);
    lcd.print(">>");
  }

} //end highlight


void runSchedule() {
  //int setHour[] = {wakeHour, leaveHour, returnHour, bedHour}; DO NOT UNCOMMENT - REFERENCE ONLY
  //int schTemp[] = {wakeTemp, leaveTemp, returnTemp, bedTemp}; DO NOT UNCOMMENT - REFERENCE ONLY

  if (hour >= setHour[0] && hour < setHour[1])  { //wake
    setTemp = schTemp[0];
  }
  if (hour >= setHour[1] && hour < setHour[2]) { //leave
    setTemp = schTemp[1];
  }
  if (hour >= setHour[2] && hour < setHour[3]) { //return
    setTemp = schTemp[2];
  }
  if (hour >= setHour[3] || hour < setHour[0]) { //bed
    setTemp = schTemp[3];
  }

} //end runSchedule


void printLCDHome() {
  //instructions to display time and temperature to LCD display
  if (currentMillis - lastPrintLCD > dispRefreshRate || printLCDNow == true ) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Time: ");
    lcd.setCursor(7, 0);
    if (t.hour < 10)        //these if statements show the zero place
      lcd.print("0"); //just in case it's not between 10 & 12
    if (t.hour >= 10)
      lcd.setCursor(7, 0);
    lcd.print(t.hour);
    lcd.print(":");
    if (t.min < 10)
      lcd.print("0");
    if (t.min >= 10)
      lcd.setCursor(10, 0);
    lcd.print(t.min);
    lcd.print(":");
    if (t.sec < 10)
      lcd.print("0");
    if (t.sec >= 10)
      lcd.setCursor(13, 0);
    lcd.print(t.sec);
    //convert to Farenheit
    lcd.setCursor(0, 1);
    lcd.print("Degrees F ");
    lcd.print(readTemp);
    lcd.print("/");
    lcd.print(setTemp);
    lastPrintLCD = currentMillis;
    printLCDNow = false;
    //Serial.print("end printLCDHome");
  }
} // end printLCDHome

void printLCDSet(int x) {
  //This function clears the lcd, moves the cursor, and prints data from variables assigned from other functions
  if (currentMillis - lastPrintLCD > dispRefreshRate || printLCDNow == true ) {
    lcd.clear();
    highlight(cursorPos); //checks cursor position and prints bracket on row 0 or 1 at column positions 7 (<<) and 14 (>>)
    lcd.setCursor(0, 0);
    lcd.print(schPeriod[x]); // prints the period at array position x in schPeriod which is the array that contains the strings (e.g., 0 = wake, 1 = );
    lcd.print(":");
    lcd.setCursor(9, 0); //moves lcd cursor to column (x,-) of row (-,y)
    if (setHour[x] < 10) // I think this just prints a 0 so it doesn't look weird. gets overwritten if the value in in setHour is 2 digits
      lcd.print("0");
    lcd.print(setHour[x]);
    lcd.print(":00");
    lcd.setCursor(0, 1);
    lcd.print("Temp:");
    lcd.setCursor(9, 1);
    lcd.print(schTemp[x]); //looks in schTemp Array position x and prints the corresponding string for the period (e.g. wake leave etc)
    lcd.print("  F");
    lastPrintLCD = currentMillis;
    printLCDNow = false;
    //Serial.print("end printLCDSet");
  } //end fsm time check
} //end printLCDSet

void timeChecker () {
  if (hour >= setHour[0] && hour < setHour[1]); //Serial.print("\nHour: " + String(hour) + " running wake cycle");
  if (hour >= setHour[1] && hour < setHour[2]); //Serial.print("\nHour: " + String(hour) + " running leave cycle");
  if (hour >= setHour[2] && hour < setHour[3]); //Serial.print("\nHour: " + String(hour) + " running return cycle");
  if (hour >= setHour[3] || hour < setHour[0]); //Serial.print("\nHour: " + String(hour) + " running bed cycle");
} // end timeChecker

void IRReceive() {
  IRCode = 0;
  if (irrecv.decode(&results)) {
    IRCode = results.value;
    irrecv.resume();
  } //end irrecv

  if (IRCode == 16736925 || IRCode == 5316027) upButton();
  if (IRCode == 16754775 || IRCode == 2747854299) downButton();
  if (IRCode == 16720605 || IRCode == 1386468383) leftButton();
  if (IRCode == 16761405 || IRCode == 553536955) rightButton();
  if (IRCode == 16712445 || IRCode == 3622325019){ // okay button
  
  }
} // end IRReceive

// ==============================================================================
// ==============================================================================
// SETUP
void setup() {

  irrecv.enableIRIn();
  //irrecv.blink13(true);

  myservo.attach(8); //assigns servo PWM control to pin 9
  myservo.write(maxServo);
  lcd.init();                      // initialize the lcd
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(3, 0); //(column, row)
  lcd.print("code name:");
  lcd.setCursor(0, 1);
  lcd.print("PROJECT  SPARTAN");

  // RTC related
  Serial.begin(9600);
  rtc.begin(); // Initialize the rtc object
  // The following lines can be uncommented to set the date and time
  //rtc.setDOW(FRIDAY);     // Set Day-of-Week to SUNDAY
  //rtc.setTime(15, 02, 0);     // Set the time to 12:00:00 (24hr format)
  //rtc.setDate(22, 3, 2019);   // Set the date in this format (dd, mm, yyyy)

  delay(500);

} //end setup

//==============================================================================
//==============================================================================
//==============================================================================
// SCRIPT LOOP

void loop()
{

  // Loop updates
  currentMillis = millis();
  updateTimeVars();                       // updates the 3 time variables: hour, min, sec
  readTemp = rtc.getTemp() * 9 / 5 + 32; // refreshes F temperature variable
  posTracker();                           //checks cursorPos and directs program to appropriate menu
  timeChecker();
  IRReceive();

  // to run manual or auto mode
  if (autoMode == 1 && currentMillis - lastPress > 5000)
    runSchedule();

  if (readTemp < setTemp)
    setHeater(1);
  if (readTemp > setTemp)
    setHeater(0);

} //end void loop




// ------------------------------------------------------------------------------
/* JUNK AND RECYCLED CODE
  String schPeriod[] = {"Wake", "Leave", "Return", "Bed"};
  int setHour[] = {wakeHour, leaveHour, returnHour, bedHour};
  int schTemp[] = {wakeTemp, leaveTemp, returnTemp, bedTemp};
  int cursorPos = 0; // sets tme & temp as initial display

*/
