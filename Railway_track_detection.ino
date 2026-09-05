/*
  ================================================================
        RAILWAY TRACK CRACK DETECTION SYSTEM
        ESP32 BASED PROTOTYPE
  ================================================================

  IMPORTANT DEVELOPMENT NOTE
  ----------------------------------------------------------------
  This program has been developed according to the hardware,
  wiring configuration, project requirements, and detection logic
  defined for this particular prototype.

  BEFORE UPLOADING THIS CODE TO THE ESP32:
  
  1. Re-check all ESP32 GPIO pin assignments.
  2. Re-check the actual sensor output logic (HIGH/LOW).
  3. Re-check the ultrasonic distance threshold.
  4. Re-check the GPS UART pins and baud rate.
  5. Re-check the GSM UART pins and baud rate.
  6. Re-check the GSM module power requirements.
  7. Re-check the GSM phone number and SMS configuration.
  8. Re-check the LCD I2C address.
  9. Re-check the motor-driver input connections.
 10. Re-check motor direction according to the actual wiring.
 11. Re-check all supply voltages and common GND connections.
 12. Re-check that no ESP32 GPIO receives a voltage above 3.3 V.
 13. Re-check the detection conditions and thresholds according
     to the actual experimental setup.
 14. Test each hardware section individually before performing
     the complete system test.

  IMPORTANT:
  ----------------------------------------------------------------
  The parameters used in this program are prototype-specific.
  They should NOT be considered universal railway safety
  parameters.

  This system is an academic/educational prototype and must not
  be connected to or operated on an active railway track.

  Before using a modified version of this program, verify:
  
      HARDWARE
         ↓
      PIN CONFIGURATION
         ↓
      SENSOR LOGIC
         ↓
      THRESHOLDS
         ↓
      COMMUNICATION SETTINGS
         ↓
      MOTOR CONTROL
         ↓
      COMPLETE SYSTEM OPERATION

  Always re-check the complete code after making hardware or
  parameter changes to prevent compilation, wiring, logic, or
  runtime errors.

  ================================================================
*/
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <TinyGPS++.h>

// =========================================================
// LCD
// =========================================================

LiquidCrystal_I2C lcd(0x27, 16, 2);

// =========================================================
// GPS
// =========================================================

TinyGPSPlus gps;

HardwareSerial GPS_Serial(2);

// GPS pins
#define GPS_RX 16
#define GPS_TX 17

// =========================================================
// GSM
// =========================================================

HardwareSerial GSM_Serial(1);

// GSM pins
#define GSM_RX 26
#define GSM_TX 27

// =========================================================
// SENSOR PINS
// =========================================================

#define IR_PIN   34

#define TRIG_PIN 5
#define ECHO_PIN 18

// =========================================================
// MOTOR DRIVER
// =========================================================

#define M1A 25
#define M1B 33

#define M2A 32
#define M2B 23

// =========================================================
// SETTINGS
// =========================================================

#define DISTANCE_LIMIT 10

const char PHONE_NUMBER[] = "+91xxxxxxxxxx";

// =========================================================
// VARIABLES
// =========================================================

int irValue = 0;

float distanceCM = 0;

int heatLevel = 0;

bool heatIncreasing = true;

bool trackIssue = false;

unsigned long lastLCDUpdate = 0;

unsigned long lastSMS = 0;

int lcdPage = 0;

const unsigned long LCD_INTERVAL = 2000;

const unsigned long SMS_INTERVAL = 15000;

// =========================================================
// SETUP
// =========================================================

