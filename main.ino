#include "HttpClient.h"
#include "Iapyx.h"
#include "InternetButton.h"
#include "SparkJson.h"

const char *HOST_NAME = "jenkins.webassign.net";
const int PORT = 80;

const char *JOB_NAME = "ReleaseTest";
const char *TRIGGER_ACTION = "Production-Deployment";
const char *PROJECT = "WASP";
const char *VERSION = "1.0.0";
const char *TOKEN = "START";
const char *CAUSE = "Internet+Button";
const Color PROGRESS_COLOR = {0, 128, 255};

#define DIRECTION_CLOCKWISE 0
#define DIRECTION_COUNTER_CLOCKWISE 1

InternetButton button = InternetButton();
HttpClient http;

http_request_t request;
http_response_t response;
http_header_t headers[] = {{"Accept", "*/*"}, {NULL, NULL}};

void setup() {
  Serial.begin(9600);
  button.begin();

  randomSeed(Time.now());
  smile();
}

void smile() {
  button.allLedsOff();

  for (int i = 0; i <= 10; i += 2) {
    button.ledOn(i, 64, 64, 0);
    button.ledOn(i + 1, 64, 64, 64);
  }

  // button.allLedsOn(random(256), random(256), random(256));
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

void progress(uint8_t direction, long duration, const Color &c,
              bool clear = true) {

  long wait = duration / 11;

  if (clear) {
    button.allLedsOff();
  }

  if (direction == DIRECTION_CLOCKWISE) {
    for (int i = 0; i <= 11; i++) {
      button.ledOff(i - 1);
      button.ledOn(i, c.red, c.green, c.blue);
      delay(wait);
    }
  } else {
    for (int i = 11; i >= 0; i--) {
      button.ledOff(i + 1);
      button.ledOn(i, c.red, c.green, c.blue);
      delay(wait);
    }
  }

  // if (direction == DIRECTION_CLOCKWISE) {
  //   for (float i = 0.0; i <= 12.0; i += 0.1) {
  //     button.smoothLedOn(i, 0, 128, 255);
  //     delay(wait);
  //   }
  // } else {
  //   for (float i = 11.0; i > 1.0; i -= 0.2) {
  //     button.smoothLedOn(i, 0, 128, 255);
  //     delay(wait);
  //   }
  // }
}

bool startBuild() {
  bool buildStarted = false;
  uint8_t retryNum = 0;

  request.hostname = HOST_NAME;
  request.port = PORT;
  request.path = String("/job/") + JOB_NAME +
                 String("/buildWithParameters?TriggerAction=") +
                 TRIGGER_ACTION + String("&Project=") + PROJECT +
                 String("&Version=") + VERSION + String("&token=") + TOKEN +
                 String("&cause=") + CAUSE;

  bool keepTrying = true;
  while (keepTrying) {
    Serial.print("Start Build: ");
    Serial.print(request.hostname);
    Serial.print(":");
    Serial.print(request.port);
    Serial.println(request.path);

    http.get(request, response, headers);

    if (response.status == 201) {
      keepTrying = false;
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
        progress(DIRECTION_CLOCKWISE, 1000, PROGRESS_COLOR);
      }
    }
  }

  return (buildStarted);
}

bool isLatestBuildInfo(ArduinoJson::JsonObject &root) {
  bool isLatest = false;
  // const char* result = root["result"];
  //
  // Serial.print("isLatestBuildInfo Result: ");
  // Serial.println(result);
  //
  // if (result == NULL || strcmp(result, "") == 0) {
  //   isLatest = true;
  // }

  bool building = root["building"];
  if (building) {
    isLatest = true;
  }

  return (isLatest);
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
    Serial.print("Last Build: ");
    Serial.print(request.hostname);
    Serial.print(":");
    Serial.print(request.port);
    Serial.println(request.path);
    http.get(request, response, headers);

    if (response.status == 200) {
      // StaticJsonBuffer<200> jsonBuffer;
      DynamicJsonBuffer jsonBuffer;

      Serial.println(response.body);

      char body[response.body.length()];
      response.body.toCharArray(body, response.body.length() + 1);
      Serial.println(body);
      JsonObject &root = jsonBuffer.parseObject(body);

      if (!root.success()) {
        Serial.println("--------- JSON Parsing Failed ----------");
        Serial.println(body);
        Serial.println("----------------------------------------");
      }
      // Serial.println("---------------------------------------");
      // bool building = root["building"];
      // const char* result = root["result"];
      //
      // Serial.println(building);
      // Serial.println(result);
      // Serial.println("---------------------------------------");

      if (isLatestBuildInfo(root)) {
        // const char* url = root["url"];
        // statusUrl = String(url) + "api/json";
        // "/job/Release/lastBuild/api/json"
        const char *id = root["id"];
        statusUrl = String("/job/") + JOB_NAME + String("/") + String(id) +
                    String("/api/json");

        keepTrying = false;
      } else {
        if (retryNum >= 5) {
          Serial.println("---> Retries Maxed Out. <---");
          keepTrying = false;
        } else {
          Serial.println("Not current build status. Sleeping. Will Retry.");
          progress(DIRECTION_CLOCKWISE, 1000, PROGRESS_COLOR);
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
      response.body.toCharArray(body, response.body.length() + 1);
      JsonObject &root = jsonBuffer.parseObject(body);

      bool building = root["building"];
      if (building == false) {
        keepTrying = false;
        const char *result = root["result"];
        if (String(result) == "SUCCESS") {
          buildOK = true;
        }
      } else {
        // still building
        progress(DIRECTION_CLOCKWISE, 1000, PROGRESS_COLOR);
      }
    } else {
      // bad response code
    }
  }

  return (buildOK);
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
    button.allLedsOn(0, 255, 0);
  } else {
    button.allLedsOn(255, 0, 0);
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

  delay(2000);
}

void cancel() {
  button.allLedsOff();

  for (int i = 1; i <= 11; i += 2) {
    button.ledOn(i, 255, 0, 0);
  }

  delay(500);
  button.allLedsOff();

  for (int i = 0; i <= 10; i += 2) {
    button.ledOn(i, 255, 0, 0);
  }

  delay(500);
  button.allLedsOn(255, 0, 0);

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

void debugIt() {
  // DynamicJsonBuffer jsonBuffer;
  // char body[] =
  //     "{ \"buidling\": false, \"success\": true, \"craig\": 12345678 }";
  // Serial.println(body);
  // JsonObject &root = jsonBuffer.parseObject(body);
  //
  // int craig = root["craig"];
  // bool building = root["building"];
  // bool success = root["success"];
  //
  // Serial.print("Building(bool): ");
  // Serial.println(building);
  // Serial.print("Success(bool): ");
  // Serial.println(success);
  // Serial.print("Craig: ");
  // Serial.println(craig);

  // Color c = {random(255), random(255), random(255)};
  progress(DIRECTION_CLOCKWISE, 500, PROGRESS_COLOR, true);
  progress(DIRECTION_CLOCKWISE, 500, PROGRESS_COLOR, true);
  // delay(5000);
  // progress(DIRECTION_COUNTER_CLOCKWISE, 500, PROGRESS_COLOR, false);
}

void loop() {

  if (button.buttonOn(3)) {
    if (confirm()) {
      release();
    } else {
      cancel();
    }
    smile();
  } else if (button.buttonOn(1)) {
    debugIt();
    smile();
  }

  delay(100);
}

long long char2LL(char *str) {
  long long result = 0; // Initialize result
  // Iterate through all characters of input string and update result
  for (int i = 0; str[i] != '\0'; ++i)
    result = result * 10 + str[i] - '0';
  return result;
}
