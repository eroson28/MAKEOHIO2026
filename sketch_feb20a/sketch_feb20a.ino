/*-- INCLUSIONS --*/
#include <ScrollText.h>

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Infrared libraries
#include <IRremoteESP8266.h>
#include <IRrecv.h>

// Internet connectivity
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <esp_wifi.h>

/*-- WIFI --*/
const char* ssid = "Registered4OSU";
const char* password = "ksX7x6cUcRfY46iU";

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
  int id;
};
const int STATUS_ID_AVAILABLE = 1;
const int STATUS_ID_AWAY = 2;
const int STATUS_ID_BUSY = 3;
const Status STATUS_AVAILABLE = { "      Open", "Please knock before entering - ", { 0, 255, 0 }, STATUS_ID_AVAILABLE };
const Status STATUS_AWAY = { "      Away", "Napping, will return soon - ", { 0, 0, 255 }, STATUS_ID_AWAY };
const Status STATUS_BUSY = { "      Busy", "Please do not enter - ", { 255, 0, 0 }, STATUS_ID_BUSY };

Status getStatusFromId(int id) {
  if (id == STATUS_ID_AVAILABLE) {
    return STATUS_AVAILABLE;
  } else if (id == STATUS_ID_AWAY) {
    return STATUS_AWAY;
  } else if (id == STATUS_ID_BUSY) {
    return STATUS_BUSY;
  }
}

/*-- MAIN --*/

// declare LCD 
LiquidCrystal_I2C lcd(LCD_ADDR, LCD_COLS, LCD_ROWS);
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

// void printMac() 
// {
//   uint8_t mac[6];
//   esp_read_mac(mac, ESP_MAC_WIFI_STA);
//   Serial.printf("%02X:%02X:%02X:%02X:%02X:%02X\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
// }

void lcdTempStatus(String text) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(text);
}

void setup_mac() {
  Serial.print("OLD ESP32 MAC Address: ");
  Serial.println(WiFi.macAddress());

  // We don't need to update anymore.
  return;

  uint8_t New_MAC_Address[] = {0x10, 0xAA, 0xBB, 0xCC, 0x33, 0xF5};
  esp_err_t result = esp_wifi_set_mac(WIFI_IF_STA, New_MAC_Address);
  
  if (result == ESP_OK) {
    Serial.println("ESP_OK");
  } else if (result == ESP_ERR_WIFI_NOT_INIT) {
    Serial.println("ESP_ERR_WIFI_NOT_INIT");
  } else if (result == ESP_ERR_INVALID_ARG) {
    Serial.println("ESP_ERR_INVALID_ARG");
  } else if (result == ESP_ERR_WIFI_MAC) {
    Serial.println("ESP_ERR_WIFI_MAC");
  } else if (result == ESP_ERR_WIFI_MODE) {
    Serial.println("ESP_ERR_WIFI_MODE");
  } else {
    Serial.print("Unknown error: ");
    Serial.println(result);
  }

  Serial.print("NEW ESP32 MAC Address: ");
  Serial.println(WiFi.macAddress());
}

void setup_wifilogin() {
  lcdTempStatus("Logging in...");
  // WiFi.mode(WIFI_MODE_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
  }
}

void setup_pinghttp(bool secure) {
  lcdTempStatus("Test WiFi...");
  if (!secure) {
    HTTPClient http;
    http.begin("http://google.com");
    int httpCode = http.GET();

    Serial.print("HTTP Code ");
    Serial.println(httpCode);
    if (httpCode > 0) {
      String payload = http.getString();
      Serial.println(payload);
    }
    http.end();
  } else {
    WiFiClientSecure client;
    client.setInsecure(); // Skip Certificate authority. for prototype ONLY!
    
    HTTPClient https;
    https.begin(client, "https://cam.natelevison.com/");
    int httpCode = https.GET();

    Serial.print("HTTPS Code ");
    Serial.println(httpCode);
    if (httpCode > 0) {
      // String payload = https.getString();
      // Discard the payload for now.
    }
    https.end();
  }
}

void setup()
{
  // Start the console
  Serial.begin(9600);

  // Set up the LCD first
  Wire.begin(PIN_SDA, PIN_SCL);
  delay(500);
  lcd.init();
  lcd.backlight();

  lcdTempStatus("Starting internet...");

  // Set up WiFi
  setup_wifilogin();
  setup_mac();
  setup_pinghttp(true);

  lcdTempStatus("Starting LED and IR...");

  // Set up the PWM for the RGB LED
  ledcAttach(PIN_RED, PWM_FREQ, PWM_RES);
  ledcAttach(PIN_GREEN, PWM_FREQ, PWM_RES);
  ledcAttach(PIN_BLUE, PWM_FREQ, PWM_RES);

  // start IR receiver
  irrecv.enableIRIn();
  
  lcdTempStatus("Done!");
  delay(250);
  lcd.clear();
}

void loop()
{
  delay(100);
  curStatusCaption.tick();
  lcd.setCursor(0, 0);
  lcd.print(curStatus.title);
  lcd.setCursor(0, 1);
  lcd.print(curStatusCaption.getText());

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
    } else if (btn == BTN_SKIP_LEFT) {
      curStatusCaption.decSpeed();
    } else if (btn == BTN_SKIP_RIGHT) {
      curStatusCaption.incSpeed();
    }
  }
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