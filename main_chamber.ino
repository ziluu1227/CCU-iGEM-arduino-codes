#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#define I2C_ADDR 0x27
#define LCD_ROWS 2
#define LCD_COLS 16
LiquidCrystal_I2C lcd(I2C_ADDR, LCD_COLS, LCD_ROWS);

#include <Servo.h>
// 定義伺服馬達物件
//Servo valve1servo;
//Servo valve2servo;
Servo shakeservo;

const int encoderPinA = 2;  // 旋轉編碼器引腳 A（CLK）
const int encoderPinB = 3;  // 旋轉編碼器引腳 B（DT）
const int confirmPin = 4; //確認引腳 10k Ω
const int PBSPin = 5; //PBS引腳
const int mGLPin = 6; //mGL引腳
const int DNasePin = 7; //DNase引腳
const int beepPin = 8; //蜂鳴器引腳
//const int valve1Pin = 9; //第一閥引腳
//const int valve2Pin = 10; //第二閥引腳
const int shakePin = 11; //搖晃引腳

int lastEncoded = 0;
long encoderValue = 0; //旋轉編碼器記數
long tempencoderValue = 0; //旋轉編碼器記數暫存
int mode = 0; //記數切換模式
int selectmode = 0; //選擇第一層模式
int dir = 0;//定義旋轉方向
int rot = 0;//是否旋轉
int modeselection; //儲存選擇的模式
int level = 1; //現在顯示的等級(圖層順序)
int ml; //liquid ml
//int valve1 = 1; //第一閥為開啟
//int valve2 = 1; //第二閥為開啟
//int valve1act = 1;//第一閥即將要進行的動作:1開0關
//int valve2act = 1;//第二閥即將要進行的動作:1開0關
int sec;//shake time
int pumptime = 620;//pumpint time,1ml per 0.62sec,must be retest after confirm every risistor and motor ans servo
int shakepos = 30;//init shake angle

void setup() {
  // 初始化串口通信
  Serial.begin(9600);
  Wire.begin();
  lcd.begin(LCD_COLS, LCD_ROWS);
  lcd.backlight();

  //蜂鳴器引腳輸出
  pinMode(beepPin, OUTPUT);

  //PBS引腳輸出
  pinMode(PBSPin, OUTPUT);
  //mGL引腳輸出
  pinMode(mGLPin, OUTPUT);
  //DNase引腳輸出
  pinMode(DNasePin, OUTPUT);

  //伺服馬達引腳輸出
  //valve1servo.attach(valve1Pin);//第一閥
  //valve2servo.attach(valve2Pin);//第二閥
  shakeservo.attach(shakePin);//搖晃馬達
  //伺服馬達先開
  //valve1servo.write(20);//valve1 opening code
  //valve2servo.write(20);//valve1 opening code
  shakeservo.write(shakepos);

  
  // 設定旋轉編碼器引腳為輸入模式
  pinMode(encoderPinA, INPUT);
  pinMode(encoderPinB, INPUT);
  pinMode(confirmPin, INPUT);
  
  // 啟用內部上拉電阻
  digitalWrite(encoderPinA, HIGH);
  digitalWrite(encoderPinB, HIGH);
  
  // 附加中斷服務例程
  attachInterrupt(digitalPinToInterrupt(encoderPinA), updateEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoderPinB), updateEncoder, CHANGE);
}

void loop() {
    // 正逆轉判斷
    if (encoderValue > tempencoderValue){
      mode++;
      tempencoderValue = encoderValue;
      dir = 1; //正轉
      rot = 1;
    }
    if (encoderValue < tempencoderValue){
      mode--;
      tempencoderValue = encoderValue;
      dir = -1; //逆轉
      rot = 1;
    }

  //Serial.println("encoderValue");
  //Serial.println(encoderValue);
  //Serial.println("tempencoderValue");
  //Serial.println(tempencoderValue);
  //Serial.println("mode");

  selectmode = abs(mode % 6); //選擇要操作的裝置 1=PBS;2=mGL;3=DNase;4=valve1;5=valve2;6=shake;
  switch (level){
    case 1://select mode to control
      switch (selectmode){ //選擇要使用的裝置部分
          case 1:
            if(mode > 0){ //正轉
              lcd.setCursor(0, 0); // 設定游標位置在第一行行首
              lcd.print("Select Mode");
              lcd.setCursor(0,1);
              lcd.print("PBS");
              modeselection = 1;
              if (digitalRead(confirmPin) == HIGH){
                level = 2;
                break;
              }
            }
            if(mode < 0){ //反轉
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
            case 0: //mode回零不顯示確認
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
  int MSB = digitalRead(encoderPinA);  // 讀取引腳 A 的狀態
  int LSB = digitalRead(encoderPinB);  // 讀取引腳 B 的狀態

  int encoded = (MSB << 1) | LSB;  // 編碼值
  
  int sum = (lastEncoded << 2) | encoded;  // 上一次和當前的編碼值組合

  // 根據旋轉方向增加或減少編碼器值
  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) {
    encoderValue++;
  } else if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) {
    encoderValue--;
  }

  lastEncoded = encoded;  // 更新上一次的編碼值
}
