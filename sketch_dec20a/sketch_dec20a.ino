#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>



// Replace these with your WiFi and MQTT server details
const char* ssid = "Wi-fi in de auto";
const char* password = "gwly1887";
const char* mqtt_server = "broker.hivemq.com";

WiFiClient espClient;
WiFiClient espClient2;
PubSubClient client(espClient);

// Callback function that is called when a MQTT message is received
void callback(char* topic, byte* payload, unsigned int length) {
  // Print the received message to the serial line
  Serial.print("Message received on topic '");
  Serial.print(topic);
  Serial.print("': ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void setup() {
  // Initialize serial communication
  Serial.begin(115200);

  // Connect to WiFi
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Connected to WiFi!");

  // Connect to MQTT server
  client.setServer(mqtt_server, 1883);
  client.setBufferSize(1024);
  client.setCallback(callback);
  while (!client.connected()) {
    Serial.print("Connecting to MQTT server: ");
    Serial.println(mqtt_server);
    if (client.connect("wemosjoan")) {
      Serial.println("Connected to MQTT server!");
      // Subscribe to the topic
      client.subscribe("topic/topic");
    } else {  
      delay(500);
    }
  }
}

void loop() {
  // Check if we are still connected to the MQTT server
  if (!client.connected()) {
    // Attempt to reconnect if we have lost the connection
    while (!client.connected()) {
      Serial.print("Reconnecting to MQTT server: ");
      Serial.println(mqtt_server);
      if (client.connect("wemosjoan")) {
        Serial.println("Reconnected to MQTT server!");
        client.subscribe("topic/topic");
      } else {  
        delay(500);
      }
    }
  }

  client.loop();
    // Make a request to the local API
  HTTPClient http;
  http.begin(espClient2, "http://jsonplaceholder.typicode.com/posts/1");
  int httpCode = http.GET();
  Serial.print("HTTP status code: ");
  Serial.println(httpCode);


  // Check the HTTP status code
  if (httpCode == 200) {
    // Parse the JSON response
    Serial.print("Free heap memory: ");
    Serial.println(ESP.getFreeHeap());

    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, http.getString());

    if (error) {
      Serial.print("Error parsing JSON: ");
      Serial.println(error.c_str());
      return;
    } else {
      Serial.println("JSON parsed successfully");
    }   

        // Get the data from the JSON object
    JsonObject data = doc.as<JsonObject>();

    // Send the data to the MQTT server

    if (data.isNull()) {
      Serial.println("Error: data is null");
      return;
    } else {
      Serial.println("Data is not null");
    }

    String dataString;
    serializeJson(data, dataString);

    if (dataString.isEmpty()) {
      Serial.println("Error: dataString is empty");
      return;
    }  else {
      Serial.println("Data string is not empty");
    }
    serializeJson(data, dataString);  
    if(!client.publish("topic/topic", dataString.c_str()) ){ //dataString.c_str()
      Serial.println("error publishing topic: " + client.state());
    }

    // Close the HTTP connection
    http.end();
  }

  // Delay for a second
  //delay(1000);
}