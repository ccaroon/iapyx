#include "InternetButton.h"
#include "HttpClient.h"
#include "SparkJson.h"

const char* HOST_NAME = "192.168.1.84";
const int PORT        = 8080;

const char* JOB_NAME       = "Release";
const char* TRIGGER_ACTION = "Production-Deployment";
const char* PROJECT        = "WASP";

#define DIRECTION_CLOCKWISE 0
#define DIRECTION_COUNTER_CLOCKWISE 1

InternetButton button = InternetButton();
HttpClient http;

http_request_t request;
http_response_t response;
http_header_t headers[] = {
    { "Accept" , "*/*"},
    { NULL, NULL }
};

void setup () {
  Serial.begin(9600);
  button.begin();

  randomSeed(Time.now());
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
  button.allLedsOn(random(256), random(256), random(256));
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

bool startBuild() {
  bool buildStarted = false;
  uint8_t retryNum = 0;

  request.hostname = HOST_NAME;
  request.port = PORT;
  // "/job/Release/buildWithParameters?TriggerAction=Production-Deployment&Project=WASP";
  request.path = String("/job/") + JOB_NAME + String("/buildWithParameters?TriggerAction=") + TRIGGER_ACTION + String("&Project=") + PROJECT;

  bool keepTrying = true;
  while (keepTrying) {
    Serial.print("Start Build: ");
    Serial.print(request.hostname);
    Serial.print(":");
    Serial.print(request.port);
    Serial.println(request.path);

    http.get(request, response, headers);

    if (response.status == 201) {
      keepTrying   = false;
      buildStarted = true;
      Serial.println("*** Build Started ***");
    } else {
      Serial.print("Status: ");
      Serial.println(response.status);

      if (retryNum > 5) {
        keepTrying = false;
        Serial.println("Retries Maxed Out.");
      } else {
        Serial.println("Retrying...");
        retryNum++;
        progress(DIRECTION_CLOCKWISE, 20);
      }
    }
  }

  return(buildStarted);
}

// if (true) {
// 2. GET last build url
//     - if `timestamp` is after `curr_timestamp`
//         - build_url = `url` + "api/json"
//         - goto #3
//     - else
//         - sleep 1
//         - goto #2
String getStatusUrl() {
  uint8_t retryNum = 0;
  String statusUrl = "";

  request.path = String("/job/") + JOB_NAME + String("/lastBuild/api/json");

  bool keepTrying = true;
  while (keepTrying) {
    http.get(request, response, headers);

    if (response.status == 200) {
      // StaticJsonBuffer<200> jsonBuffer;
      DynamicJsonBuffer jsonBuffer;
      char body[response.body.length()];
      response.body.toCharArray(body, response.body.length()+1);
      // Serial.println(body);
      JsonObject& root = jsonBuffer.parseObject(body);

      // const char* tsStr = root["timestamp"];
      bool building = root["building"];
      // long long timestamp = atoi(tsStr);
      // unsigned long timestamp = 0;
      // Serial.print("Jenkins TS (String): ");
      // Serial.println(building);

      // Serial.print("Jenkins TS (long long): ");
      // Serial.println(timestamp);

      // if (timestamp > current_time) {
      if (building == true) {
        // Serial.print("timestamp is good: ");
        // Serial.println(timestamp);

        // const char* url = root["url"];
        // statusUrl = String(url) + "api/json";
        // "/job/Release/lastBuild/api/json"
        const char* id = root["id"];
        statusUrl = String("/job/") + JOB_NAME + String("/") + String(id) + String("/api/json");
        // Serial.print("statusUrl: ");
        // Serial.println(statusUrl);

        keepTrying = false;
      } else {
        if (retryNum >= 10) {
          Serial.println("---> Retries Maxed Out. <---");
          keepTrying = false;
        } else {
          Serial.print("building: "); Serial.println(building);
          Serial.println("Not current build status. Sleeping. Will Retry.");
          // delay(1000);
          progress(DIRECTION_CLOCKWISE, 20);
        }
        retryNum++;
      }
    } else {
      Serial.println("---> Error Requesting Last Build Information <---");

      Serial.print("Status: ");
      Serial.println(response.status);

      keepTrying = false;
    }
  }

    return (statusUrl);
}

bool getBuildStatus(String url) {
  bool buildOK = false;
  uint8_t retryNum = 0;

  request.hostname = HOST_NAME;
  request.port = PORT;
  request.path = url;

  bool keepTrying = true;
  while (keepTrying) {
    Serial.print("Checking Status: ");
    Serial.print(request.hostname);
    Serial.print(":");
    Serial.print(request.port);
    Serial.println(request.path);

    http.get(request, response, headers);

    if (response.status == 200) {
      DynamicJsonBuffer jsonBuffer;
      char body[response.body.length()];
      response.body.toCharArray(body, response.body.length()+1);
      JsonObject& root = jsonBuffer.parseObject(body);

      bool building = root["building"];
      if (building == false) {
        keepTrying = false;
        const char* result = root["result"];
        if (String(result) == "SUCCESS") {
          buildOK = true;
        }
      } else {
        // still building
        progress(DIRECTION_CLOCKWISE, 20);
      }
    } else {
      // bad response code
    }
  }

  return(buildOK);
}

void release() {
  // unsigned long current_time = Time.now();
  bool OK = false;

  if (startBuild()) {
    String statusUrl = getStatusUrl();
    if (statusUrl != "") {
      Serial.print("Status URL: ");
      Serial.println(statusUrl);

      OK = getBuildStatus(statusUrl);
    }
  } else {
    Serial.println("---> Error Starting Build <---");
    OK = false;
  }

  if (OK == true) {
    button.allLedsOn(0,255,0);
  } else {
    button.allLedsOn(255,0,0);
  }

  delay(5000);

  // 3. GET job specific url
  // 4. Check `building`
  //     - if `building` is true
  //         - sleep 1
  //         - goto #3
  //     else
  //         - goto #5
  // 5. Check `result`
  //     - if "SUCCESS"
  //         - light-up green
  //     - else
  //         - light-up red
  //----------------------------------------------------------------------------

  // progress(DIRECTION_CLOCKWISE, 25);
  // progress(DIRECTION_CLOCKWISE, 25);
  //
  // button.allLedsOn(0,255,0);

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

  // Serial.print("char: ");
  // Serial.println(sizeof(unsigned char));
  //
  // Serial.print("int: ");
  // Serial.println(sizeof(unsigned int));
  //
  // Serial.print("long: ");
  // Serial.println(sizeof(unsigned long));
  //
  // Serial.print("long long: ");
  // Serial.println(sizeof(unsigned long long));

  delay(1000);
}

void loop() {

  if (button.buttonOn(3)) {
    button.playNote("Bb5",16);
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

long long char2LL(char *str)
{
  long long result = 0; // Initialize result
  // Iterate through all characters of input string and update result
  for (int i = 0; str[i] != '\0'; ++i)
    result = result*10 + str[i] - '0';
  return result;
}
