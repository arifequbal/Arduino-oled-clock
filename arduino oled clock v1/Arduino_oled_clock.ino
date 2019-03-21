#include <Arduino.h>
#include <U8g2lib.h>
#include "ds3231.h" //Real Time Clock library
#include <Wire.h>   //I2C communication library
#include <EEPROM.h> //This library allows reading and writing to the EEPROM


#define beeper //Uncomment if 5 volt continuous tone beeper or buzzer is connected to pin 10 (5 Hz output)

//Uncomment following line to enable display dimming between 10 PM and 5 AM
#define dimming
#include <Vcc.h>
const float VccMin   = 3.3;            // Minimum expected Vcc level, in Volts.
const float VccMax   = 4.16;           // Maximum expected Vcc level, in Volts.
const float VccCorrection = 4.99/4.86; // Measured Vcc by multimeter divided by reported Vcc

Vcc vcc(VccCorrection);

int framecount2 = 0; //Counter for number of display update periods
uint8_t secset = 0; //Index for second RTC setting
uint8_t minset = 1; //Index for minute RTC setting
uint8_t hourset = 2; //Index for hour RTC setting
uint8_t wdayset = 3; //Index for weekday RTC setting
uint8_t mdayset = 4; //Index for date RTC setting
uint8_t monset = 5; //Index for month RTC setting
uint8_t yearset = 6; //Index for year RTC setting

//Alarm time variables
uint8_t wake_HOUR = 0;
uint8_t wake_MINUTE = 0;
uint8_t wake_SECOND = 0;
uint8_t wake_SET = 1; //Default alarm to ON in case of power failure or reset

//Relay on time variable
uint8_t relay_HOUR = 0;
uint8_t relay_MINUTE = 0;
uint8_t relay_SECOND = 0;
 uint8_t relay = 0;

//rela off time variable
uint8_t relayoff_HOUR = 0;
uint8_t relayoff_MINUTE = 0;
uint8_t relayoff_SECOND = 0;
uint8_t relayoff = 0;

//torch control
uint8_t led_SET = 0;
int beepcounts = 600;
//unsigned wdelay = 60000;



unsigned long prev, interval = 100; //Variables for display/clock update rate
byte flash = 0; //Flag for display flashing - toggle once per update interval
byte mode = 0; //Mode for time and date setting
int tempset; //Temporary variable for setting time/date
int beepcount = 0; //Variable for number of 100ms intervals since alarm started sounding
const int alarmEE = 0; //EEPROM alarm status storage location
unsigned long previousMillis = 0;
const long intervals = 1000; 
/*
  U8glib Example Overview:
    Frame Buffer Examples: clearBuffer/sendBuffer. Fast, but may not work with all Arduino boards because of RAM consumption
    Page Buffer Examples: firstPage/nextPage. Less RAM usage, should work with all Arduino boards.
    U8x8 Text Only Example: No RAM usage, direct communication with display controller. No graphics, 8x8 Text only.
    
  This is a page buffer example.    
*/

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);


 
void setup() {
   
  u8g2.begin();
   //Wire.begin();  //Initialize I2C communication library
  
  
//DS3231_init(0x00); //Initialize Real Time Clock for 1Hz square wave output (no RTC alarms on output pin)

pinMode(8, INPUT); //Set pin for time/date mode button to input
digitalWrite(8, HIGH); //Turn on pullup resistors

pinMode(9, INPUT); //Set pin for time/date set button to input
digitalWrite(9, HIGH); //Turn on pullup resistors

pinMode(7, INPUT); //Set pin for time/date set button to input
digitalWrite(7, HIGH); //Turn on pullup resistors

//Set pin for weather mode button to input
pinMode(12, OUTPUT);//Turn on pullup resistors
digitalWrite(12, HIGH);


pinMode(6, OUTPUT); //Set pin for external alarm indicator output
digitalWrite(6, LOW);//Initialize external alarm to off state


pinMode(10, OUTPUT);
//digitalWrite(10, LOW);

pinMode(11, OUTPUT);
//digitalWrite(11, LOW);




//Read stored alarm set status and initialize to on at first run or on error  
wake_SET = EEPROM.read(alarmEE);
if (wake_SET != 0 && wake_SET != 1){
  wake_SET = 1;
  EEPROM.write(alarmEE, wake_SET);}

 
}

