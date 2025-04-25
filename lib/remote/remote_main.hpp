#ifndef __REMOTE_MAIN_H__
#define __REMOTE_MAIN_H__

#include <resources.hpp>
#include <tasks.hpp>
#include <memory>

// tasks
#include <task_rtc.hpp>

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
}

void __setupTasks() {
    Resources::sched().addTask(
        (TaskOptions) {
            .name = "READ_RTC",
            .intervalTime = 1000,
            .complexity = TaskComplexity::TC_STANDARD,
            .priority = TaskPriority::TP_LOW,
            .core = ESPCore::ESPC_1
        },
        TaskAction::make<RTCTask>()
    );
}

}  // namespace remote

#endif // __REMOTE_MAIN_H__