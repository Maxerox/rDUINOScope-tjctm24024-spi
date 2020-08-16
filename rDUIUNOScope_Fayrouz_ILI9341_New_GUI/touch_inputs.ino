//
//    rDUINOScope - Arduino based telescope control system (GOTO).
//    Copyright (C) 2016 Dessislav Gouzgounov (Desso)
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    any later version.
//
//    PROJECT Website: http://rduinoscope.tk/
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

// ......................................................................
//  This part of the code take care of:
//  - Handles ALL TOUCH SCREEN Inputs;
//  - Screens are separated like:
//    * CURRENT_SCREEN==0  - drawGPSScreen() Where the GPS coordinates are displayed
//    * CURRENT_SCREEN==1  - drawClockScreen() Captures updates on the time and date
//    * CURRENT_SCREEN==2  -                - not used
//    * CURRENT_SCREEN==3  - Select Alignment method (only have 3 buttons)
//    * CURRENT_SCREEN==4  - drawMainScreen() Captures all clicks on the MAIN Screen of the application
//    * CURRENT_SCREEN==5  - drawCoordinatesScreen() Only have "back" button
//    * CURRENT_SCREEN==6  - drawLoadScreen() Captures input on Load screen (all of them: Messier && Treasurres)
//    * CURRENT_SCREEN==7  -                - not used
//    * CURRENT_SCREEN==8  -                - not used
//    * CURRENT_SCREEN==9  -                - not used
//    * CURRENT_SCREEN==10  - drawSTATScreen()
//    * CURRENT_SCREEN==11  -               - not used
//    * CURRENT_SCREEN==12  - drawStarSyncScreen() - To Select Alignment Star;
//    * CURRENT_SCREEN==13  - drawConstelationScreen(int indx) - to actually align on Star. Called few times per alignment procedure.
//    * CURRENT_SCREEN==14  - drawAutoGuidingScreen();
//    * CURRENT_SCREEN==15  - confirm_sun_track()
//
//  - Please note, that Touches are separated in 2 section to capture OnPress && OnRelease!
//    You will notice that "if (lx > 0 && ly > 0 )" this part defines OnPress activities.

