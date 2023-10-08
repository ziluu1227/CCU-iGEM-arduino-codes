#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#define I2C_ADDR 0x3F
#define LCD_ROWS 2
#define LCD_COLS 16
LiquidCrystal_I2C lcd(I2C_ADDR, LCD_COLS, LCD_ROWS);

const int encoderPinA = 2;  // Rotary encoder pin A（CLK）
const int encoderPinB = 3;  // Rotary encoder pin B（DT）
const int confirmPin = 4; //Rotary encoder confirm pin with 10k Ω

int lastEncoded = 0;
long encoderValue = 0; //encoderValue
long tempencoderValue = 0; //encoderValue temp
int mode = 0; //mode level
int selectmode = 0; //level 1 mode selection
int dir = 0;//define rotary encoder rotate direction
int rot = 0;//define rotary encoder rotate ot not
int modeselection; //save the mode selected
int level = 1; //now select level

// stepmotor control pins
const int stepPin = 5;
const int dirPin = 6;
const int enablePin = 7;

// stepAngle and stepsPerRevolution of stepmotor
const float stepAngle = 1.8; //stepAngle (degree)
const int stepsPerRevolution = 200; // stepsPerRevolution

// rotate diretection of stepmotor
const int clockwise = HIGH;
const int counterclockwise = LOW;
int direction; //1=>CW;0=>CCW;

// rotate speed of stepmotor (steps/sec)
int stepperSpeed = 500; // Set the value of steps per second to control the rotation speed
int steps; //移動步數

void setup() {
  Serial.begin(9600);
  Wire.begin();
  lcd.begin(LCD_COLS, LCD_ROWS);
  lcd.backlight();
  // set control pins
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(enablePin, OUTPUT);

  //  Set the rotary encoder pin to input mode
  pinMode(encoderPinA, INPUT);
  pinMode(encoderPinB, INPUT);
  pinMode(confirmPin, INPUT);

  // Enable internal pull-up resistor
  digitalWrite(encoderPinA, HIGH);
  digitalWrite(encoderPinB, HIGH);

  // Attach interrupt service routine
  attachInterrupt(digitalPinToInterrupt(encoderPinA), updateEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoderPinB), updateEncoder, CHANGE);
  
  // disable stepmotor
  digitalWrite(enablePin, HIGH);
}

