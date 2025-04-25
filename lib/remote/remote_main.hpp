#ifndef __REMOTE_MAIN_H__
#define __REMOTE_MAIN_H__

#include <resources.hpp>
#include <tasks.hpp>
#include <memory>

// tasks
#include <task_can.hpp>
#include <task_gps.hpp>
#include <task_log.hpp>
#include <task_rtc.hpp>
#include <task_wireless.hpp>

using namespace tasks;

namespace remote {

void __setupTasks();

void setup() {
    
    // begin serial
    Serial.begin(115200);

    __setupTasks();

    Resources::sched().start();
}

void loop() {
    // no-op 
}

void __setupTasks() {
    Resources::sched().addTask(
        (TaskOptions) {
            .name = "READ_CAN",
            .intervalTime = 50,
            .complexity = TaskComplexity::TC_HIGH,
            .priority = TaskPriority::TP_NORMAL,
            .core = ESPCore::ESPC_ANY
        },
        TaskAction::make<CANTask>()
    );

    Resources::sched().addTask(
        (TaskOptions) {
            .name = "READ_GPS",
            .intervalTime = 100,
            .complexity = TaskComplexity::TC_STANDARD,
            .priority = TaskPriority::TP_LOW,
            .core = ESPCore::ESPC_1
        },
        TaskAction::make<GPSTask>()
    );

    Resources::sched().addTask(
        (TaskOptions) {
            .name = "LOG",
            .intervalTime = 100,
            .complexity = TaskComplexity::TC_STANDARD,
            .priority = TaskPriority::TP_HIGH,
            .core = ESPCore::ESPC_1
        },
        TaskAction::make<LogTask>()
    );

    Resources::sched().addTask(
        (TaskOptions) {
            .name = "READ_RTC",
            .intervalTime = 10,
            .complexity = TaskComplexity::TC_STANDARD,
            .priority = TaskPriority::TP_LOW,
            .core = ESPCore::ESPC_1
        },
        TaskAction::make<RTCTask>()
    );

    Resources::sched().addTask(
        (TaskOptions) {
            .name = "WIRLESS",
            .intervalTime = 100,
            .complexity = TaskComplexity::TC_STANDARD,
            .priority = TaskPriority::TP_LOW,
            .core = ESPCore::ESPC_1
        },
        TaskAction::make<RTCTask>()
    );
}

}  // namespace remote

#endif // __REMOTE_MAIN_H__