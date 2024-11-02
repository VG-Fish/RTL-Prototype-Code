#include <Arduino_LSM6DSOX.h>


#include <SPI.h>
#include <MFRC522.h>


// Timer
unsigned long previousMilliseconds = 0;
const unsigned long interval = 1000; // One second
unsigned long counter = 0;


// Acceleration
float px, py, pz;
float x, y, z;
const float THRESHOLD = 0.01;
int intruder_counter = 0;
bool opening = false;


// RFID
bool can_open = false;
bool start_timer = false;
#define RST_PIN 9
#define SS_PIN 10
MFRC522 mfrc522(SS_PIN, RST_PIN);


// Motor Driver
int motor_pin = 0; // Placeholder


void setup() {
  Serial.begin(9600);


  // For acceleration
  while (!Serial);  // creates infinite loop that halts everything.


  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");


    while (1);
  }


  Serial.print("Accelerometer sample rate = ");
  Serial.print(IMU.accelerationSampleRate());
  Serial.println(" Hz");
  Serial.println();
  Serial.println("Acceleration in g's");
  Serial.println("X\tY\tZ");


  // For RFID
  SPI.begin();      // Init SPI bus
  mfrc522.PCD_Init();   // Init MFRC522
  delay(4);       // Optional delay. Some board do need more time after init to be ready, see Readme
  mfrc522.PCD_DumpVersionToSerial();  // Show details of PCD - MFRC522 Card Reader details
  Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));


  // Motor Driver
  pinMode(motor_pin, OUTPUT)
}


void loop() {
  // Acceleration
  px = x;
  py = y;
  pz = z;
  if (IMU.accelerationAvailable()) {
    IMU.readAcceleration(x, y, z);
  }


  // RFID
  if (mfrc522.PICC_IsNewCardPresent()) {
    start_timer = true;
    // Dump debug info about the card; PICC_HaltA() is automatically called
    mfrc522.PICC_DumpToSerial(&(mfrc522.uid));
  }


  // Timer
  if (millis() - previousMilliseconds > interval) {
    // millis() is constantly increasing, so we have to add interval
    previousMilliseconds += interval;
    if (start_timer) {
      can_open = true;
      counter += 1;
    }
  } else if (counter == 5) {
    can_open = false;
    counter = 0;
    start_timer = false;
  }
 
  opening = (abs(x - px) >= THRESHOLD) || (abs(y - py) >= THRESHOLD)  || (abs(z - pz) >= THRESHOLD);
  if (opening && !can_open) {
        intruder_counter += 1;
  } else if (opening && can_open) {
Serial.println("User opening.");
analogWrite(motor_pin, 255); // You can change speed (range: 0-255)
delay(3000);
Serial.println("User closing.");
analogWrite(motor_pin, 0);
  }
  if (intruder_counter == 10) {
    intruder_counter = 0;
    Serial.println("Intruder!!!!");
  }
