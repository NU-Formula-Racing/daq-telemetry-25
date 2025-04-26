#ifndef __REMOTE_MAIN_H__
#define __REMOTE_MAIN_H__

#include <resources.hpp>
#include <tasks.hpp>
#include <memory>

// tasks
#include <task_can.hpp>
#include <task_log.hpp>
#include <task_sensors.hpp>
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
            .name = "READ_SENSORS",
            .intervalTime = 100,
            .complexity = TaskComplexity::TC_STANDARD,
            .priority = TaskPriority::TP_LOW,
            .core = ESPCore::ESPC_1
        },
        TaskAction::make<SensorsTask>()
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
}

}  // namespace remote

#endif // __REMOTE_MAIN_H__