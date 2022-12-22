//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////Snaky - Arduino Game///////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////Ion Apelia////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



//LIBRARIES//////////////////////////////////////////////////////////////////////////////////////
#include <LiquidCrystal.h>
#include "LedControl.h" 
#include <EEPROM.h>


//BUZZER////////////////////////////////////////////////////////////////////////////////////////
const int buzzerPin = 13;

//JOYSTICK///////////////////////////////////////////////////////////////////////////////////////
const int xPin = A0;
const int yPin = A1;
const int pinSW = 2;

//min max Threshold
const int minThreshold = 400;
const int maxThreshold = 600;

bool joyMoved = false;
bool previousJoyMove = false;

bool didTransition = false;
bool willTransition = false;

byte swState ;  //button state
byte lastSwState = HIGH;  //last button state

int xValue = 0;
int yValue = 0;

//for long button press
long holdingTime;
long previousHoldingTime;
unsigned long firstButtonPressTime;
unsigned long longPressTime = 2000;

//for joystick button press
int press;  //press state

bool shortPress = false;
bool longPress = false;

//for debounce
unsigned long lastDebounceTime = 0;  //for scrolling
const unsigned long debounceDelay = 100;


//LCD///////////////////////////////////////////////////////////////////////////////
const byte rs = 9;
const byte en = 8;
const byte d4 = 7;
const byte d5 = 3;  //swap
const byte d6 = 5;
const byte d7 = 4;
const byte LCD_Backlight = 6;  //swap
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
byte lcdBrightness = 150;


//MATRIX////////////////////////////////////////////////////////////////////////////////
const int dinPin = 12;
const int clockPin = 11;
const int loadPin = 10;

LedControl lc = LedControl(dinPin, clockPin, loadPin, 1); // DIN, CLK, LOAD, No. DRIVER


byte matrixBrightness = 4;
const byte matrixSize = 8;


byte smiley[matrixSize][matrixSize] = {
  {0,0,1,1,1,1,0,0},
  {0,1,0,0,0,0,1,0},
  {1,0,1,0,0,1,0,1},
  {1,0,0,0,0,0,0,1},
  {1,0,1,0,0,1,0,1},
  {1,0,0,1,1,0,0,1},
  {0,1,0,0,0,0,1,0},
  {0,0,1,1,1,1,0,0}
};

byte sad[matrixSize][matrixSize] = {
  {0,0,1,1,1,1,0,0},
  {0,1,0,0,0,0,1,0},
  {1,0,1,0,0,1,0,1},
  {1,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,1},
  {1,0,1,1,1,1,0,1},
  {0,1,0,0,0,0,1,0},
  {0,0,1,1,1,1,0,0}
};

//


//GAME//////////////////////////////////////////////////////////////////////////////////// 
byte xPos = 0;
byte yPos = 0;


int foodX, foodY; // Coordinates of the food

int gameScore = 0;
int lives = 3;

//for blinking food
unsigned long lastBlinkTime = 0;
const unsigned long blinkDuration = 300;
bool foodOn = false;




bool matrixChanged = false;

bool gameShouldRun = false;

//snake vars
const int snakeMaxLength = 64; //all the leds on the matrix
int snakeX[snakeMaxLength];  // X position of each segment of the snake
int snakeY[snakeMaxLength];  // Y position of each segment of the snake
int snakeLength = 3;  // Length of the snake 
//obs actual snake length is snakelength-1 (what is displayed)
int snakeSpeed = 500; //speed of the snake
int snakeDirection = 1; 
bool shouldUpdateInfo = false;
bool shouldLvUp = false;

//for moving the snake
unsigned long long lastMoved = 0;
unsigned long lastMoveTime;  // Time of the last move





//Structs///////////////////////////////////////////////////////////////////////////////////

struct highscores{
  char name[10] = "Unknown";
  unsigned int score = 0;
};


// EEPROM ADRESSES//////////////////////////////////////////////////////////////////////////////

//const unsigned int hsAddress[] = {967, 978, 989, 1000, 1012};

const unsigned short int hsAddress[] = { 100, 150, 200, 250, 300 };
const unsigned short int soundAdd = 350;
const unsigned short int lcdBrightnessAdd = 400;
const unsigned short int matrixBrightnessAdd = 450;


//MENU///////////////////////////////////////////////////////////////////////////////////////////

// used in a switch to display the main menu or the submenu
int mainMenu = 0;  
//
bool settingsMenuRunning = false;
//
bool settingsSubMenuRunning = false;
// settings menu option
int option = 0;
//time to display first welcome message
unsigned long greetingTime = 2000;
// checks if the display was cleared after the welcome message was displayed
bool clear = 0;

