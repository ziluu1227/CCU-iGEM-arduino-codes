#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#define I2C_ADDR 0x3F
#define LCD_ROWS 2
#define LCD_COLS 16
LiquidCrystal_I2C lcd(I2C_ADDR, LCD_COLS, LCD_ROWS);

const int encoderPinA = 2;  // 旋轉編碼器引腳 A（CLK）
const int encoderPinB = 3;  // 旋轉編碼器引腳 B（DT）
const int confirmPin = 4; //確認引腳 10k Ω

int lastEncoded = 0;
long encoderValue = 0; //旋轉編碼器記數
long tempencoderValue = 0; //旋轉編碼器記數暫存
int mode = 0; //記數切換模式
int selectmode = 0; //選擇第一層模式
int dir = 0;//定義旋轉方向
int rot = 0;//是否旋轉
int modeselection; //儲存選擇的模式
int level = 1; //現在顯示的等級(圖層順序)

// 步進馬達控制引腳
const int stepPin = 5;
const int dirPin = 6;
const int enablePin = 7;

// 步進馬達的步進角度和腳位數
const float stepAngle = 1.8; // 步進角度 (度)
const int stepsPerRevolution = 200; // 步進馬達的腳位數

// 步進馬達旋轉方向
const int clockwise = HIGH;
const int counterclockwise = LOW;
int direction; //1=>CW;0=>CCW;

// 步進馬達旋轉速度 (步驟/秒)
int stepperSpeed = 500; // 設定步驟/秒的數值，控制旋轉速度 要改的參數
int steps; //移動步數

void setup() {
  Serial.begin(9600);
  Wire.begin();
  lcd.begin(LCD_COLS, LCD_ROWS);
  lcd.backlight();
  // 設定控制引腳為輸出
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(enablePin, OUTPUT);

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
  
  // 禁用步進馬達 (預設為低電平)
  digitalWrite(enablePin, HIGH);
}

void loop() {
  // 編碼器正逆轉判斷
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


  selectmode = abs(mode % 2); //選擇要操作的模式:1=>速度;2=>步數
  switch (level){
    case 1:
    lcd.setCursor(0,0);
    lcd.print("Speed");
    if(mode >= 0){
      lcd.setCursor(0, 1);
      lcd.print(mode*100);
      stepperSpeed = mode*100;
      lcd.print("s"); // 之後要改成速率或是對毫升數
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
      lcd.print("steps"); // 之後要改成速率或是對毫升數
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
      digitalWrite(dirPin, clockwise); // 設定方向為順時針
      for (int i = 0; i < steps; i++) {
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(stepperSpeed); // 設定步進馬達的步進速度
        digitalWrite(stepPin, LOW);
        delayMicroseconds(stepperSpeed); // 設定步進馬達的步進速度
      }
    }
    if(direction == 0){
      digitalWrite(dirPin, counterclockwise); // 設定方向為逆時針
      for (int i = 0; i < steps; i++) {
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(stepperSpeed); // 設定步進馬達的步進速度
        digitalWrite(stepPin, LOW);
        delayMicroseconds(stepperSpeed); // 設定步進馬達的步進速度
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
  // 順時針旋轉
  rotateClockwise(500); // 旋轉200步驟
  delay(1000); // 延遲 1 秒
  
  // 逆時針旋轉
  rotateCounterclockwise(500); // 旋轉100步驟
  delay(1000); // 延遲 1 秒

  digitalWrite(enablePin,HIGH);
  delay(100);
  */
}
/*
// 步進馬達順時針旋轉指定步數
void rotateClockwise(int steps) {
  digitalWrite(dirPin, clockwise); // 設定方向為順時針
  for (int i = 0; i < steps; i++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(stepperSpeed); // 設定步進馬達的步進速度
    digitalWrite(stepPin, LOW);
    delayMicroseconds(stepperSpeed); // 設定步進馬達的步進速度
  }
}

// 步進馬達逆時針旋轉指定步數
void rotateCounterclockwise(int steps) {
  digitalWrite(dirPin, counterclockwise); // 設定方向為逆時針
  for (int i = 0; i < steps; i++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(stepperSpeed); // 設定步進馬達的步進速度
    digitalWrite(stepPin, LOW);
    delayMicroseconds(stepperSpeed); // 設定步進馬達的步進速度
  }
}
*/
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

