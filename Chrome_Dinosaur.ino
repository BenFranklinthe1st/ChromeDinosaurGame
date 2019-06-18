#include <LiquidCrystal.h>
#include <EEPROM.h>
#include "bitmaps.h"
#include "pitches.h"

/*
 * Title: Chrome DInosaur Game
 * Author: Frank Li
 * Date: June 16, 2019
 * Purpose:To recreate the chrome dinosaur game on the arduino.
 */

LiquidCrystal lcd(12,11,5,4,3,2);

//Music
int melody[] = {
  NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4
};
int noteDurations[] = {
  4, 8, 8, 4, 4, 4, 4, 4
};

const int buttonPin = 13;
int buttonPressed = 0;

int score = 0;
int charHeight = 1;

//time controls for jump
int jumpInterval = 1000;
unsigned long jumpPrevMillis = 0;
unsigned long jumpCurrentMillis = 0;
boolean jumpDelayStart = false;

//time controls for button delay
int buttonInterval = 300;
unsigned long buttonPrevMillis = 0;
unsigned long buttonCurrentMillis = 0;
boolean buttonDelayStart = false;

//time for barriers
int barrierInterval = 400;
unsigned long barrierPrevMillis = 0;
unsigned long barrierCurrentMillis = 0;

int barrierLocations[2][16];

//This function is used to write an int to EEPROM,
//since you can only write multiple bytes
void EEPROMWriteInt(int address, int value) 
{
  byte two = (value & 0xFF);
  byte one = ((value >> 8) & 0xFF);

  EEPROM.update(address, two);
  EEPROM.update(address + 1, one);
}

int EEPROMReadInt(int address)
{
  long two = EEPROM.read(address);
  long one = EEPROM.read(address + 1);

  return ((two << 0) & 0xFFFFFF) + ((one << 8) & 0xFFFFFFFF);
}

void checkCollision(){
  
  //check if player dies, if so play melody
  if(barrierLocations[charHeight][0] == 1){
    for (int thisNote = 0; thisNote < 8; thisNote++) {

    // to calculate the note duration, take one second divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = 1000 / noteDurations[thisNote];
    tone(7, melody[thisNote], noteDuration);

    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(7);
    }
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Your score: ");
    lcd.print(score);
    delay(1000);
    //checks to see if needs to write to EEPROM, helps save cycles 
    //although .update() does so already in the EEPROMWriteInt()
    if(score > EEPROMReadInt(0))
      EEPROMWriteInt(0, score);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Highest Score: ");
    lcd.setCursor(0, 1);
    lcd.print(EEPROMReadInt(0));

    while(true){
      delay(10000);
    }
  }
}

void spawnBarrier(){
  
  //clear screen of barriers
  for(int i = 0; i < 15; i++){
    if(barrierLocations[0][i] == 1){
      lcd.setCursor(i, 0);
      lcd.print(" ");
    }else if(barrierLocations[1][i] == 1){
      lcd.setCursor(i, 1);
      lcd.print(" ");
    }
  }

  //slide all barriers over
  for(int i = 0; i < 15; i++){
    barrierLocations[0][i] = barrierLocations[0][i+1];
    barrierLocations[1][i] = barrierLocations[1][i+1];
  }
  barrierLocations[0][15] = 0;
  barrierLocations[1][15] = 0;

  score++;
  
  //increase speed of game
  if(score%20 == 0 && score != 0){
    barrierInterval = (int)(barrierInterval*0.9);
    jumpInterval = (int)(jumpInterval*0.9);
    buttonInterval = (int)(buttonInterval*0.9);
    tone(7, NOTE_D5, 800);
  }

  if(random(3) == 2){
    //Add random barrier
    int randomBarrier = random(2);
    if(randomBarrier == 1){
      //check if there are any previous barriers before or on top of it
      if(barrierLocations[0][14] == 0 && (barrierLocations[1][14] == 0 && barrierLocations[1][13] == 0)){
        barrierLocations[randomBarrier][15] = 1;
      }
    }else{
      if(barrierLocations[1][14] == 0){
        barrierLocations[randomBarrier][15] = 1;
      }
    }
  }

  //Rewrite all barriers
  for(int i = 0; i < 15; i++){
    //cacti
    if(barrierLocations[0][i] == 1){
      lcd.setCursor(i, 0);
      lcd.write(byte(2));
    //bird
    }else if(barrierLocations[1][i] == 1){
      lcd.setCursor(i, 1);
      lcd.write(byte(1));
    }
  }
  checkCollision();
}

void jump() {

  tone(7, NOTE_A4, 100);
  jumpDelayStart = true;
  jumpPrevMillis = millis();
  lcd.setCursor(0, charHeight);
  lcd.print(" ");
  charHeight--;
  lcd.setCursor(0,charHeight);
  lcd.write(byte(0));

  checkCollision();
}

void setup() {
  lcd.begin(16,2);
  lcd.print("Chrome Dinosaur");
  lcd.setCursor(0,1);
  lcd.createChar(0,dino);
  lcd.createChar(1, cacti);
  lcd.createChar(2, bird);

  lcd.setCursor(0, charHeight);
  lcd.write(byte(0));
  pinMode(buttonPin, INPUT);

  randomSeed(analogRead(0));
}

void loop() {

  //loops through this, updating the previous millis and current millis to spawn barrier
  barrierCurrentMillis = millis();
  if(barrierCurrentMillis - barrierPrevMillis >= barrierInterval){
    spawnBarrier();
    barrierPrevMillis = millis();
  }
  
  if(jumpDelayStart){
    jumpCurrentMillis = millis();
    if(jumpCurrentMillis - jumpPrevMillis >= jumpInterval){
      jumpDelayStart = false;
      lcd.setCursor(0, charHeight);
      lcd.print(" ");
      charHeight++;
      lcd.setCursor(0, charHeight);
      lcd.write(byte(0));

      checkCollision();

      //if jump has been pressed then there should be a button delay after
      buttonDelayStart = true;
      buttonPrevMillis = millis();
    }
  }

  //this is the button delay timer, this then resets the button delay
  if(buttonDelayStart){
    buttonCurrentMillis = millis();
    if(buttonCurrentMillis - buttonPrevMillis >= buttonInterval){
      buttonDelayStart = false;
    }
  }

  //Only jumps if it is in the bottom row, th ebutton is pressed, and there is no button delay
  buttonPressed = digitalRead(buttonPin);
  if(buttonPressed == HIGH && charHeight == 1 && !buttonDelayStart){
    jump();
  }
}