unsigned long startTime = 0;
unsigned long currentTime ;
//page to display
int page = 0;
// toogles on off sound
bool sound;
 //position for left-right scroll
int pos = 0;
//level (dificulty)
int level = 1;
int levels = 2;
bool settingLevel = false;
bool setMatrixBrightness = false;
bool setLcdBrightness = false;
bool newHighscore = false;  //checks is the player has a highscore



//CUSTOM STUFF//////////////////////////////////////////////////////////////////////////////

//custom character - Up Arrow
byte upArrow[] = {
  B00000,
  B00100,
  B01110,
  B11111,
  B00100,
  B00100,
  B00100,
  B00000
};
//custom character - Down Arrow
byte downArrow[] = {
  B00000,
  B00100,
  B00100,
  B00100,
  B11111,
  B01110,
  B00100,
  B00000
};
//custom character - happy face
byte happy[] = {
  B00000,
  B00000,
  B01010,
  B01010,
  B00000,
  B10001,
  B01110,
  B00000
};
//custom character - sun
byte sun[] = {
  B00000,
  B00100,
  B10101,
  B01110,
  B01110,
  B10101,
  B00100,
  B00000
};
//custom character - heart
byte heart[] = {
  B00000,
  B00000,
  B11011,
  B11111,
  B11111,
  B01110,
  B00100,
  B00000
};

//blinking text
#define Blink_interval  500            
unsigned long previousMillis = 0;
bool BlankOnOff = false;

bool blinkCharacterCalled = false;


// strings to display/////////////////////////////////////////////////////////////////////////
String  gameName = "Game: Snakey";
String creator = "Creator: Apelia-Ion";
String github = "https://github.com/Apelia-Ion/Snake-Matrix-Game";
String instructions = "Use the joystick to play. Click to exit. That's all!";
String Pname = "__________ ";




////////////////////////////// SETUP //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {

  
  pinMode(pinSW, INPUT_PULLUP); 
  pinMode(LCD_Backlight, OUTPUT); 
  pinMode(buzzerPin, OUTPUT);

  analogWrite(LCD_Backlight, lcdBrightness);

  
  lc.shutdown(0, false); // turn off power saving, enables display
  lc.setIntensity(0, matrixBrightness); // sets brightness (0~15 possible values)
  lc.clearDisplay(0);// clear screen

 
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.createChar(0, upArrow);
  lcd.createChar(1, downArrow);
  lcd.createChar(2, happy);
  lcd.createChar(3, sun);
  lcd.createChar(4, heart);


 // Initialize the snake
  //snakeLength = INITIAL_LENGTH;
  snakeDirection = 1;
  for (int i = 0; i < snakeLength; i++) {
    snakeX[i] = i;
    snakeY[i] = 0;
  }

    // Generate the first food
  generateFood();

// get from eeprom
  EEPROM.get(soundAdd, sound);
  EEPROM.get(lcdBrightnessAdd, lcdBrightness);
  EEPROM.get(matrixBrightnessAdd, matrixBrightness);


 Serial.begin(9600);
}




////////////////////////////// LOOP //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void loop() {

  Welcome();
}





/////////////////////////////////// GAME FUNCTIONS ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//levels up the player
void LvUp(){
  
  if(shouldLvUp)
  {
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Level UP: ");
    if(level < 2)
    {
      level++;
      snakeSpeed = snakeSpeed - 150;

      shouldLvUp = false;
    gameShouldRun = false;
    lcd.setCursor(10, 0);
    lcd.print(level);
    lcd.setCursor(0, 1);
    lcd.print("Walls + Speed");
    lc.clearDisplay(0);
    delay(3000);
    lcd.clear();
    gameShouldRun = true;

    }
    

    //Serial.println(level);
  }

}

 // Generate random x and y coordinates for the food
void generateFood() {
  // Generate random coordinates for the food
  foodX = random(matrixSize);
  foodY = random(matrixSize);

  // Check if the food is placed on top of the snake
  for (int i = 0; i < snakeLength; i++) {
    if (foodX == snakeX[i] && foodY == snakeY[i]) {
      // If the food is placed on top of the snake, regenerate the coordinates
      generateFood();
      return;
    }
  }
  matrixChanged = true;
}

//makes a noise when the snake eats
void eatBuzz(){
  if(sound){
     tone(buzzerPin, 1000);
     delay(50);
     noTone(buzzerPin);
  }
 

  //analogWrite(LCD_Backlight, lcdBrightness);  //pt ca nu am avut timp sa asez firele a.i sa nu interfereze buzzerul cu lcd-ul -> light flicker

}

