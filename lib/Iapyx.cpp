#include "Iapyx.h"

Iapyx::Iapyx(String host, int port, String jobName, String params,
             bool debug = false) {
    this->host = host;
    this->port = port;
    this->jobName = jobName;
    this->params = params;
    this->debugMode = debug;

    Particle.function("remoteCntl", &Iapyx::remoteControl, this);

    button = InternetButton();
    button.begin();

    // Set Your Timezone offset here
    Time.zone(-5);
    randomSeed(Time.now());
}

void Iapyx::loop() {

    if (button.buttonOn(1)) {
        button1Handler();
    } else if (button.buttonOn(2)) {
        button2Handler();
    } else if (button.buttonOn(3)) {
        button3Handler();
    } else if (button.buttonOn(4)) {
        button4Handler();
    }

    homeScreen();
}

int Iapyx::remoteControl(String cmd) {
    if (cmd == "test") {
        button4Handler();
    } else if (cmd == "deploy") {
        rainbow();
        delay(1000);
        // } else if (cmd == "deploy") {
        //     startJobAndMonitor();
    } else if (cmd == "spin") {
        spinner(3000, {0, 200, 0}, true);
    }

    return 0;
}

void Iapyx::button1Handler() {
    // playground();
    clock();
    delay(5000);
};

void Iapyx::button2Handler() {
    for (uint8_t i = 0; i < 25; i++) {
        rainbow();
        delay(150);
    }
};

void Iapyx::button3Handler() {
    if (confirm()) {
        startJobAndMonitor();
    } else {
        cancel();
    }
};

void Iapyx::button4Handler() {

    button.playSong("C5,16,C#5,16,D5,16,D#5,16,E5,16,F5,16,F#5,16,G5,16,G#5,16,"
                    "A5,16,A#5,16,B5,16,\n");

    button.playSong("C5,16,C#5,16,D5,16,D#5,16,E5,16,F5,16,F#5,16,G5,16,G#5,16,"
                    "A5,16,A#5,16,B5,16,\n");

    // button.playSong(
    //     "DS5,16,E5,16,FS5,8,B5,8,DS5,16,E5,16,FS5,16,B5,16,CS6,16,"
    //     "DS6,16,CS6,16,AS5,16,B5,8,FS5,8,DS5,16,E5,16,FS5,8,B5,8,"
    //     "CS6,16,AS5,16,B5,16,CS6,16,E6,16,DS6,16,E6,16,B5,16\n");
};

void Iapyx::setHomeScreen(void (*f)()) {
    this->homeScreenFunc = f;
}

void Iapyx::homeScreen() {
    homeScreenFunc();
}

void Iapyx::solidColor(uint8_t r, uint8_t g, uint8_t b) {
    button.allLedsOn(r, g, b);
}

void Iapyx::rainbow() {
    for (uint8_t i = 1; i <= 11; i++) {
        uint8_t r = random(255);
        uint8_t g = random(255);
        uint8_t b = random(255);

        button.ledOn(i, r, g, b);
    }
}

void Iapyx::smile() {
    button.allLedsOff();

    button.ledOn(1, 128, 128, 0);
    button.ledOn(11, 128, 128, 0);

    button.ledOn(4, 128, 128, 0);
    button.ledOn(5, 128, 128, 0);
    button.ledOn(6, 128, 128, 0);
    button.ledOn(7, 128, 128, 0);
    button.ledOn(8, 128, 128, 0);
}

void Iapyx::clock() {
    uint8_t hour = Time.hour();
    if (hour > 12) {
        hour -= 12;
    }

    uint8_t min = Time.minute();
    min /= 5;

    button.allLedsOff();

    // Hour and Minute use the same LED
    if (hour == min && hour > 0 && hour < 12) {
        button.ledOn(hour, 0, 64, 64);
    } else {
        if (hour > 0 && hour < 12) {
            button.ledOn(hour, 0, 0, 64);
        }
        if (min > 0 && min < 12) {
            button.ledOn(min, 0, 64, 0);
        }
    }
}

// duration - Approximate amount of time to use to march the LEDs from one side
//            to the other
void Iapyx::progress(uint8_t direction, long duration, const Color &c,
                     bool clear = true) {

    long wait = duration / 11;

    if (clear) {
        button.allLedsOff();
    }

    if (direction == DIRECTION_CLOCKWISE) {
        for (int i = 1; i <= 11; i++) {
            button.ledOff(i - 1);
            button.ledOn(i, c.red, c.green, c.blue);
            delay(wait);
        }
        button.ledOff(11);
    } else {
        for (int i = 11; i >= 1; i--) {
            button.ledOff(i + 1);
            button.ledOn(i, c.red, c.green, c.blue);
            delay(wait);
        }
        button.ledOff(0);
    }
}