void setup()
{
  Serial.begin(115200);

  delay(1000);

  Serial.println();
  Serial.println("======================================");
  Serial.println("🚂 Railway Track Detection System");
  Serial.println("======================================");

  // -------------------------------------------------------
  // I2C
  // -------------------------------------------------------

  Wire.begin(21, 22);

  // -------------------------------------------------------
  // LCD
  // -------------------------------------------------------

  lcd.init();
  lcd.backlight();

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Railway Safety");

  lcd.setCursor(0, 1);
  lcd.print("ESP32 Starting");

  // -------------------------------------------------------
  // GPS
  // -------------------------------------------------------

  GPS_Serial.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);

  // -------------------------------------------------------
  // GSM
  // -------------------------------------------------------

  GSM_Serial.begin(9600, SERIAL_8N1, GSM_RX, GSM_TX);

  // -------------------------------------------------------
  // Sensor pins
  // -------------------------------------------------------

  pinMode(IR_PIN, INPUT);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // -------------------------------------------------------
  // Motor pins
  // -------------------------------------------------------

  pinMode(M1A, OUTPUT);
  pinMode(M1B, OUTPUT);

  pinMode(M2A, OUTPUT);
  pinMode(M2B, OUTPUT);

  stopCar();

  delay(2000);

  lcd.clear();

  Serial.println("✅ System initialized");
  Serial.println("📡 GPS UART started");
  Serial.println("📡 GSM UART started");
  Serial.println("📺 LCD started");
  Serial.println("======================================");
}

// =========================================================
// LOOP
// =========================================================

void loop()
{
  // -------------------------------------------------------
  // Read GPS continuously
  // -------------------------------------------------------

  readGPS();

  // -------------------------------------------------------
  // Read IR
  // -------------------------------------------------------

  irValue = digitalRead(IR_PIN);

  // -------------------------------------------------------
  // Heat simulation
  // -------------------------------------------------------

  updateHeat();

  // -------------------------------------------------------
  // Ultrasonic
  // -------------------------------------------------------

  distanceCM = readDistanceCM();

  // -------------------------------------------------------
  // Track condition
  // -------------------------------------------------------

  checkTrackCondition();

  // -------------------------------------------------------
  // LCD
  // -------------------------------------------------------

  updateLCD();

  delay(100);
}

// =========================================================
// GPS FUNCTION
// =========================================================

void readGPS()
{
  while (GPS_Serial.available())
  {
    gps.encode(GPS_Serial.read());
  }
}

// =========================================================
// ULTRASONIC FUNCTION
// =========================================================

float readDistanceCM()
{
  digitalWrite(TRIG_PIN, LOW);

  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);

  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);

  if (duration == 0)
  {
    return -1;
  }

  float distance = duration * 0.0343 / 2.0;

  return distance;
}

// =========================================================
// HEAT SIMULATION
// =========================================================

void updateHeat()
{
  static unsigned long lastHeatUpdate = 0;

  if (millis() - lastHeatUpdate >= 100)
  {
    lastHeatUpdate = millis();

    if (heatIncreasing)
    {
      heatLevel++;

      if (heatLevel >= 100)
      {
        heatLevel = 100;
        heatIncreasing = false;
      }
    }
    else
    {
      heatLevel--;

      if (heatLevel <= 0)
      {
        heatLevel = 0;
        heatIncreasing = true;
      }
    }
  }
}

// =========================================================
// TRACK CONDITION
// =========================================================

void checkTrackCondition()
{
  bool issue = false;

  // -------------------------------------------------------
  // IR detection
  // -------------------------------------------------------

  if (irValue == LOW)
  {
    issue = true;
  }

  // -------------------------------------------------------
  // Ultrasonic detection
  // -------------------------------------------------------

  if (distanceCM > 0 && distanceCM < DISTANCE_LIMIT)
  {
    issue = true;
  }

  // -------------------------------------------------------
  // Track issue
  // -------------------------------------------------------

  if (issue)
  {
    trackIssue = true;

    stopCar();

    Serial.println();
    Serial.println("⚠️ ===============================");
    Serial.println("⚠️ TRACK ISSUE DETECTED!");
    Serial.println("🛑 Vehicle STOPPED");
    Serial.println("⚠️ ===============================");

    sendSMSAlert();
  }
  else
  {
    trackIssue = false;

    moveCar();

    Serial.println();
    Serial.println("✅ TRACK STATUS: SAFE");
    Serial.println("🚗 Vehicle MOVING");
  }

  // -------------------------------------------------------
  // Serial sensor data
  // -------------------------------------------------------

  Serial.print("🔎 IR Sensor: ");
  Serial.println(irValue);

  Serial.print("📏 Ultrasonic: ");

  if (distanceCM < 0)
  {
    Serial.println("No Echo");
  }
  else
  {
    Serial.print(distanceCM, 1);
    Serial.println(" cm");
  }

  Serial.print("🔥 Heat Level: ");
  Serial.print(heatLevel);
  Serial.println(" %");

  if (gps.location.isValid())
  {
    Serial.print("📍 Latitude: ");
    Serial.println(gps.location.lat(), 6);

    Serial.print("📍 Longitude: ");
    Serial.println(gps.location.lng(), 6);
  }
  else
  {
    Serial.println("📍 GPS: Waiting for fix...");
  }
}

