#include <DHT.h>
#include <WiFiNINA.h>
#include <ArduinoHttpClient.h>

// Replace with your WiFi credentials
char ssid[] = "Gaurav's iPhone";
char pass[] = "thepassword";

// Replace with your actual IFTTT key and event name
String eventName = "Event_Triggered";
String iftttKey = "jfkszENz0AX-6llYBnApTkmuG3LYciLnGPMnf09w5X9";

char server[] = "maker.ifttt.com";
int port = 80;

WiFiClient wifi;
HttpClient client = HttpClient(wifi, server, port);

#define DHTPIN 3
#define DHTTYPE DHT11
#define SOIL_PIN A0

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
  dht.begin();

  // Connect to WiFi
  Serial.print("Connecting to WiFi...");
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("Connected!");

  Serial.println("Arduino ready.");
}

void loop() {
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  int soil = analogRead(SOIL_PIN);

  if (!isnan(temp) && !isnan(hum)) {
    String data = "TEMP:" + String(temp, 2) +
                  ",HUM:" + String(hum, 2) +
                  ",SOIL:" + String(soil);

    Serial.println(data);

    // Trigger IFTTT if needed
    if (temp > 30 || soil < 300 || hum < 30) {
      triggerIFTTT(temp, hum, soil);
    }
  } else {
    Serial.println("Failed to read DHT sensor!");
  }

  delay(5000);
  
}

void triggerIFTTT(float temp, float hum, int soil) {
  String alertMsg = "";

  if (soil < 300) {
    alertMsg = "Soil is dry!";
  } else if (temp > 30) {
    alertMsg = "Temperature is high!";
  } else if (hum < 30) {
    alertMsg = "Humidity is low!";
  } else {
    alertMsg = "Sensor alert!";
  }

  String url = "/trigger/" + eventName + "/with/key/" + iftttKey;
  String postData = "value1=" + alertMsg +
                    "&value2=Temp: " + String(temp, 2) + "°C" +
                    "&value3=Humidity: " + String(hum, 2) + "%, Soil: " + String(soil);

  Serial.println("Sending IFTTT request...");
  client.post(url, "application/x-www-form-urlencoded", postData);

  int status = client.responseStatusCode();
  String response = client.responseBody();

  Serial.print("Status code: ");
  Serial.println(status);
  Serial.print("Response: ");
  Serial.println(response);
}
