#include <SPI.h>
#include <Ethernet.h>
#include <ArduinoHttpClient.h>
#include <Keypad.h>
#include <MFRC522.h>

/////// RFID Settings ///////
#define SS_PIN 53
#define RST_PIN 10
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.
String registeredCard = "A0 C4 74 A3";

/////// Ethernet Settings ///////
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 1, 222);
char serverAddress[] = "192.168.1.51";  // server address
int port = 80;

/////// HttpClient Settings ///////
EthernetClient ethr;
HttpClient client = HttpClient(ethr, serverAddress, port);
String response;
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

// Status Variables //
int rStatus = 0;
int pStatus = 0;

void setup() {
  /////// Starting Serial ///////
  Serial.begin(9600);
  delay(1000);

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

  Serial.println("Please tap RFID Card.");
}

void loop() {
  if (rStatus == 0)
  {
    readRFID();
  }
  else if (rStatus == 1)
  {
    keypad.getKey();
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
  Serial.print("UID tag :");
  String content = "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
  Serial.print("Message : ");
  content.toUpperCase();
  if (content.substring(1) == registeredCard) //change here the UID of the card/cards that you want to give access
  {
    Serial.println("UID Found.");
    Serial.println();
    delay(3000);
    rStatus = 1;
    Serial.println("Please enter 4 digit PIN.");
  }

  else   {
    Serial.println(" Access denied");
    delay(3000);
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
      delay(10);
      i++;

      switch (eKey)
      {
        case '#': checkPassword(); delay(1); break;

        case '*': i = 0; delay(1); break;

        default: delay(1);
      }
  }
}

void checkPassword()
{
  pass = "";
  pass.concat(key[0]);
  pass.concat(key[1]);
  pass.concat(key[2]);
  pass.concat(key[3]);

  if (pass == "1234")
  {
    Serial.println("PIN Accepted");
    i = 0;
    pStatus = 1;
    delay(10);
  } else
  {
    Serial.println(" PIN Denied"); //if passwords wrong keep box locked
    i = 0;
    pStatus = 0;
    delay(10);
  }
}

void getRequest() {
  Serial.println("making GET request");
  client.get("/get.php?tag=160196116163179&pin=");

  // read the status code and body of the response
  statusCode = client.responseStatusCode();
  response = client.responseBody();

  Serial.print("Status code: ");
  Serial.println(statusCode);
  Serial.print("Response: ");
  Serial.println(response);
  Serial.println("Wait five seconds");
  delay(5000);
}
