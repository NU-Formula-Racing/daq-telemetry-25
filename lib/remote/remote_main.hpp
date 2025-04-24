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
    __setupTasks();
}

void loop() {
}

void __setupTasks() {
    Resources::sched().addTask(
        (TaskOptions) {
            .name = "READ_RTC",
            .intervalTime = 1000,
            .priority = 255,
            .core = ESPCore::ESPC_0
        },
        TaskAction::make<RTCTask>()
    );
}

}  // namespace remote

#endif // __REMOTE_MAIN_H__