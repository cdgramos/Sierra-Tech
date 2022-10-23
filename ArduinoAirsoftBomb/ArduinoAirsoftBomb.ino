#include <Wire.h>

#include <LiquidCrystal_I2C.h>

#include <Keypad.h>

#include <time.h>

#include <EEPROM.h>

//uncomment this line if using a Common Anode LED
#define COMMON_ANODE

// Display configurations (library NewliquidCrystal)
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

// Led configurations  PINS
const int redLightPin = 4;
const int greenLightPin = 3;
const int blueLightPin = 7;

// Buzzer configurations PINS
const int buzzerPin = 9;

// Domination configurations PINS
const int dominationRed = 12;
const int dominationBlue = 6;

int redButtonState = 0;
int blueButtonState = 0;
int teamRed = 1;
int teamBlue = 2;
int teamWhite = 3;
int teamYellow = 4;
int teamControlling = 0;

//  Actions
#define ACTION_CONFIG_TIME 0
#define ACTION_CONFIG_SECRET 1
#define ACTION_CONFIG_TRIES 2
#define ACTION_START_BOMB 3
#define ACTION_TIME_COUTING 4
#define ACTION_DEACTIVATED 5
#define ACTION_BLASTED 6
#define ACTION_ENDED 7

#define ACTION_MENU 8

#define ACTION_DOMINATION 9
#define ACTION_DOMINATION_END 10

#define ACTION_BEACON 11
#define ACTION_BEACON_END 12

// General purpose iterators
int i = 0;
int j = 0;

int auxMinutos = 0;
int auxSegundos = 0;

/* createDelayFlag
 * 0 - Do nothing
 * 1 - Start couting the delay time until next action
 * 2 - Delay ended
 */
int createDelayFlag = 0;

// Delay incrementor, used to check if the configured amount of ticks have been done
int delayIncrementor = 0;

// Number of delay ticks.
int delayCriteria = 1;

// Debugging purposes
char userInput[16] = "";

// Current status of the application
int currentStatus;

/* infiniteTriesFlag
 * 0 - The number of tries have a limit
 * 1 - User can do infinite tries
 */
int infiniteTriesFlag = 0;

// Total number of tries
int numberOfTries;

/*/
Tries formatting and length
*/
char triesString[4] = "";
int triesLength;

/*
Controls the Error message when a wrong attempt is made
*/
int displayError = 0;
int displayErrorTimer = 0;

// Total time until blast in seconds
long timeInSeconds;

char inputTime[7] = "";

char timeTemp[3] = "";

int inputLength;

char secret[9] = "";
int secretLength;

char attempt[9] = "";
int attemptLength = 0;

char formattedTime[17] = "00:00:00";

char formattedTries[4] = "000";

char timeString[16] = "HH:MM:SS";
char timeStringEmpty[16] = "00:00:00";
char timeStringFill[16] = "00:00:00";

unsigned long previousMillis = 0;
unsigned long previousMillis1 = 0;
unsigned long previousMillis2 = 0;
const long interval = 1000;
int startBomb = 0;

// Cursor variables
int timeStringFillPos = 0;
int showTimeStringFlag = 0;
int timeStringFillAux = 0;

int secretStringFillPos = 0;
int showSecretStringFlag = 0;
int secretStringFillAux = 0;

int triesStringFillPos = 0;
int showTriesStringFlag = 0;
int triesStringFillAux = 0;

int attemptStringFillPos = 0;
int showAttemptStringFlag = 0;
int attemptStringFillAux = 0;

// Auxiliar variable to convert integers
char inputTimeAux[2] = "";

int lockKey = 0;
const int lockKeyInterval = 5000;

//EEPROM variables
int addrGameMode = 0;
int addrGameModeValue;
int gameModeTerrorist = 1;
int gameModeDomination = 2;
int gameModeBeacon = 3;

char redTeamStatic[7] = "RED:  ";
char blueTeamStatic[7] = "BLUE: ";
char formattedTimeRed[10] = "00:00:00";
char formattedTimeBlue[10] = "00:00:00";
char redTeamTimer[17];
char blueTeamTimer[17];
int timeRed = 0;
int timeBlue = 0;

int analogRed = 0;
int analogBlue = 0;

int startBeacon = 0;



/*char formattedTimeRed[10] = "00:00:00";
char formattedTimeBlue[10] = "00:00:00";
char redTeamTimer[17];
char blueTeamTimer[17];
int timeRed = 0;
int timeBlue = 0;
*/




/* setup Function.
 *
 * Program initial configurations.
 * @return void
 */