void loop() {
float v = vcc.Read_Volts();  
int p = vcc.Read_Perc(VccMin, VccMax);

char tempF[6]; //Local variable to store converted temperature reading from Real Time Clock module
float temperature; //Intermediate temperature variable to convert Celsius to Farenheit
unsigned long now = millis(); //Local variable set to current value of Arduino internal millisecond run-time timer
struct ts t; //Structure for retrieving and storing time and date data from real time clock

//Draw and update display every refresh period (100ms)
 
if ((now - prev > interval)) { //Determine whether to start a time and screen update
  framecount2 = framecount2 + 1; //Update counter of refresh periods
  if(framecount2 > 300){
    framecount2 = 0; //Wrap the refresh period counter to 0 after 300 updates, 
    mode = 0; //Reset mode to normal every cycle unless setting buttons pressed to reset cycle counter
    }
  if(flash == 0){flash = 1;}else{flash = 0;} //Toggle flash flag for cursor blinking later
  DS3231_get(&t); //Get time and date and save in t structure
  get_alarm(); //Retrieve current alarm setting
  
 #if defined(dimming)
  if(t.hour >= 22 || t.hour < 6){
    u8g2.setContrast(1);} //Dim the display between 10 PM and 5 AM
    else {u8g2.setContrast(255);} //Otherwise set display to full brightness
  #endif

  #if defined(beeper)
  digitalWrite(6, LOW);
  //digitalWrite(11, LOW);
  //Turn off external alarm every cycle - no effect if alarm not on
  #endif
  
  if((DS3231_get_addr(0x0E) & 0x20) == 0){DS3231_init(0x20);} //Check for CONV flag to see if conversion is in progress first, else start conversion
  temperature = DS3231_get_treg(); //Get temperature from real time clock
  temperature = temperature; // Convert Celsius to Fahrenheit
  dtostrf(temperature, 5, 1, tempF); //Convert temperature to string for display






  
 u8g2.firstPage();  
  do {  
  
  u8g2.setFontPosTop();
  u8g2.setFontDirection(0);

/* hourly alarm */
if((t.hour == 0 && t.min==0 && t.sec==0) ||(t.hour == 1 && t.min==0 && t.sec==0) || (t.hour == 2 && t.min==0 && t.sec==0) || (t.hour == 3 && t.min==0 && t.sec==0) || (t.hour == 4 && t.min==0 && t.sec==0) || (t.hour == 5 && t.min==0 && t.sec==0) || (t.hour == 6 && t.min==0 && t.sec==0) || (t.hour == 7 && t.min==0 && t.sec==0) || (t.hour == 8 && t.min==0 && t.sec==0) || (t.hour == 9 && t.min==0 && t.sec==0) || (t.hour == 10 && t.min==0 && t.sec==0) || (t.hour == 11 && t.min==0 && t.sec==0) || (t.hour == 12 && t.min==0 && t.sec==0) || (t.hour == 13 && t.min==0 && t.sec==0) || (t.hour == 14 && t.min==0 && t.sec==0) || (t.hour == 15 && t.min==0 && t.sec==0) || (t.hour == 16 && t.min==0 && t.sec==0) || (t.hour == 17 && t.min==0 && t.sec==0) || (t.hour == 18 && t.min==0 && t.sec==0) || (t.hour == 19 && t.min==0 && t.sec==0) || (t.hour == 20 && t.min==0 && t.sec==0) || (t.hour == 21 && t.min==0 && t.sec==0) || (t.hour == 22 && t.min==0 && t.sec==0) || (t.hour == 23 && t.min==0 && t.sec==0) ){ //Display/sound alarm if enabled and triggered
      beepcount = beepcount + 1;
      if(beepcount <= 2){ //Sound alarm for 60 seconds
         if(!flash){      //Flash display and sound interrupted beeper
       
       
        #if defined(beeper)
        digitalWrite(6, HIGH);  //Flash external alarm if alarm triggered, regardless of mode
        #endif
        }  
      }
      else{beepcount = 0;} //If alarm has sounded for 1 minute, reset alarm timer counter and alarm flag
  }




if(wake_SET && DS3231_triggered_a1()){ //Display/sound alarm if enabled and triggered
      beepcount = beepcount + 1;
      if(beepcount <= beepcounts){ //Sound alarm for 60 seconds
         if(!flash){      
        #if defined(beeper)
        digitalWrite(6, HIGH);  //Flash external alarm if alarm triggered, regardless of mode
        #endif
       }
      }
      else{beepcount = 0; DS3231_clear_a1f();} //If alarm has sounded for 1 minute, reset alarm timer counter and alarm flag
  }
if(mode <=7){
  if(relay_HOUR == t.hour && relay_MINUTE == t.min){
   digitalWrite(10, HIGH);}
   else if(relayoff_HOUR == t.hour && relayoff_MINUTE == t.min){ digitalWrite(10, LOW); }
  }

if((mode <=7) || (mode > 7 && mode <= 10 )){
       u8g2.setFont(u8g2_font_profont22_mr );
       u8g2.setCursor(90, 43);
       u8g2.print(":"); //Display minute-seconds separator
       if(t.sec<10){u8g2.print("0");} //Add leading zero for single-digit seconds
       u8g2.print(t.sec); //Display retrieved seconds
       //u8g2.drawStr(44,49,tempF);
       u8g2.setCursor(44,64);//Send temperature to display buffer
       u8g2.print(tempF);
       u8g2.drawCircle(105,49,2);  //Draw degree symbol after temperature
       //u8g2.drawStr(110,49,"C");
       u8g2.setCursor(110,64);
       u8g2.print("C");

      
        
            
                   
}  

  if(mode <=7){
       u8g2.setFont(u8g2_font_ncenB08_tf);
       u8g2.setCursor(100, 26 );
       if(t.hour < 12){
       u8g2.print(" AM");} //Display AM indicator, as needed
       else{u8g2.print(" PM");} //Display PM indicator, as needed

       if (mode <= 7){ //Alarm indicators and actions in normal and time set display mode only
       if (wake_SET){ //Display alarm on indicator if alarm turned on

       u8g2.setCursor(30,64);
       u8g2.print("ON");
       }
       }
  }
 if (mode > 7 && mode <= 10 ){
    u8g2.setFont(u8g2_font_ncenB08_tf);
    u8g2.setCursor(4, 12);
    u8g2.print("Alarm Set: ");
     
     if(wake_SET){u8g2.print("ON");}else{u8g2.print("OFF");}
            u8g2.setCursor(100, 26);
            if(wake_HOUR < 12){
              
             u8g2.print(" AM");} //Display AM indicator, as needed
             else{
             u8g2.print(" PM");} //Display PM indicator, as needed 
 }
  

if (mode <=7){
  u8g2.setFont(u8g2_font_calibration_gothic_nbp_tf );
  u8g2.setCursor(0,14); //Position cursor for day-of-week display
  printDay(t.wday); //Lookup day of week string from retrieved RTC data and write to display buffer
  u8g2.setCursor(33,14);
  printMonth(t.mon); //Lookup month string from retrieved RTC data and write to display buffer
        
  if(t.mday<10){  
    u8g2.print("0");} //Add leading zero to date display if date is single-digit
    u8g2.print(t.mday); //Write date to display buffer
    u8g2.print(", "); //Write spaces and comma between date and year
     u8g2.setCursor(91,14);   
    u8g2.print(t.year); //Write year to display buffer
    //u8g2.setFont(u8g2_font_ncenB18_tf);
    u8g2.setFont(u8g2_font_freedoomr25_tn);  
    u8g2.setCursor(0, 45); //Position text cursor for time display
        
  //RTC is operated in 24-hour mode and conversion to 12-hour mode done here, in software



  
  if(t.hour == 0){
    u8g2.print("12");} //Convert zero hour for 12-hour display
    else if(t.hour < 13 && t.hour >= 10){u8g2.print(t.hour);} //Just display hour if double digit hour
        else if(t.hour < 10){//u8g2.print("  ");
        u8g2.setCursor(19, 45);  
        u8g2.print(t.hour);} //If single digit hour, add leading space
        else if(t.hour >= 13 && t.hour >= 22){u8g2.print(t.hour-12);} //If double digit and PM, convert 24 to 12 hour
        else{//u8g2.print("  ");
        u8g2.setCursor(19, 45); 
        u8g2.print(t.hour-12);} //If single digit and PM, convert to 12 hour and add leading space
        u8g2.setCursor(37, 41);
        if(t.sec >0 || t.sec <=59){ 
          unsigned long currentMillis = millis();
          if (currentMillis - previousMillis >= intervals) {
        // save the last time you blinked the LED
          previousMillis = currentMillis;
           u8g2.print(" "); //Display hour-minute separator
          }
        else{u8g2.print(":");}
        }
        u8g2.setCursor(52, 45);
        if(t.min<10){u8g2.print("0");} //Add leading zero if single-digit minute
        u8g2.print(t.min); //Display retrieved minutes

       

         
 
  }

//Display 4-level battery gauge - flashes at lowest level  
if(mode <=7){
  if(v > 3.30){u8g2.drawBox(20,56,2,5);}else if(flash){u8g2.drawBox(20,56,2,5);} 
  if(v > 3.30){u8g2.drawFrame(0,53,20,11);}else if(flash){u8g2.drawFrame(0,53,20,11);}
  if(v > 4.10){u8g2.drawBox(2,55,16,7);}
  if(v > 3.95 && v <=4.10){u8g2.drawBox(2,55,14,7);}
  if(v > 3.80 && v <=3.95){u8g2.drawBox(2,55,10,7);}
  if(v > 3.65 && v <=3.80){u8g2.drawBox(2,55,6,7);}
  if(v > 3.50 && v <=3.65){u8g2.drawBox(2,55,2,7);}
  if(v > 3.35 && v <=3.50){u8g2.drawBox(2,55,1,7);}
}


  if (mode > 7 && mode <= 10 ){
    //u8g2.setFont(u8g2_font_ncenB10_tf);
    u8g2.setFont(u8g2_font_freedoomr25_tn); 
     
     u8g2.setCursor(0, 45); //Position text cursor for time display
        
      //RTC is operated in 24-hour mode and conversion to 12-hour mode done here, in software
     if(wake_HOUR == 0){u8g2.print("12");} //Convert zero hour for 12-hour display
            else if(wake_HOUR < 13 && wake_HOUR >= 10){u8g2.print(wake_HOUR);} //Just display hour if double digit hour
            else if(wake_HOUR < 10){//u8g2.print("  "); 
              u8g2.setCursor(19, 45); 
              u8g2.print(wake_HOUR);} //If single digit hour, add leading space
            else if(wake_HOUR >= 13 && wake_HOUR >= 22){u8g2.print(wake_HOUR-12);} //If double digit and PM, convert 24 to 12 hour
            else{//u8g2.print("  ");
              u8g2.setCursor(19, 45);  
              u8g2.print(wake_HOUR-12);} //If single digit and PM, convert to 12 hour and add leading space
              u8g2.setCursor(37, 41); 
              u8g2.print(":"); //Display hour-minute separator
              u8g2.setCursor(52, 45); 
            if(wake_MINUTE<10){u8g2.print("0");} //Add leading zero if single-digit minute
              u8g2.print(wake_MINUTE); //Display retrieved minutes
        
            
           
  }
  if (mode > 10 ){
      //on relay
      u8g2.setFont(u8g2_font_6x10_tf);       
        u8g2.setCursor(0, 4);
        u8g2.print("Snooze time:  ");
        u8g2.setCursor(78, 4);
        u8g2.print(beepcounts / 600);
        u8g2.setCursor(100, 4);
        u8g2.print("min"); 
        u8g2.setCursor(0, 16);
        u8g2.print("Torch light: ");
if(led_SET){u8g2.print("ON");digitalWrite(11, HIGH);}else{u8g2.print("OFF"); digitalWrite(11, LOW);}      
         u8g2.setCursor(0, 52);
         u8g2.print("cell volt:   ");
         u8g2.print(v);
         u8g2.setCursor(107, 52);
         u8g2.print(p);
         u8g2.setCursor(100, 52);
         u8g2.print(",");
         u8g2.setCursor(121, 52);
         u8g2.print("%");
         u8g2.setCursor(0, 28); //Position text cursor for time display
         u8g2.print("S on  time: ");
         u8g2.setCursor(78, 28); 
      //RTC is operated in 24-hour mode and conversion to 12-hour mode done here, in software
      if(relay_HOUR == 0){u8g2.print("12");} //Convert zero hour for 12-hour display
        else if(relay_HOUR < 13 && relay_HOUR >= 10){u8g2.print(relay_HOUR);} //Just display hour if double digit hour
            else if(relay_HOUR < 10){u8g2.print(" "); u8g2.print(relay_HOUR);} //If single digit hour, add leading space
            else if(relay_HOUR >= 13 && relay_HOUR >= 22){u8g2.print(relay_HOUR-12);} //If double digit and PM, convert 24 to 12 hour
            else{u8g2.print(" "); u8g2.print(relay_HOUR-12);} //If single digit and PM, convert to 12 hour and add leading space
        
            u8g2.print(":"); //Display hour-minute separator
            if(relay_MINUTE<10){u8g2.print("0");} //Add leading zero if single-digit minute
               u8g2.print(relay_MINUTE); //Display retrieved minutes
        
                       
            if(relay_HOUR < 12){
               u8g2.print(" AM");} //Display AM indicator, as needed
               else{
               u8g2.print(" PM");} //Display PM indicator, as needed


              
           //off relay
             u8g2.setCursor(0, 40); //Position text cursor for time display
             u8g2.print("S off time:");
             u8g2.setCursor(78, 40); //Position text cursor for time display
        
      //RTC is operated in 24-hour mode and conversion to 12-hour mode done here, in software
      if(relayoff_HOUR == 0){u8g2.print("12");} //Convert zero hour for 12-hour display
        else if(relayoff_HOUR < 13 && relayoff_HOUR >= 10){u8g2.print(relayoff_HOUR);} //Just display hour if double digit hour
            else if(relayoff_HOUR < 10){u8g2.print(" "); u8g2.print(relayoff_HOUR);} //If single digit hour, add leading space
            else if(relayoff_HOUR >= 13 && relayoff_HOUR >= 22){u8g2.print(relayoff_HOUR-12);} //If double digit and PM, convert 24 to 12 hour
            else{u8g2.print(" "); u8g2.print(relayoff_HOUR-12);} //If single digit and PM, convert to 12 hour and add leading space
        
            u8g2.print(":"); //Display hour-minute separator
            if(relayoff_MINUTE<10){u8g2.print("0");} //Add leading zero if single-digit minute
            u8g2.print(relayoff_MINUTE); //Display retrieved minutes
        
                       
               if(relayoff_HOUR < 12){
              
            u8g2.print(" AM");} //Display AM indicator, as needed
            else{
              u8g2.print(" PM");} //Display PM indicator, as needed
 

  
}

//Time/Date setting button processing and cursor flashing
//CURSOR COORDINATES ARE SET TO MATCH TIME/DATE FIELD - DO NOT CHANGE!!
//Digital and analog time/date display updates with new settings at 5Hz as settings are changed
  switch(mode)
  {
    case 0: break;
    case 1: //Day-of-week setting
      if(flash){u8g2.drawRFrame(0, 0,32,15, 0);} //Display rectangle cursor every other display update (5Hz blink)
      if(!digitalRead(7) && (!flash)){ //Update setting at 5Hz rate if button held down
        tempset = t.wday; //Get the current weekday and save in temporary variable
        tempset = tempset + 1; //Increment the day at 5Hz rate      
        if(tempset > 7){tempset = 1;} //Roll over after 7 days
        t.wday = tempset; //After each update, write the day back to the time structure
        set_rtc_field(t, wdayset); //Write the set field only back to the real time clock module after each update
      }

if(flash){u8g2.drawRFrame(0, 0,32,15, 0);} //Display rectangle cursor every other display update (5Hz blink)
      if(!digitalRead(9) && (!flash)){ //Update setting at 5Hz rate if button held down
        tempset = t.wday; //Get the current weekday and save in temporary variable
        tempset = tempset - 1; //Increment the day at 5Hz rate      
        if(tempset < 1){tempset = 7;} //Roll over after 7 days
        t.wday = tempset; //After each update, write the day back to the time structure
        set_rtc_field(t, wdayset); //Write the set field only back to the real time clock module after each update
      }
      
      
      break;
    case 2: //Month setting
      if(flash){u8g2.drawRFrame(32, 0,32,15, 2);} //Display rectangle cursor every other display update (5Hz blink)
      if(!digitalRead(7) && (!flash)){ //Update setting at 5Hz rate if button held down
        tempset = t.mon; //Get the current month and save in temporary variable
        tempset = tempset + 1; //Increment the month at 5Hz rate
        if(tempset > 12){tempset = 1;} //Roll over after 12 months
        t.mon = tempset; //After each update, write the month back to the time structure
        set_rtc_field(t, monset); //Write the set field only back to the real time clock module after each update
      }
if(flash){u8g2.drawRFrame(32, 0,32,15, 2);} //Display rectangle cursor every other display update (5Hz blink)
      if(!digitalRead(9) && (!flash)){ //Update setting at 5Hz rate if button held down
        tempset = t.mon; //Get the current month and save in temporary variable
        tempset = tempset - 1; //Increment the month at 5Hz rate
        if(tempset < 1){tempset = 12;} //Roll over after 12 months
        t.mon = tempset; //After each update, write the month back to the time structure
        set_rtc_field(t, monset); //Write the set field only back to the real time clock module after each update
      }
      
      break;
    case 3: //Date setting
     if(flash){u8g2.drawRFrame(63, 0,24,15, 2);}  //Display rectangle cursor every other display update (5Hz blink)
      if(!digitalRead(7) && (!flash)){ //Update setting at 5Hz rate if button held down
        tempset = t.mday; //Get the current date and save in temporary variable
        tempset = tempset + 1; //Increment the date at 5Hz rate
        //(RTC allows incorrect date setting for months < 31 days, but will use correct date rollover for subsequent months.
        if(tempset > 31){tempset = 1;} //Roll over after 31 days
        t.mday = tempset; //After each update, write the date back to the time structure 
        set_rtc_field(t, mdayset); //Write the set field only back to the real time clock module after each update
      }
if(flash){u8g2.drawRFrame(63, 0,24,15, 2);}  //Display rectangle cursor every other display update (5Hz blink)
      if(!digitalRead(9) && (!flash)){ //Update setting at 5Hz rate if button held down
        tempset = t.mday; //Get the current date and save in temporary variable
        tempset = tempset - 1; //Increment the date at 5Hz rate
        //(RTC allows incorrect date setting for months < 31 days, but will use correct date rollover for subsequent months.
        if(tempset < 1){tempset = 31;} //Roll over after 31 days
        t.mday = tempset; //After each update, write the date back to the time structure 
        set_rtc_field(t, mdayset); //Write the set field only back to the real time clock module after each update
      }
      
      break;
    case 4: //Year setting
      if(flash){u8g2.drawRFrame(90, 0,38,15, 2);} //Display rectangle cursor every other display update (5Hz blink)
      if(!digitalRead(7) && (!flash)){ //Update setting at 5Hz rate if button held down
        tempset = t.year; //Get the current year and save in temporary variable
        tempset = tempset + 1; //Increment the year at 5Hz rate
        //RTC allows setting from 1900, but range limited here to 2000 to 2099
        if(tempset > 2099){tempset = 2000;} //Roll over after 2099 to 2000
        t.year = tempset; //After each update, write the year back to the time structure
        set_rtc_field(t, yearset); //Write the set field only back to the real time clock module after each update
      }
if(flash){u8g2.drawRFrame(90, 0,38,15, 2);} //Display rectangle cursor every other display update (5Hz blink)
      if(!digitalRead(9) && (!flash)){ //Update setting at 5Hz rate if button held down
        tempset = t.year; //Get the current year and save in temporary variable
        tempset = tempset - 1; //Increment the year at 5Hz rate
        //RTC allows setting from 1900, but range limited here to 2000 to 2099
        if(tempset < 2000){tempset = 2099;} //Roll over after 2099 to 2000
        t.year = tempset; //After each update, write the year back to the time structure
        set_rtc_field(t, yearset); //Write the set field only back to the real time clock module after each update
      }
       
      break;
    case 5: //Hour setting
      if(flash){u8g2.drawRFrame(4, 16,35,29, 2);} //Display rectangle cursor every other display update (5Hz blink)
      if(!digitalRead(7) && (!flash)){ //Update setting at 5Hz rate if button held down
        tempset = t.hour; //Get the current hour and save in temporary variable
        tempset = tempset + 1; //Increment the hour at 5Hz rate
        if(tempset > 23){tempset = 0;} //Roll over hour after 23rd hour (setting done in 24-hour mode)
        t.hour = tempset; //After each update, write the hour back to the time structure
        set_rtc_field(t, hourset); //Write the set field only back to the real time clock module after each update
      }
if(flash){u8g2.drawRFrame(4, 16,35,29, 2);} //Display rectangle cursor every other display update (5Hz blink)
      if(!digitalRead(9) && (!flash)){ //Update setting at 5Hz rate if button held down
        tempset = t.hour; //Get the current hour and save in temporary variable
        tempset = tempset - 1; //Increment the hour at 5Hz rate
        if(tempset < 0){tempset = 23;} //Roll over hour after 23rd hour (setting done in 24-hour mode)
        t.hour = tempset; //After each update, write the hour back to the time structure
        set_rtc_field(t, hourset); //Write the set field only back to the real time clock module after each update
      }
      
      break;
    case 6: //Minute setting
      if(flash){u8g2.drawRFrame(51, 17,41,29, 2);} //Display rectangle cursor every other display update (5Hz blink)
      if(!digitalRead(7) && (!flash)){ //Update setting at 5Hz rate if button held down
        tempset = t.min; //Get the current minute and save in temporary variable
        tempset = tempset + 1; //Increment the minute at 5Hz rate
        if(tempset > 59){tempset = 0;} //Roll over minute to zero after 59th minute
        t.min = tempset; //After each update, write the minute back to the time structure
        set_rtc_field(t, minset); //Write the set field only back to the real time clock module after each update
      }
if(flash){u8g2.drawRFrame(51, 17,41,29, 2);} //Display rectangle cursor every other display update (5Hz blink)
      if(!digitalRead(9) && (!flash)){ //Update setting at 5Hz rate if button held down
        tempset = t.min; //Get the current minute and save in temporary variable
        tempset = tempset - 1; //Increment the minute at 5Hz rate
        if(tempset < 0){tempset = 59;} //Roll over minute to zero after 59th minute
        t.min = tempset; //After each update, write the minute back to the time structure
        set_rtc_field(t, minset); //Write the set field only back to the real time clock module after each update
      }
      
      break;
      
     //Set clock + 1 minute, then press and hold to freeze second setting.
     //Release button at 00 seconds to synchronize clock to external time source.
    case 7: //Second synchronization
       if(flash){u8g2.drawRFrame(100, 29,25,16, 2);}//Display rectangle cursor every other display update (5Hz blink)
      if(!digitalRead(7) && (!flash)){ //Reset second to zero at 5Hz rate if button held down
        t.sec = 0; //After each update, write the zeroed second back to the time structure
        set_rtc_field(t, secset); //Write the set field only back to the real time clock module after each update
      }
      break;
      
      
    case 8: //Alarm hour setting
      if(flash){u8g2.drawRFrame(4, 16,35,29, 2);} //Display rectangle cursor every other display update (5Hz blink)
      if(!digitalRead(7) && (!flash)){ //Update setting at 5Hz rate if button held down
        tempset = wake_HOUR; //Get the current hour and save in temporary variable
        tempset = tempset + 1; //Increment the hour at 5Hz rate
        if(tempset > 23){tempset = 0;} //Roll over hour after 23rd hour (setting done in 24-hour mode)
        wake_HOUR = tempset; //After each update, write the hour back to the alarm variable
        
         set_alarm();//Write the alarm setting back to the RTC after each update
      }

if(flash){u8g2.drawRFrame(4, 16,35,29, 2);} //Display rectangle cursor every other display update (5Hz blink)
      if(!digitalRead(9) && (!flash)){ //Update setting at 5Hz rate if button held down
        tempset = wake_HOUR; //Get the current hour and save in temporary variable
        tempset = tempset - 1; //Increment the hour at 5Hz rate
        if(tempset < 0){tempset = 23;} //Roll over hour after 23rd hour (setting done in 24-hour mode)
        wake_HOUR = tempset; //After each update, write the hour back to the alarm variable
        
         set_alarm();//Write the alarm setting back to the RTC after each update
      }
      
    break;
    
    
    case 9: //Alarm minute setting
      if(flash){u8g2.drawRFrame(51, 17,41,29, 2);} //Display rectangle cursor every other display update (5Hz blink)
      if(!digitalRead(7) && (!flash)){ //Update setting at 5Hz rate if button held down
        tempset = wake_MINUTE; //Get the current minute and save in temporary variable
        tempset = tempset + 1; //Increment the minute at 5Hz rate
        if(tempset > 59){tempset = 0;} //Roll over minute to zero after 59th minute
        wake_MINUTE = tempset; //After each update, write the minute back to the alarm variable
        set_alarm();//Write the alarm setting back to the RTC after each update
      }

if(flash){u8g2.drawRFrame(51, 17,41,29, 2);} //Display rectangle cursor every other display update (5Hz blink)
      if(!digitalRead(9) && (!flash)){ //Update setting at 5Hz rate if button held down
        tempset = wake_MINUTE; //Get the current minute and save in temporary variable
        tempset = tempset - 1; //Increment the minute at 5Hz rate
        if(tempset < 0){tempset = 59;} //Roll over minute to zero after 59th minute
        wake_MINUTE = tempset; //After each update, write the minute back to the alarm variable
        set_alarm();//Write the alarm setting back to the RTC after each update
      }
      
    break;
    
     case 10: //Alarm enable/disable
       if(flash){u8g2.drawRFrame(60, 0,30,15, 2);}//Display rectangle cursor every other display update (5Hz blink)
      if(!digitalRead(7) && (!flash)){ //Update setting at 5Hz rate if button held down
        if(wake_SET){wake_SET = 0;}else{wake_SET = 1; } //Toggle alarm on/of variable at 5 Hz
        EEPROM.write(alarmEE, wake_SET); //Save alarm enable setting to EEPROM
      }

if(flash){u8g2.drawRFrame(60, 0,30,15, 2);}//Display rectangle cursor every other display update (5Hz blink)
      if(!digitalRead(9) && (!flash)){ //Update setting at 5Hz rate if button held down
        if(wake_SET){wake_SET = 0;}else{wake_SET = 1; } //Toggle alarm on/of variable at 5 Hz
        EEPROM.write(alarmEE, wake_SET); //Save alarm enable setting to EEPROM
      }
      
    break;

       case 11: //Snooze setting
      if(flash){u8g2.drawRFrame(77, 3,14,12, 2);} //Display rectangle cursor every other display update (5Hz blink)
      if(!digitalRead(7) && (!flash)){ //Update setting at 5Hz rate if button held down
        
        tempset = beepcounts; //Get the current hour and save in temporary variable
        tempset = tempset + 600; //Increment the hour at 5Hz rate
        if(tempset > 3000){tempset = 600;} //Roll over hour after 23rd hour (setting done in 24-hour mode)
        beepcounts = tempset; //After each update, write the hour back to the alarm variable
         //Write the alarm setting back to the RTC after each update
      }

if(flash){u8g2.drawRFrame(77, 3,14,12, 2);} //Display rectangle cursor every other display update (5Hz blink)
      if(!digitalRead(9) && (!flash)){ //Update setting at 5Hz rate if button held down
        
        tempset = beepcounts; //Get the current hour and save in temporary variable
        tempset = tempset - 600; //Increment the hour at 5Hz rate
        if(tempset < 600){tempset = 3000;} //Roll over hour after 23rd hour (setting done in 24-hour mode)
        beepcounts = tempset; //After each update, write the hour back to the alarm variable
         //Write the alarm setting back to the RTC after each update
      }
      
    break;

      case 12: //Torch enable/disable
        if(flash){u8g2.drawRFrame(77, 15,21,10, 2);}//Display rectangle cursor every other display update (5Hz blink)
      if(!digitalRead(7) && (!flash)){ //Update setting at 5Hz rate if button held down
        if(led_SET){led_SET = 0;}else{led_SET = 1; } //Toggle alarm on/of variable at 5 Hz
        //Save alarm enable setting to EEPROM
      }

if(flash){u8g2.drawRFrame(77, 15,21,10, 2);}//Display rectangle cursor every other display update (5Hz blink)
      if(!digitalRead(9) && (!flash)){ //Update setting at 5Hz rate if button held down
        if(led_SET){led_SET = 0;}else{led_SET = 1; } //Toggle alarm on/of variable at 5 Hz
        //Save alarm enable setting to EEPROM
      }
      
    break;
   

         case 13: //Relay_ON hour setting
      if(flash){u8g2.drawRFrame(77, 27,14,12, 2);} //Display rectangle cursor every other display update (5Hz blink)
      digitalWrite(10, LOW);
      if(!digitalRead(7) && (!flash)){ //Update setting at 5Hz rate if button held down
        
        tempset = relay_HOUR; //Get the current hour and save in temporary variable
        tempset = tempset + 1; //Increment the hour at 5Hz rate
        if(tempset > 23){tempset = 0;} //Roll over hour after 23rd hour (setting done in 24-hour mode)
        relay_HOUR = tempset; //After each update, write the hour back to the alarm variable
         //Write the alarm setting back to the RTC after each update
      }

if(flash){u8g2.drawRFrame(77, 27,14,12, 2);} //Display rectangle cursor every other display update (5Hz blink)
      digitalWrite(10, LOW);
      if(!digitalRead(9) && (!flash)){ //Update setting at 5Hz rate if button held down
        
        tempset = relay_HOUR; //Get the current hour and save in temporary variable
        tempset = tempset - 1; //Increment the hour at 5Hz rate
        if(tempset < 0){tempset = 23;} //Roll over hour after 23rd hour (setting done in 24-hour mode)
        relay_HOUR = tempset; //After each update, write the hour back to the alarm variable
         //Write the alarm setting back to the RTC after each update
      }
      
    break;
    
    
    case 14: //Relay_ON minute setting
      if(flash){u8g2.drawRFrame(95, 27,14,12, 2);} //Display rectangle cursor every other display update (5Hz blink)
      if(!digitalRead(7) && (!flash)){ //Update setting at 5Hz rate if button held down
        tempset = relay_MINUTE; //Get the current minute and save in temporary variable
        tempset = tempset + 1; //Increment the minute at 5Hz rate
        if(tempset > 59){tempset = 0;} //Roll over minute to zero after 59th minute
        relay_MINUTE = tempset; //After each update, write the minute back to the alarm variable
         //Write the alarm setting back to the RTC after each update
      }

if(flash){u8g2.drawRFrame(95, 27,14,12, 2);} //Display rectangle cursor every other display update (5Hz blink)
      if(!digitalRead(9) && (!flash)){ //Update setting at 5Hz rate if button held down
        tempset = relay_MINUTE; //Get the current minute and save in temporary variable
        tempset = tempset - 1; //Increment the minute at 5Hz rate
        if(tempset < 0){tempset = 59;} //Roll over minute to zero after 59th minute
        relay_MINUTE = tempset; //After each update, write the minute back to the alarm variable
         //Write the alarm setting back to the RTC after each update
      }
      
    break;

        case 15: //Relay_OFF hour setting
      if(flash){u8g2.drawRFrame(77, 39,14,12, 2);} //Display rectangle cursor every other display update (5Hz blink)
      if(!digitalRead(7) && (!flash)){ //Update setting at 5Hz rate if button held down
        tempset = relayoff_HOUR; //Get the current hour and save in temporary variable
        tempset = tempset + 1; //Increment the hour at 5Hz rate
        if(tempset > 23){tempset = 0;} //Roll over hour after 23rd hour (setting done in 24-hour mode)
        relayoff_HOUR = tempset; //After each update, write the hour back to the alarm variable
         //Write the alarm setting back to the RTC after each update
      }

if(flash){u8g2.drawRFrame(77, 39,14,12, 2);} //Display rectangle cursor every other display update (5Hz blink)
      if(!digitalRead(9) && (!flash)){ //Update setting at 5Hz rate if button held down
        tempset = relayoff_HOUR; //Get the current hour and save in temporary variable
        tempset = tempset - 1; //Increment the hour at 5Hz rate
        if(tempset < 0){tempset = 23;} //Roll over hour after 23rd hour (setting done in 24-hour mode)
        relayoff_HOUR = tempset; //After each update, write the hour back to the alarm variable
         //Write the alarm setting back to the RTC after each update
      }
      
    break;
    
    
    case 16: //Relay_OFF minute setting
      if(flash){u8g2.drawRFrame(95, 39,14,12, 2);} //Display rectangle cursor every other display update (5Hz blink)
      if(!digitalRead(7) && (!flash)){ //Update setting at 5Hz rate if button held down
        tempset = relayoff_MINUTE; //Get the current minute and save in temporary variable
        tempset = tempset + 1; //Increment the minute at 5Hz rate
        if(tempset > 59){tempset = 0;} //Roll over minute to zero after 59th minute
        relayoff_MINUTE = tempset; //After each update, write the minute back to the alarm variable
         //Write the alarm setting back to the RTC after each update
      }

if(flash){u8g2.drawRFrame(95, 39,14,12, 2);} //Display rectangle cursor every other display update (5Hz blink)
      if(!digitalRead(9) && (!flash)){ //Update setting at 5Hz rate if button held down
        tempset = relayoff_MINUTE; //Get the current minute and save in temporary variable
        tempset = tempset - 1; //Increment the minute at 5Hz rate
        if(tempset < 0){tempset = 59;} //Roll over minute to zero after 59th minute
        relayoff_MINUTE = tempset; //After each update, write the minute back to the alarm variable
         //Write the alarm setting back to the RTC after each update
      }
      
    break;
 

   
}
   
          
     prev = now;//Reset variable for display and time update rate


//For enable/disable esp8266 weather station mode
if(mode < 1){
    if(!digitalRead(7)){ 
   
   if(!digitalRead(7)){
    delay(100);
    digitalWrite(12, LOW) ;
    u8g2.setPowerSave(1);
    delay(120000);
    u8g2.setPowerSave(0);
    digitalWrite(12, HIGH) ;
   
    }     
   }
  }
     } while ( u8g2.nextPage() );
 }         
  
  
    
//Clock setting mode set - outside time/display update processing for faster button response
if(!digitalRead(8)){ //Read setting mode button
  delay(25); //100ms debounce time
  if(!digitalRead(8)){ //Activate setting mode change after 100ms button press
    mode = mode + 1; //Increment the time setting mode on each button press
    framecount2 = 0;  //Reset cycle counter if button pressed to delay auto return to normal mode
    if(mode > 16){mode = 0;} //Roll the mode setting after 7th mode
    while(!digitalRead(8)){} //Wait for button release (freezes all display processing and time updates while button held, but RTC continues to keep time)
  }
 }
 
 if(!digitalRead(9)){ //Reset alarm flag if set button pressed
   delay(25); //25ms debounce time
   if(!digitalRead(9)){
    digitalWrite(11, LOW);
    DS3231_clear_a1f(); //Reset cycle counter if button pressed to delay auto return to normal mode
    beepcount = 0; //Reset alarm timeout counter if alarm stopped by pushing button
    framecount2 = 0;   
     }     
   }
 }

 

