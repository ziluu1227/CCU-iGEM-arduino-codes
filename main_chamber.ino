#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#define I2C_ADDR 0x27
#define LCD_ROWS 2
#define LCD_COLS 16
LiquidCrystal_I2C lcd(I2C_ADDR, LCD_COLS, LCD_ROWS);

#include <Servo.h>
// Define a servo motor object
//Servo valve1servo;
//Servo valve2servo;
Servo shakeservo;

const int encoderPinA = 2;  // Rotary encoder pin A (CLK)
const int encoderPinB = 3;  // Rotary encoder pin B（DT）
const int confirmPin = 4; // Rotary encoder confirm pin with 10k Ω
const int PBSPin = 5; //PBS pin
const int mGLPin = 6; //mGL pin
const int DNasePin = 7; //DNase pin Dnase was used to be a cutter of zinc finger protin,now we replace Dnase with zinc ion aqueous solution
const int beepPin = 8; //Buzzer pin
//const int valve1Pin = 9; //valve1 pin
//const int valve2Pin = 10; //valve2 pin
const int shakePin = 11; //shake pin

int lastEncoded = 0;
long encoderValue = 0; //encoderValue
long tempencoderValue = 0; //encoderValue temp
int mode = 0; //mode level
int selectmode = 0; //level 1 mode selection
int dir = 0;//define rotary encoder rotate direction
int rot = 0;//define rotary encoder rotate ot not
int modeselection; //save the mode selected
int level = 1; //now select level
int ml; //liquid ml
//int valve1 = 1; //valve1 open
//int valve2 = 1; //valve2 open
//int valve1act = 1;//valve1 going to do ;1 => open; 0 => close
//int valve2act = 1;//valve2 going to do ;1 => open; 0 => close
int sec;//shake time
int pumptime = 620;//pumpint time,1ml per 0.62sec,must be retest after confirm every risistor and motor ans servo
int shakepos = 30;//init shake angle