void setup() {

  Serial.begin(9600);
  Serial.println("SETUP");

  addrGameModeValue = EEPROM.read(addrGameMode);
  Serial.print("addrGameModeValue: ");
  Serial.println(addrGameModeValue);

  if (addrGameModeValue != gameModeTerrorist && addrGameModeValue != gameModeDomination && addrGameModeValue != gameModeBeacon) {
    Serial.println("Initializing device as gameModeTerrorist");
    // set default game mode
    EEPROM.update(addrGameMode, gameModeTerrorist);
    addrGameModeValue = gameModeTerrorist;
  }

  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0, 0);
  if (addrGameModeValue == gameModeTerrorist) {
    lcd.print("TIME UNTIL BLAST");
  } else if (addrGameModeValue == gameModeDomination) {
    lcd.print("TIME UNTIL WIN");
  } else if (addrGameModeValue == gameModeBeacon) {
    lcd.print("TIME UNTIL END");
  }

  lcd.setCursor(0, 1);
  lcd.print(timeString);

  currentStatus = ACTION_CONFIG_TIME;

  inputLength = 0;
  secretLength = 0;
  triesLength = 0;
  timeInSeconds = 0;
  numberOfTries = 0;

  pinMode(redLightPin, OUTPUT);
  pinMode(greenLightPin, OUTPUT);
  pinMode(blueLightPin, OUTPUT);

  ledRGBColor(0, 0, 0);

  pinMode(buzzerPin, OUTPUT);

  pinMode(dominationRed, OUTPUT);
  pinMode(dominationBlue, INPUT);

  digitalWrite(dominationRed, HIGH);
  digitalWrite(dominationBlue, LOW);

  analogRed = 0;
  analogBlue = 0;

}

/*
A = Config / Menu
B = Back 
C = <-
D = ->
* = Clean
# = Ok
*/
char get_button() {
  int val = analogRead(A0);
  if (val > 1000)
    return '#';
  else if (val > 900)
    return 'D';
  else if (val > 820)
    return '0';
  else if (val > 750)
    return 'C'; //Menu
  else if (val > 660)
    return '*';
  else if (val > 620)
    return '9';
  else if (val > 585)
    return '8';
  else if (val > 540)
    return '7';
  else if (val > 500)
    return 'B';
  else if (val > 475)
    return '6';
  else if (val > 455)
    return '5';
  else if (val > 425)
    return '4';
  else if (val > 370)
    return 'A';
  else if (val > 300)
    return '3';
  else if (val > 260)
    return '2';
  else if (val > 200)
    return '1';
  else
    return NULL;
}

/* getFormattedTries Function.
 * @tries the number of tries to format.
 * @formattedTries pointer to the string where the formatted Tries will be written.
 *
 * For a given number of tries formats it as 3 mandatory digits string.
 * @return void
 */
void getFormattedTries(int tries, char * formattedTries) {
  char tempTries[3] = "";

  //Serial.println("Formatted tries");
  //Serial.println(tries);

  itoa(tries, tempTries, 10);

  if (tries > 99) {
    formattedTries[0] = tempTries[0];
    formattedTries[1] = tempTries[1];
    formattedTries[2] = tempTries[2];

  } else if (tries <= 99 && tries > 9) {
    formattedTries[0] = '0';
    formattedTries[1] = tempTries[0];
    formattedTries[2] = tempTries[1];

  } else {
    formattedTries[0] = '0';
    formattedTries[1] = '0';
    formattedTries[2] = tempTries[0];
  }

}

/* ledRGBColor Function.
 * @redLightValue the red light intensity.
 * @greenLightValue the green light intensity.
 * @blueLightValue the blue light intensity.
 *
 * For a given number of tries formats it as 3 mandatory digits string.
 * @return void
 */
void ledRGBColor(int redLightValue, int greenLightValue, int blueLightValue) {
  #ifdef COMMON_ANODE
  redLightValue = 255 - redLightValue;
  greenLightValue = 255 - greenLightValue;
  blueLightValue = 255 - blueLightValue;
  #endif
  analogWrite(redLightPin, redLightValue);
  analogWrite(greenLightPin, greenLightValue);
  analogWrite(blueLightPin, blueLightValue);
}

/* getFormattedTimeFromSeconds Function.
 * @totSeconds number os seconds to be formatted.
 * @formattedTime pointer to the string where the formatted time will be written.
 *
 * For a given number of seconds formats it as a HH:MM:SS string.
 * @return void
 */
