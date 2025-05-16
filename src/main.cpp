#include <Arduino.h>

#if defined(APP_REMOTE)
#include <remote_main.hpp>
#define APP remote
#elif defined(APP_BASE)
#include <base_main.hpp>
#define APP base
#else
#define APP unknown
#endif

void setup() {
    APP::setup();
}

void loop() {
    APP::loop();
}