int considerTouchInput(int lx, int ly, int lz, int prevlz)
{
  //**************************************************************
  //
  //      BUTTON DOWN Events start here
  //
  //      - only executed when the user touches the screen - PRESS
  //**************************************************************
  if (lx > 0 && ly > 0 && lz > prevlz )
  {
    if (lx >= 0 && ly >= 0 ) // Sound on any Screen touch
    {
      if (IS_SOUND_ON) {
        SoundOn(800, 8);
      }
    }
    // Make sure you WakeUp the TFT
    // in case the lid is OFF
    if (IS_TFT_ON == false)
    {
      analogWrite(TFTBright, TFT_Brightness);
      IS_TFT_ON = true;
      TFT_Timer = millis();
      delay(200);
      return lz ;
    }
    TFT_Timer = millis();
    if (CURRENT_SCREEN == 0)
    { // captures touches on drawGPSScreen()
      if (lx > 45 && lx < 195 && ly > 267 && ly < 317)
      {
        last_button = 1;
      }
    }

    ////////////////////////////////////////////////// Clock Screen Touch Actions ///////////////////////////////////////////////////////
    else if (CURRENT_SCREEN == 1)
    { // captures touches on drawClockScreen()
      if (lx > 165 && lx < 240 && ly > 270 && ly < 320)
      {
        // BTN OK pressed
        int changes = 0;
        for (int y = 0; y < 12; y++)
        {
          if (w_DateTime[y] != 0)
          {
            changes = 1;
          }
        }
        if (changes == 1)
        {
          // Do the magic as the date and time has been updated... Update the RTC accordingly
          int hh = (w_DateTime[8] * 10)   + w_DateTime[9];
          int mm = (w_DateTime[10] * 10)  + w_DateTime[11];
          int dd = (w_DateTime[0] * 10)   + w_DateTime[1];
          int mo = (w_DateTime[2] * 10)   + w_DateTime[3];
          int yy = (w_DateTime[4] * 1000) + (w_DateTime[5] * 100) + (w_DateTime[6] * 10) + w_DateTime[7];
          rtc.setTime(hh, mm, 00);
          rtc.setDate(dd, mo, yy);
        }
        int ora, date_delay = 0; //, month_delay = 0, year_delay = 0;
        int time_delay = TIME_ZONE;
        time_delay += Summer_Time;

        String Date_q = String(rtc.getDateStr());
        String Time_q = String(rtc.getTimeStr());
        int D = Date_q.substring(0, 2).toInt();
        int M = Date_q.substring(3, 5).toInt();
        int Y = Date_q.substring(6).toInt();
        int H = Time_q.substring(0, 2).toInt();
        int MN = Time_q.substring(3, 5).toInt();
        int S = Time_q.substring(6).toInt();
        //update the value of the variable ora
        //ora = hh - time_delay;
        ora = H - time_delay;
        //to update the real time
        if (ora < 0) // In all these calculations it is not necessary to correct the month or year (although it may work)
        {
          ora += 24;
          date_delay = -1;
          /*if (dd + date_delay < 1)
            {
            dd = (mo==2 || mo==4 || mo==6 || mo==8 || mo==9 || mo==11 || mo==1? 31: (mo!=3? 30: (yy%4==0 and yy%100!=0 or yy%400==0? 29: 28)));
            date_delay = 0;
            month_delay = -1;
            if (mo + month_delay < 1)
            {
              mo = 12;
              month_delay = 0;
              year_delay = -1;
            }
            }*/
        }
        // convert to epoch
        //setTime(ora, mm, 00, dd + date_delay, mo + month_delay, yy + year_delay); // <-- too mantain sync with UTC
        //setTime(ora, mm, 00, dd + date_delay, mo, yy); // <-- too mantain sync with UTC
        setTime(ora, MN, S, D + date_delay, M, Y); // <-- too mantain sync with UTC

        Start_date = String(rtc.getDateStr()).substring(0, 2) + " " + rtc.getMonthStr(FORMAT_SHORT) + " " + String(rtc.getDateStr()).substring(6);
        old_d = rtc.getDateStr(FORMAT_LONG, FORMAT_LITTLEENDIAN, '/');
        START_TIME = rtc.getTimeStr(FORMAT_SHORT);
        _Stemp = dht.readTemperature();;
        _Shumid = dht.readHumidity();

        drawSelectAlignment();
        //drawStarSyncScreen();
        //drawMainScreen();
      }
      else if (lx > 11 && lx < 71 && ly > 150 && ly < 183)
      {
        // BTN 1 pressed x
        tft.drawRect(11, 150, 60, 33, title_bg);
        last_button = 1;
        removeTime_addXX();

      }
      else if (lx > 90 && lx < 150 && ly > 150 && ly < 183)
      {
        // BTN 2 pressed
        tft.drawRect(90, 150, 60, 33, title_bg);
        last_button = 2;
        removeTime_addXX();

      }
      else if (lx > 172 && lx < 210 && ly > 150 && ly < 183)
      {
        // BTN 3 pressed
        tft.drawRect(172, 150, 60, 33, title_bg);
        last_button = 3;
        removeTime_addXX();

      }
      else if (lx > 11 && lx < 71 && ly > 190 && ly < 223)
      {
        // BTN 4 pressed
        tft.drawRect(11, 190, 60, 33, title_bg);
        last_button = 4;
        removeTime_addXX();

      }
      else if (lx > 90 && lx < 150 && ly > 190 && ly < 223)
      {
        // BTN 5 pressed
        tft.drawRect(90, 190, 60, 33, title_bg);
        last_button = 5;
        removeTime_addXX();

      }
      else if (lx > 172 && lx < 210 && ly > 190 && ly < 223)
      {
        // BTN 6 pressed
        tft.drawRect(172, 190, 60, 33, title_bg);
        last_button = 6;
        removeTime_addXX();

      }
      else if (lx > 11 && lx < 71 && ly > 231 && ly < 264)
      {
        // BTN 7 pressed
        tft.drawRect(11, 231, 60, 33, title_bg);
        last_button = 7;
        removeTime_addXX();

      }
      else if (lx > 90 && lx < 150 && ly > 231 && ly < 264)
      {
        // BTN 8 pressed
        tft.drawRect(90, 231, 60, 33, title_bg);
        last_button = 8;
        removeTime_addXX();

      }
      else if (lx > 172 && lx < 210 && ly > 231 && ly < 264)
      {
        // BTN 9 pressed
        tft.drawRect(172, 231, 60, 33, title_bg);
        last_button = 9;
        removeTime_addXX();

      }
      else if (lx > 90 && lx < 150 && ly > 272 && ly < 319)
      {
        // BTN 0 pressed
        tft.drawRect(90, 272, 60, 33, title_bg);
        last_button = 10;
        removeTime_addXX();

      }
      else if (lx > 23 && lx < 75 && ly > 270 && ly < 307)
      {
        // BTN SummerTime pressed
        last_button = 22;
      }
    }

    ////////////////////////////////////////////////// Select Alignment Screen Touch Actions ///////////////////////////////////////////////////////

    else if (CURRENT_SCREEN == 3)
    {
      if (lx > 30 && lx < 211 && ly > 59 && ly < 105)
      {
        // BTN "1 Star Alignment" pressed
        tft.drawRect(30, 59, 181, 46, btn_l_border);   //x 30>211 y 89>158
        ALLIGN_TYPE = 1;
        drawStarSyncScreen();
      }
      else if (lx > 30 && lx < 211 && ly > 129 && ly < 175)
      {
        // BTN "Iterative Alignment" pressed
        tft.drawRect(30, 130, 181, 45, btn_l_border);  //x 30>281 y 194>262
        ALLIGN_TYPE = 3;

        // I'll take some time to Initialize the Sub ARRAY with suitable stars (only for Northern Hemi)
        tft.cursorToXY(0, 3);
        int cc = 0;
        for (int t = 0; t < 203; t++)
        {
          int i1 = Stars[t].indexOf(';');
          int i2 = Stars[t].indexOf(';', i1 + 1);
          int i3 = Stars[t].indexOf(';', i2 + 1);
          OBJECT_NAME = Stars[t].substring(i1 + 1, i2) + " from " + Stars[t].substring(0, i1);
          String OBJ_RA = Stars[t].substring(i2 + 1, i3);
          OBJECT_RA_H = OBJ_RA.substring(0, OBJ_RA.indexOf('h')).toFloat();
          OBJECT_RA_M = OBJ_RA.substring(OBJ_RA.indexOf('h') + 1, OBJ_RA.length() - 1).toFloat();
          float ts_RA = OBJECT_RA_H + OBJECT_RA_M / 100;
          String OBJ_DEC = Stars[t].substring(i3, Stars[t].length());
          String sign = OBJ_DEC.substring(0, 1);
          OBJECT_DEC_D = OBJ_DEC.substring(1, OBJ_DEC.indexOf('°')).toFloat();
          if (sign == "-")
          {
            OBJECT_DEC_D *= (-1);
          }
          OBJECT_DEC_M = 0;
          if (ts_RA > 5.3 && ts_RA < 23.3)
          {
            calculateLST_HA();
            if (AZ > 90 && AZ < 270 && ALT > 20)
            {
              Iter_Stars[cc] = Stars[t];
              cc += 1;
              if (cc > 49)
              {
                break;
              }
            }
          }
          int_star_count = floor(cc / 15) + 1;
        }
        delay(500);
        drawStarSyncScreen();
      } else if (lx > 50 && lx < 196 && ly > 276 && ly < 320) {
        // BTN "Skip Alignment" pressed
        tft.drawRect(50, 276, 146, 33, btn_l_border);  //x 67>262 y 276>480
        IS_IN_OPERATION = true;
        drawMainScreen();
      }
    }

    ////////////////////////////////////////////////// Main Screen Touch Actions ///////////////////////////////////////////////////////
    else if (CURRENT_SCREEN == 4)
    { // captures touches on drawMainScreen()
      if (lx > 0 && lx < 67 && ly > 0 && ly < 67) {
        // Left Side Touched
        // Load the GPS screen to capture new data && correct time if needed on the next screen (Time Screen)
        CURRENT_SCREEN = 0;
        GPS_iterrations = 0;
        IS_IN_OPERATION = false;
        ALLIGN_STEP = 0;
        ALLIGN_TYPE = 0;
        drawGPSScreen();
      }

      if (lx > 187 && lx < 240 && ly > 0 && ly < 67)
      {
        // Right Side Touched
        drawOptionsScreen();
      }
      if (lx > 7 && lx < 74 && ly > 250 && ly < 279 && IS_BT_MODE_ON == false) {
        // BTN 1 pressed
        if (MAIN_SCREEN_MENU == 0) {
          last_button = 1;
        } else {
          last_button = 7;
        }
      } else if (lx > 7 && lx < 74 && ly > 285 && ly < 320  && IS_BT_MODE_ON == false) {
        // BTN 4 pressed
        if (MAIN_SCREEN_MENU == 0) {
          last_button = 4;
        } else {
          last_button = 10;
        }
      } else if (lx > 86 && lx < 154 && ly > 250 && ly < 280 && IS_BT_MODE_ON == false) {
        // BTN 2 pressed
        if (MAIN_SCREEN_MENU == 0) {
          last_button = 2;
        } else {
          last_button = 8;
        }
      } else if (lx > 86 && lx < 154 && ly > 285 && ly < 320) {
        // BTN 5 pressed
        if (MAIN_SCREEN_MENU == 0) {
          last_button = 5;
        } else {
          last_button = 11;
        }
      } else if (lx > 166 && lx < 240 && ly > 250 && ly < 280) {
        // BTN 3 pressed
        if (MAIN_SCREEN_MENU == 0) {
          last_button = 3;
        } else {
          last_button = 9;
        }
      } else if (lx > 166 && lx < 240 && ly > 285 && ly < 320) {
        // BTN 6 pressed
        if (MAIN_SCREEN_MENU == 0) {
          last_button = 6;
        } else {
          last_button = 12;
        }
      }
    }
    
    ////////////////////////////////////////////////// Coordinates Screen Touch Actions ///////////////////////////////////////////////////////
    else if (CURRENT_SCREEN == 5)
    { // captures touches on drawCoordinatesScreen()
      if (lx > 157 && lx < 240 && ly > 6 && ly < 40) {
        // BTN back pressed
        drawMainScreen();
      }

    }

    ////////////////////////////////////////////////// Load Screen Touch Actions ///////////////////////////////////////////////////////
    else if (CURRENT_SCREEN == 6)
    { // captures touches on drawLoadScreen() .. the one that loads objects from DB
      if (lx > 157 && lx < 240 && ly > 7 && ly < 40)
      {
        // BTN Back pressed
        drawMainScreen();
      }
      if (lx > 165 && lx < 240 && ly > 280 && ly < 320) {
        // BTN next> pressed  TREAS_PAGER
        if (LOAD_SELECTOR == 1)
        {
          MESS_PAGER += 1;
          if (MESS_PAGER <= 5)
          {
            drawLoadObjects();
          }
          else
          {
            MESS_PAGER = 5;
          }
        }
        else if (LOAD_SELECTOR == 2)
        {
          TREAS_PAGER += 1;
          if (TREAS_PAGER <= 6)
          {
            drawLoadObjects();
          }
          else
          {
            TREAS_PAGER = 6;
            //drawLoadScreen();
          }
        }
      }
      if (lx > 0 && lx < 75 && ly > 280 && ly < 320)
      {
        // BTN <prev pressed
        if (LOAD_SELECTOR == 1)
        {
          MESS_PAGER -= 1;
          if (MESS_PAGER >= 0)
          {
            drawLoadObjects();
          }
          else
          {
            MESS_PAGER = 0;
          }
        }
        else if (LOAD_SELECTOR == 2)
        {
          TREAS_PAGER -= 1;
          if (TREAS_PAGER >= 0)
          {
            drawLoadObjects();
          }
          else
          {
            TREAS_PAGER = 0;
          }
        }
      }
      char obj_name[50];
      String OBJ_NAME = "";
      if (!IS_NIGHTMODE) {
        OBJ_NAME = "UI/day/";
      } else {
        OBJ_NAME = "UI/night/";
      }

      if (lx > 0 && lx < 60 && ly > 40 && ly < 87)
      {
        // BTN Messier pressed
        LOAD_SELECTOR = 1;
        OBJ_NAME += "btn_load_1.bin";
        if (SD.open(OBJ_NAME)/* && (IS_TRACKING)*/)
        {
          OBJ_NAME.toCharArray(obj_name, 50);
          drawBin(obj_name, 0, 40, 240, 53); // Drawing selected Object Tumbnail 140*140 Pixels
        }
        drawLoadObjects();
      }
      if (lx > 61 && lx < 121 && ly > 40 && ly < 87)
      {
        LOAD_SELECTOR = 2;
        OBJ_NAME += "btn_load_2.bin";
        if (SD.open(OBJ_NAME)/* && (IS_TRACKING)*/)
        {
          OBJ_NAME.toCharArray(obj_name, 50);
          drawBin(obj_name, 0, 40, 240, 53); // Drawing selected Object Tumbnail 140*140 Pixels
        }
        drawLoadObjects();
      }
      if (lx > 121 && lx < 178 && ly > 40 && ly < 87)
      {
        // BTN Solar System pressed
        LOAD_SELECTOR = 3;
        OBJ_NAME += "btn_load_3.bin";
        if (SD.open(OBJ_NAME)/* && (IS_TRACKING)*/)
        {
          OBJ_NAME.toCharArray(obj_name, 50);
          drawBin(obj_name, 0, 40, 240, 53); // Drawing selected Object Tumbnail 140*140 Pixels
        }
        drawLoadObjects();
      }
      if (lx > 178 && lx < 238 && ly > 40 && ly < 87)
      {
        // BTN custom.csv pressed
        LOAD_SELECTOR = 4;
        OBJ_NAME += "btn_load_4.bin";
        if (SD.open(OBJ_NAME)/* && (IS_TRACKING)*/)
        {
          OBJ_NAME.toCharArray(obj_name, 50);
          drawBin(obj_name, 0, 40, 240, 53); // Draw Selection Button Thumbnail
        }
        drawLoadObjects();
      }

      if (LOAD_SELECTOR == 1)
      {
        // I'm in MESSIER selector and need to check which Messier object is pressed
        for (int i = 0; i < 5; i++) {
          for (int j = 0; j < 4; j++) {
            if (lx > ((j * 56) + 9) && lx < ((j * 56) + 61) && ly > ((i * 33) + 106) && ly < ((i * 33) + 135)) {
              //56 box width ,9start from left , 33 box hight , 110 start from up

              // found button pressed.... ora I need to get his ID and link to the ARRAY;
              int zz = (MESS_PAGER * 20) + (i * 4) + j;
              if (Messier_Array[zz] != "") {
                selectOBJECT_M(zz, 0);
                calculateLST_HA();
                if (ALT > 0) {
                  OnScreenMsg(1);
                  if (IS_SOUND_ON) {
                    SoundOn(note_C, 32);
                    delay(200);
                    SoundOn(note_C, 32);
                    delay(200);
                    SoundOn(note_C, 32);
                    delay(1000);
                  }
                  // Stop Interrupt procedure for tracking.
                  Timer3.stop(); //
                  IS_TRACKING = false;
                  IS_OBJ_FOUND = false;
                  IS_OBJECT_RA_FOUND = false;
                  IS_OBJECT_DEC_FOUND = false;
                  Tracking_type = 1; // Preparar el seguimiento que se usara, 1: Sidereal, 2: Solar, 0: Lunar;
                  Tracking_Mode = "Celest";
                  Slew_timer = millis();
                  Slew_RA_timer = Slew_timer + 20000;   // Give 20 sec. advance to the DEC. We will revise later.
                }
                UpdateObservedObjects();
                MESS_PAGER == 0;
                sun_confirm = false;
                drawMainScreen();
                tft.setTextScale(3);

              }
            }
          }
        }
      }
      else if (LOAD_SELECTOR == 2)
      {
        // I'm in TREASURES selector and need to check which Treasure object is pressed
        for (int i = 0; i < 5; i++)
        {
          for (int j = 0; j < 4; j++)
          {
            if (lx > ((j * 56) + 9) && lx < ((j * 56) + 61) && ly > ((i * 33) + 106) && ly < ((i * 33) + 135))
            {
              //75 box width ,12start from left , 50 box hight , 110 start from up

              // found button pressed.... ora I need to get his ID and link to the ARRAY;
              int zz = (TREAS_PAGER * 20) + (i * 4) + j;
              if (Treasure_Array[zz] != "")
              {
                selectOBJECT_M(zz, 1);
                calculateLST_HA();
                if (ALT > 0)
                {
                  OnScreenMsg(1);
                  if (IS_SOUND_ON)
                  {
                    SoundOn(note_C, 32);
                    delay(200);
                    SoundOn(note_C, 32);
                    delay(200);
                    SoundOn(note_C, 32);
                    delay(1000);
                  }
                  // Stop Interrupt procedure for tracking.
                  Timer3.stop(); //
                  IS_TRACKING = false;
                  IS_OBJ_FOUND = false;
                  IS_OBJECT_RA_FOUND = false;
                  IS_OBJECT_DEC_FOUND = false;
                  Tracking_type = 1; // Preparar el seguimiento que se usara, 1: Sidereal, 2: Solar, 0: Lunar;
                  Tracking_Mode = "Celest";
                  Slew_timer = millis();
                  Slew_RA_timer = Slew_timer + 20000;   // Give 20 sec. advance to the DEC. We will revise later.
                }
                UpdateObservedObjects();
                TREAS_PAGER == 0;
                sun_confirm = false;
                drawMainScreen();
              }
            }
          }
        }
      }
      else if (LOAD_SELECTOR == 3)
      {
        // I'm in Solar System selector and need to check which Solar System object is pressed
        for (int i = 0; i < 5; i++)
        {
          for (int j = 0; j < 4; j++)
          {
            if (lx > ((j * 56) + 9) && lx < ((j * 56) + 61) && ly > ((i * 33) + 106) && ly < ((i * 33) + 135))
            {
              // found button pressed.... ora I need to get his ID and link to the ARRAY;
              int zz = (i * 4) + j;
#ifdef serial_debug
              Serial.print("zz = ");
              Serial.println(zz);
#endif
              if (zz == 3)
              {
                OnScreenMsg(7);
                delay(2000);
                drawMainScreen();
              }
              else if (zz <= 10)
              {
                if (zz == 0) // I chose the Sun
                {
                  drawConfirmSunTrack();
                }
                else
                {
                  planet_pos(zz);
                }
                if (sun_confirm || zz != 0)
                {
                  calculateLST_HA();
                  if (ALT > 0)
                  {
                    OnScreenMsg(1);
                    if (IS_SOUND_ON)
                    {
                      SoundOn(note_C, 32);
                      delay(200);
                      SoundOn(note_C, 32);
                      delay(200);
                      SoundOn(note_C, 32);
                      delay(1000);
                    }
                    // Stop Interrupt procedure for tracking.
                    Timer3.stop(); //
                    IS_TRACKING = false;

                    IS_OBJ_FOUND = false;
                    IS_OBJECT_RA_FOUND = false;
                    IS_OBJECT_DEC_FOUND = false;
                    switch (zz)
                    {
                      case 0:
                        Tracking_type = 2; // Preparar el seguimiento que se usara, 1: Sidereal, 2: Solar, 0: Lunar;
                        Tracking_Mode = "Solar";
                        break;
                      case 10:
                        Tracking_type = 0; // Preparar el seguimiento que se usara, 1: Sidereal, 2: Solar, 0: Lunar;
                        Tracking_Mode = "Lunar";
                        break;
                      default:
                        Tracking_type = 1; // Preparar el seguimiento que se usara, 1: Sidereal, 2: Solar, 0: Lunar;
                        Tracking_Mode = "Celest";
                        break;
                    }
                    Slew_timer = millis();
                    Slew_RA_timer = Slew_timer + 20000;   // Give 20 sec. advance to the DEC. We will revise later.
                  }
                  UpdateObservedObjects();
                  //CUSTOM_PAGER == 0;
                  sun_confirm = false;
                  drawMainScreen();
                }
              }
            }
          }
        }
      }
      else if (LOAD_SELECTOR == 4)
      {
        // I'm in CUSTOM.CSV selector and need to check which Custom object is pressed
        for (int i = 0; i < 5; i++)
        {
          for (int j = 0; j < 4; j++)
          {
            if (lx > ((j * 56) + 9) && lx < ((j * 56) + 61) && ly > ((i * 33) + 106) && ly < ((i * 33) + 135))
            {
              //75 box width ,12start from left , 50 box hight , 110 start from up

              // found button pressed.... ora I need to get his ID and link to the ARRAY;
              int zz = (CUSTOM_PAGER * 20) + (i * 4) + j;
              if (custom_Array[zz] != "")
              {
                selectOBJECT_M(zz, 4);
                calculateLST_HA();
                if (ALT > 0)
                {
                  OnScreenMsg(1);
                  if (IS_SOUND_ON)
                  {
                    SoundOn(note_C, 32);
                    delay(200);
                    SoundOn(note_C, 32);
                    delay(200);
                    SoundOn(note_C, 32);
                    delay(1000);
                  }
                  // Stop Interrupt procedure for tracking.
                  Timer3.stop(); //
                  IS_TRACKING = false;
                  IS_OBJ_FOUND = false;
                  IS_OBJECT_RA_FOUND = false;
                  IS_OBJECT_DEC_FOUND = false;
                  Tracking_type = 1; // Preparar el seguimiento que se usara, 1: Sidereal, 2: Solar, 0: Lunar;
                  Tracking_Mode = "Celest";
                  Slew_timer = millis();
                  Slew_RA_timer = Slew_timer + 20000;   // Give 20 sec. advance to the DEC. We will revise later.
                }
                UpdateObservedObjects();
                CUSTOM_PAGER == 0;
                sun_confirm = false;
                drawMainScreen();
              }
            }
          }
        }
      }
    }

    ////////////////////////////////////////////////// Options Screen Touch Actions ///////////////////////////////////////////////////////
    else if (CURRENT_SCREEN == 7)
    {
      // captures touches on drawOptionsScreen()
      if (lx > 157 && lx < 240 && ly > 0 && ly < 23) {
        // BTN <Back pressed
        storeOptions_SD();
        drawMainScreen();
      }
      if (lx > 157 && lx < 236 && ly > 215 && ly < 246) {
        // Celestial Tracking Selected
        Tracking_type = 1;
        Tracking_Mode = "Celest";
        updateTrackingMode_opt();
      }
      if (lx > 157 && lx < 236 && ly > 247 && ly < 278) {
        // Solar Tracking Selected
        Tracking_type = 2;
        Tracking_Mode = "Solar";
        updateTrackingMode_opt();
      }
      if (lx > 157 && lx < 236 && ly > 279 && ly < 313) {
        // Lunar Tracking Selected
        Tracking_type = 0;
        Tracking_Mode = "Lunar";
        updateTrackingMode_opt();
      }
      if (lx > 259 && lx < 236 && ly > 30 && ly < 77)
      {
        // Screen MAX Brightness
        analogWrite(TFTBright, 255);
        TFT_Brightness = 255;
        updateTriangleBrightness_opt();
      }
      if (lx > 4 && lx < 191 && ly > 33 && ly < 66) {
        // Screen REDUCE Brightness   "Triangle"
        TFT_Brightness = 190 + ((lx - 5) * 0.25);
        analogWrite(TFTBright, TFT_Brightness);
        updateTriangleBrightness_opt();
      }
      if (lx > 3 && lx < 41 && ly > 147 && ly < 180) {
        // ECO Mode - Timeout in seconds  Never
        TFT_timeout = 0;
        TFT_Time = "AL-ON";
        updateScreenTimeout_opt();
      }
      if (lx > 42 && lx < 79 && ly > 147 && ly < 180) {
        // ECO Mode - Timeout in seconds  30 Seconds
        TFT_timeout = 30000;
        TFT_Time = "30 s";
        updateScreenTimeout_opt();
      }
      if (lx > 80 && lx < 118 && ly > 147 && ly < 180) {
        // ECO Mode - Timeout in seconds   60 Seconds
        TFT_timeout = 60000;
        TFT_Time = "60 s";
        updateScreenTimeout_opt();
      }
      if (lx > 119 && lx < 157 && ly > 147 && ly < 180) {
        // ECO Mode - Timeout in seconds   2 Minutes
        TFT_timeout = 120000;
        TFT_Time = "2 min";
        updateScreenTimeout_opt();
      }
      if (lx > 158 && lx < 197 && ly > 147 && ly < 180) {
        // ECO Mode - Timeout in seconds  5 Minutes
        TFT_timeout = 300000;
        TFT_Time = "5 min";
        updateScreenTimeout_opt();
      }
      if (lx > 198 && lx < 238 && ly > 147 && ly < 180) {
        // ECO Mode - Timeout in seconds  10 Minutes
        TFT_timeout = 600000;
        TFT_Time = "10 min";
        updateScreenTimeout_opt();
      }
      if (lx > 83 && lx < 154 && ly > 215 && ly < 246) {
        // ON Meridian Flip
        IS_MERIDIAN_FLIP_AUTOMATIC = true;
        Mer_Flip_State = "AUTO";
        updateMeridianFlip_opt();
      }
      if (lx > 83 && lx < 154 && ly > 247 && ly < 280) {
        // OFF Meridian Flip
        IS_MERIDIAN_FLIP_AUTOMATIC = false;
        Mer_Flip_State = "OFF";
        updateMeridianFlip_opt();
      }
      if (lx > 40 && lx < 72 && ly > 108 && ly < 127) {
        // ON Sound
        IS_SOUND_ON = true;
        Sound_State = "ON";
        updateSound_opt();
      }
      if (lx > 1 && lx < 39 && ly > 108 && ly < 127) {
        // OFF Sound
        IS_SOUND_ON = false;
        Sound_State = "OFF";
        updateSound_opt();
      }
      if (lx > 120 && lx < 153 && ly > 108 && ly < 127)
      {
        // ON Stepper Motors
        IS_STEPPERS_ON = true;
        digitalWrite(RA_EN, LOW);
        digitalWrite(DEC_EN, LOW);
        Stepper_State = "ON";
        updateStepper_opt();
      }
      if (lx > 86 && lx < 119 && ly > 108 && ly < 127)
      {
        // OFF Stepper Motors
        IS_STEPPERS_ON = false;
        digitalWrite(RA_EN, HIGH);
        digitalWrite(DEC_EN, HIGH);
        Stepper_State = "OFF";
        updateStepper_opt();
      }
      if (lx > 202 && lx < 240 && ly > 108 && ly < 127)
      {
        // ON Focus Stepper Motor
        Focus_Motor_State = "ON";
        IS_FOCUS_ON = true;
        digitalWrite(Focus_EN, LOW);
        updateFocus_stepper_opt();
      }
      if (lx > 168 && lx < 201 && ly > 108 && ly < 127)
      {
        // Off Focus Stepper Motor
        Focus_Motor_State = "OFF";
        IS_FOCUS_ON = false;
        digitalWrite(Focus_EN, HIGH);
        updateFocus_stepper_opt();
      }
      //Touched GPS configuration
      if (lx > 7 && lx < 75 && ly > 233 && ly < 320)
      {
        storeOptions_SD();
        CURRENT_SCREEN = 0;
        drawGPSScreen();
      }
      //Touched TFT Calibration
    }

    ////////////////////////////////////////////////// STAT Screen Touch Actions ///////////////////////////////////////////////////////
    else if (CURRENT_SCREEN == 10) // captures touches on drawSTATScreen()
    {
      if (lx > 157 && lx < 240 && ly > 6 && ly < 40)
      {
        // BTN Back pressed
        IS_IN_OPERATION = true;
        drawMainScreen();
        IS_CONNECTED = true;
        IS_PulseGuiding = false;
      }
    }

    //////////////////////////////////////////////////Star Maps Screen Touch Actions ///////////////////////////////////////////////////////
    else if (CURRENT_SCREEN == 11) // captures touches on drawStarMap()
    {
      if (lx > 150 && lx < 240 && ly > 0 && ly < 33)
      {
        // BTN Back pressed
        IS_IN_OPERATION = true;
        drawMainScreen();
      }
      // Take care of Map move...
      // When user touches left, right, top and bottom part of the image,
      // the system loads the corresponding Star Map - moving left, right, top and bottom.
      if (lx > 0 && lx < 37 && ly > 60 && ly < 280) // Left side touched... show next StarMap image
      {
        if (map_c < 8)
        {
          map_c += 1;
          IS_CUSTOM_MAP_SELECTED = true;
          drawStarMap();
        }
      }
      if (lx > 142 && lx < 240 && ly > 60 && ly < 280) {
        // Right side touched... show next StarMap image
        if (map_c > 1) {
          map_c -= 1;
          IS_CUSTOM_MAP_SELECTED = true;
          drawStarMap();
        }
      }
      if (lx > 0 && lx < 240 && ly > 27 && ly < 60) {
        // Top side touched... show next StarMap image
        if (map_r > 1) {
          map_r -= 1;
          IS_CUSTOM_MAP_SELECTED = true;
          drawStarMap();
        }
      }
      if (lx > 0 && lx < 240 && ly > 293 && ly < 320) {
        // Bottom side touched... show next StarMap image
        if (map_r < 3) {
          map_r += 1;
          IS_CUSTOM_MAP_SELECTED = true;
          drawStarMap();
        }
      }
    }

    ////////////////////////////////////////////////// Star Sync Screen Touch Actions ///////////////////////////////////////////////////////
    else if (CURRENT_SCREEN == 12) // captures touches on drawStarSyncScreen()
    {
      if (lx > 157 && lx < 240 && ly > 7 && ly < 40)
      {
        // BTN Done pressed
        IS_IN_OPERATION = true;
        drawMainScreen();
      }
      int do_kolko = 0;
      if (ALLIGN_TYPE == 3)
      {
        // Chage the 14 to represent the real count of screens.
        // They need to be dynamically calculated... not fixed
        do_kolko = int_star_count;
      }
      else
      {
        do_kolko = 14;
      }
      if (lx > 165 && lx < 240 && ly > 280 && ly < 320) {
        // BTN next> pressed
        STARS_PAGER += 1;
        if (STARS_PAGER < do_kolko) {
          //drawStarSyncScreen();
          drawAlignObjects_ali();
        } else {
          STARS_PAGER = do_kolko - 1;
        }
      }
      if (lx > 0 && lx < 75 && ly > 280 && ly < 320) {
        // BTN <prev pressed
        STARS_PAGER -= 1;
        if (STARS_PAGER >= 0) {
          //drawStarSyncScreen();
          drawAlignObjects_ali();
        } else {
          STARS_PAGER = 0;
        }
      }

      if (ALLIGN_TYPE < 3)
      {
        // I'm in STARS selector and need to check which Star object is pressed
        for (int i = 0; i < 6; i++)
        {
          for (int j = 0; j < 4; j++)
          {
            if (lx > ((j * 56) + 9) && lx < ((j * 56) + 61) && ly > ((i * 33) + 57) && ly < ((i * 33) + 85))
            {
              // found button pressed.... ora I need to get his ID and link to the ARRAY; aca estamos
              int zz = (STARS_PAGER * 24) + (i * 4) + j;
              if (Stars[zz] != "")
              {
                selectOBJECT_M(zz, 2);
                calculateLST_HA();
                if (ALT < 0)
                {
                  OnScreenMsg(3);
                  if (IS_SOUND_ON)
                  {
                    SoundOn(note_C, 96);
                    delay(2000);
                  }
                  drawStarSyncScreen();
                }
                else
                {
                  OnScreenMsg(1);
                  if (IS_SOUND_ON)
                  {
                    SoundOn(note_C, 32);
                    delay(200);
                    SoundOn(note_C, 32);
                    delay(200);
                    SoundOn(note_C, 32);
                    delay(200);
                  }
                  // Stop Interrupt procedure for tracking.
                  Timer3.stop(); //
                  IS_TRACKING = false;

                  IS_OBJ_FOUND = false;
                  IS_OBJECT_RA_FOUND = false;
                  IS_OBJECT_DEC_FOUND = false;
                  Slew_timer = millis();
                  Slew_RA_timer = Slew_timer + 20000;   // Give 20 sec. advance to the DEC. We will revise later.
                  STARS_PAGER == 0;
                  SELECTED_STAR = zz;
                  ALLIGN_STEP += 1;
                  //drawConstelationScreen(zz);
                }
              }
            }
          }
        }
      }
      else
      {
        // I'm in STARS selector and need to check which Star object is pressed
        for (int i = 0; i < 6; i++)
        {
          for (int j = 0; j < 4; j++)
          {
            if (lx > ((j * 56) + 7) && lx < ((j * 56) + 59) && ly > ((i * 33) + 57) && ly < ((i * 33) + 87))
            {
              // found button pressed.... ora I need to get his ID and link to the ARRAY;
              int zz = (STARS_PAGER * 24) + (i * 3) + j;
              if (Iter_Stars[zz] != "")
              {
                // selectOBJECT_M(zz,3);
                Iterative_Star_Index = zz;
                // replaced the above with the below row...
                // to make sure each press selects the same star (inital or on Step 2)
                // change made after Test on 17th March.
                selectOBJECT_M(Iterative_Star_Index, 3);
                calculateLST_HA();
                OnScreenMsg(1);
                if (IS_SOUND_ON)
                {
                  SoundOn(note_C, 32);
                  delay(200);
                  SoundOn(note_C, 32);
                  delay(200);
                  SoundOn(note_C, 32);
                  delay(200);
                }
                // Stop Interrupt procedure for tracking.
                Timer3.stop(); //
                IS_TRACKING = false;
                IS_OBJ_FOUND = false;
                IS_OBJECT_RA_FOUND = false;
                IS_OBJECT_DEC_FOUND = false;
                Slew_timer = millis();
                Slew_RA_timer = Slew_timer + 20000;   // Give 20 sec. advance to the DEC. We will revise later.
                STARS_PAGER == 0;
                SELECTED_STAR = zz;
                ALLIGN_STEP += 1;
                // drawConstelationScreen(zz);
              }
            }
          }
        }
      }
    }

    ////////////////////////////////////////////////// Constellations Screen Touch Actions ///////////////////////////////////////////////////////
    else if (CURRENT_SCREEN == 13)
    { // captures touches on drawConstelationScreen(int indx)
      if (lx > 0 && lx < 75 && ly > 280 && ly < 320)
      {
        // BTN "<Repeat" or "<EXIT" pressed
        if (ALLIGN_TYPE == 3)
        {
          // delta_a_RA = 0;
          // delta_a_DEC = 0;
          STARS_PAGER = 0;
          IS_TRACKING = false;
          IS_IN_OPERATION = true;
          drawMainScreen();
        }
        else
        {
          ALLIGN_STEP -= 1;
          drawStarSyncScreen();
        }
      }
      if (lx > 165 && lx < 240 && ly > 280 && ly < 320)
      {
        // BTN "ALIGN!" pressed
        // Here we need to kora which Star is this - 1st, 2nd, 3rd... etc ?
        // In order to use Ralph Pass alignment procedure described on http://rppass.com/
        // http://rppass.com/align.pdf - the actual PDF

        if (ALLIGN_STEP == 1)
        {
          if (ALLIGN_TYPE == 1)
          {
            IS_TRACKING = false;
            IS_IN_OPERATION = true;
            drawMainScreen();
          }
          else if (ALLIGN_TYPE == 3)
          {
            // Select Polaris and SlewTo...
            IS_TRACKING = false; // Select Polaris and SlewTo...
            IS_TRACKING = false;
            //
            //
            // ora I have to SYNC on that STAR!
            // Assign Values calculated for
            // SLEW_RA_microsteps and SLEW_DEC_microsteps
            //
            selectOBJECT_M(192, 2); // Polaris in on Index 192 in the Stars Array
            calculateLST_HA();
            OnScreenMsg(1);
            if (IS_SOUND_ON)
            {
              SoundOn(note_C, 32);
              delay(200);
              SoundOn(note_C, 32);
              delay(200);
              SoundOn(note_C, 32);
              delay(200);
            }
            // Stop Interrupt procedure for tracking.
            Timer3.stop(); //
            IS_TRACKING = false;
            IS_OBJ_FOUND = false;
            IS_OBJECT_RA_FOUND = false;
            IS_OBJECT_DEC_FOUND = false;
            Slew_timer = millis();
            Slew_RA_timer = Slew_timer + 20000;   // Give 20 sec. advance to the DEC. We will revise later.
            //drawConstelationScreen(0);
            ALLIGN_STEP = 2;
          }
        }
        else if (ALLIGN_STEP == 2)
        {
          if (ALLIGN_TYPE == 3)
          {
            // Select First Star Again and SlewTo...
            IS_TRACKING = false;
            selectOBJECT_M(Iterative_Star_Index, 3); // Load First Star from the Stars Array
            calculateLST_HA();
            OnScreenMsg(1);
            if (IS_SOUND_ON)
            {
              SoundOn(note_C, 32);
              delay(200);
              SoundOn(note_C, 32);
              delay(200);
              SoundOn(note_C, 32);
              delay(200);
            }
            // Stop Interrupt procedure for tracking.
            Timer3.stop(); //
            IS_TRACKING = false;
            IS_OBJ_FOUND = false;
            IS_OBJECT_RA_FOUND = false;
            IS_OBJECT_DEC_FOUND = false;
            Slew_timer = millis();
            Slew_RA_timer = Slew_timer + 20000;   // Give 20 sec. advance to the DEC. We will revise later.
            //drawConstelationScreen(0);
            ALLIGN_STEP = 1;
          }
        }
      }
    }
    ////////////////////////////////////////////////// Auto-Guiding Screen Touch Actions ///////////////////////////////////////////////////////
    else if (CURRENT_SCREEN == 14) // captures touches on drawAutoGuidingScreen()
    {
      if (lx > 82 && lx < 157 && ly > 280 && ly < 320)
      {
        // BTN Back pressed
        IS_IN_OPERATION = true;
        CURRENT_SCREEN = 4;
        drawMainScreen();
      }
    }
    /////////////////////////////////////////////// Confirm Sun Tracking Screen Touch Actions /////////////////////////////////////////////////////////
    else if (CURRENT_SCREEN == 15)
    {
      // Capture touch inputs on drawConfirmSunTrack()
      if (lx > 7 && lx < 112 && ly > 253 && ly < 307) //BTN YES
      {
        DrawButton(7, 253, 105, 53, "YES", 0, btn_l_selection, btn_l_text, 3, false);
        planet_pos(0);
        sun_confirm = true;

        if (sun_confirm)
        {
          calculateLST_HA();
          if (ALT > 0)
          {
            OnScreenMsg(1);
            if (IS_SOUND_ON)
            {
              SoundOn(note_C, 32);
              delay(200);
              SoundOn(note_C, 32);
              delay(200);
              SoundOn(note_C, 32);
              delay(1000);
            }
            // Stop Interrupt procedure for tracking.
            Timer3.stop(); //
            IS_TRACKING = false;
            IS_OBJ_FOUND = false;
            IS_OBJECT_RA_FOUND = false;
            IS_OBJECT_DEC_FOUND = false;
            Tracking_type = 2; // Change the tracking type Automatically. 1: Sidereal, 2: Solar, 0: Lunar;
            Tracking_Mode = "Solar";
            updateTrackingMode_opt();
            Slew_timer = millis();
            Slew_RA_timer = Slew_timer + 20000;   // Give 20 sec. advance to the DEC. We will revise later.
          }
          UpdateObservedObjects();
          sun_confirm = false;
          drawMainScreen();
        }
      }
      else if (lx > 170 && lx < 310 && ly > 380 && ly < 460) //BTN NO
      {
        DrawButton(127, 254, 105, 53, "NO", 0, btn_l_selection, btn_l_text, 3, false);
        drawLoadScreen();
        sun_confirm = false;
      }
    }
  }
  else
  {
    //**************************************************************
    //
    //      BUTTON UP Events start here
    //
    //      - only executed when the user touches the screen - RELEASE
    //**************************************************************

    if (CURRENT_SCREEN == 0) {
      if (last_button == 1) {
        last_button = 0;
        // Set the earth rotation direction depending on the Hemisphere...
        // HIGH and LOW are substituted
        if (OBSERVATION_LATTITUDE > 0) {
          STP_FWD = LOW;
          STP_BACK = HIGH;
          STP_FWD_RA = LOW;
          STP_BACK_RA = HIGH;
        } else {
          STP_FWD = HIGH;
          STP_BACK = LOW;
          STP_FWD_RA = LOW;
          STP_BACK_RA = HIGH;
        }
#ifdef serial_debug
        Serial.println(OBSERVATION_LATTITUDE);
        Serial.print("STP_FWD = ");
        Serial.println(STP_FWD);
        Serial.print("STP_BACK = ");
        Serial.println(STP_BACK);
#endif
        CURRENT_SCREEN = 1;
        drawClockScreen();
      }
    } else if (CURRENT_SCREEN == 1) {
      if (last_button == 1) {
        last_button = 0;
        tft.drawRect(11, 150, 60, 33, messie_btn);
        TimerUpdateDraw(1);
      }
      if (last_button == 2) {
        last_button = 0;
        tft.drawRect(90, 150, 60, 33, messie_btn);
        TimerUpdateDraw(2);
      }
      if (last_button == 3) {
        last_button = 0;
        tft.drawRect(172, 150, 60, 33, messie_btn);
        TimerUpdateDraw(3);
      }
      if (last_button == 4) {
        last_button = 0;
        tft.drawRect(11, 190, 60, 33, messie_btn);
        TimerUpdateDraw(4);
      }
      if (last_button == 5) {
        last_button = 0;
        tft.drawRect(90, 190, 60, 33, messie_btn);
        TimerUpdateDraw(5);
      }
      if (last_button == 6) {
        last_button = 0;
        tft.drawRect(172, 190, 60, 33, messie_btn);
        TimerUpdateDraw(6);
      }
      if (last_button == 7) {
        last_button = 0;
        tft.drawRect(11, 231, 60, 33, messie_btn);
        TimerUpdateDraw(7);
      }
      if (last_button == 8) {
        last_button = 0;
        tft.drawRect(90, 231, 60, 33, messie_btn);
        TimerUpdateDraw(8);
      }
      if (last_button == 9) {
        last_button = 0;
        tft.drawRect(172, 231, 60, 33, messie_btn);
        TimerUpdateDraw(9);
      }
      if (last_button == 10) {
        last_button = 0;
        tft.drawRect(90, 272, 60, 33, messie_btn);
        TimerUpdateDraw(0);
      }
      if (last_button == 22) {
        String OBJ_NAME_1 = "";
        char obj_name[50];
        if (Summer_Time == 1) {
          Summer_Time = 0;
          if (!IS_NIGHTMODE) {
            OBJ_NAME_1 = "UI/day/bg_time_2_off.bin";
          } else {
            OBJ_NAME_1 = "UI/night/bg_time_2_off.bin";
          }
        } else {
          Summer_Time = 1;
          if (!IS_NIGHTMODE) {
            OBJ_NAME_1 = "UI/day/bg_time_2_on.bin";
          } else {
            OBJ_NAME_1 = "UI/night/bg_time_2_on.bin";
          }
        }
        if (SD.open(OBJ_NAME_1)) {
          OBJ_NAME_1.toCharArray(obj_name, 50);
          drawBin(obj_name, 0, 277, 240, 43); // Drawing selected Object Tumbnail 140*140 Pixels
        }
        tft.setTextColor(btn_l_text);
        tft.setTextScale(3);
        last_button = 0;
      }


    } else if (CURRENT_SCREEN == 4) {
      tft.setTextColor(btn_l_text);

      if (last_button == 1 && IS_BT_MODE_ON == false) {
        last_button = 0;

        if (IS_STEPPERS_ON == true) {
          drawLoadScreen();
        } else if (IS_STEPPERS_ON == false) {
          //tft.fillRect(1, 325, 100, 70, BLACK);
          //DrawButton( 1, 325, 100, 70, "LOAD", btn_d_border, btn_l_border, btn_l_text, 2, false);
          OnScreenMsg(5);
          delay(1000);
          CURRENT_SCREEN = 7;
          drawOptionsScreen();
        }
      }
      if (last_button == 4 && IS_BT_MODE_ON == false) {
        last_button = 0;
        drawSTATScreen();
      }

      if (last_button == 2 && IS_BT_MODE_ON == false && IS_STEPPERS_ON == true) {
        last_button = 0;
        IS_TRACKING = false;
        Timer3.stop();
        OnScreenMsg(6);
        OBJECT_NAME = "CP";
        OBJECT_DESCR = "Celestial pole";
        OBJECT_RA_H = 12;
        OBJECT_RA_M = 0;
        OBJECT_DEC_D = 90;
        OBJECT_DEC_M = 0;
        IS_OBJ_FOUND = false;
        IS_OBJECT_RA_FOUND = false;
        IS_OBJECT_DEC_FOUND = false;
        Slew_timer = millis();
        Slew_RA_timer = Slew_timer + 20000;   // Give 20 sec. advance to the DEC. We will revise later.
        OBJECT_DETAILS = "The north and south celestial poles are the two imaginary points in the sky where the Earth's axis of rotation, intersects the celestial sphere";
      }

      if (last_button == 2 && IS_BT_MODE_ON == false && IS_STEPPERS_ON == false) {
        last_button = 0;
        IS_TRACKING = false;
        Timer3.stop();
        OnScreenMsg(5);
        delay(1000);
        drawOptionsScreen();
      }

      if ((last_button == 5) || (last_button == 11)) {
        last_button = 0;
        if (MAIN_SCREEN_MENU == 0) {
          MAIN_SCREEN_MENU = 1;
        } else {
          MAIN_SCREEN_MENU = 0;
        }
        drawMainScreen_Menu(1);
      }
      if (last_button == 3) {
        last_button = 0;
        if (IS_STEPPERS_ON == true) {
          if (IS_TRACKING == false) {
            IS_TRACKING = true;
            if (!IS_NIGHTMODE) {
              drawBin("UI/day/btn_track_on.bin", 166.5, 250, 68, 29);
            } else {
              drawBin("UI/night/btn_track_on.bin", 166.5, 250, 68, 29);
            }
            tft.setTextScale(2);
            tft.fillRect(0, 87, 240, 19, BLACK);
            tft.setTextColor(btn_l_border);
            tft.cursorToXY(1, 89);
            if (OBJECT_NAME.length() > 7) {
              tft.setTextScale(2);
              tft.cursorToXY(1, 97);
              tft.print("TRK:");
            } else {
              tft.setTextScale(2);
              tft.print("TRACKING:");
            }
            tft.setTextColor(title_bg);
            tft.print(OBJECT_NAME);
            setmStepsMode("R", MICROSteps);
            if (Tracking_type == 1) { // 1: Sidereal, 2: Solar, 0: Lunar;
              Timer3.start(Clock_Sidereal);
            } else if (Tracking_type == 2) {
              Timer3.start(Clock_Solar);
            } else if (Tracking_type == 0) {
              Timer3.start(Clock_Lunar);
            }
          } else {
            IS_TRACKING = false;
            if (!IS_NIGHTMODE) {
              drawBin("UI/day/btn_track_off.bin", 166.5, 250, 68, 29);
            } else {
              drawBin("UI/night/btn_track_off.bin", 166.5, 250, 68, 29);
            }
            tft.setTextScale(2);
            tft.fillRect(0, 87, 240, 195, BLACK);
            tft.setTextColor(l_text);
            tft.cursorToXY(1, 89);

            if (OBJECT_NAME.length() > 7) {
              tft.setTextScale(2);
              tft.cursorToXY(1, 97);
              tft.print("OBS:");
            } else {
              tft.setTextScale(2);
              tft.print("OBSERVING:");
            }
            tft.setTextColor(title_bg);
            tft.print(OBJECT_NAME);
            //OnScreenMsg(2);
            setmStepsMode("R", 1);
            Timer3.stop(); //
          }
        }
        else if (IS_STEPPERS_ON == false) {
          if (!IS_NIGHTMODE) {
            drawBin("UI/day/btn_track_off.bin", 166.5, 250, 68, 29);
          } else {
            drawBin("UI/night/btn_track_off.bin", 166.5, 250, 68, 29);
          }
          OnScreenMsg(5);
          delay(1000);
          CURRENT_SCREEN = 7;
          drawOptionsScreen();
        }
      }

      if (last_button == 6) {   // BlueTooth Mode
        if (IS_STEPPERS_ON == true) {

          if (IS_BT_MODE_ON == true) {
            IS_BT_MODE_ON = false;
          } else {
            IS_BT_MODE_ON = true;
            // Initialize Bluetooth communication on PINs: 15 (RX) and 14 (TX)
            OBJECT_NAME = "BT";
            drawBin("objects/BT.bin", 0, 95, 240, 94);
          }
          last_button = 0;
          drawMainScreen();
        }
        else if (IS_STEPPERS_ON == false) {
          last_button = 0;
          OnScreenMsg(5);
          delay(1000);
          CURRENT_SCREEN = 7;
          drawOptionsScreen();
        }
      }

      if (last_button == 7 && IS_BT_MODE_ON == false) {
        last_button = 0;
        if ((IS_TRACKING == true) && (IS_SOUND_ON)) {
          SoundOn(note_C, 32);
        }
        drawCoordinatesScreen();
      }
      if (last_button == 8 && IS_BT_MODE_ON == false) {
        last_button = 0;
        drawStarMap();
      }
      if (last_button == 9)
      {
        last_button = 0;
        if (IS_FAN1_ON)
        {
          IS_FAN1_ON = false;

          if (!IS_NIGHTMODE) {
            drawBin("UI/day/btn_fan1_off.bin", 166.5, 250, 68, 29);
          } else {
            drawBin("UI/night/btn_fan1_off.bin", 166.5, 250, 68, 29);
          }
          digitalWrite(FAN1, LOW);
          Fan1_State = "OFF";
          drawStatusBar();
        }
        else
        {
          IS_FAN1_ON = true;
          if (!IS_NIGHTMODE) {
            drawBin("UI/day/btn_fan1_on.bin", 166.5, 250, 68, 29);
          } else {
            drawBin("UI/night/btn_fan1_on.bin", 166.5, 250, 68, 29);
          }
          digitalWrite(FAN1, HIGH);
          Fan1_State = "ON";
          drawStatusBar();
        }
        storeOptions_SD();
      }
      if (last_button == 10)
      {
        last_button = 0;
        drawOptionsScreen();
      }
      if (last_button == 12)
      {
        last_button = 0;
        if (IS_FAN2_ON)
        {
          IS_FAN2_ON = false;
          if (!IS_NIGHTMODE) {
            drawBin("UI/day/btn_fan2_off.bin", 166.5, 285, 68, 29);
          } else {
            drawBin("UI/night/btn_fan2_off.bin", 166.5, 285, 68, 29);
          }
          digitalWrite(FAN2, LOW);
          Fan2_State = "OFF";
          drawStatusBar();
        }
        else
        {
          IS_FAN2_ON = true;
          if (!IS_NIGHTMODE) {
            drawBin("UI/day/btn_fan2_on.bin", 166.5, 285, 68, 29);
          } else {
            drawBin("UI/night/btn_fan2_on.bin", 166.5, 285, 68, 29);
          }
          digitalWrite(FAN2, HIGH);
          Fan2_State = "ON";
          drawStatusBar();
        }
        storeOptions_SD();
      }
    }
  }
  return lz;
}
