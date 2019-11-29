//only getting HC05 to work when baud is 9600. maybe particular to an HC05 setting when being configed in AT mode???
//Note When RTC not connected, script pauses on any RTC lines, e.g., rtc.getTemp




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
int RECV_PIN = 7;     // IR receive pin # 7
IRrecv irrecv(RECV_PIN);
decode_results results;

//USER SETTING VARIABLES
//These variables are for initialization only! Use setHour, schTemp, or schPeriod

int wakeHour = 7;
int leaveHour = 15;
int returnHour = 22;
int bedHour = 23;

int wakeTemp = 74;
int leaveTemp = 45;
int returnTemp = 72;
int bedTemp = 60;

bool autoMode = false; // run the schedule or keep it at hold temp?
int setTemp = 74;   //variable holding the current target temperature (either manual or scheduled); set here to 72 by default

//OTHER VARIABLES

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
//bluetooth
char BTChar; // to use... BTChar = Serial.read() // which will add one character to
String BTString; // to consolidate Serial.read characters, use 'BTString += BTChar'
int BTNum;

// ==============================================================================
// ==============================================================================
//FUNCTIONS

void receiveBT() {

  BTString = ""; //clears the BTString var
  BTNum = 0;

  while (Serial.available() > 0) {
    BTChar = Serial.read();
    BTString += BTChar; // adds one character at a time to BTString
    delay(10); //this delay exists because the arduino will cycle faster than the serial buffer empties, appearing to empty the buffer when some is there
  }
  //get the digits in the 3rd and 4th position
  BTNum = BTString.substring(2, 4).toInt();
   //if (BTNum != 0) Serial.println(BTNum);
} //end receiveBT

void BTCommands() {

  if (BTString == "hello") Serial.println("back atcha");

  if (BTString == "reset") {
    wakeHour = 7; leaveHour = 15; returnHour = 22; bedHour = 23; setTemp = 74; wakeTemp = 74; leaveTemp = 45; returnTemp = 72; bedTemp = 60; autoMode = 0;
    Serial.println("reset Complete");
  }

  if (BTString.substring(0, 2) == "lv") {
    leaveHour = hour - 1;
    returnHour = BTNum;
    Serial.println(String(hour));
    Serial.println("leave now: " + String(leaveHour) + ". Return: " + String (returnHour));
  }

  if (BTString == "list") {
    Serial.println("wakeHour: " + String(wakeHour) + "// wakeTemp: " + String(wakeTemp));
    Serial.println("leaveHour: " + String(leaveHour) + "// leaveTemp: " + String(leaveTemp));
    Serial.println("returnHour: " + String(returnHour) + "// returnTemp: " + String(returnTemp));
    Serial.println("bedHour: " + String(bedHour) + "// bedTemp: " + String(bedTemp));
  }

  if (BTString == "mm") {
    autoMode = 0;
    Serial.println("automode = 0");
  }
  if (BTString == "am") {
    autoMode = 1;
    Serial.println("automode = 1");
  }

  // SET Period Times
  if (BTString.substring(0, 2) == "wh") {
    wakeHour = BTNum;
    Serial.println("wakeHour set to: " + String(wakeHour));
  }
  if (BTString.substring(0, 2) == "lh") {
    leaveHour = BTNum;
    Serial.println("leaveHour set to: " + String(leaveHour));
  }
  if (BTString.substring(0, 2) == "rh") {
    returnHour = BTNum;
    Serial.println("returnHour set to: " + String(returnHour));
  }
  if (BTString.substring(0, 2) == "bh") {
    bedHour = BTNum;
    Serial.println("bedHour set to: " + String(bedHour));
  }

  // Sets period temps
  if (BTString.substring(0, 2) == "wt") {
    wakeTemp = BTNum;
    Serial.println("wakeTemp set to: " + String(wakeTemp));
  }
  if (BTString.substring(0, 2) == "lt") {
    leaveTemp = BTNum;
    Serial.println("leaveTemp set to: " + String(leaveTemp));
  }
  if (BTString.substring(0, 2) == "rt") {
    returnTemp = BTNum;
    Serial.println("returnTemp set to: " + String(returnTemp));
  }
  if (BTString.substring(0, 2) == "bt") {
    bedTemp = BTNum;
    Serial.println("bedTemp set to: " + String(bedTemp));
  }


  //if(BTString.indexOf("wh") >= 0) Serial.println("Search worked!"); // this is how to search a string!








} // end BTCommands