// =========================================================
// MOVE CAR
// =========================================================

void moveCar()
{
  digitalWrite(M1A, HIGH);
  digitalWrite(M1B, LOW);

  digitalWrite(M2A, HIGH);
  digitalWrite(M2B, LOW);
}

// =========================================================
// STOP CAR
// =========================================================

void stopCar()
{
  digitalWrite(M1A, LOW);
  digitalWrite(M1B, LOW);

  digitalWrite(M2A, LOW);
  digitalWrite(M2B, LOW);
}

// =========================================================
// LCD UPDATE
// =========================================================

void updateLCD()
{
  if (millis() - lastLCDUpdate < LCD_INTERVAL)
  {
    return;
  }

  lastLCDUpdate = millis();

  lcd.clear();

  switch (lcdPage)
  {
    case 0:

      lcd.setCursor(0, 0);
      lcd.print("IR Sensor:");

      lcd.setCursor(0, 1);

      if (irValue == LOW)
      {
        lcd.print("ISSUE DETECTED");
      }
      else
      {
        lcd.print("Track OK");
      }

      break;

    case 1:

      lcd.setCursor(0, 0);
      lcd.print("Distance:");

      lcd.setCursor(0, 1);

      if (distanceCM < 0)
      {
        lcd.print("No Echo");
      }
      else
      {
        lcd.print(distanceCM, 1);
        lcd.print(" cm");
      }

      break;

    case 2:

      lcd.setCursor(0, 0);
      lcd.print("Heat Level:");

      lcd.setCursor(0, 1);

      lcd.print(heatLevel);
      lcd.print(" %");

      break;

    case 3:

      lcd.setCursor(0, 0);
      lcd.print("Track Status:");

      lcd.setCursor(0, 1);

      if (trackIssue)
      {
        lcd.print("!! ISSUE !!");
      }
      else
      {
        lcd.print("SAFE");
      }

      break;

    case 4:

      lcd.setCursor(0, 0);
      lcd.print("GPS Status:");

      lcd.setCursor(0, 1);

      if (gps.location.isValid())
      {
        lcd.print("FIX OK");
      }
      else
      {
        lcd.print("SEARCHING");
      }

      break;
  }

  lcdPage++;

  if (lcdPage > 4)
  {
    lcdPage = 0;
  }
}

// =========================================================
// GSM SMS ALERT
// =========================================================

void sendSMSAlert()
{
  // Prevent continuous SMS transmission

  if (millis() - lastSMS < SMS_INTERVAL)
  {
    return;
  }

  lastSMS = millis();

  Serial.println("📡 Preparing GSM SMS...");

  GSM_Serial.println("AT");

  delay(500);

  GSM_Serial.println("AT+CMGF=1");

  delay(500);

  GSM_Serial.print("AT+CMGS=\"");

  GSM_Serial.print(PHONE_NUMBER);

  GSM_Serial.println("\"");

  delay(1000);

  GSM_Serial.println("WARNING!");

  GSM_Serial.println("Railway Track Issue Detected.");

  GSM_Serial.print("Distance: ");

  if (distanceCM >= 0)
  {
    GSM_Serial.print(distanceCM, 1);
    GSM_Serial.println(" cm");
  }
  else
  {
    GSM_Serial.println("Unknown");
  }

  GSM_Serial.print("Heat: ");

  GSM_Serial.print(heatLevel);

  GSM_Serial.println("%");

  if (gps.location.isValid())
  {
    GSM_Serial.print("Latitude: ");

    GSM_Serial.println(gps.location.lat(), 6);

    GSM_Serial.print("Longitude: ");

    GSM_Serial.println(gps.location.lng(), 6);
  }
  else
  {
    GSM_Serial.println("GPS location unavailable.");
  }

  GSM_Serial.write(26);

  delay(3000);

  Serial.println("📡 SMS command sent.");
}