//makes a noise when the snake dies (loses a life)
void deathBuzz(){
   if(sound){
     tone(buzzerPin, 600);
     delay(100);
     noTone(buzzerPin);
  }
}

//checks for snake-snake or snake-Wall colision (deppends on the level)
bool checkCollision() {
  // Check if the snake has collided with itself (skip the first segment)
  for (int i = 2; i < snakeLength; i++) {
    if (snakeX[0] == snakeX[i] && snakeY[0] == snakeY[i]) {
      lives--;
      shouldUpdateInfo = true;
      deathBuzz();
      return true;
    }
  }

  // Check if the snake has collided with the walls
  if (snakeX[0] < 0 || snakeX[0] >= matrixSize || snakeY[0] < 0 || snakeY[0] >= matrixSize) {
    lives--;
    shouldUpdateInfo = true;
    deathBuzz();
    return true;
  }

  return false;
}

//bliks the food pellet
void blink() {
  if (millis() - lastBlinkTime > blinkDuration) {
    lastBlinkTime = millis();
    foodOn = !foodOn;
    lc.setLed(0, foodX, foodY, foodOn);
  }
}

//draws the snake on the matrix
void drawSnake() {
  // Clear the matrix
  lc.clearDisplay(0);

  // Draw each segment of the snake
  for (int i = 0; i < snakeLength; i++) {
    lc.setLed(0, snakeX[i], snakeY[i], true);
  }
}

//draws teh food on the matrix
void drawFood() {
  // Draw the food
  lc.setLed(0, foodX, foodY, true);
  foodOn = true;
}

//checks weather the snake has eaten the food
void eatenFood(){
   if (snakeX[0] == foodX && snakeY[0] == foodY) {
      // Increase the snake's length
      snakeLength++;
      if(level == 1)
      {
        gameScore++;
      }
      else
      {
        gameScore = gameScore + 5;
      }
      
      shouldUpdateInfo = true;
      eatBuzz();

      // Generate a new piece of food
      generateFood();
    }
    if((snakeLength-3) == 3)  // when he ate 3 pellets
  {
     shouldLvUp = true;
  }
}

//gets called when the game ends aka no more lives
void gameOver() {

  //save the score and the player in eeprom if necessary
  highscores smallestHighScore;
  EEPROM.get(hsAddress[4], smallestHighScore);
  if (gameScore > smallestHighScore.score) 
  {
    Serial.println("new hs");
    newHighscore = true;
    updateHighScores();
  }
 


  //stop the game
  gameShouldRun = false;
  //show finish screens, click to return to main menu
  endGame();
  //reset the game (score, player name, level, lives)
  prepareForRestart();

  
}

//reads the imput from the joystick and establishes the moving direction
void jsMove() {

 int xValue = analogRead(yPin);
 int yValue = analogRead(xPin);
 


 if (xValue < minThreshold)   // UP 0
 {
   if (xPos > 0) 
    {
      xPos--;
      snakeDirection = 0;
    }
    else 
    {
      xPos = matrixSize - 1;
    }
 }

 if (xValue > maxThreshold) //DOWN 2
 {
   if (xPos < (matrixSize - 1)) 
   {
      xPos++;
      snakeDirection = 2;

    } 
    else 
    {
      xPos = 0;
    }
 }


  if (yValue > maxThreshold) // RIGHT 1
  {
    if (yPos < (matrixSize - 1)) 
    {
      yPos++;
      snakeDirection = 1;

    } 
    else 
    {
      yPos = 0;
    }
  }

  if (yValue < minThreshold) //LEFT 3
  {
    if (yPos > 0) 
    {
      yPos--;
      snakeDirection = 3;
    }
    else 
    {
      yPos = matrixSize - 1;
    }
  }

}

//updates the snakeX and snakeY arrays in order to make it move
void updateSnakePosition() {
  // Update the position of the head of the snake
  if (snakeDirection == 0) {
    snakeX[0]--;
  } else if (snakeDirection == 1) {
    snakeY[0]++;
  } else if (snakeDirection == 2) {
    snakeX[0]++;
  } else if (snakeDirection == 3) {
    snakeY[0]--;
  }

  // Update the position of each segment of the snake
  for (int i = snakeLength - 1; i > 0; i--) {
    snakeX[i] = snakeX[i - 1];
    snakeY[i] = snakeY[i - 1];
  }

  if(level == 1)
  {
      if (snakeX[0] < 0) {
      snakeX[0] = matrixSize - 1;
    } else if (snakeX[0] >= matrixSize) {
      snakeX[0] = 0;
    }
    if (snakeY[0] < 0) {
      snakeY[0] = matrixSize - 1;
    } else if (snakeY[0] >= matrixSize) {
      snakeY[0] = 0;
    }
  }


  matrixChanged = true;
}

