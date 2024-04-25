#include "Inkplate.h"
#include "WiFi.h"
#include "driver/rtc_io.h" // Include ESP32 library for RTC pin I/O (needed for rtc_gpio_isolate() function)
#include <rom/rtc.h>       // Include ESP32 library for RTC (needed for rtc_get_reset_reason() function)

Inkplate display(INKPLATE_3BIT);

#define CLEAN_UPDATE_THRESHOLD 50

char *SSID = "<wifi name>";
char *PASSWORD = "<wifi password>";
char *IMAGE_URL = "http://url_to_png/";

RTC_DATA_ATTR bool isCleanBoot = true;

void setup() {
  Serial.begin(115200);
  display.begin();
  isCleanBoot = false;
  display.rtcClearAlarmFlag();

  if (isCleanBoot) {
    display.setDisplayMode(INKPLATE_1BIT);
    display.clearDisplay();
  
    // Setup WiFi
    display.setTextSize(3);
    display.println();
    display.print("  Connecting to WiFi ");
    display.print(SSID);
    display.print("... ");
    display.partialUpdate(false, true);
  }

  if (!connectToWifi()) {
    return;
  }

  setNtpTime();
  displayImage();

  display.rtcSetAlarmEpoch(display.rtcGetEpoch() + 600, RTC_ALARM_MATCH_DHHMMSS);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_39, 0);
  esp_deep_sleep_start();
}

void displayImage() {
  if (isCleanBoot) {
    display.println("");
    display.print("  Loading image... ");
  }
  display.setDisplayMode(INKPLATE_3BIT);
  if(!display.drawPngFromWeb(IMAGE_URL, 0, 0)) {    
    display.setDisplayMode(INKPLATE_1BIT);
    display.println("FAILED TO LOAD IMAGE");
    display.partialUpdate(false, true);
  } else {
    display.display();
  }
}

void setNtpTime() {
  if (display.rtcIsSet()) {
    return;
  }

  time_t val = 0;
  if (isCleanBoot) {
    display.println("");
    display.print("  Getting NTP time...");
    display.partialUpdate(false, true);
  }
  if (display.getNTPEpoch(&val)) {
    display.print("OK... Setting time!");
    display.partialUpdate(false, true);
    display.rtcSetEpoch((uint32_t)val);
  } else if (!display.rtcIsSet()) {
    display.rtcSetEpoch(1589610300);
  }
}

bool connectToWifi() {  
  WiFi.setHostname("InkPlate10");
  if (display.connectWiFi(SSID, PASSWORD, 600, true)) {
    if (isCleanBoot) {
      display.print("Connected!");    
      display.partialUpdate(false, true);
    }
    WiFi.setAutoConnect(true);
    WiFi.setAutoReconnect(true);    
    delay(500);
    return true;
  } else {
    display.print("Failed to connect :(");
    display.partialUpdate();
    return false;
  }
}

void loop() {
  // put your main code here, to run repeatedly:
}