// duration - Approximate amount of time to use to march the LEDs from one side
//            to the other
void Iapyx::spinner(long duration, const Color &c, bool clear = true) {

    long wait = duration / 11;

    if (clear) {
        button.allLedsOff();
    }

    uint8_t prev_i = 0, prev_j = 6;
    for (uint8_t i = 0; i <= 11; i++) {
        uint8_t j;

        if (i < 6) {
            j = i + 6;
        } else {
            j = (i + 6) - 12;
        }

        button.ledOff(prev_i);
        button.ledOff(prev_j);

        button.ledOn(i, c.red, c.green, c.blue);
        button.ledOn(j, c.red, c.green, c.blue);

        prev_i = i;
        prev_j = j;

        delay(wait);
    }
    button.allLedsOff();
}

bool Iapyx::startBuild() {
    bool buildStarted = false;
    uint8_t retryNum = 0;

    request.hostname = this->host;
    request.port = this->port;

    // request.path = String("/job/") + JOB_NAME +
    //                String("/buildWithParameters?TriggerAction=") +
    //                TRIGGER_ACTION + String("&ProjectName=") + PROJECT +
    //                String("&Repository=") + REPOSITORY + String("&Branch=") +
    //                BRANCH + String("&token=") + TOKEN + String("&cause=") +
    //                CAUSE;
    request.path = String("/job/") + this->jobName +
                   String("/buildWithParameters?") + this->params;

    bool keepTrying = true;
    while (keepTrying) {
        if (debugMode) {
            Serial.print("Start Build: ");
            Serial.print(request.hostname);
            Serial.print(":");
            Serial.print(request.port);
            Serial.println(request.path);
        }

        http.post(request, response, headers);

        if (response.status == 201) {
            keepTrying = false;
            buildStarted = true;
            if (debugMode)
                Serial.println("*** Build Started ***");
        } else {
            if (debugMode) {
                Serial.print("Status: ");
                Serial.println(response.status);
            }

            if (retryNum > 5) {
                keepTrying = false;
                if (debugMode)
                    Serial.println("Retries Maxed Out.");
            } else {
                if (debugMode)
                    Serial.println("Retrying...");
                retryNum++;
                // progress(DIRECTION_CLOCKWISE, 1000, PROGRESS_COLOR);
                spinner(1000, SPINNER_COLOR1);
            }
        }
    }

    return (buildStarted);
}

bool Iapyx::isLatestBuildInfo(ArduinoJson::JsonObject &root) {
    bool isLatest = false;

    bool building = root["building"];
    if (building) {
        isLatest = true;
    }

    return (isLatest);
}

String Iapyx::getStatusUrl() {
    uint8_t retryNum = 0;
    String statusUrl = "";

    request.path = String("/job/") + this->jobName +
                   String("/lastBuild/api/json?tree=id,building,result");

    bool keepTrying = true;
    while (keepTrying) {
        if (debugMode) {
            Serial.print("Last Build: ");
            Serial.print(request.hostname);
            Serial.print(":");
            Serial.print(request.port);
            Serial.println(request.path);
        }

        http.get(request, response, headers);

        if (response.status == 200) {
            // StaticJsonBuffer<200> jsonBuffer;
            DynamicJsonBuffer jsonBuffer;

            char body[response.body.length()];
            response.body.toCharArray(body, response.body.length() + 1);
            JsonObject &root = jsonBuffer.parseObject(body);

            if (!root.success() && debugMode) {
                Serial.println("--------- JSON Parsing Failed ----------");
                Serial.println(body);
                Serial.println("----------------------------------------");
            }

            if (isLatestBuildInfo(root)) {
                const char *id = root["id"];
                statusUrl = String("/job/");
                statusUrl += this->jobName;
                statusUrl += "/";
                statusUrl += String(id);
                statusUrl += "/api/json?tree=building,result";

                if (debugMode) {
                    Serial.print("getStatusUrl: ");
                    Serial.println(statusUrl);
                }

                keepTrying = false;
            } else {
                if (retryNum >= 10) {
                    if (debugMode)
                        Serial.println("---> Retries Maxed Out. <---");
                    keepTrying = false;
                } else {
                    if (debugMode)
                        Serial.println(
                            "Not current build status. Sleeping. Will Retry.");
                    // progress(DIRECTION_CLOCKWISE, 1000, PROGRESS_COLOR);
                    spinner(1000, SPINNER_COLOR1);
                }
                retryNum++;
            }
        } else {
            if (debugMode) {
                Serial.println(
                    "---> Error Requesting Last Build Information <---");
                Serial.print("Status: ");
                Serial.println(response.status);
            }

            keepTrying = false;
        }
    }

    return (statusUrl);
}