//implements the whole game logic
void Game(){

  updateGameInfoLcd();
  jsMove();
  blink();
  LvUp();

   // Check if it's time to move the snake
  if (millis() - lastMoveTime > snakeSpeed) {
    // Update the snake's position
    updateSnakePosition();

    // Check if the snake has collided with itself or the walls
    if (checkCollision()) {
      resetGame();
      return;
    }

    // Check if the snake has eaten the food
    eatenFood();
   

    if(matrixChanged == true) 
  {
    drawSnake();
    drawFood();
    matrixChanged = false;
  }
    


    lastMoveTime = millis();
  }

}

//displays the game info on dhe lcd durig the game
void updateGameInfoLcd(){

  if(shouldUpdateInfo)
  {
   lcd.setCursor(0,1);
    lcd.print("Scor:");
    lcd.setCursor(5, 1);
    lcd.print(gameScore);
    lcd.setCursor(10, 1);
    lcd.write(byte(4));
    lcd.setCursor(12, 1);
    lcd.print(lives);


  }

}

//resets the game when the player lost a life
void resetGame() {
  if(lives > 0)
  {
    // Reset the snake's position and direction
  snakeX[0] = 3;
  snakeY[0] = 3;
  snakeDirection = 1;


  // Generate a new piece of food
  generateFood();
  }
  else //altfel jocul s-a terminat
  {
    gameOver();
  }
  
}

void prepareForRestart(){
  snakeX[0] = 3;
  snakeY[0] = 3;
  snakeDirection = 1;

  // Reset the snake's length
  snakeLength = 3;
  gameScore = 0;
  lives = 3;
  Pname = "__________ ";

  // Generate a new piece of food
  generateFood();

}






///////////////////////////////////////////// Functions for GAME MENU ////////////////////////////////////////////////////////////////////////////////////////////////////////

// Clear text by overwriting it with padding - prevents flickering
void paddingClear(){
  lcd.setCursor(0, 0);
  lcd.print("                "); 
  lcd.setCursor(0, 1);
  lcd.print("                ");
}

// Welcome message - displays welcomeing message for 5 seconds, then clears the lcd and displays the menu
void Welcome(){
  if (millis() <= greetingTime)
  {
    lcd.setCursor(0, 0);
    lcd.print("Hi there gamer!");
    lcd.setCursor(0, 1);
    lcd.print("      ^-^      ");  
  }
  else
  {
    if (!clear) {
      lcd.clear();
      clear = 1;
    }

   switch(mainMenu){
     case 0:    // 0 -. suntem in meniul principal
     {

      shortPress = false;
      
      page = pagesScroll(4);
      

      Menu(page);

      buttonPress2();

      if (shortPress == true)
      {
        mainMenu = 1;
      }       
      
     }
     break;

     case 1:  //-> suntem in meniul secundar
     {
       if (!settingsMenuRunning && !settingsSubMenuRunning){
          SubMenu(page);
       }
      
       if (gameShouldRun)
       {
         if (Pname == "__________ ")
         {
           lcd.clear();
           settingsSubMenuRunning = true;
           SettingsSubMenu(option);
           settingsSubMenuRunning = false;
           lcd.clear();
           SubMenu(0);

         }
         else
         {
          Game();
         }
          
         
         
       }
       

       if(settingsMenuRunning == 1)
       {
         settingsMenu(); 
       }

       if(settingsSubMenuRunning && !(settingsMenuRunning)){
         SettingsSubMenu(option);

       }


       buttonPress2();
    

      if (shortPress == true && !settingsMenuRunning && !settingsSubMenuRunning)
      {
        mainMenu = 0;
        page = 0;  //revenim la prima pagina
        clear = 0;
        shortPress = false;
        longPress = false;

        if(gameShouldRun){
          gameShouldRun = false;
          lc.clearDisplay(0);
        }

      }

      if ((longPress == true) &&  ((!settingsMenuRunning) && (!settingsSubMenuRunning)) )
      {
        mainMenu = 0;
        page = 0;  //revenim la prima pagina
        clear = 0;
        longPress = false;

      }




     }
     break;
   } 
  }
}

