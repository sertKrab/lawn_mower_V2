

#include <Executive.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <IBusBM.h>
#include <EEPROM.h>
#include <avr/wdt.h>
IBusBM IBus;
// Use names not numbers - then there is only one place you need to change it
const int PIN = 13;  // 'normal' Arduino
//const int PIN = 17;	// Sparkfun Pro Micro
LiquidCrystal_I2C lcd(0x27, 20, 4);
int rcLeft_Right = 250;  // Left - Right
int rcFor_Rev = 250;     // Forward - Reverse
int rc_linear = 0;

int LR_check = 70;
int FR_check = 70;

int time_minute = 0;
int time_hour = 0;

int eeprom_s_add = 0;
int eeprom_h_add = 0;

unsigned long real_time = 0;
bool check_time_point = false;

#define pwmMotorRight 3
#define pwmMotorLeft 5

#define dirMotorRight 8
#define dirMotorLeft 9
#define linearRMo 10
#define linearLMo 11

// the setup function runs once when you press reset or power the board


void setup() {
  // initialise digital pin for LED as an output.
  IBus.begin(Serial);
  pinMode(PIN, OUTPUT);
  pinMode(pwmMotorRight, OUTPUT);
  pinMode(dirMotorRight, OUTPUT);
  pinMode(pwmMotorLeft, OUTPUT);
  pinMode(dirMotorLeft, OUTPUT);
  pinMode(linearRMo, OUTPUT);
  pinMode(linearLMo, OUTPUT);


  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("* V2 *");
  // Define our schedule
  setupTasks();
  real_time = millis();
  read_time_eeprom();
 
}

// the loop function runs over and over again forever
void loop() {
  Exec.loop();  // Hand control over to Exec (never returns)
}


void displayLCD() {

  lcd.setCursor(10, 0);

  unsigned long check_time = millis();
  unsigned long div_time = (check_time - real_time);
  if (div_time > 60000) {
    real_time = millis();

    time_minute++;

    if ((time_minute % 2) == 0) {
      writeIntIntoEEPROM(eeprom_s_add * 2, time_minute);
      eeprom_s_add++;
      writeIntIntoEEPROM(eeprom_s_add * 2, -1);

      if (eeprom_s_add > 300) {
        eeprom_s_add = 0;
      }
    }
    if (time_minute > 60) {
      time_hour++;
      time_minute = 0;
      writeIntIntoEEPROM((eeprom_h_add + 305) * 2, time_hour);
      eeprom_h_add++;
      writeIntIntoEEPROM((eeprom_h_add + 305) * 2, -1);

      if (eeprom_h_add > 100) {
        eeprom_h_add = 0;
      }
      if (time_hour > 999) {
        time_hour = 0;
      }
    }
  }


  if (time_hour < 10) {
    lcd.print("0");
  }
  lcd.print(time_hour);
  if (check_time_point) {
    lcd.print(":");

  } else {
    lcd.print(" ");
  }
  if (time_minute < 10) {
    lcd.print("0");
  }
  lcd.print(time_minute);


  check_time_point = !check_time_point;
}

void controlCarDir() {
  rcLeft_Right = readChannel(0, -255, 255, 0);
  rcFor_Rev = readChannel(1, -255, 255, 0);
  uint16_t ch = readChannel(3, -255, 255, 0);
  rc_linear = ch;
}
int readChannel(byte channelInput, int minLimit, int maxLimit, int defaultValue) {
  uint16_t ch = IBus.readChannel(channelInput);
  if (ch < 100) return defaultValue;

  return map(ch, 1000, 2000, minLimit, maxLimit);
}