void getFormattedTimeFromSeconds(long totSeconds, char * formattedTime) {

  int hour = totSeconds / 3600;
  totSeconds %= 3600;
  int minutes = totSeconds / 60;
  totSeconds %= 60;
  int seconds = totSeconds;

  char tempTime[2] = "";

  itoa(hour, tempTime, 10);

  if (hour < 10) {
    formattedTime[0] = '0';
    formattedTime[1] = tempTime[0];
  } else {
    formattedTime[0] = tempTime[0];
    formattedTime[1] = tempTime[1];
  }

  itoa(minutes, tempTime, 10);

  if (minutes < 10) {
    formattedTime[3] = '0';
    formattedTime[4] = tempTime[0];
  } else {
    formattedTime[3] = tempTime[0];
    formattedTime[4] = tempTime[1];
  }

  itoa(seconds, tempTime, 10);

  if (seconds < 10) {
    formattedTime[6] = '0';
    formattedTime[7] = tempTime[0];
  } else {
    formattedTime[6] = tempTime[0];
    formattedTime[7] = tempTime[1];
  }

}

void showFormattedInputTime() {

}

/* loop Function.
 *
 * Code that will be running endlessly.
 * @return void
 */
void loop() {

  char key = get_button();

  if (key == NULL) {
    lockKey = 0;
  }

  if (key && lockKey == 0) {
    lockKey = 1;

    delay(250);
    // Serial.println(key);
    // strncat(userInput, & key, 1);
    // Serial.println(userInput);

    if (currentStatus == ACTION_MENU) {
      Serial.println("ACTION_MENU");
      Serial.print("Current action is:");
      Serial.println(addrGameModeValue);

      //D is ->
      //C is <- 
      if ((key == 'D') and(addrGameModeValue == gameModeTerrorist)) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("GAME MODE:");
        lcd.setCursor(0, 1);
        lcd.print("DOMINATION");
        addrGameModeValue = gameModeDomination;
      } else if ((key == 'D') and(addrGameModeValue == gameModeDomination)) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("GAME MODE:");
        lcd.setCursor(0, 1);
        lcd.print("BEACON");
        addrGameModeValue = gameModeBeacon;
      } else if ((key == 'D') and(addrGameModeValue == gameModeBeacon)) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("GAME MODE:");
        lcd.setCursor(0, 1);
        lcd.print("PLANT THE BOMB");
        addrGameModeValue = gameModeTerrorist;
      } else if ((key == 'C') and(addrGameModeValue == gameModeTerrorist)) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("GAME MODE:");
        lcd.setCursor(0, 1);
        lcd.print("BEACON");
        addrGameModeValue = gameModeBeacon;
      } else if ((key == 'C') and(addrGameModeValue == gameModeBeacon)) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("GAME MODE:");
        lcd.setCursor(0, 1);
        lcd.print("DOMINATION");
        addrGameModeValue = gameModeDomination;
      } else if ((key == 'C') and(addrGameModeValue == gameModeDomination)) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("GAME MODE:");
        lcd.setCursor(0, 1);
        lcd.print("PLANT THE BOMB");
        addrGameModeValue = gameModeTerrorist;
      }

      // Save option to EEPROM
      if (key == '#') {
        EEPROM.update(addrGameMode, addrGameModeValue);

        tone(buzzerPin, 1000, 100);
        currentStatus = ACTION_CONFIG_TIME;

        lcd.clear();
        lcd.setCursor(0, 0);
        if (addrGameModeValue == gameModeTerrorist) {
          lcd.print("TIME UNTIL BLAST");
        } else if (addrGameModeValue == gameModeDomination) {
          lcd.print("TIME UNTIL WIN");
        } else if (addrGameModeValue == gameModeBeacon) {
          lcd.print("TIME UNTIL END");
        }
        lcd.setCursor(0, 1);
        lcd.print(timeString);

      }

      if (key == 'B') {
        tone(buzzerPin, 1000, 100);
        currentStatus = ACTION_CONFIG_TIME;

        lcd.clear();
        lcd.setCursor(0, 0);
        if (addrGameModeValue == gameModeTerrorist) {
          lcd.print("TIME UNTIL BLAST");
        } else if (addrGameModeValue == gameModeDomination) {
          lcd.print("TIME UNTIL WIN");
        } else if (addrGameModeValue == gameModeBeacon) {
          lcd.print("TIME UNTIL END");
        }
        lcd.setCursor(0, 1);
        lcd.print(timeString);
      }

    }

    if (currentStatus == ACTION_CONFIG_TIME) {
      Serial.println("ACTION_CONFIG_TIME");

      if (key == 'A') {
        tone(buzzerPin, 1000, 100);
        currentStatus = ACTION_MENU;

        showTimeStringFlag = 0;

        secretLength = 0;

        strncpy(secret, "", sizeof(secret));

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("GAME MODE:");
        lcd.setCursor(0, 1);

        if (addrGameModeValue == gameModeTerrorist) {
          lcd.print("PLANT THE BOMB");
        } else if (addrGameModeValue == gameModeDomination) {
          lcd.print("DOMINATION");
        } else if (addrGameModeValue == gameModeBeacon) {
          lcd.print("BEACON");
        }

      }

      if (key == '#') {
        Serial.println("card");

        long auxTime;

        //go to next status
        if (inputLength > 0) {
          tone(buzzerPin, 1000, 100);

          //seconds
          timeTemp[0] = timeStringFill[6];
          timeTemp[1] = timeStringFill[7];

          sscanf(timeTemp, "%ld", & auxTime);

          timeInSeconds += auxTime;

          //minutes
          timeTemp[0] = timeStringFill[3];
          timeTemp[1] = timeStringFill[4];

          sscanf(timeTemp, "%ld", & auxTime);

          timeInSeconds += (auxTime * 60);

          //hours
          timeTemp[0] = timeStringFill[0];
          timeTemp[1] = timeStringFill[1];

          sscanf(timeTemp, "%ld", & auxTime);

          timeInSeconds += (auxTime * 60 * 60);

          Serial.println("Total time in seconds is:");
          Serial.println(timeInSeconds);
          Serial.println("Formatted time:");
          getFormattedTimeFromSeconds(timeInSeconds, formattedTime);
          Serial.println(formattedTime);

          if (timeInSeconds > 0) {
            showTimeStringFlag = 0;

            if (addrGameModeValue == gameModeTerrorist) {
              currentStatus = ACTION_CONFIG_SECRET;
              showSecretStringFlag = 1;

              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("SECRET CODE");
              lcd.setCursor(0, 1);
              lcd.print("          ");

            } else if (addrGameModeValue == gameModeDomination) {
              currentStatus = ACTION_DOMINATION;
              teamControlling = 0;

              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print(redTeamStatic);
              lcd.setCursor(7, 0);
              getFormattedTimeFromSeconds(timeRed, formattedTimeRed);
              lcd.print(formattedTimeRed);

              lcd.setCursor(0, 1);
              lcd.print(blueTeamStatic);
              lcd.setCursor(7, 1);
              getFormattedTimeFromSeconds(timeBlue, formattedTimeBlue);
              lcd.print(formattedTimeBlue);

            } else if (addrGameModeValue == gameModeBeacon) {
              currentStatus = ACTION_BEACON;
              teamControlling = 0;
              createDelayFlag = 1;
              startBeacon = 1;

              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("TEAM: ");
              lcd.setCursor(0, 1);
              getFormattedTimeFromSeconds(timeInSeconds, formattedTime);
              lcd.print(timeInSeconds);

            }

          }

        }
      }

      if (key == '*') {
        Serial.println("ast");

        tone(buzzerPin, 1000, 100);

        // Reset the input
        inputLength = 0;
        timeStringFillPos = 0;
        timeStringFillAux = 0;

        auxMinutos = 0;
        auxSegundos = 0;

        strncpy(timeStringFill, "00:00:00", sizeof(timeStringFill));

        lcd.setCursor(0, 1);
        lcd.print(timeStringEmpty);

        strncpy(inputTime, "", sizeof(inputTime));

      }

      if (isdigit(key)) {
        Serial.println("digit");
        /*inputTimeAux[0] = key;
        int numericValueKey = atoi(inputTimeAux);
        Serial.println(numericValueKey);*/

        int isToUpdate = 1;

        if (inputLength < 6) {
          showTimeStringFlag = 1;

          inputLength++;
          strncat(inputTime, & key, 1);
        } else {
          isToUpdate = 0;
        }

        if (inputLength == 1) {
          tone(buzzerPin, 1000, 100);
          timeStringFill[0] = key;
          timeStringFillPos = 1;
        }
        if (inputLength == 2 && isToUpdate == 1) {
          tone(buzzerPin, 1000, 100);
          timeStringFill[1] = key;
          timeStringFillPos = 3;
        }
        if (inputLength == 3) {
          tone(buzzerPin, 1000, 100);
          timeStringFill[3] = key;
          timeStringFillPos = 4;
        }
        if (inputLength == 4) {
          tone(buzzerPin, 1000, 100);
          timeStringFill[4] = key;
          timeStringFillPos = 6;
        }
        if (inputLength == 5) {
          tone(buzzerPin, 1000, 100);
          timeStringFill[6] = key;
          timeStringFillPos = 7;
        }
        if (inputLength == 6) {
          tone(buzzerPin, 1000, 100);
          timeStringFill[7] = key;
          timeStringFillPos = -1;
        }

      }

      Serial.println("inputTime is:");
      Serial.println(inputTime);
    }


