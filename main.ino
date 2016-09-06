#include "Iapyx.h"

// const char *TRIGGER_ACTION = "Production-Deployment";
// const char *PROJECT = "Prod";
// const char *REPOSITORY = "armory,portal,sentinel";
// const char *BRANCH = "production";
// const char *TOKEN = "START";
// const char *CAUSE = "Internet+Button";

// Jenkins Host, Port
// Jenkins Job Name
// Job Query String Params
// Debug?
Iapyx iapyx("jenkins.webassign.net", 80,
            // "Icarus/job/the_button",
            "Daedalus", "TriggerAction=Production-Deployment&ProjectName=Prod&"
                        "Repository=pandora&Branch=production&token=START&"
                        "cause=Internet+Button",
            true);

void setup() { Serial.begin(9600); }

long lastTimeCheck = 0;
void loop() {

    iapyx.loop();

    if (Time.now() - 60 > lastTimeCheck) {
        lastTimeCheck = Time.now();
        iapyx.clock();
    }

    delay(100);
}
