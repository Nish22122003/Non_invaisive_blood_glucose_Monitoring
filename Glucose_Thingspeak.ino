#include <Wire.h>
#include "MAX30105.h"
#include <WiFi.h>             // For ESP32/ESP8266 WiFi connectivity
#include <ThingSpeak.h>       // For uploading data to ThingSpeak
#include <cmath>

// MAX30105 Sensor object
MAX30105 particleSensor;

// WiFi credentials
const char* ssid = "nishant";          // Replace with your WiFi network name
const char* password = "rishu123";  // Replace with your WiFi network password

// ThingSpeak API setup
unsigned long myChannelNumber = 2260063;  // Replace with your ThingSpeak Channel number
const char* myWriteAPIKey = "G0RAQ9SP9TZPYL7R";     // Replace with your ThingSpeak API write key

WiFiClient client;  // Create a WiFi client object

#define debug Serial // Uncomment this line if you're using an Uno or ESP
//#define debug SerialUSB // Uncomment this line if you're using a SAMD21

void setup()
{
  debug.begin(9600);
  debug.println("MAX30105 Basic Readings with ThingSpeak Upload");

  // Initialize sensor
  if (particleSensor.begin() == false)
  {
    debug.println("MAX30105 was not found. Please check wiring/power.");
    while (1);
  }

  particleSensor.setup(); // Configure sensor. Use 6.4mA for LED drive

  // Connect to WiFi
  connectToWiFi();

  // Initialize ThingSpeak
  ThingSpeak.begin(client);
}

void loop()
{
  // Get sensor readings
  int redValue = particleSensor.getRed();
  int irValue = particleSensor.getIR();
  int greenValue = particleSensor.getGreen();

  // Print the values to debug
  debug.print(" R[");
  debug.print(redValue);
  debug.print("] IR[");
  debug.print(irValue);
  debug.print("] G[");
  debug.print(greenValue);
  debug.print("]");

  // Perform the glucose calculation
  double expValue1 = pow(10, -3);
  double expValue2 = pow(10, -6);
  double coeff = 1.64955159;
  double coeff2 = -3.28468210;
  int glucoseLevel = -76.465709552304 + (irValue * coeff * expValue1) + (redValue * coeff2 * expValue2);

  // Output glucose level to Serial
  Serial.println("Glucose Level: ");
  Serial.print(glucoseLevel);

  // Send data to ThingSpeak
  uploadToThingSpeak(redValue, irValue, greenValue, glucoseLevel);

  // Delay before next loop
  delay(1000);
}

// Function to connect to WiFi
void connectToWiFi()
{
  debug.print("Connecting to ");
  debug.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    debug.print(".");
  }

  debug.println("");
  debug.println("WiFi connected");
  debug.println("IP address: ");
  debug.println(WiFi.localIP());
}

// Function to upload data to ThingSpeak
void uploadToThingSpeak(int redValue, int irValue, int greenValue, int glucoseLevel)
{
  // Set the field values
  ThingSpeak.setField(1, redValue);        // Field 1 for Red
  ThingSpeak.setField(2, irValue);         // Field 2 for IR
  ThingSpeak.setField(3, greenValue);      // Field 3 for Green
  ThingSpeak.setField(4, glucoseLevel);    // Field 4 for Glucose Level

  // Write to ThingSpeak
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

  if (x == 200) {
    debug.println("Data successfully uploaded to ThingSpeak");
  } else {
    debug.print("Problem uploading data. HTTP error code: ");
    debug.println(x);
  }
}