// TO DO
    if (currentStatus == ACTION_BEACON) {
      Serial.println("ACTION_BEACON");

      //We need the analog buttons too so here we only define the Key inputs

     /* if (key == 'B') {
        Serial.println("Back");
        tone(buzzerPin, 1000, 100);
        currentStatus = ACTION_CONFIG_TIME;

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("TIME UNTIL END");
        lcd.setCursor(0, 1);
        lcd.print(timeStringFill);
        showTimeStringFlag = 1;

        showSecretStringFlag = 0;

        secretLength = 0;
        secretStringFillAux = 0;
        secretStringFillPos = 0;

        auxMinutos = 0;
        auxSegundos = 0;

        timeInSeconds = 0;

        strncpy(secret, "", sizeof(secret));
      }*/
      
      // yellow
      if (key == '*') {
        Serial.println("yellowTeam");
        teamControlling = teamYellow;
        lcd.setCursor(6, 0);
        lcd.print("      ");
        lcd.setCursor(6, 0);
        lcd.print("YELLOW");
        }
       if (key == '0') {
        Serial.println("whiteTeam");
        teamControlling = teamWhite;
        lcd.setCursor(6, 0);
        lcd.print("      ");
        lcd.setCursor(6, 0);
        lcd.print("WHITE");
        }

    }



    if (currentStatus == ACTION_CONFIG_SECRET) {
      Serial.println("ACTION_CONFIG_SECRET");

      if (key == 'B') {
        Serial.println("Back");
        tone(buzzerPin, 1000, 100);
        currentStatus = ACTION_CONFIG_TIME;

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("TIME UNTIL BLAST");
        lcd.setCursor(0, 1);
        lcd.print(timeStringFill);
        showTimeStringFlag = 1;

        showSecretStringFlag = 0;

        secretLength = 0;
        secretStringFillAux = 0;
        secretStringFillPos = 0;

        auxMinutos = 0;
        auxSegundos = 0;

        timeInSeconds = 0;

        strncpy(secret, "", sizeof(secret));

      }

      if (key == '#') {

        Serial.println("card");

        if (secretLength > 0 && secretLength < 9) {
          showSecretStringFlag = 0;
          showTriesStringFlag = 1;
          currentStatus = ACTION_CONFIG_TRIES;
          tone(buzzerPin, 1000, 100);
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("MAXIMUM  TRIES");
          lcd.setCursor(0, 1);
          lcd.print("          ");
          createDelayFlag = 1;
        }
      }

      if (isdigit(key)) {
        Serial.println("digit");

        if (secretLength < 8) {
          secretLength++;
          tone(buzzerPin, 1000, 100);
          strncat(secret, & key, 1);

          secretStringFillPos = secretLength;

          if (secretLength == 8)
            secretStringFillPos = -1;

          //lcd.setCursor(0, 1);
          //lcd.print(secret);

        }
      }

      if (key == '*') {
        Serial.println("ast");
        tone(buzzerPin, 1000, 100);
        secretLength = 0;
        secretStringFillAux = 0;
        secretStringFillPos = 0;

        strncpy(secret, "", sizeof(secret));

        lcd.setCursor(0, 1);
        lcd.print("          ");

      }

    }

    if (currentStatus == ACTION_CONFIG_TRIES) {
      Serial.println("ACTION_CONFIG_TRIES");

      if (key == 'B') {
        Serial.println("Back");
        tone(buzzerPin, 1000, 100);
        currentStatus = ACTION_CONFIG_SECRET;

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("SECRET CODE");
        lcd.setCursor(0, 1);
        lcd.print(secret);
        showSecretStringFlag = 1;

        showTriesStringFlag = 0;

        triesLength = 0;
        triesStringFillAux = 0;
        triesStringFillPos = 0;

        strncpy(triesString, "", sizeof(triesString));

      }

      if (key == '#' && createDelayFlag == 2) {
        Serial.println("card");

        if (triesLength >= 0 && triesLength < 1000) {
          currentStatus = ACTION_START_BOMB;
          tone(buzzerPin, 1000, 100);
          showTriesStringFlag = 0;
          // then there is no limit
          if (triesLength == 0) {
            infiniteTriesFlag = 1;
          }

          numberOfTries = atoi(triesString);

          if (numberOfTries == 0) {
            infiniteTriesFlag = 1;
          }

          createDelayFlag = 1;

          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("PRESS  OK  TO");
          lcd.setCursor(0, 1);
          lcd.print("START");

        }
      }

      if (isdigit(key)) {
        Serial.println("digit");

        if (triesLength < 3) {
          triesLength++;
          tone(buzzerPin, 1000, 100);
          strncat(triesString, & key, 1);

          triesStringFillPos = triesLength;

          if (triesLength == 3)
            triesStringFillPos = -1;

          //lcd.setCursor(0, 1);
          //lcd.print(triesString);

        }
      }

      if (key == '*') {
        Serial.println("ast");
        tone(buzzerPin, 1000, 100);
        triesLength = 0;
        triesStringFillAux = 0;
        triesStringFillPos = 0;

        strncpy(triesString, "", sizeof(triesString));

        lcd.setCursor(0, 1);
        lcd.print("          ");

      }
    }

    if (currentStatus == ACTION_START_BOMB) {

      Serial.println("ACTION_START_BOMB");

      if (key == 'B') {
        Serial.println("Back");
        tone(buzzerPin, 1000, 100);
        currentStatus = ACTION_CONFIG_TRIES;

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("MAXIMUM  TRIES");
        lcd.setCursor(0, 1);
        lcd.print(triesString);

        showTriesStringFlag = 1;

      }

      if (key == '#' && createDelayFlag == 2) {
        Serial.println("detonation to start");
        lcd.clear();
        startBomb = 1;
        tone(buzzerPin, 1000, 100);
        currentStatus = ACTION_TIME_COUTING;
        showAttemptStringFlag = 1;

        int i;

        for (i = 0; i < secretLength; i++) {
          lcd.setCursor(i, 1);
          lcd.print("_");
        }

        createDelayFlag = 1;

      }
    }

    if (currentStatus == ACTION_TIME_COUTING) {
      Serial.println("ACTION_TIME_COUTING");
      Serial.println(key);

      if (key == '#' && createDelayFlag == 2 && attemptLength == secretLength) {
        Serial.println("card");
        if (strcmp(secret, attempt) == 0) {

          // Equal, it will be deactivated!
          showAttemptStringFlag = 0;
          currentStatus = ACTION_DEACTIVATED;
          startBomb = 0;
          lcd.setCursor(0, 1);
          lcd.print("BOMB DEACTIVATED");
          ledRGBColor(0, 255, 0);

        } else {
          // Wrong try
          if (infiniteTriesFlag == 0) {
            //if not infiniteTriesFlag then reduce one
            numberOfTries--;

            Serial.println("TRIES LEFT");
            Serial.println(numberOfTries);

            if (numberOfTries == 0) {
              Serial.println("TOO MANY WRONG TRIES");
              currentStatus = ACTION_BLASTED;
              startBomb = 0;

              lcd.setCursor(9, 0);
              lcd.print("|");

              lcd.setCursor(11, 0);
              getFormattedTries(numberOfTries, formattedTries);
              lcd.print(formattedTries);

              tone(buzzerPin, 1000, 10000);

            }

          } else {
            Serial.println("Infinite tries");
          }

          attemptStringFillAux = 0;
          attemptStringFillPos = 0;

          displayError = 1;

          attemptLength = 0;

          strncpy(attempt, "", sizeof(attempt));

          for (i = 0; i < secretLength; i++) {
            lcd.setCursor(i, 1);
            lcd.print("_");
          }
        }

      }

      if (isdigit(key)) {
        Serial.println("digit");

        if (attemptLength <= (secretLength - 1)) {

          attemptLength++;

          attemptStringFillPos = attemptLength;
          if (attemptLength == secretLength) {
            attemptStringFillPos = -1;
          }

          strncat(attempt, & key, 1);

          //lcd.setCursor(0, 1);
          //lcd.print(attempt);
        }
      }

      if (key == '*') {
        Serial.println("ast");

        attemptLength = 0;
        attemptStringFillAux = 0;
        attemptStringFillPos = 0;

        strncpy(attempt, "", sizeof(attempt));

        for (i = 0; i < secretLength; i++) {
          lcd.setCursor(i, 1);
          lcd.print("_");
        }
      }
    }

  }

  // Actions that are Key based end here