//Function to display month string from numerical month argument
void printMonth(int month)
{
  switch(month)
  {
     
    case 1: u8g2.print("Jan ");break;
    case 2: u8g2.print("Feb ");break;
    case 3: u8g2.print("Mar ");break;
    case 4: u8g2.print("Apr ");break;
    case 5: u8g2.print("May ");break;
    case 6: u8g2.print("Jun ");break;
    case 7: u8g2.print("Jul ");break;
    case 8: u8g2.print("Aug ");break;
    case 9: u8g2.print("Sep ");break;
    case 10: u8g2.print("Oct ");break;
    case 11: u8g2.print("Nov ");break;
    case 12: u8g2.print("Dec ");break;
    default: u8g2.print("--- ");break; //Display dashes if error - avoids scrambling display
  
  } 
}



//Function to display day-of-week string from numerical day-of-week argument
void printDay(int day)
{
  switch(day)
  {
   
    
    case 1: u8g2.print("Mon ");break;
    case 2:u8g2.print("Tue ");break;
    case 3: u8g2.print("Wed ");break;
    case 4: u8g2.print("Thu ");break;
    case 5: u8g2.print("Fri   ");break;
    case 6: u8g2.print("Sat  ");break;
    case 7: u8g2.print("Sun ");break;
    default: u8g2.print("--- ");break; //Display dashes if error - avoids scrambling display
  
  }
   
}