bool Iapyx::getBuildStatus(String url) {
    bool buildOK = false;
    uint8_t retryNum = 0;

    request.hostname = this->host;
    request.port = this->port;
    request.path = url;

    bool keepTrying = true;
    while (keepTrying) {
        if (debugMode) {
            Serial.print("Checking Status: ");
            Serial.print(request.hostname);
            Serial.print(":");
            Serial.print(request.port);
            Serial.println(request.path);
        }

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
                // progress(DIRECTION_CLOCKWISE, 1000, PROGRESS_COLOR);
                spinner(1000, SPINNER_COLOR2);
            }
        } else {
            if (retryNum >= 10) {
                if (debugMode)
                    Serial.println("---> Retries Maxed Out. <---");
                keepTrying = false;
            } else {
                if (debugMode) {
                    Serial.print("-- Error Checking build status: ");
                    Serial.println(response.status);
                }
                // progress(DIRECTION_CLOCKWISE, 1000, PROGRESS_COLOR);
                spinner(1000, SPINNER_COLOR1);
            }
            retryNum++;
        }
    }

    return (buildOK);
}

void Iapyx::playground() {
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
    // progress(DIRECTION_CLOCKWISE, 1000, PROGRESS_COLOR, true);
    // delay(750);
    // progress(DIRECTION_CLOCKWISE, 1000, PROGRESS_COLOR, true);
    // delay(750);
    // progress(DIRECTION_CLOCKWISE, 1000, PROGRESS_COLOR, true);
    // delay(750);
    // progress(DIRECTION_CLOCKWISE, 1000, PROGRESS_COLOR, true);
    // delay(750);

    // progress(DIRECTION_CLOCKWISE, 500, PROGRESS_COLOR, true);
    // delay(5000);
    // progress(DIRECTION_COUNTER_CLOCKWISE, 5000, PROGRESS_COLOR, true);
    // Serial.println(Time.now());
    // Serial.println(Time.hour());
    // Serial.println(Time.minute());

    button.allLedsOff();
    // for (int i = 1; i <= 11; i++) {
    //     button.ledOn(i, 64, 0, 0);
    //     delay(1000);
    // }

    button.ledOn(1, 255, 255, 255);
    // spinner(2000, SPINNER_COLOR1, true);
    // delay(1000);
    // spinner(2000, SPINNER_COLOR2, true);

    delay(5000);
}

bool Iapyx::confirm() {
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

void Iapyx::startJobAndMonitor() {
    bool OK = false;

    if (startBuild()) {
        String statusUrl = getStatusUrl();
        if (statusUrl != "") {
            if (debugMode) {
                Serial.print("Status URL: ");
                Serial.println(statusUrl);
            }

            OK = getBuildStatus(statusUrl);
        }
    } else {
        if (debugMode)
            Serial.println("---> Error Starting Build <---");
        OK = false;
    }

    if (OK == true) {
        button.allLedsOn(0, 255, 0);
    } else {
        button.allLedsOn(255, 0, 0);
    }

    delay(10000);
}

void Iapyx::cancel() {
    button.allLedsOff();

    for (int i = 1; i <= 11; i += 2) {
        button.ledOn(i, 255, 0, 0);
    }
    button.playNote("Gb1", 1);
    delay(250);

    button.allLedsOff();

    for (int i = 0; i <= 10; i += 2) {
        button.ledOn(i, 255, 0, 0);
    }
    delay(500);

    button.allLedsOff();

    for (int i = 1; i <= 11; i += 2) {
        button.ledOn(i, 255, 0, 0);
    }
    delay(500);

    // button.allLedsOn(255, 0, 0);

    delay(2000);
}

// ****************************************************************************
// long long char2LL(char *str) {
//   long long result = 0; // Initialize result
//   // Iterate through all characters of input string and update result
//   for (int i = 0; str[i] != '\0'; ++i)
//     result = result * 10 + str[i] - '0';
//   return result;
// }
// ****************************************************************************