// function that allows to scroll through the menu options (up-down)
int pagesScroll(int pagesNr){
 
    yValue = analogRead(yPin);

    joyMoved = false;

    int nextPage = 0;

    if (yValue < minThreshold) 
    {
      
      if(page > 0)
      {
        //lcd.clear();
        nextPage = page - 1;
        joyMoved = true;
      }
      else
      {
        nextPage = page;
        joyMoved = false;
      }
    }

    if (yValue > maxThreshold) 
    {
      
      if(page < pagesNr)
      {
        //lcd.clear();
        nextPage = page + 1;
        joyMoved = true;
      }
      else
      {
        nextPage = page;
        joyMoved = false;
      }
    }

    if (joyMoved != previousJoyMove) 
    {
      lastDebounceTime = millis();
    }
    if ((millis() - lastDebounceTime) > debounceDelay) 
    {
      willTransition = joyMoved;
    }

    previousJoyMove = joyMoved;

    if (willTransition && !didTransition) 
    {
      page = nextPage;
      lcd.clear();
    }

    didTransition = willTransition;

    return page;   
  
}  

// function that displays the menu options (pages)
void Menu(int page){
  switch (page) {
   
    case 0:{     //Page 0
      lcd.setCursor(0,0);
      lcd.print(">");
      lcd.setCursor(2,0);
      lcd.print("START GAME");
      lcd.setCursor(2,1);
      lcd.print("Highscore");
      lcd.setCursor(15,1);
      lcd.write(byte(1));
    }
    break;

    case 1:{     //Page 1
      lcd.setCursor(0,0);
      lcd.print(">");
      lcd.setCursor(2,0);
      lcd.print("Highscore");
      lcd.setCursor(2,1);
      lcd.print("Settings");
      lcd.setCursor(15,0);
      lcd.write(byte(0));
      lcd.setCursor(15,1);
      lcd.write(byte(1));

    }
    break;

    case 2: { //Page 2 
     lcd.setCursor(0,0);
     lcd.print(">");
     lcd.setCursor(2,0);
     lcd.print("Settings");
     lcd.setCursor(2,1);
     lcd.print("About");
     lcd.setCursor(15,0);
     lcd.write(byte(0));
     lcd.setCursor(15,1);
     lcd.write(byte(1)); 
    
    }
    break;

    case 3: {   //Page 3
     lcd.setCursor(0,0);
     lcd.print(">");
     lcd.setCursor(2,0);
     lcd.print("About");
     lcd.setCursor(2,1);
     lcd.print("How to play");
     lcd.setCursor(15,0);
     lcd.write(byte(0));
     lcd.setCursor(15,1);
     lcd.write(byte(1));
    
    }
    break;

    case 4: {   //Page 4
     lcd.setCursor(0,0);
     lcd.print(">");
     lcd.setCursor(2,0);
     lcd.print("How to play");
     lcd.setCursor(15,0);
     lcd.write(byte(0));
    
    }
    break;
  }
      
}

// tests if there was a button press
void buttonPress2() {  
  swState = digitalRead(pinSW);
  if (swState == LOW && lastSwState == HIGH && (millis() - firstButtonPressTime) > debounceDelay) 
  {
   firstButtonPressTime = millis();
  }
  holdingTime = (millis() - firstButtonPressTime);
  if (holdingTime > debounceDelay) 
  {
    if (swState == LOW && holdingTime > previousHoldingTime) 
    {
     // Serial.println("button is pressed");
     // digitalWrite(ledPin, HIGH);
    }
    if (swState == HIGH && lastSwState == LOW) 
    {
      if (holdingTime <= longPressTime)
      {         
        // Serial.println("short button press");
        shortPress = true;
      }
      if (holdingTime > longPressTime) 
      {
        // Serial.println("long button press");
        longPress = true;
      }
    }
  }
  lastSwState = swState;
  previousHoldingTime = holdingTime;

}

//resets button state
void resetButtonState(){
  longPress = false;
  shortPress = false;
}

