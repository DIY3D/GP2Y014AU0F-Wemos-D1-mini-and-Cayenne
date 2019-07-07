#include <CayenneMQTTESP8266.h>
#include <DHT.h>
#define CAYENNE_DEBUG
#define CAYENNE_PRINT Serial
#define DHTTYPE DHT11
#define DHTPIN D5 //GPIO 14 or D5, D6 (gpio12) or D7 (gpio13)

DHT dht(DHTPIN, DHTTYPE); //Initialize DHT sensor

// WiFi network info.
char ssid[] = "******";
char wifiPassword[] = "******";

// Cayenne authentication info. This should be obtained from the Cayenne Dashboard.
char username[] = "*************";     //Cayenne MQTT Username
char password[] = "*************"; // Cayenne MQTT PassWord
char clientID[] = "*************";     // Client ID

unsigned long lastMillis = 0;

// GP2Y014AU0F Dust sensor
int measuringPin = A0;
int ledPin = D2;

float voltsMeasured = 0;
float calcVoltage = 0;
float dustDensity = 0;

//RGB
const int red = D6;
const int green = D7;
const int blue = D8;

void setup()
{
  Serial.begin(9600);
  Cayenne.begin(username, password, clientID, ssid, wifiPassword);
  pinMode(ledPin, OUTPUT);
  
  // DHT
  Serial.begin(115200);
  Serial.println("DHTxx test!");
  dht.begin();
  
  //RGB
  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(blue, OUTPUT);

}

void loop()
{
  Cayenne.loop();

  // Wait a few seconds between measurements.
  delay(5000);

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f))
  {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  // float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  // float hic = dht.computeHeatIndex(t, h, false);


//For testing
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" C\t");
  // Serial.print(f);
  // Serial.print(" *F\t");
  // Serial.print("Heat index: ");
  // Serial.print(hic);
  // Serial.print(" *C ");
  // Serial.print(hif);
  // Serial.println(" *F");

  digitalWrite(red, LOW);
  digitalWrite(green, LOW);
  digitalWrite(blue, LOW);

  // Channel 4. 
  if (t < 16)
  {
    digitalWrite(blue, HIGH);
    digitalWrite(green, LOW);
    digitalWrite(red, LOW);
    Cayenne.virtualWrite(4, HIGH, "digital_sensor", "d");
  }
  else 
  {
    digitalWrite(blue, LOW);
    Cayenne.virtualWrite(4, LOW, "digital_sensor", "d");
  }

  // Channel 5. 
  if ((t > 16 && t < 22) && (calcVoltage <= 1.0))
  {
    digitalWrite(green, HIGH); 
    Cayenne.virtualWrite(5, HIGH, "digital_sensor", "d");
  }
    else 
  {
    digitalWrite(green, LOW);
    Cayenne.virtualWrite(5, LOW, "digital_sensor", "d");
  }
   
  // Channel 6.
  if ((t > 25) || (calcVoltage > 1.5))
  {
    digitalWrite(red, HIGH);
    Cayenne.virtualWrite(6, HIGH, "digital_sensor", "d");
  }
    else  
  {
    digitalWrite(red, LOW);
    Cayenne.virtualWrite(6, LOW, "digital_sensor", "d");
  }

  //Publish data every 10 seconds (10000 milliseconds). Change this value to publish at a different interval.
  if (millis() - lastMillis > 5000)
  {
    lastMillis = millis();

    digitalWrite(ledPin, LOW);
    delayMicroseconds(280);

    voltsMeasured = analogRead(measuringPin);

    delayMicroseconds(40);
    digitalWrite(ledPin, HIGH);
    delayMicroseconds(9680);

    Serial.print("Output Voltage: ");
    Serial.print(calcVoltage);
    Serial.print(" V\t");
    Serial.print("Dust density: ");
    Serial.print(dustDensity);
    Serial.print(" mg/m3\r");
    Serial.println();

    calcVoltage = voltsMeasured * (4.7 / 1024.0); //Voltage on 5V pin on Arduino, Voltage Wemos D1 Mini is 4.7 V
    dustDensity = (0.17 * calcVoltage - 0.1);
    Cayenne.virtualWrite(0, calcVoltage, "voltage", "v");
    Cayenne.virtualWrite(1, dustDensity, "pm", "mg");
    Cayenne.virtualWrite(2, h, "rel_hum", "p");
    Cayenne.virtualWrite(3, t, "temp", "c");
    //Cayenne.virtualWrite(4, blue, "digital_sensor", "d");
    //Cayenne.virtualWrite(5, green, "digital_sensor", "d");
    //Cayenne.virtualWrite(6, red, "digital_sensor", "d");

  }
}