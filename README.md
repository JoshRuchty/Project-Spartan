# Project-Spartan
#Vanlife Camper thermostat &amp; space heater servo - daily schedule, 2x16 matrix LCD, IR remote

Summary:
Using an Arduino, a 2x16 LCD display, thermistor, IR remote control, and a stepper motor glued to a spaceheater, I designed a programmable thermostat to efficiently and comfortably manage the heat output of a space-heater inside the 11" travel-camper I used to live in. 

Design:
The design intention is simply to output heat only when it was needed according to a schedule set by the user. Using only the IR remote's directional buttons, the user is able to view and adjust the daily temperature and time schedule (for Wake, leave, return, & sleep periods) on the LCD display. Throughout the day and for each time period, the ambient temperature is measured and checked against the user-settemperature, and the servo engages or disengages the spaceheater's power switch appropriately.

User interaction:
By default, the LCD displays the Time, current ambient temperature, and the temperature set by the user in the following Home screen:

    Time:             15:38:29
    Temp:             70/72         // (ambient temp is 70F compared to set temp of 72F; space heater is ON until temp > 72F)

To set the scheduled times (military) and temperature (F) for each period, the user simply scrolls up or down through the various time periods using the remote control's down button. The display honors "page breaks" for each time period (see groupings below), so both the Time & Temp for any period are always displayed simultaneously on the 2-line LCD. Within each of these pages, the user can move the cursor up or down over the Time and Temperature digits, and trying to scroll off the screen presents the user with the previous/next ordered page:

    Wake Time         0600  
    Wake Temp       <<60F>>     // pressing Down from this position would then display the 2 "Leave Time" "Leave Temp" lines below
                                // Pressing Up once would select 0600 like: <<0600>>
                                // Pressing Up twice would return the user to the Homescreen
                                // Pressing Right would change Wake Temp to 61F

    Leave Time        1100
    Leave Temp        60F

    Return Time       1700
    Return Temp       60F

    Sleep Time        2100
    Sleep Temp        60F


Adjusting the time or temperature is simply a matter of pressing the remote control's left or right buttons when the cursor is selecting the desired digits. As soon as the user makes the adjustment, the setting is saved.

The user also has the option of running a "hold temperature" function that disables the scheduled periods and holds a temperature set by the user. To do this, the user simply scrolls Left or Right from the Home screen. This temperature is held until the user makes an adjustment to the schedule (by changing the time or temperature), which then runs the daily schedule. 
