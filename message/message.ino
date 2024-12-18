#include <WiFi.h>
#include <HTTPClient.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>

// WiFi credentials
const char* ssid = "iPhone";                 // WiFi name
const char* password = "12345678";           // WiFi password

// Backend URL
const char* serverUrl = "http://134.122.89.123/api/vibrations"; // Backend address

// Create an instance of the accelerometer
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);

// Reference acceleration for dB calculation (in m/s^2)
const float reference_acceleration = 0.01;

void setup(void) {
  Serial.begin(115200);
  Serial.println("Accelerometer Test");

  // Connect to WiFi
  connectToWiFi();

  // Initialize the accelerometer
  if (!accel.begin()) {
    Serial.println("Ooops, no ADXL345 detected ... Check your wiring!");
    while (1);
  }

  // Set the range to 16G
  accel.setRange(ADXL345_RANGE_16_G);
}

void loop(void) {
  sensors_event_t event; 
  accel.getEvent(&event);
  
  // Calculate the magnitude of the acceleration vector (in m/s^2)
  float acceleration_magnitude = sqrt(pow(event.acceleration.x, 2) + 
                                      pow(event.acceleration.y, 2) + 
                                      pow(event.acceleration.z, 2));

  // Convert the magnitude to dB
  float acceleration_dB = 20 * log10(abs(acceleration_magnitude) / reference_acceleration);

  // Print to Serial
  Serial.println(acceleration_dB);

  // Send the data to the backend
  sendDataToBackend(acceleration_dB);

  delay(100);  // Delay to avoid flooding the backend
}

// Function to connect to WiFi
void connectToWiFi() {
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

// Function to send data to backend
void sendDataToBackend(float acceleration_dB) {
  if (WiFi.status() == WL_CONNECTED) {  // Check if connected to WiFi
    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");  // Set content type to JSON

    // Create JSON payload
    String payload = "{\"value\": " + String(acceleration_dB) + "}";

    // Send POST request
    int httpResponseCode = http.POST(payload);

    // Print response
    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("Error in sending POST: ");
      Serial.println(http.errorToString(httpResponseCode));
    }

    // Close connection
    http.end();
  } else {
    Serial.println("WiFi not connected!");
  }
}