// TO DO
    if (currentStatus == ACTION_BEACON) {
      Serial.println("ACTION_BEACON in analog");

    
        analogBlue = analogRead(A2);
        analogRed = analogRead(A1);


    
        if (analogBlue > 1020) {
          teamControlling = teamBlue;
        } else if (analogRed > 1020) {
          teamControlling = teamRed;
        }

  
      if (analogBlue > 1020) {
        teamControlling = teamBlue;
        Serial.println("blueTeam");
        lcd.setCursor(6, 0);
        lcd.print("      ");
        lcd.setCursor(6, 0);
        lcd.print("BLUE");
      } else if (analogRed > 1020) {
        teamControlling = teamRed;
        Serial.println("redTeam");
        lcd.setCursor(6, 0);
        lcd.print("      ");
        lcd.setCursor(6, 0);
        lcd.print("RED");
      }

  

 unsigned long currentMillis = millis();

    if (timeInSeconds % 2 == 0) {
      if (teamControlling == teamRed)
        ledRGBColor(255, 0, 0);
      if (teamControlling == teamBlue)
        ledRGBColor(0, 0, 255);
      if (teamControlling == teamYellow)
        ledRGBColor(255, 255, 0);
      if (teamControlling == teamWhite)
        ledRGBColor(255, 255, 255);
    } else {
      ledRGBColor(0, 0, 0);
    }
    

    }

  

  if (currentStatus == ACTION_DOMINATION) {
    // redButtonState = digitalRead(dominationRed);
    // Show the state of pushbutton on serial monitor
    //Serial.print("TEAM: ");
    //Serial.println(teamControlling);

    // blueButtonState = digitalRead(dominationBlue);
    // Show the state of pushbutton on serial monitor

    analogBlue = analogRead(A2);
    analogRed = analogRead(A1);

    if (analogBlue > 1020) {
      teamControlling = teamBlue;
    } else if (analogRed > 1020) {
      teamControlling = teamRed;
    }



    /*START*/

    unsigned long currentMillis = millis();

    if (timeInSeconds % 2 == 0) {
      if (teamControlling == teamRed)
        ledRGBColor(255, 0, 0);
      if (teamControlling == teamBlue)
        ledRGBColor(0, 0, 255);
    } else {
      ledRGBColor(0, 0, 0);
    }

    if (currentMillis - previousMillis >= interval) {

      previousMillis = currentMillis;

      if (teamControlling != 0)
        tone(buzzerPin, 1000, 100);

      //if (timeRed != timeInSeconds && timeBlue != timeInSeconds){
      if (teamControlling == teamRed)
        timeRed++;
      if (teamControlling == teamBlue)
        timeBlue++;
      //0  }

      if (teamControlling != 0) {
        //lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(redTeamStatic);
        lcd.setCursor(7, 0);
        getFormattedTimeFromSeconds(timeRed, formattedTimeRed);
        lcd.print(formattedTimeRed);

        lcd.setCursor(0, 1);
        lcd.print(blueTeamStatic);
        lcd.setCursor(7, 1);
        getFormattedTimeFromSeconds(timeBlue, formattedTimeBlue);
        lcd.print(formattedTimeBlue);
      }

    }
    /*END*/

    if (timeRed == timeInSeconds || timeBlue == timeInSeconds) {
      lcd.clear();

      Serial.println("WE HAVE A DOMINATOR");
      currentStatus = ACTION_DOMINATION_END;

      tone(buzzerPin, 1000, 10000);
    }

  }

  if (currentStatus == ACTION_DOMINATION_END) {

    lcd.setCursor(0, 0);
    if (teamControlling == teamRed) {
      ledRGBColor(255, 0, 0);

      lcd.print("RED TEAM WINS!");
    }
    if (teamControlling == teamBlue) {
      ledRGBColor(0, 0, 255);

      lcd.print("BLUE TEAM WINS!");
    }

  }

  if (currentStatus == ACTION_BLASTED) {
    Serial.println("ACTION_BLASTED");

    showAttemptStringFlag = 0;

    lcd.setCursor(0, 1);
    lcd.print("               ");
    lcd.setCursor(0, 1);
    lcd.print("BOMB EXPLODED");
    currentStatus = ACTION_ENDED;
    ledRGBColor(255, 0, 0);

  }

  if (currentStatus == ACTION_BEACON_END) {
    Serial.println("ACTION_BEACON_END");

    currentStatus = ACTION_ENDED;

    lcd.setCursor(0, 1);
    lcd.print("               ");
    lcd.setCursor(0, 1);
    if( (teamControlling == teamRed) or (teamControlling == teamBlue) or (teamControlling == teamYellow) or (teamControlling == teamWhite)){
      lcd.print("BEACON CONQUEST");
    }
    else{
      lcd.print("BEACON LOST");
    }
    
     if (teamControlling == teamRed)
        ledRGBColor(255, 0, 0);
      if (teamControlling == teamBlue)
        ledRGBColor(0, 0, 255);
      if (teamControlling == teamYellow)
        ledRGBColor(255, 255, 0);
      if (teamControlling == teamWhite)
        ledRGBColor(255, 255, 255);

  }


  if (startBeacon == 1) {
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= interval) {

      previousMillis = currentMillis;

      tone(buzzerPin, 1000, 100);

      lcd.setCursor(0, 1);
      getFormattedTimeFromSeconds(timeInSeconds, formattedTime);
      lcd.print(formattedTime);

      timeInSeconds--;

      if (timeInSeconds < 0) {
        Serial.println("TIME IS UP");
        startBomb = 0;
        currentStatus = ACTION_BEACON_END;

        tone(buzzerPin, 1000, 10000);

        startBeacon = 0;
      }
    }
  }
  

  if (startBomb == 1) {
    unsigned long currentMillis = millis();

    if (timeInSeconds % 2 == 0) {
      ledRGBColor(255, 0, 0);
    } else {
      ledRGBColor(0, 0, 0);
    }

    if (currentMillis - previousMillis >= interval) {

      previousMillis = currentMillis;

      tone(buzzerPin, 1000, 100);

      lcd.setCursor(0, 0);
      getFormattedTimeFromSeconds(timeInSeconds, formattedTime);
      lcd.print(formattedTime);

      if (infiniteTriesFlag == 0) {
        lcd.setCursor(9, 0);
        lcd.print("|");

        lcd.setCursor(11, 0);
        getFormattedTries(numberOfTries, formattedTries);
        lcd.print(formattedTries);
      }

      timeInSeconds--;

      if (timeInSeconds < 0) {
        Serial.println("TIME IS UP");
        startBomb = 0;
        currentStatus = ACTION_BLASTED;

        tone(buzzerPin, 1000, 10000);
      }

      if (displayError == 1) {
        displayErrorTimer++;

        lcd.setCursor(10, 1);
        lcd.print("ERROR");

        if (displayErrorTimer == 3) {
          displayError = 0;
          displayErrorTimer = 0;
        }

      } else {
        lcd.setCursor(10, 1);
        lcd.print("     ");
      }

    }
  }

  // Show tries configuration string
  if (showTriesStringFlag == 1) {

    unsigned long currentMillis2 = millis();

    if (currentMillis2 - previousMillis2 >= 500) {

      previousMillis2 = currentMillis2;

      triesStringFillAux++;

      // Render with or without the square
      if (triesStringFillAux == 2 && triesStringFillPos != -1) {

        lcd.setCursor(0, 1);
        lcd.print(triesString);

        lcd.setCursor(triesStringFillPos, 1);
        lcd.print((char) 255);

        triesStringFillAux = 0;
      } else {
        lcd.setCursor(0, 1);
        lcd.print(triesString);

        if (triesStringFillPos != -1) {
          lcd.setCursor(triesStringFillPos, 1);
          lcd.print(" ");
        }
      }

    }

  }

  // Show attempt configuration string
  if (showAttemptStringFlag == 1) {

    unsigned long currentMillis2 = millis();

    if (currentMillis2 - previousMillis2 >= 500) {

      previousMillis2 = currentMillis2;

      attemptStringFillAux++;

      // Render with or without the square
      if (attemptStringFillAux == 2 && attemptStringFillPos != -1) {

        lcd.setCursor(0, 1);
        lcd.print(attempt);

        lcd.setCursor(attemptStringFillPos, 1);
        lcd.print((char) 255);

        attemptStringFillAux = 0;
      } else {
        lcd.setCursor(0, 1);
        lcd.print(attempt);

        if (attemptStringFillPos != -1) {
          lcd.setCursor(attemptStringFillPos, 1);
          lcd.print("_");
        }
      }

    }

  }

  // Show secret configuration string
  if (showSecretStringFlag == 1) {

    unsigned long currentMillis2 = millis();

    if (currentMillis2 - previousMillis2 >= 500) {

      previousMillis2 = currentMillis2;

      secretStringFillAux++;

      // Render with or without the square
      if (secretStringFillAux == 2 && secretStringFillPos != -1) {

        lcd.setCursor(0, 1);
        lcd.print(secret);

        lcd.setCursor(secretStringFillPos, 1);
        lcd.print((char) 255);

        secretStringFillAux = 0;
      } else {
        lcd.setCursor(0, 1);
        lcd.print(secret);

        if (secretStringFillPos != -1) {
          lcd.setCursor(secretStringFillPos, 1);
          lcd.print(" ");
        }
      }

    }

  }

  // Show time configuration string
  if (showTimeStringFlag == 1) {

    unsigned long currentMillis2 = millis();

    if (currentMillis2 - previousMillis2 >= 500) {

      previousMillis2 = currentMillis2;

      timeStringFillAux++;

      // Render with or without the square
      if (timeStringFillAux == 2 && timeStringFillPos != -1) {

        lcd.setCursor(0, 1);
        lcd.print(timeStringFill);

        lcd.setCursor(timeStringFillPos, 1);
        lcd.print((char) 255);

        timeStringFillAux = 0;
      } else {
        lcd.setCursor(0, 1);
        lcd.print(timeStringFill);
      }

    }

  }

  // Tries delay
  if (createDelayFlag == 1) {

    unsigned long currentMillis1 = millis();

    if (currentMillis1 - previousMillis1 >= interval) {

      previousMillis1 = currentMillis1;

      delayIncrementor++;

      if (delayIncrementor == delayCriteria) {
        createDelayFlag = 2;
        delayIncrementor = 0;
      }
    }

  }

}