void loop() {
  // clockwise or counterclockwise judgement(CW & CCW)
  if (encoderValue > tempencoderValue){
    mode++;
    tempencoderValue = encoderValue;
    dir = 1; //CW
    rot = 1;
  }
  if (encoderValue < tempencoderValue){
    mode--;
    tempencoderValue = encoderValue;
    dir = -1; //CCW
    rot = 1;
  }


  selectmode = abs(mode % 2); //select one part to cintrol:1 => speed; 2 => steps
  switch (level){
    case 1:
    lcd.setCursor(0,0);
    lcd.print("Speed");
    if(mode >= 0){
      lcd.setCursor(0, 1);
      lcd.print(mode*100);
      stepperSpeed = mode*100;
      lcd.print("s"); // 
      if (digitalRead(confirmPin) == HIGH){
        level = 2;
      }
    }
    else{
      mode = 0;
    }
    break;
    case 2:
    lcd.setCursor(0,0);
    lcd.print("Set direction");
    if(abs(mode % 2 == 0)){
      lcd.setCursor(0, 1);
      lcd.print("forward");
      if (digitalRead(confirmPin) == HIGH){
        direction = 1; //1=>CW
        level = 3;
      }
    }
    else{
      lcd.setCursor(0, 1);
      lcd.print("backward");
      if (digitalRead(confirmPin) == HIGH){
        direction = 0; //1=>CW
        level = 3;
      }
    }
    break;
    case 3:
    lcd.setCursor(0,0);
    lcd.print("Set steps");
    if(mode >= 0){
      lcd.setCursor(0, 1);
      lcd.print(mode*100);
      steps = mode*100;
      lcd.print("steps"); // 
      if (digitalRead(confirmPin) == HIGH){
        level = 4;
      }
    }
    break;
    case 4:
    lcd.setCursor(0,0);
    if(direction == 1){
      lcd.print("forward");
    }
    if(direction == 0){
      lcd.print("backward");
    }
    lcd.setCursor(0,1);
    lcd.print(stepperSpeed);lcd.print("s ");
    lcd.print(steps);lcd.print("steps");
    if(mode < -5){
      level = 7;
    }
    if (digitalRead(confirmPin) == HIGH){
      level = 5;
    }
    break;
    case 5:
    lcd.setCursor(0, 0);
    lcd.print("Working...");
    digitalWrite(enablePin,LOW);
    if(direction == 1){
      digitalWrite(dirPin, clockwise); // CW
      for (int i = 0; i < steps; i++) {
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(stepperSpeed); // speeds of stepmotor
        digitalWrite(stepPin, LOW);
        delayMicroseconds(stepperSpeed); // speeds of stepmotor
      }
    }
    if(direction == 0){
      digitalWrite(dirPin, counterclockwise); // CCW
      for (int i = 0; i < steps; i++) {
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(stepperSpeed); // speeds of stepmotor
        digitalWrite(stepPin, LOW);
        delayMicroseconds(stepperSpeed); // speeds of stepmotor
      }
    }
    level = 6;
    break;
    case 6:
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Work Done");
    lcd.setCursor(0, 1);
    lcd.print("Press to go back");
    if (digitalRead(confirmPin) == HIGH){
      level = 1;
    }
    break;
    case 7:
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("return?");
    if(abs(mode % 2) == 1){
      lcd.setCursor(0,1);
      lcd.print(">Yes<No");
      if (digitalRead(confirmPin) == HIGH){
        level = 1;
      }
    }
    if(abs(mode % 2) == 0){
      lcd.setCursor(0,1);
      lcd.print(" Yes>No<");
      if (digitalRead(confirmPin) == HIGH){
        level = 4;
      }
    }


    break;
  }
  //confirmPin
  if(digitalRead(confirmPin) == HIGH){
    encoderValue = 0;
    tempencoderValue = 0;
    mode = 0;
    selectmode = 0;
    dir = 0; //reset direction
    rot = 0; //reset rotation
    //level = level + 1;
  }
  delay(300);
  lcd.clear();
  /*
  // CW
  rotateClockwise(500); // rotate 500 steps
  delay(1000);  
  
  // CCW
  rotateCounterclockwise(500); // rotate 500 steps
  delay(1000); 

  digitalWrite(enablePin,HIGH);
  delay(100);
  */
}
/*
// Rotate the stepper motor CW by a specified number of steps
void rotateClockwise(int steps) {
  digitalWrite(dirPin, clockwise); // CW
  for (int i = 0; i < steps; i++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(stepperSpeed); // set the speed of stepmotor
    digitalWrite(stepPin, LOW);
    delayMicroseconds(stepperSpeed); // set the speed of stepmotor
  }
}

// Rotate the stepper motor CCW by a specified number of steps
void rotateCounterclockwise(int steps) {
  digitalWrite(dirPin, counterclockwise); // CCW
  for (int i = 0; i < steps; i++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(stepperSpeed); // set the speed of stepmotor
    digitalWrite(stepPin, LOW);
    delayMicroseconds(stepperSpeed); // set the speed of stepmotor
  }
}
*/
void updateEncoder() {
  int MSB = digitalRead(encoderPinA);  // Read the status of pin A
  int LSB = digitalRead(encoderPinB);  // Read the status of pin B

  int encoded = (MSB << 1) | LSB;  // encoded valve
  
  int sum = (lastEncoded << 2) | encoded;  // Combination of last and current coded values

  // Increase or decrease the encoder value depending on the direction of rotation
  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) {
    encoderValue++;
  } else if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) {
    encoderValue--;
  }

  lastEncoded = encoded;  // Update the last encoding value
}