// acts according to the menu option -> enters the submenu
void SubMenu(int page){
  

  if(shortPress)
  {
    lcd.clear();
    switch (page) 
    {
   
      case 0:{     //Page 0
        lcd.setCursor(0,0);
        lcd.print("Good Luck");
        lcd.setCursor(0,1);
        lcd.print("Scor:");
        lcd.setCursor(5, 1);
        lcd.print(gameScore);
        lcd.setCursor(10, 1);
        lcd.write(byte(4));
        lcd.setCursor(12, 1);
        lcd.print(lives);

        gameShouldRun = true;
       // rudimentaryGame();

      }
      break;

      case 1:{     //Page 1
        lcd.setCursor(0,0);
        lcd.print("Highscore");
        highscore();

      }
      break;

      case 2: { //Page 2 
      lcd.setCursor(0,0);
      settingsMenuRunning = 1;
      option = 0;
      lcd.clear();
      //settingsMenu();
      }
      break;

      case 3: {   //Page 3
      lcd.setCursor(0,0);
      lcd.print("About");
      about();
      }
      break;

      case 4: {   //Page 4
      lcd.setCursor(0,0);
      lcd.print("How to play");
      instruction();
      }
      break;
    }
  

  }

   shortPress = false;
}

// initializes the hish score in eeprom to 0 and unknown names
void initializeHighScoreEeprom() {
  highscores highscores[5];
  for (int i = 0; i < 5; i++) {
    EEPROM.put(hsAddress[i], highscores[i]);
  }
}

//displays the highscore in the submenu option
void highscore(){
   highscores highscores[5];
  for (int i = 0; i < 5; i++) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Highscore");
    EEPROM.get(hsAddress[i], highscores[i]);
    lcd.setCursor(0, 1);
    lcd.print((int) i+1);
    lcd.setCursor(1, 1);
    lcd.print(highscores[i].name);
    lcd.setCursor(11, 1);
    lcd.print(highscores[i].score);
    delay(500);
  }


}

//function for scrolling text 
void scrollText(String text){
  for (int i=0; i < 16; i++) {
    text = " " + text;  
  } 
  text = text + " "; 
  for (int position = 0; position < text.length(); position++) {
    lcd.setCursor(0, 1);
    lcd.print(text.substring(position, position + 16));
    delay(200);
  }

 
}

// it's called in the about submenu to display scrolling text
void about(){
  scrollText(gameName);
  scrollText(creator);
  scrollText(github);
}

//it's called in the "how to play" submenu to display scrolling text
void instruction(){
  scrollText(instructions);
}

// it's called in the settings submenu -> implements settings menu functionalities
int settingsMenu(){

  if(settingsMenuRunning)
  {
    //afisam meniul de setari
    settingsSubMenuRunning = false;
    settings(option);
    option = pagesScroll(5);

    if(longPress){
      settingsMenuRunning = false;
      settingsSubMenuRunning = false;
      // Serial.println("Exit Settings");

    }

    if(shortPress){
      settingsMenuRunning = false;
      //shortPress = false;
      settingsSubMenuRunning = true;
      // Serial.println("Enter Setting");

   }

  }
   if(settingsSubMenuRunning){
    // Serial.println("In setting");
    settingsMenuRunning = false;
    lcd.clear();
    SettingsSubMenu(option);
  }

}

// displays the submenu
void settings(int option){
  
  switch(option)
  {
    case 0:{     //Page 0
        lcd.setCursor(0,0);
        lcd.print(">");
        lcd.setCursor(2, 0);
        lcd.print("Name");
        lcd.setCursor(2, 1);
        lcd.print("Start Level");

        lcd.setCursor(15,1);
        lcd.write(byte(1));


      }
      break;

      case 1:{     //Page 1

        lcd.setCursor(0,0);
        lcd.print(">");
        lcd.setCursor(2, 0);
        lcd.print("Start Level");
        lcd.setCursor(2, 1);
        lcd.print("LCD ");
        lcd.setCursor(15,0);
        lcd.write(byte(0));
        lcd.setCursor(15,1);
        lcd.write(byte(1));

      }
      break;

      case 2: { //Page 2 
        lcd.setCursor(0,0);
        lcd.print(">");
        lcd.setCursor(2, 0);
        lcd.print("LCD ");
        lcd.setCursor(2, 1);
        lcd.print("Matrix");
        lcd.setCursor(15,0);
        lcd.write(byte(0));
        lcd.setCursor(15,1);
        lcd.write(byte(1));
      }
      break;

      case 3: {   //Page 3
        lcd.setCursor(0,0);
        lcd.print(">");
        lcd.setCursor(2, 0);
        lcd.print("Matrix");
        lcd.setCursor(2, 1);
        lcd.print("Sound");
        if (sound){
          lcd.setCursor(10, 1);
          lcd.print("ON");
        }
        else{
          lcd.setCursor(10, 1);
          lcd.print("OFF");
        }
        lcd.setCursor(15,0);
        lcd.write(byte(0));
        lcd.setCursor(15,1);
        lcd.write(byte(1));
      }
      break;

      case 4: {   //Page 4
        lcd.setCursor(0,0);
        lcd.print(">");
        lcd.setCursor(2, 0);
        lcd.print("Sound");
        if (sound){
          lcd.setCursor(10, 0);
          lcd.print("ON");
        }
        else{
          lcd.setCursor(10, 0);
          lcd.print("OFF");
        }
        lcd.setCursor(15,0);
        lcd.write(byte(0));

        lcd.setCursor(1,1);
        lcd.print("Reset");
        lcd.setCursor(15,1);
        lcd.write(byte(1));



      }
      break;
     case 5: {   //Page 4
        lcd.setCursor(0,0);
        lcd.print(">");
        lcd.setCursor(1, 0);
        lcd.print("Reset highscore");
        lcd.setCursor(0,1);
        lcd.print("and settings");
      }
      break;
  }
  
}

