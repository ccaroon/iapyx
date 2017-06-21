#include "Iapyx.h"

// const char *TRIGGER_ACTION = "Production-Deployment";
// const char *BRANCH = "production";
// const char *TOKEN = "START";
// const char *CAUSE = "Internet+Button";

// Jenkins Host, Port
// Jenkins Job Name
// Job Query String Params
// Debug?
Iapyx iapyx("jenkins.webassign.net", 80,
            // "Icarus/job/the_button",
            "Pandora",
            "ProductName=pandora&"
            "Branch=production&"
            "TriggerAction=Production-Deployment&"
            "token=START&"
            "cause=Internet+Button",
            true);

void home() {
    // uint8_t r = random(255);
    // uint8_t g = random(255);
    // uint8_t b = random(255);
    // iapyx.solidColor(r, g, b);

    // iapyx.smile();
    // showClock();

    iapyx.solidColor(0, 0, 0);
    // iapyx.twinkle();
}

long lastTimeCheck = 0;
void showClock() {
    if (Time.now() - 60 > lastTimeCheck) {
        lastTimeCheck = Time.now();
        iapyx.clock();
    }
}

void setup() {
    Serial.begin(9600);
    iapyx.setHomeScreen(home);
}

void loop() {
    iapyx.loop();
    delay(100);
}