void controlMotor() {


  if (rcLeft_Right < LR_check && rcLeft_Right > -LR_check && rcFor_Rev < FR_check && rcFor_Rev > -FR_check) {
    lcd.setCursor(0, 1);
    lcd.print("1S 0 0      ");
    changeDirMotor(false, false);
    changePwmMotor(0, 0);

  } else if (rcLeft_Right > LR_check && rcFor_Rev < FR_check && rcFor_Rev > -FR_check) {
    lcd.setCursor(0, 1);
    lcd.print("2R 250 250 ");
    changeDirMotor(true, true);
    changePwmMotor(255, 255);

  } else if (rcLeft_Right < -LR_check && rcFor_Rev < FR_check && rcFor_Rev > -FR_check) {
    lcd.setCursor(0, 1);
    lcd.print("3L 250 250 ");
    changeDirMotor(false, false);
    changePwmMotor(255, 255);

  } else if (rcLeft_Right > LR_check && rcFor_Rev > FR_check) {
    lcd.setCursor(0, 1);
    lcd.print("4R 0 " + String(rcFor_Rev) + " ");
    changeDirMotor(true, true);
    changePwmMotor(0, abs(rcFor_Rev));

  } else if (rcLeft_Right > LR_check && rcFor_Rev < -FR_check) {
    lcd.setCursor(0, 1);
    lcd.print("5L 0 " + String(rcFor_Rev) + " ");
    changeDirMotor(false, false);
    changePwmMotor(0, abs(rcFor_Rev));

  } else if (rcLeft_Right < -LR_check && rcFor_Rev > FR_check) {
    lcd.setCursor(0, 1);
    lcd.print("6L " + String(rcFor_Rev) + " 0   ");
    changeDirMotor(false, false);
    changePwmMotor(abs(rcFor_Rev), 0);

  } else if (rcLeft_Right < -LR_check && rcFor_Rev < -FR_check) {
    lcd.setCursor(0, 1);
    lcd.print("7R " + String(rcFor_Rev) + " 0 ");
    changeDirMotor(true, true);
    changePwmMotor(abs(rcFor_Rev), 0);

  } else if (rcLeft_Right < LR_check && rcLeft_Right > -LR_check && rcFor_Rev > FR_check) {
    lcd.setCursor(0, 1);
    lcd.print("8S " + String(rcFor_Rev) + " " + String(rcFor_Rev) + "  ");
    changeDirMotor(false, true);
    changePwmMotor(abs(rcFor_Rev), abs(rcFor_Rev));
  } else if (rcLeft_Right < LR_check && rcLeft_Right > -LR_check && rcFor_Rev < -FR_check) {
    lcd.setCursor(0, 1);
    lcd.print("9S " + String(rcFor_Rev) + " " + String(rcFor_Rev) + "  ");
    changeDirMotor(true, false);
    changePwmMotor(abs(rcFor_Rev), abs(rcFor_Rev));
  } else {
    lcd.setCursor(0, 1);
    lcd.print("1S 0 0  ");
    changeDirMotor(false, true);
    changePwmMotor(0, 0);
  }

  controlLinearMortor();
}


void changePwmMotor(int pwmLeft, int pwmRight) {

  analogWrite(pwmMotorLeft, abs(pwmLeft));
  analogWrite(pwmMotorRight, abs(pwmRight));
}

void changeDirMotor(bool dirLeft, bool dirRight) {

  if (dirLeft) {
    digitalWrite(dirMotorLeft, LOW);
  } else {
    digitalWrite(dirMotorLeft, HIGH);
  }

  if (dirRight) {
    digitalWrite(dirMotorRight, LOW);
  } else {
    digitalWrite(dirMotorRight, HIGH);
  }
}

void controlLinearMortor() {

  lcd.setCursor(7, 0);

  if (rc_linear > LR_check) {
    lcd.print("U ");
    digitalWrite(linearRMo, HIGH);
    digitalWrite(linearLMo, LOW);
  } else if (rc_linear < -LR_check) {
    lcd.print("D ");
    digitalWrite(linearRMo, LOW);
    digitalWrite(linearLMo, HIGH);
  } else {
    lcd.print("S ");
    digitalWrite(linearRMo, LOW);
    digitalWrite(linearLMo, LOW);
  }
}




// Read the channel and return a boolean value
bool readSwitch(byte channelInput, bool defaultValue) {
  int intDefaultValue = (defaultValue) ? 100 : 0;
  int ch = readChannel(channelInput, 0, 100, intDefaultValue);
  return (ch > 50);
}
// Put this at the end of your sketch
void setupTasks() {
  Exec.addTask(200, controlCarDir);      // Turn on every 2s
  Exec.addTask(200, controlMotor, 300);  // Turn off every 2s, but start 1s later
  Exec.addTask(900, displayLCD,100);   

}

void writeIntIntoEEPROM(int address, int number) {
  EEPROM.write(address, number >> 8);
  EEPROM.write(address + 1, number & 0xFF);
}
int readIntFromEEPROM(int address) {
  return (EEPROM.read(address) << 8) + EEPROM.read(address + 1);
}


void read_time_eeprom() {
  // int eeprom_s_add = 0;
  // int eeprom_h_add = 0
  int last_s = 0;
  for (int i = 0; i < 300; i++) {
    int min = readIntFromEEPROM(i * 2);

    eeprom_s_add = i;

    if (min == -1) {
      i = 400;
      time_minute = last_s;
    }

    if (min < 0) {
      min = 0;
    }
    last_s = min;
  }
  int last_h = 0;
  for (int i = 0; i < 100; i++) {
    int ho = readIntFromEEPROM((i + 305) * 2);

    eeprom_h_add = i;

    if (ho == -1) {
      i = 400;
      time_hour = last_h;
    }

    if (ho < 0) {
      ho = 0;
    }
    last_h = ho;
  }
}
