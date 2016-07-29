#include "InternetButton.h"
#include "HttpClient.h"
#include "SparkJson.h"

#define DIRECTION_CLOCKWISE 0
#define DIRECTION_COUNTER_CLOCKWISE 1
// #define SPARK_CORE (1)

HttpClient http;
InternetButton button = InternetButton();

http_header_t headers[] = {
    //  { "Content-Type", "application/json" },
    // { "Accept" , "application/json" },
    { "Accept" , "*/*"},
    { NULL, NULL } // NOTE: Always terminate headers will NULL
};

void setup () {
  Serial.begin(9600);
  button.begin();
  smile();
}

void smile() {
  button.allLedsOff();
  // button.ledOn(1,  128, 128, 0);
  // button.ledOn(11, 128, 128, 0);
  //
  // button.ledOn(4,  128, 128, 0);
  // button.ledOn(5,  128, 128, 0);
  // button.ledOn(6,  128, 128, 0);
  // button.ledOn(7,  128, 128, 0);
  // button.ledOn(8,  128, 128, 0);
  button.allLedsOn(64,64,64);
}

bool confirm() {
  bool buttonPressed = false;
  bool OK = false;

  button.allLedsOff();
  button.ledOn(3, 255, 0, 0);
  button.ledOn(9, 0, 255, 0);

  while (!buttonPressed) {
    if (button.buttonOn(4)) {
      OK = true;
      buttonPressed = true;
    } else if (button.buttonOn(2)) {
      OK = false;
      buttonPressed = true;
    }
  }

  button.allLedsOff();
  return (OK);
}

void progress(uint8_t direction, long wait) {
  if (direction == DIRECTION_CLOCKWISE) {
    for (float i = 0.0; i <= 12.0; i += 0.1) {
      button.smoothLedOn(i, 0, 128, 255);
      delay(wait);
    }
  } else {
    for (float i = 11.0; i > 1.0; i -= 0.2) {
      button.smoothLedOn(i, 0, 128, 255);
      delay(wait);
    }
  }
}

void release() {
  // button.rainbow(25);
  // startRelease();
  parseSomeJSON();

  progress(DIRECTION_CLOCKWISE, 25);
  progress(DIRECTION_CLOCKWISE, 25);

  button.allLedsOn(0,255,0);

  delay(2000);
}

void cancel() {
  button.allLedsOff();

  for (int i = 1; i <= 11; i+=2) {
    button.ledOn(i, 255, 0, 0);
  }

  delay(500);
  button.allLedsOff();

  for (int i = 0; i <= 10; i+=2) {
    button.ledOn(i, 255, 0, 0);
  }

  delay(500);
  button.allLedsOn(255,0,0);

  delay(1000);
}

void startRelease() {
  http_request_t request;
  http_response_t response;

  request.hostname = "192.168.1.84";
  request.port = 8080;
  request.path = "/job/Release/buildWithParameters?Project=WASP&TriggerAction=Production-Deployment";

  http.get(request, response);

  Serial.print("Status: ");
  Serial.println(response.status);
  Serial.print("Body: ");
  Serial.println(response.body);
  // response.status
  // response.body
}

void parseSomeJSON() {
  Serial.println("--- parseSomeJSON start ---");
  StaticJsonBuffer<200> jsonBuffer;

  Serial.println("set json string");
  char json[] =
      "{\"sensor\":\"gps\",\"time\":1351824120,\"data\":[48.756080,2.302038]}";

  Serial.println("create JsonObject");
  JsonObject& root = jsonBuffer.parseObject(json);

  if (!root.success()) {
    Serial.println("parseObject() failed");
    return;
  }

  Serial.println("access data");
  const char* sensor = root["sensor"];
  long time = root["time"];
  double latitude = root["data"][0];
  double longitude = root["data"][1];

  Serial.println(sensor);
  Serial.println(time);
  Serial.println(latitude, 6);
  Serial.println(longitude, 6);

  Serial.println("--- parseSomeJSON end ---");
}

void loop() {

  if (button.buttonOn(3)) {
    // button.playNote("Bb5",16);
    // button.playSong("C5,16,C#5,16,D5,16,D#5,16,E5,16,F5,16,F#5,16,G5,16,G#5,16,A5,16,A#5,16,B5,16\n");
    if (confirm()) {
      release();
    } else {
      cancel();
    }
    smile();
  }

  delay(100);
}
