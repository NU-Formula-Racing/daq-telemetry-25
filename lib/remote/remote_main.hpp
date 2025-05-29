#ifndef __REMOTE_MAIN_H__
#define __REMOTE_MAIN_H__

#include <memory>
#include <remote_debug.hpp>
#include <resources.hpp>
#include <tasks.hpp>

// tasks
#include <task_can.hpp>
#include <task_log.hpp>
#include <task_sensors.hpp>
#include <task_wireless.hpp>

// can building
#include <builder/builder.hpp>
#include <sd_token_reader.hpp>

using namespace tasks;
using namespace common;

namespace remote {

static void __setupTasks(can::TelemetryOptions options);
static Result<can::TelemetryOptions> __setupConfig();

void setup() {
    // begin serial
    Serial.begin(115200);

    REMOTE_DEBUG_PRINTLN("DAQ Telemetry Init!");
    Result<can::TelemetryOptions> optRes = __setupConfig();

    if (optRes.isError()) {
        REMOTE_DEBUG_PRINT_ERROR("Unable to log! %s\n", optRes.error().c_str());
        while (true) {
        }
    }

    __setupTasks(optRes.value());

    Resources::sched().start();
}

void loop() {
    // no-op
}

void __setupTasks(can::TelemetryOptions options) {
    REMOTE_DEBUG_PRINTLN("Adding tasks!");
    Resources::sched().addTask((TaskOptions){.name = "READ_CAN",
                                             .intervalTime = 10,
                                             .complexity = TaskComplexity::TC_EXTREME,
                                             .priority = TaskPriority::TP_CRITICAL,
                                             .core = ESPCore::ESPC_0},
                               TaskAction::make<CANTask>());

    // Resources::sched().addTask((TaskOptions){.name = "READ_SENSORS",
    //                                          .intervalTime = 100,
    //                                          .complexity = TaskComplexity::TC_HIGH,
    //                                          .priority = TaskPriority::TP_LOW,
    //                                          .core = ESPCore::ESPC_1},
    //                            TaskAction::make<SensorsTask>());

    Resources::sched().addTask((TaskOptions){.name = "LOG",
                                              .intervalTime = options.logPeriodMs,
                                             .complexity = TaskComplexity::TC_HIGH,
                                             .priority = TaskPriority::TP_HIGH,
                                             .core = ESPCore::ESPC_1},
                               TaskAction::make<LogTask>());

    // Resources::sched().addTask((TaskOptions){.name = "WIRELESS",
    //                                          .intervalTime = options.wirelessPeriodMs,
    //                                          .complexity = TaskComplexity::TC_VERY_HIGH,
    //                                          .priority = TaskPriority::TP_HIGH,
    //                                          .core = ESPCore::ESPC_1},
    //                            TaskAction::make<WirelessTask>());
}

Result<can::TelemetryOptions> __setupConfig() {
    REMOTE_DEBUG_PRINTLN("Setting up configuration!");

    uint32_t start = millis();

    FileGuard gaurd = Resources::file("/config.telem", FILE_READ, false);
    SDTokenReader reader(gaurd);
    can::Tokenizer tokenizer(reader);
    can::TelemBuilder builder(tokenizer);

    REMOTE_DEBUG_PRINTLN("Building...");
    Result<can::TelemetryOptions> telemOptRes = builder.build(Resources::drive());

    if (telemOptRes.isError()) {
        REMOTE_DEBUG_PRINTLN("%s", telemOptRes.error().c_str());
    } else {
        REMOTE_DEBUG_PRINTLN("Successfully configured!");
    }

    uint32_t time = millis() - start;
    REMOTE_DEBUG_PRINTLN("Took %d ms", time);
    Resources::drive().printBus(std::cout);

    Resources::drive().initialize();

    return telemOptRes;
}

}  // namespace remote

#endif  // __REMOTE_MAIN_H__