#ifndef __IAPYX_H__
#define __IAPYX_H__

#include "HttpClient.h"
#include "InternetButton.h"
#include "SparkJson.h"

#define DIRECTION_CLOCKWISE 0
#define DIRECTION_COUNTER_CLOCKWISE 1

typedef struct {
    int red;
    int green;
    int blue;
} Color;

const Color PROGRESS_COLOR = {0, 128, 255};
const Color SPINNER_COLOR1 = {128, 128, 0};
const Color SPINNER_COLOR2 = {0, 128, 128};

class Iapyx {
  public:
    Iapyx(String host, int port, String jobName, String params, bool debug);

    // Main "Screens"
    void smile();
    void clock();
    void rainbow();
    void solidColor(uint8_t, uint8_t, uint8_t);
    void homeScreen();
    void setHomeScreen(void (*f)());
    // TODO: LED color change based on time?

    void loop();

  private:
    bool debugMode;

    void (*homeScreenFunc)();

    String host;
    int port;
    String jobName;
    String params;

    InternetButton button;

    HttpClient http;
    http_request_t request;
    http_response_t response;
    http_header_t headers[2] = {{"Accept", "*/*"}, {NULL, NULL}};

    bool confirm();
    void cancel();
    void startJobAndMonitor();

    bool startBuild();
    String getStatusUrl();
    bool getBuildStatus(String url);
    bool isLatestBuildInfo(ArduinoJson::JsonObject &root);

    void progress(uint8_t, long, const Color &, bool);
    void spinner(long, const Color &, bool);
    void playground();

    // TODO: Make these user settable
    void button1Handler();
    void button2Handler();
    void button3Handler();
    void button4Handler();
};

#endif
