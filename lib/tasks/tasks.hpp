#ifndef __TASKS_H__
#define __TASKS_H__

#include <stdint.h>

#include <initializer_list>
#include <memory>

enum ESPCore : uint8_t { ESPC_0,
                         ESPC_1 };

struct TaskOptions {
    const char *name;
    uint32_t intervalTime;
    uint8_t priority;
    ESPCore core;
};

class Task {
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

template <uint8_t numTasks>
class TaskScheduler {
   public:
    TaskScheduler(std::initializer_list<std::unique_ptr<Task>> tasks) {
        _tasks = tasks;
    }

   private:
    std::array<std::unique_ptr<Task>, numTasks> _tasks;
};

#endif  // __TASKS_H__