#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <ScrollText.h>

/*-- CONSTANTS --*/

// Buttons on the remote
const int BTN_POW = 1;
const int BTN_MODE = 2;
const int BTN_MUTE = 3;
const int BTN_PLAY = 4;
const int BTN_SKIP_LEFT = 5;
const int BTN_SKIP_RIGHT = 6;
const int BTN_EQ = 7;
const int BTN_MINUS = 8;
const int BTN_PLUS = 9;
const int BTN_0 = 10;
const int BTN_CYCLE = 11;
const int BTN_USD = 12;
const int BTN_1 = 13;
const int BTN_2 = 14;
const int BTN_3 = 15;
const int BTN_4 = 16;
const int BTN_5 = 17;
const int BTN_6 = 18;
const int BTN_7 = 19;
const int BTN_8 = 20;
const int BTN_9 = 21;

// LCD info
const int LCD_ROWS = 16;
const int LCD_COLS = 2;
const int LCD_ADDR = 0x27;

// RGB LED PWM info ?
const int PWM_FREQ = 5000;
const int PWM_RES = 8;

// Pins
const uint16_t PIN_IR_RECEIVE = 14;
const uint16_t PIN_SDA = 21;
const uint16_t PIN_SCL = 23;
const uint16_t PIN_RED = 27;
const uint16_t PIN_GREEN = 26;
const uint16_t PIN_BLUE = 25;

// Status values
struct Color {
  int r;
  int g;
  int b;
};
struct Status {
  String title;
  String caption;
  Color color;
};
const Status STATUS_AVAILABLE = { "Available", "Please knock before entering! - ", { 255, 255, 255 } };
const Status STATUS_AWAY = { "Away", "Out of office right now - ", { 255, 255, 0 } };
const Status STATUS_BUSY = { "Busy", "Please do not enter - ", { 255, 0, 0 } };

/*-- MAIN --*/

// declare LCD 
LiquidCrystal_I2C lcd(LCD_ADDR, LCD_ROWS, LCD_COLS);
IRrecv irrecv(PIN_IR_RECEIVE);
decode_results results;

// Current status
Status curStatus = STATUS_AVAILABLE;

// Stuff displaying the current status
ScrollText curStatusCaption(curStatus.caption);
Color curStatusColor = curStatus.color;

// power state variable
bool power = true;
int count = 0;

void setup()
{
  // Start the console
  Serial.begin(115200);
  Serial.println("Hello, World!");

  // Something to do with the LCD
  Wire.begin(PIN_SDA, PIN_SCL);

  // Set up the PWM for the RGB LED
  ledcAttach(PIN_RED, PWM_FREQ, PWM_RES);
  ledcAttach(PIN_GREEN, PWM_FREQ, PWM_RES);
  ledcAttach(PIN_BLUE, PWM_FREQ, PWM_RES);

  // Initialize the displays
  lcd.init();           
  lcd.backlight();      
  lcd.setCursor(0,0);
  lcd.print("Loading");
  setColor({ 255, 255, 255 });
  delay(1000);
  setColor({ 0, 255, 0 });
  // start IR receiver
  irrecv.enableIRIn();
}

void loop()
{
  delay(100);
  Serial.println("Looping.");

  Serial.println("Ticking...");
  curStatusCaption.tick();
  Serial.println("Finished tick.");

  // lcd.clear();
  Serial.println("Getting caption location...");
  String s = curStatusCaption.getText();
  Serial.println("Got caption location.");
  
  Serial.println("Writing text...");
  lcd.setCursor(0, 0);
  lcd.print(s);
  Serial.println("Wrote text.");

  Serial.println("Getting remote...");
  int btn = getRemoteButtonPressed();
  if (btn != -1) {
    if (btn == BTN_POW) {
      power = !power;
      if (power) {
        lcd.display();
        setColor(curStatusColor);
      } else {
        lcd.noDisplay();
        setColor({ 0, 0, 0 });
      }
    } else if (btn == BTN_1) {
      setStatus(STATUS_AVAILABLE);
    } else if (btn == BTN_2) {
      setStatus(STATUS_AWAY);
    } else if (btn == BTN_3) {
      setStatus(STATUS_BUSY);
    }
  }
  Serial.println("Got remote.");
}

void setColor(Color c) {
  ledcWrite(PIN_RED, c.r);
  ledcWrite(PIN_GREEN, c.g);
  ledcWrite(PIN_BLUE, c.b);
}

void setStatus(Status status) {
  curStatus = status;
  curStatusCaption.setText(curStatus.caption);
  curStatusColor = curStatus.color;
  setColor(curStatusColor);
}

int getRemoteButtonPressed() {
  if (irrecv.decode(&results)) {
    int btn = decodeKeyValue(results.value);
    irrecv.resume();
    return btn;
  } else {
    return -1;
  }
}

// Function to decode the value of the IR signal
int decodeKeyValue(long result)
{
  switch(result){
    case 0xFF6897:
      return BTN_0;
    case 0xFF30CF:
      return BTN_1; 
    case 0xFF18E7:
      return BTN_2; 
    case 0xFF7A85:
      return BTN_3; 
    case 0xFF10EF:
      return BTN_4; 
    case 0xFF38C7:
      return BTN_5; 
    case 0xFF5AA5:
      return BTN_6; 
    case 0xFF42BD:
      return BTN_7; 
    case 0xFF4AB5:
      return BTN_8; 
    case 0xFF52AD:
      return BTN_9; 
    case 0xFF906F:
      return BTN_PLUS; 
    case 0xFFA857:
      return BTN_MINUS; 
    case 0xFFE01F:
      return BTN_EQ; 
    case 0xFFB04F:
      return BTN_USD;
    case 0xFF9867:
      return BTN_CYCLE;         
    case 0xFF22DD:
      return BTN_PLAY;   
    case 0xFF02FD:
      return BTN_SKIP_LEFT;   
    case 0xFFC23D:
      return BTN_SKIP_RIGHT;   
    case 0xFFA25D:
      return BTN_POW;   
    case 0xFFE21D:
      return BTN_MUTE;   
    case 0xFF629D:
      return BTN_MODE;
    default:
      return -1;
  }
}