void checkTime () {
  if (hour >= setHour[0] && hour < setHour[1])  { //wake
    //Serial.print("\nHour: " + String(hour) + " running wake cycle");
  }
  if (hour >= setHour[1] && hour < setHour[2]) { //leave
    //Serial.print("\nHour: " + String(hour) + " running leave cycle");
  }
  if (hour >= setHour[2] && hour < setHour[3]) { //return
    //Serial.print("\nHour: " + String(hour) + " running return cycle");
  }
  if (hour >= setHour[3] || hour < setHour[0]) { //bed
    //Serial.print("\nHour: " + String(hour) + " running bed cycle");
  }
} //end check time

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
  readTemp = rtc.getTemp() * 9 / 5 + 32; // refreshes F temperature variable
}
// end updateTimeVars

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

void leftButton() {
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
      if (setTemp < 61)
        setTemp = 72;
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
  if (currentMillis - lastPress < timeOutLen) {  // has more than 5 seconds gone by without a button press? then show user placed cursor
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

void readRemote() {

  // IR REMOTE BUTTONS

  if (irrecv.decode(&results)) { // the function irrecv.decode will return true if a code is received and the program will execute the code in the if statement
    if (results.value == 0XFFFFFFFF) // this and the next 3 lines make clicks more recognizable (in this order)
      results.value = keyValue;

    irrecv.resume();

    if (results.value == 0xFF629D) { // upbutton. note the 0x which cues the processor to expect hexadecimal
      //Serial.print("\nup");
      upButton();
      //delay(100);
    } //end up

    if (results.value == 0xFFA857) { //down button
      //Serial.print("\ndown");
      downButton();
      //delay(100);
    } //end down

    if (results.value == 0xFF22DD) { //left Button
      //Serial.print("\nleft");
      leftButton();
    } //end left

    if (results.value == 0xFFC23D ) { //right Button
      //Serial.print("\nright");
      rightButton();
    } //end right button

    if (results.value == 0xFFFFFFFF) { //repeat
      //Serial.print("\nFFFFFFFF");
    } // end repeat button
  } //end if irrecv
}

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

void manAutoMode () {
  // to run manual or auto mode
  if (autoMode == 1 && currentMillis - lastPress > 5000)
    runSchedule();

  if (readTemp < setTemp)
    setHeater(1);
  if (readTemp > setTemp)
    setHeater(0);
} //end manAutoMode

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
  //rtc.setDOW(THURSDAY);     // Set Day-of-Week to SUNDAY (all caps)
  //rtc.setTime(12, 9, 0);     // Set the time to 12:00:00 (24hr format; no insignificant zeroes)
  //rtc.setDate(14, 3, 2019);   // Set the date in this format (dd, mm, yyyy)

  //HC05 Bluetooth initialization test

  Serial.println("Arduino ready");

} //end setup

//==============================================================================
//==============================================================================
//==============================================================================
// SCRIPT LOOP

void loop()
{
  currentMillis = millis();
  //updateTimeVars // comment out if RTC is disconnected
  posTracker();                           //checks cursorPos and directs program to appropriate menu
  checkTime();
  readRemote();
  manAutoMode();
  receiveBT();
  BTCommands();
  delay(1000);
}


//end void loop

// ------------------------------------------------------------------------------
/* JUNK AND RECYCLED CODE

*/
