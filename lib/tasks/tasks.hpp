#ifndef __TASKS_H__
#define __TASKS_H__

#include <stdint.h>

#include <initializer_list>
#include <memory>
#include <vector>

namespace tasks {

enum ESPCore : uint8_t { ESPC_0,
                         ESPC_1 };

struct TaskOptions {
    const char *name;
    uint32_t intervalTime;
    uint8_t priority;
    ESPCore core;
};

class TaskAction {
   public:
    virtual const TaskOptions getOptions() const {
        return (TaskOptions){
            .name = "Invalid",
            .intervalTime = 0,
            .priority = 255,
            .core = ESPCore::ESPC_0};
    }

    virtual bool initialize() {}
    virtual void run() {}
    virtual void end() {}
};

struct TaskDescription {
    TaskOptions options;
    TaskAction action;
};

class TaskScheduler {
   public:
    TaskScheduler() {}

    TaskScheduler(std::initializer_list<std::unique_ptr<TaskDescription>> &tasks) {
        _tasks = tasks;
    }

   private:
    std::vector<std::unique_ptr<TaskDescription>> _tasks;
};

}  // namespace tasks

#endif  // __TASKS_H__