// records joystick move (left-right)
int joystickScroll(){
   xValue = analogRead(xPin);

    joyMoved = false;

    int nextPos = 0;

    if (xValue > maxThreshold) 
    {
      if(pos < 10)
      {
        nextPos = pos + 1;
        joyMoved = true;
      }
      else
      {
        nextPos = pos;
        joyMoved = false;
      }
      
      
    }

    if (xValue < minThreshold) 
    {
      if(pos > 0)
      {
        //lcd.clear();
        nextPos = pos - 1;
        joyMoved = true;
      }
      else
      {
        nextPos = pos;
        joyMoved = false;
      }
      
      
    }

    if (joyMoved != previousJoyMove) 
    {
      lastDebounceTime = millis();
    }
    if ((millis() - lastDebounceTime) > debounceDelay) 
    {
      willTransition = joyMoved;
    }

    previousJoyMove = joyMoved;

    if (willTransition && !didTransition) 
    {
      pos = nextPos;
      lcd.clear();
    }

    didTransition = willTransition; 
    // Serial.println(pos);
    // Serial.println(xValue);
    return pos; 


}

//enters the selected setting and allows the user to modify the chosen game setting
void SettingsSubMenu(int option){
   switch(option)
  {
    case 0:
    {     //Page 0

        String alphabet ="abcdefghijklmnopqrstuvwxyz";

        int letter = 0;
        while (pos < 10)
        {
          shortPress = false;
          lcd.setCursor(1, 0);
          lcd.print("Name:");
          lcd.setCursor(0, 1);
          lcd.print(Pname);
          lcd.setCursor(12, 1);
          lcd.write(byte(2));



          pos = joystickScroll();
          //lcd.setCursor(pos, 1);
          buttonPress2();

          // blinkCharacterCalled = true;
          // blinkCharacter(Pname[pos], pos, 1 );

      
        //  Serial.println(alphabet[letter]);
        //  Serial.println(pos);
        //  Serial.println(Pname);

          if(shortPress)
          {
            if(letter == 27){
              letter = 0;
            }
            Pname[pos] = alphabet[letter];
            letter = letter + 1;

          }

          
        }

        lcd.setCursor(1, 0);
        lcd.print("Name:");
        lcd.setCursor(0, 1);
        lcd.print(Pname);
        lcd.setCursor(11, 1);
        lcd.write("set");
 
    }
      break;

      case 1:{     //Page 1

        shortPress = false;
        lcd.setCursor(1, 0);
        lcd.print("Start Level");
        lcd.setCursor(1, 1);
        lcd.print(level);
        buttonPress2();
        if(level > levels)
        {
          level = 1;
        }
        if(shortPress)
        {
          settingLevel = true;
          if(settingLevel)
          {
            level = level + 1;
            shortPress = false;
           // Serial.println("changed level");
          }
        }
      }
      break;

      case 2: { //Page 2 
      shortPress = false;
      lcd.setCursor(1, 0);
      lcd.print("LCD ");
      lcd.setCursor(1, 1);
      lcd.print(lcdBrightness);
      lcd.setCursor(5, 1);
      lcd.write(byte(3));




      buttonPress2();
      if(lcdBrightness > 250)
      {
        lcd.clear();
        lcdBrightness = 150;
      }
        if(shortPress)
        {
          setLcdBrightness = true;
          if(setLcdBrightness)
          {
            lcdBrightness = lcdBrightness + 50;
            analogWrite(LCD_Backlight, lcdBrightness);
            EEPROM.update(lcdBrightnessAdd, lcdBrightness);
            shortPress = false;
            // Serial.println("changed brightness");
          }
        }

      }
      break;

      case 3: {   //Page 3
       shortPress = false;
        lcd.setCursor(1, 0);
        lcd.print("Matrix");
        lcd.setCursor(1, 1);
        lcd.print(matrixBrightness);
        buttonPress2();
        if(matrixBrightness > 15)
        {
          lcd.clear();
          matrixBrightness = 1;
        }
        if(shortPress)
        {
          setMatrixBrightness = true;
          if(setMatrixBrightness)
          {
            matrixBrightness = matrixBrightness + 1;
            EEPROM.update(matrixBrightnessAdd, matrixBrightness);
            lc.setIntensity(0, matrixBrightness);
            shortPress = false;
            // Serial.println("changed matrix brightness");
          }
        }
      
       
      }
      break;

      case 4: {   //Page 4

      if(shortPress)
      {

      sound = !sound;
      EEPROM.update(soundAdd, sound);

      if (sound == true){
        eatBuzz();
      }
      settingsMenuRunning = true;

      }
    
      }
      break;

      case 5: {   //Page 5

      EEPROM.update(soundAdd, 1);
      EEPROM.update(matrixBrightnessAdd, 4);
      EEPROM.update(lcdBrightnessAdd, 150);
      //reinitializeaza tabela de highscores
      initializeHighScoreEeprom();
      

      

      settingsMenuRunning = true;
     
      }
      break;
  }


  shortPress = false;
  settingLevel = false;
  setMatrixBrightness = false;
  setLcdBrightness = false;

  if(longPress && gameShouldRun){
    settingsSubMenuRunning = false;
    SubMenu(0);
  // Serial.println("Exit Settings sub Menu");
  }
  else  if(longPress){
    settingsSubMenuRunning = false;
  }

}

