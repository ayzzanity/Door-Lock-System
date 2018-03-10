#include <SPI.h>
#include <Ethernet.h>
#include <ArduinoHttpClient.h>
#include <Keypad.h>
#include <MFRC522.h>
#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

/////// LCD Settings ///////
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
/////// RFID Settings ///////
#define SS_PIN 53
#define RST_PIN 10
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.
/////// Ethernet Settings ///////
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 1, 222);
char serverAddress[] = "192.168.1.51";  // server address
int port = 80;
/////// HttpClient Settings ///////
EthernetClient ethr;
HttpClient client = HttpClient(ethr, serverAddress, port);
int statusCode = 0;
/////// Keypad Settings ///////
char key[4];
int i = 0;
String pass;
const byte ROWS = 4; // Four rows
const byte COLS = 4; // columnsffff
// Define the Keymap
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
// Connect keypad ROW0, ROW1, ROW2 and ROW3 to these Arduino pins.
byte rowPins[ROWS] = {29, 28, 27, 26}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {25, 24, 23, 22}; //connect to the column pinouts of the keypad
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
/////// Servo Settings ///////
Servo lock; //declares servo
/////// Status Variables & Strings ///////
int rStatus = 0; // RFID Status
int pStatus = 0; // PIN Status
int lStatus = 0; // Lock Status
String getReq, tag, rfid, pincode;

void setup() {
  /////// Starting Serial ///////
  Serial.begin(9600);
  delay(1000);
  /////// LCD Setup ///////
  lcd.begin(16, 2);
  lcd.backlight();
  lcd.home();
  lcd.print("Initializing...");
  /////// Ethernet Setup ///////
  Serial.println("Initializing Ethernet.");
  Ethernet.begin(mac, ip);
  Serial.println("Connection Success.");
  Serial.println("");
  delay(1000);
  /////// RFID Setup ///////
  Serial.println("Initializing RFID Module.");
  mfrc522.PCD_Init();   // Initiate MFRC522
  Serial.println("RFID Ready.");
  Serial.println("");
  delay(1000);
  /////// Keypad Setup ///////
  Serial.println("Initializing Keypad Module...");
  keypad.addEventListener(keypadEvent); //add an event listener for this keypad
  Serial.println("Keypad Ready.");
  Serial.println("");
  delay(1000);
  /////// Servo Setup ///////
  Serial.println("Setting up servo motor...");
  lock.attach(13);
  for (int pos = 90; pos >= 0; pos -= 1) {
    lock.write(pos);
    delay(15);
  }
  Serial.println("Servo Motor Ready.");
  Serial.println("");
  delay(1000);
  /////// R E A D Y ///////
  Serial.println("System is Ready.");
  lcd.clear();
  lcd.home();
  lcd.print("System is Ready.");
  delay(1000);
}

void loop() {
  if (rStatus == 0)
  {
    lcd.home();
    lcd.print("Tap your RFID...");
    readRFID();
  }
  else if (rStatus == 1 && pStatus == 0)
  {
    lcd.home();
    lcd.print("Enter PIN:");
    keypad.getKey();
  }
  if (rStatus == 1 && pStatus == 1)
  {
    delay(1000);
    lcd.clear();
    lcd.home();
    lcd.print("Door Unlocked.");
    lcd.setCursor(0, 1);
    lcd.print("Tap RFID to Lock");
    if (lStatus == 1)
    {
      for (int pos = 0; pos <= 90; pos += 1) {
        lock.write(pos);
        delay(15);
      }
      lStatus = 0;
    }
    lockDoor();
  }
}
/////// Module Methods - RFID ///////
void readRFID()
{
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent())
  {
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial())
  {
    return;
  }
  //Show UID on serial monitor
  Serial.print("RFID UID : ");
  String content = "";
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    content.concat(String(mfrc522.uid.uidByte[i]));
  }
  Serial.println(content);
  tag = String(content);
  lcd.setCursor(0, 1);
  lcd.print("Checking...");
  getTagRequest();
  Serial.print("DB UID : ");
  Serial.println(rfid);
  lcd.clear();
  lcd.home();
  lcd.print("Message:");
  if (tag == rfid)
  {
    lcd.setCursor(0, 1);
    lcd.print("RFID Authorized");
    delay(3000);
    lcd.clear();
    rStatus = 1;
    Serial.println("Please enter 4 digit PIN.");
  }
  else   {
    lcd.setCursor(0, 1);
    lcd.print("RFID Denied");
    delay(3000);
    lcd.clear();
    rStatus = 0;
  }
}
/////// Module Methods - Keypad ///////
void keypadEvent(KeypadEvent eKey)
{
  switch (keypad.getState())
  {
    case PRESSED:
      key[i] = eKey;
      Serial.print("Enter: ");
      Serial.println(eKey);
      lcd.setCursor(i, 1);
      lcd.print("*");
      delay(10);
      i++;
      switch (eKey)
      {
        case '#': checkPassword(); delay(1); break;

        case '*': i = 0; lcd.setCursor(0, 1); lcd.print("    "); delay(1); break;

        default: delay(1);
      }
  }
}

void checkPassword()
{
  lcd.setCursor(0, 1);
  lcd.print("Checking...");
  pass = "";
  pass.concat(key[0]);
  pass.concat(key[1]);
  pass.concat(key[2]);
  pass.concat(key[3]);
  Serial.print("Input PIN: ");
  Serial.println(pass);
  delay(1000);
  getPinRequest();
  Serial.print("DB PIN: ");
  Serial.println(pincode);
  lcd.clear();
  if (pass == pincode)
  {
    lcd.print("PIN Accepted");
    i = 0;
    pStatus = 1;
    lStatus = 1;
    delay(10);
  } else
  {
    lcd.print("PIN Denied");
    i = 0;
    pStatus = 0;
    delay(10);
  }
}
/////// Lock Door Again ///////
void lockDoor() {
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent())
  {
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial())
  {
    return;
  }
  //Show UID on serial monitor
  Serial.print("RFID UID : ");
  String content = "";
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    content.concat(String(mfrc522.uid.uidByte[i]));
  }
  Serial.println(content);
  tag = String(content);
  lcd.setCursor(0, 1);
  lcd.print("Checking........");
  getTagRequest();
  Serial.print("DB UID : ");
  Serial.println(rfid);
  lcd.clear();
  lcd.home();
  if (tag == rfid)
  {
    lcd.setCursor(0, 0);
    lcd.print("RFID Authorized");
    lcd.setCursor(0, 1);
    lcd.print("Door will lock.");
    delay(3000);
    lcd.clear();
    for (int pos = 90; pos >= 0; pos -= 1) {
      lock.write(pos);
      delay(15);
    }
    delay(1000);
    rStatus = 0;
    pStatus = 0;
    lStatus = 0;
    delay(1000);
  }
  else
  {
    lcd.setCursor(0, 1);
    lcd.print("RFID Denied");
    delay(3000);
    lcd.clear();
  }
}
/////// Database Transactions via PHP ///////
void getTagRequest() {
  //Making GET Request
  getReq = "";
  getReq.concat("/getTag.php?tag=");
  getReq.concat(tag);
  client.get(getReq);
  rfid = client.responseBody();

  client.stop();
  delay(1000);
}
void getPinRequest() {
  //Making GET Request
  getReq = "";
  getReq.concat("/getPin.php?tag=");
  getReq.concat(tag);
  getReq.concat("&pin=");
  getReq.concat(pass);
  client.get(getReq);
  pincode = client.responseBody();

  client.stop();
  delay(1000);
}