void setup() {
  // Initialize serial communication
  Serial.begin(9600);
  Wire.begin();
  lcd.begin(LCD_COLS, LCD_ROWS);
  lcd.backlight();

  //beepPin
  pinMode(beepPin, OUTPUT);

  //PBSPin
  pinMode(PBSPin, OUTPUT);
  //mGLPin
  pinMode(mGLPin, OUTPUT);
  //DNasePin
  pinMode(DNasePin, OUTPUT);

  //servos
  //valve1servo.attach(valve1Pin);//valve1
  //valve2servo.attach(valve2Pin);//valve2
  shakeservo.attach(shakePin);//shakePin
  //open valves initially(let servos keep on open angle)
  //valve1servo.write(20);//valve1 opening code
  //valve2servo.write(20);//valve1 opening code
  shakeservo.write(shakepos);

  
  // Set the rotary encoder pin to input mode
  pinMode(encoderPinA, INPUT);
  pinMode(encoderPinB, INPUT);
  pinMode(confirmPin, INPUT);
  
  // Enable internal pull-up resistor
  digitalWrite(encoderPinA, HIGH);
  digitalWrite(encoderPinB, HIGH);
  
  // Attach interrupt service routine
  attachInterrupt(digitalPinToInterrupt(encoderPinA), updateEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoderPinB), updateEncoder, CHANGE);
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

  //Serial.println("encoderValue");
  //Serial.println(encoderValue);
  //Serial.println("tempencoderValue");
  //Serial.println(tempencoderValue);
  //Serial.println("mode");

  selectmode = abs(mode % 6); //select one part to cintrol 1=PBS;2=mGL;3=DNase;4=valve1;5=valve2;6=shake;
  switch (level){
    case 1://select mode to control
      switch (selectmode){ //select one part to cintrol
          case 1:
            if(mode > 0){ //CW
              lcd.setCursor(0, 0); // set LCD start on [1,1]
              lcd.print("Select Mode");
              lcd.setCursor(0,1);
              lcd.print("PBS");
              modeselection = 1;
              if (digitalRead(confirmPin) == HIGH){
                level = 2;
                break;
              }
            }
            if(mode < 0){ //CCW
              lcd.setCursor(0, 0);
              lcd.print("Select Mode");
              lcd.setCursor(0,1);
              lcd.print("drain main cham");
              modeselection = 5;
              if (digitalRead(confirmPin) == HIGH){
                level = level + 1;
              }
            }
            break;
          case 2:
            if(mode > 0 ){
              lcd.setCursor(0, 0);
              lcd.print("Select Mode");
              lcd.setCursor(0,1);
              lcd.print("mGL");
              modeselection = 2;
              if (digitalRead(confirmPin) == HIGH){
                level = level + 1;
              }
            }
            if(mode < 0){
              lcd.setCursor(0, 0);
              lcd.print("Select Mode");
              lcd.setCursor(0, 1);
              lcd.print("shake");
              modeselection = 4;
              if (digitalRead(confirmPin) == HIGH){
                level = level + 1;
              }
            }
            break;
          case 3:
            if(mode > 0){
              lcd.setCursor(0, 0);
              lcd.print("Select Mode");
              lcd.setCursor(0, 1);
              lcd.print("DNase");
              modeselection = 3;
              if (digitalRead(confirmPin) == HIGH){
                level = level + 1;
              }
            }
            if(mode < 0){
              lcd.setCursor(0, 0);
              lcd.print("Select Mode");
              lcd.setCursor(0, 1);
              lcd.print("DNase");
              modeselection = 3;
              if (digitalRead(confirmPin) == HIGH){
                level = level + 1;
              }
            }
            break;
          /*case 4:
            if(mode > 0){
              lcd.setCursor(0, 0);
              lcd.print("Select Mode");
              lcd.setCursor(0, 1);
              lcd.print("valve 1");
              modeselection = 4;
              if (digitalRead(confirmPin) == HIGH){
                level = level + 1;
              }
            }
            if(mode < 0){
              lcd.setCursor(0, 0);
              lcd.print("Select Mode");
              lcd.setCursor(0, 1);
              lcd.print("DNase");
              modeselection = 3;
              if (digitalRead(confirmPin) == HIGH){
                level = level + 1;
              }
            }
            break;
            case 5:
            if(mode > 0){
              lcd.setCursor(0, 0);
              lcd.print("Select Mode");
              lcd.setCursor(0, 1);
              lcd.print("valve 2");
              modeselection = 5;
              if (digitalRead(confirmPin) == HIGH){
                level = level + 1;
              }
            }
            if(mode < 0){
              lcd.setCursor(0, 0);
              lcd.print("Select Mode");
              lcd.setCursor(0, 1);
              lcd.print("mGL");
              modeselection = 2;
              if (digitalRead(confirmPin) == HIGH){
                level = level + 1;
              }
            }
            break;*/
            case 4:
            if(mode > 0){
              lcd.setCursor(0, 0);
              lcd.print("Select Mode");
              lcd.setCursor(0, 1);
              lcd.print("shake");
              modeselection = 4;
              if (digitalRead(confirmPin) == HIGH){
                level = level + 1;
              }
            }
            if(mode < 0){
              lcd.setCursor(0, 0);
              lcd.print("Select Mode");
              lcd.setCursor(0, 1);
              lcd.print("mGL");
              modeselection = 2;
              if (digitalRead(confirmPin) == HIGH){
                level = level + 1;
              }
            }
            break;
            case 5:
            if(mode > 0){
              lcd.setCursor(0, 0);
              lcd.print("Select Mode");
              lcd.setCursor(0, 1);
              lcd.print("drain main cham");
              modeselection = 5;
              if (digitalRead(confirmPin) == HIGH){
                level = level + 1;
              }
            }
            if(mode < 0){
              lcd.setCursor(0, 0);
              lcd.print("Select Mode");
              lcd.setCursor(0, 1);
              lcd.print("PBS");
              modeselection = 1;
              if (digitalRead(confirmPin) == HIGH){
                level = level + 1;
              }
            }
            break;
            case 0: //if mode come back to 0, let mode += 1 or -= 1(not to show the home page but keep on select page)
            if(mode % 6 == 0 && rot == 1){
              if(mode >= 0 && dir == 1){
                mode = mode + 1;
              }
              if (mode <= 0 && dir == -1){
                mode = mode - 1;
              }
              if(mode >= 0 && dir == -1){
                mode = mode - 1;
              }
              if (mode <= 0 && dir == 1){
                mode = mode + 1;
              } 
            }
            if(mode == 0 && rot == 0){
              lcd.setCursor(0, 0);
              lcd.print("rotate to select");
              lcd.setCursor(0, 1);
              lcd.print("press to confirm");
            }
            break;
        }
      //Serial.println(modeselection);
    break;
    case 2://in each mode
    switch(modeselection){
      case 1://PBS
      lcd.setCursor(0, 0);
      lcd.print("PBS");
      if(mode >= 0){
        lcd.setCursor(0, 1);
        if(mode >= 5){
          mode = 5;
        }
        lcd.print(mode*10);
        ml = mode*10;
        lcd.print(" ml");
        if (digitalRead(confirmPin) == HIGH){
          level = 3;
          //channeling...&ml num
        }
      }
      if(mode < 0){ //return
        level = 10;
      }
      break;
      case 2://mGL
      lcd.setCursor(0, 0);
      lcd.print("mGL");
      if(mode >= 0){
        if(mode >= 5){
          mode = 5;
        }
        lcd.setCursor(0, 1);
        lcd.print(mode * 10);
        ml = mode*10;
        lcd.print(" ml");
        if (digitalRead(confirmPin) == HIGH){
          level = 3;
          //channeling...&ml num
        }
      }
      if(mode < 0){ //return
        level = 10;
      }
      break;
      case 3://DNase
      lcd.setCursor(0, 0);
      lcd.print("DNase");
      if(mode >= 0){
        lcd.setCursor(0, 1);
        if(mode >= 5){
          mode = 5;
        }
        lcd.print(mode * 10);
        ml = mode*10;
        lcd.print(" ml");
        if (digitalRead(confirmPin) == HIGH){
          level = 3;
          //channeling...&ml num
        }
      }
      if(mode < 0){ //return
        level = 10;
      }
      break;
      /*
      case 4://valve1
      lcd.setCursor(0, 0);
      lcd.print("valve 1");
      if(valve1 == 1){//now open
        lcd.print(" opened");
        if(abs(mode % 2) == 1){
          lcd.setCursor(0, 1);
          lcd.print(">close< return");
          if(digitalRead(confirmPin) == HIGH){
            level = 3;
            valve1act = 0; //close
          }
        }
        else{
          lcd.setCursor(0, 1);
          lcd.print(" close >return<");
          if(digitalRead(confirmPin) == HIGH){
            level = 10;
          }
        }
      }
      if(valve1 == 0){//now close
        lcd.print(" closed");
        if(abs(mode % 2) == 1){
          lcd.setCursor(0, 1);
          lcd.print(">open< return");
          if(digitalRead(confirmPin) == HIGH){
            level = 3;
            valve1act = 1;//open
          }
        }
        else{
          lcd.setCursor(0, 1);
          lcd.print(" open >return<");
          if(digitalRead(confirmPin) == HIGH){
            level = 10;
          }
        }
      }
      break;
      case 5://valve2
      lcd.setCursor(0, 0);
      lcd.print("valve 2");
      if(valve2 == 1){//now open
        lcd.print(" opened");
        if(abs(mode % 2) == 1){
          lcd.setCursor(0, 1);
          lcd.print(">close< return");
          if(digitalRead(confirmPin) == HIGH){
            level = 3;
            valve2act = 0; //close
          }
        }
        else{
          lcd.setCursor(0, 1);
          lcd.print(" close >return<");
          if(digitalRead(confirmPin) == HIGH){
            level = 10;
          }
        }
      }
      if(valve2 == 0){//now close
        lcd.print(" closed");
        if(abs(mode % 2) == 1){
          lcd.setCursor(0, 1);
          lcd.print(">open< return");
          if(digitalRead(confirmPin) == HIGH){
            level = 3;
            valve2act = 1;//open
          }
        }
        else{
          lcd.setCursor(0, 1);
          lcd.print(" open >return<");
          if(digitalRead(confirmPin) == HIGH){
            level = 10;
          }
        }
      }
      break;
      */
      case 4://shake
      lcd.setCursor(0, 0);
      lcd.print("shake main cham");
      if(mode >= 0){
        lcd.setCursor(0, 1);
        sec = mode * 10;
        lcd.print(mode * 10);
        lcd.print(" second");
        if(digitalRead(confirmPin) == HIGH){
          level = 3;
        }
      }
      if(mode < 0){
        level = 10;
      }
      break;
      case 5://drain
      lcd.setCursor(0, 0);
      lcd.print("drain main cham?");
      lcd.setCursor(0, 1);
      if(mode % 2 == 0){
        lcd.print(">Yes<Return");
        if(digitalRead(confirmPin) == HIGH){
          level = 3;
        }
      }
      else{
        lcd.print(" Yes>Return<");
        if(digitalRead(confirmPin) == HIGH){
          level = 10;
        }
      }
      break;
    }
    break;
    case 3://level;mode detial
    switch (modeselection){
      case 1://PBS
      lcd.setCursor(0, 0);
      lcd.print("PBS ");
      lcd.print(ml);
      lcd.print(" ml");
      lcd.setCursor(0, 1);
      lcd.print("pumping...");
      digitalWrite(PBSPin,HIGH);
      delay(pumptime * ml);
      digitalWrite(PBSPin,LOW);
      level = 9;
      break;
      case 2://mGL
      lcd.setCursor(0, 0);
      lcd.print("mGL ");
      lcd.print(ml);
      lcd.print(" ml");
      lcd.setCursor(0, 1);
      lcd.print("pumping...");
      digitalWrite(mGLPin,HIGH);
      delay(pumptime * ml);
      digitalWrite(mGLPin,LOW);
      level = 9;
      break;
      case 3://DNase
      lcd.setCursor(0, 0);
      lcd.print("DNase ");
      lcd.print(ml);
      lcd.print(" ml");
      lcd.setCursor(0, 1);
      lcd.print("pumping...");
      digitalWrite(DNasePin,HIGH);
      delay(pumptime * ml);
      digitalWrite(DNasePin,LOW);
      level = 9;
      break;
      /*
      case 4://valve1
      if(valve1act == 0){//going to close
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("valve 1");
        lcd.setCursor(0, 1);
        lcd.print("closing...");
        valve1servo.write(0);
        delay(500);//closing time
        //valve1 closing code
        valve1 = 0; //now close
        level = 9;
      }
      else{
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("valve 1");
        lcd.setCursor(0, 1);
        lcd.print("opening...");
        valve1servo.write(20);//valve1 opening code
        delay(500);//opening time
        valve1 = 1; //now open
        level = 9;
      }
      break;
      case 5://valve2
      if(valve2act == 0){//going to close
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("valve 2");
        lcd.setCursor(0, 1);
        lcd.print("closing...");
        valve2servo.write(0);//valve1 closing code
        delay(500);//closing time
        valve2 = 0; //now close
        level = 9;
      }
      else{
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("valve 2");
        lcd.setCursor(0, 1);
        lcd.print("opening...");
        valve2servo.write(20);//valve1 opening code
        delay(500);//opening time
        valve2 = 1; //now open
        level = 9;
      }
      break;
      */
      case 4://shake
      lcd.setCursor(0, 0);
      lcd.print("shake for ");
      lcd.print(sec);
      lcd.print(" s");
      //delay(shaketime)
      lcd.setCursor(0, 1);
      lcd.print("shaking...");
      for(int shaketimes = 0;shaketimes <= sec;shaketimes += 1){
        for(shakepos = 27; shakepos <= 33;shakepos += 1){
          shakeservo.write(shakepos);
          delay(80);
        }
        for(shakepos = 33; shakepos >= 27;shakepos -= 1){
          shakeservo.write(shakepos);
          delay(80);
        }
      }
      level = 9;
      break;
      case 5://drain
      lcd.setCursor(0, 0);
      lcd.print("Draining...");
      shakeservo.write(27);
      digitalWrite(PBSPin,HIGH);
      if(digitalRead(confirmPin) == HIGH){
        digitalWrite(PBSPin,LOW);
        shakeservo.write(30);
        level = 9;
      }
      break;
    }
    break;
    case 9: // press to go back
    switch(modeselection){
      case 1://PBS
      lcd.setCursor(0, 0);
      lcd.print("PBS ");
      lcd.print(ml);
      lcd.print(" ml,Done");
      lcd.setCursor(0, 1);
      lcd.print("press to go back");
      if(digitalRead(confirmPin) == HIGH){
        level = 1;
      }
      break;
      case 2://mGL
      lcd.setCursor(0, 0);
      lcd.print("mGL ");
      lcd.print(ml);
      lcd.print(" ml,Done");
      lcd.setCursor(0, 1);
      lcd.print("press to go back");
      if(digitalRead(confirmPin) == HIGH){
        level = 1;
      }
      break;
      case 3://DNase
      lcd.setCursor(0, 0);
      lcd.print("DNase ");
      lcd.print(ml);
      lcd.print(" ml,Done");
      lcd.setCursor(0, 1);
      lcd.print("press to go back");
      if(digitalRead(confirmPin) == HIGH){
        level = 1;
      }
      break;
      /*
      case 4://valve1
      if(valve1act == 0){
        lcd.setCursor(0, 0);
        lcd.print("valve 1 closed");
        lcd.setCursor(0, 1);
        lcd.print("press to go back");
        if(digitalRead(confirmPin) == HIGH){
          level = 1;
          valve1act == 1;
        }
      }
      else{
        lcd.setCursor(0, 0);
        lcd.print("valve 1 opened");
        lcd.setCursor(0, 1);
        lcd.print("press to go back");
        if(digitalRead(confirmPin) == HIGH){
          level = 1;
          valve1act == 0;
        }
      }
      break;
      case 5://valve2
      if(valve2act == 0){
        lcd.setCursor(0, 0);
        lcd.print("valve 2 closed");
        lcd.setCursor(0, 1);
        lcd.print("press to go back");
        if(digitalRead(confirmPin) == HIGH){
          level = 1;
          valve2act == 1;

        }
      }
      else{
        lcd.setCursor(0, 0);
        lcd.print("valve 2 opened");
        lcd.setCursor(0, 1);
        lcd.print("press to go back");
        if(digitalRead(confirmPin) == HIGH){
          level = 1;
          valve2act == 1;
        }
      }
      break;
      */
      case 4://shake
      lcd.setCursor(0, 0);
      lcd.print("shake Done");
      lcd.setCursor(0, 1);
      lcd.print("press to go back");
      if(digitalRead(confirmPin) == HIGH){
        level = 1;
      }
      break;
      case 5:
      lcd.setCursor(0, 0);
      lcd.print("Drain Done");
      lcd.setCursor(0, 1);
      lcd.print("press to go back");
      if(digitalRead(confirmPin) == HIGH){
        level = 1;
      }
      break;
    }
    break;

    case 10: //return ; level
    switch (modeselection){
      case 1://PBS
      lcd.setCursor(0, 0);
      lcd.print("PBS");
      if (abs(mode % 2 == 0)){
        lcd.setCursor(0, 1);
        lcd.print("return?>YES< NO");
        if (digitalRead(confirmPin) == HIGH){
          mode = 1;
          level = 1;
        }
      }
      else{
        lcd.setCursor(0, 1);
        lcd.print("return? YES >NO<");
        if (digitalRead(confirmPin) == HIGH){
          mode = 1;
          level = 2;
        }
      }
      break;
      case 2://mGL
      lcd.setCursor(0, 0);
      lcd.print("mGL");
      if (abs(mode % 2 == 0)){
        lcd.setCursor(0, 1);
        lcd.print("return?>YES< NO");
        if (digitalRead(confirmPin) == HIGH){
          mode = 1;
          level = 1;
        }
      }
      else{
        lcd.setCursor(0, 1);
        lcd.print("return? YES >NO<");
          if (digitalRead(confirmPin) == HIGH){
            mode = 1;
            level = 2;
        }
      }
      break;
      case 3://DNase
      lcd.setCursor(0, 0);
      lcd.print("DNase");
      if (abs(mode % 2 == 0)){
        lcd.setCursor(0, 1);
        lcd.print("return?>YES< NO");
        if (digitalRead(confirmPin) == HIGH){
          mode = 1;
          level = 1;
        }
      }
      else{
        lcd.setCursor(0, 1);
        lcd.print("return? YES >NO<");
          if (digitalRead(confirmPin) == HIGH){
            mode = 1;
            level = 2;
          }
      }
      break;
      /*
      case 4://valve1
      if(valve1 == 1){
        lcd.setCursor(0, 0);
        lcd.print("valve 1 opened");
      }
      if(valve1 == 0){
        lcd.setCursor(0, 0);
        lcd.print("valve 1 closed");
      }
      if (abs(mode % 2 == 0)){
        lcd.setCursor(0, 1);
        lcd.print("return?>YES< NO");
        if (digitalRead(confirmPin) == HIGH){
          mode = 1;
          level = 1;
        }
      }
      else{
        lcd.setCursor(0, 1);
        lcd.print("return? YES >NO<");
          if (digitalRead(confirmPin) == HIGH){
            mode = 1;
            level = 2;
          }
      }
      break;
      case 5://valve2
      lcd.setCursor(0, 0);
      lcd.print("valve 2");
      if (abs(mode % 2 == 0)){
        lcd.setCursor(0, 1);
        lcd.print("return?>YES< NO");
        if (digitalRead(confirmPin) == HIGH){
          mode = 1;
          level = 1;
        }
      }
      else{
        lcd.setCursor(0, 1);
        lcd.print("return? YES >NO<");
          if (digitalRead(confirmPin) == HIGH){
            mode = 1;
            level = 2;
          }
      }
      break;
      */
      case 4://shake
      lcd.setCursor(0, 0);
      lcd.print("shake main cham");
      if (abs(mode % 2 == 0)){
        lcd.setCursor(0, 1);
        lcd.print("return?>YES< NO");
        if (digitalRead(confirmPin) == HIGH){
          mode = 1;
          level = 1;
        }
      }
      else{
        lcd.setCursor(0, 1);
        lcd.print("return? YES >NO<");
          if (digitalRead(confirmPin) == HIGH){
            mode = 1;
            level = 2;
          }
      }
      break;
      case 5://drain
      lcd.setCursor(0, 0);
      lcd.print("drain main cham");
      if (abs(mode % 2 == 0)){
        lcd.setCursor(0, 1);
        lcd.print("return?>YES< NO");
        if (digitalRead(confirmPin) == HIGH){
          mode = 1;
          level = 1;
        }
      }
      else{
        lcd.setCursor(0, 1);
        lcd.print("return? YES >NO<");
          if (digitalRead(confirmPin) == HIGH){
            mode = 1;
            level = 2;
          }
      }
      break;
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
    digitalWrite(beepPin,HIGH);
    delay(20);
    digitalWrite(beepPin,LOW);
  }
  //valve constant open or close, two servo's GND can be connected in same GND to UNO
  /*if(valve1 == 1){
    valve1servo.write(20);
  }
  if(valve1 == 0){
    valve1servo.write(0);
  }
  if(valve2 == 1){
    valve2servo.write(20);
  }
  if(valve2 == 0){
    valve2servo.write(0);
  }
  */
  //lcd.print(mode);
  Serial.println("mode");
  Serial.println(mode);
  //Serial.println("selectmode");
  //Serial.println(selectmode);
  //Serial.println("modeselection");
  //Serial.println(modeselection);
  Serial.println("level");
  Serial.println(level);
  //Serial.println("valve1act");
  //Serial.println(valve1act);
  delay(500);//must be 500 or it will rotate too fast
  lcd.clear();
  
}

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

  lastEncoded = encoded;  //Update the last encoding value
}