//blinks a given character in the name setting in order to make a nicer interface for the user
void blinkCharacter(char c, int col, int row) {
  // Set the cursor at the specified position
  lcd.setCursor(col, row);

  // Display the character
  lcd.print(c);
  
    //lcd.setCursor(col-1, row); 

  // Store the current time
  unsigned long previousMillis = millis();

  // Loop indefinitely
  while (true) {
    // Check if half a second has passed
    if (millis() - previousMillis >= 500) {
      // If so, turn off the blink effect
      lcd.noBlink();
      //lcd.print(" ");

      // Update the previous time
      previousMillis = millis();
    } else {
      // If not, turn on the blink effect
      lcd.blink();
      //lcd.print(c);
    }

      if (blinkCharacterCalled) {
      // If so, exit the loop
      break;
    }
  }
}

void endGame(){
  lc.clearDisplay(0);
  lcd.clear();
  String msg1 = "Congratulations";
  String msg2 = "Score";
  String msg3 = "Lv";
  lcd.setCursor(0, 0);
  lcd.print(msg1);
  lcd.setCursor(0, 1);
  lcd.print(msg2);
  lcd.setCursor(6, 1);
  lcd.print(gameScore);
  lcd.setCursor(10, 1);
  lcd.print(msg3);
  lcd.setCursor(13, 1);
  lcd.print(level);

  delay(2000);


  lcd.clear();
  String msg4 = "You Won ^-^";
  String msg5 = "You LOST *-*";
  String msg6 = "Highscore: ";
  lcd.setCursor(0, 0);
 if(newHighscore)
  {
    lcd.print(msg4);
    lcd.setCursor(0, 1);
    lcd.print(msg6);
    lcd.setCursor(11, 1);
    lcd.print(gameScore);
  }
  else
  {
    lcd.print(msg5);
    lcd.setCursor(0, 1);
    lcd.print(msg2);
    lcd.setCursor(7, 1);
    lcd.print(gameScore);
  }
  
}

//updates the high scores if the player ste a new record.
void updateHighScores() {
  highscores highScores[5];

  char charArray [Pname.length() + 1];
  stringToCharArray(Pname, charArray);

  // Serial.println("Update");
  for (int i = 0; i < 5; i++) {
    EEPROM.get(hsAddress[i], highScores[i]);
  }
  short int position;
  highscores currentHighScore;
  strcpy(currentHighScore.name, charArray);
  currentHighScore.score = gameScore;

  for (int i = 0; i < 5; i++) {
    if (gameScore > highScores[i].score) {
      position = i;
      break;
    }
  }
  for (int i = 4; i > position; i--) {
    EEPROM.put(hsAddress[i], highScores[i - 1]);
  }
  EEPROM.put(hsAddress[position], currentHighScore);
}

void stringToCharArray(String str, char* charArray) {
  str.toCharArray(charArray, str.length() + 1);
}

  
  