//Subroutine to adjust a single date/time field in the RTC
void set_rtc_field(struct ts t,  uint8_t index)
{
    uint8_t century;

    if (t.year > 2000) {
        century = 0x80;
        t.year_s = t.year - 2000;
    } else {
        century = 0;
        t.year_s = t.year - 1900;
    }

    uint8_t TimeDate[7] = { t.sec, t.min, t.hour, t.wday, t.mday, t.mon, t.year_s };

    Wire.beginTransmission(DS3231_I2C_ADDR);
    Wire.write(index);
        TimeDate[index] = dectobcd(TimeDate[index]);
        if (index == 5){TimeDate[5] += century;}
        Wire.write(TimeDate[index]);
    Wire.endTransmission();
    
    //Adjust the month setting, per data sheet, if the year is changed
    if (index == 6){
      Wire.beginTransmission(DS3231_I2C_ADDR);
      Wire.write(5);
      TimeDate[5] = dectobcd(TimeDate[5]);
      TimeDate[5] += century;
      Wire.write(TimeDate[5]);
      Wire.endTransmission();
    } 
}




//Subroutine to set alarm 1
void set_alarm()
{

    // flags define what calendar component to be checked against the current time in order
    // to trigger the alarm - see datasheet
    // A1M1 (seconds) (0 to enable, 1 to disable)
    // A1M2 (minutes) (0 to enable, 1 to disable)
    // A1M3 (hour)    (0 to enable, 1 to disable) 
    // A1M4 (day)     (0 to enable, 1 to disable)
    // DY/DT          (dayofweek == 1/dayofmonth == 0)
    byte flags[5] = { 0, 0, 0, 1, 1 }; //Set alarm to trigger every 24 hours on time match

    // set Alarm1
    DS3231_set_a1(0, wake_MINUTE, wake_HOUR, 0, flags); //Set alarm 1 RTC registers

}

//Subroutine to get alarm 1
void get_alarm()
{
    uint8_t n[4];
    uint8_t t[4];               //second,minute,hour,day
    uint8_t f[5];               // flags
    uint8_t i;

    Wire.beginTransmission(DS3231_I2C_ADDR);
    Wire.write(DS3231_ALARM1_ADDR);
    Wire.endTransmission();

    Wire.requestFrom(DS3231_I2C_ADDR, 4);

    for (i = 0; i <= 3; i++) {
        n[i] = Wire.read();
        f[i] = (n[i] & 0x80) >> 7;
        t[i] = bcdtodec(n[i] & 0x7F);
    }

    f[4] = (n[3] & 0x40) >> 6;
    t[3] = bcdtodec(n[3] & 0x3F);
    
    wake_SECOND = t[0];
    wake_MINUTE = t[1];
    wake_HOUR = t[2];
}
