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
    const char* name;
    uint32_t intervalTime;
    uint8_t priority;
    ESPCore core;
};

class TaskAction {
   public:
    virtual bool initialize() {}
    virtual void run() {}
    virtual void end() {}
};

struct TaskDescription {
    TaskOptions options;
    std::unique_ptr<TaskAction> action;

    TaskDescription(const TaskOptions& opt,
                    std::unique_ptr<TaskAction>&& act) noexcept
        : options(opt), action(std::move(act)) {}

    // non-copyable, but movable
    TaskDescription(const TaskDescription&) = delete;
    TaskDescription& operator=(const TaskDescription&) = delete;
    TaskDescription(TaskDescription&&) noexcept = default;
    TaskDescription& operator=(TaskDescription&&) noexcept = default;
};

class TaskScheduler {
   public:
    TaskScheduler() {}

    void addTask(const TaskOptions& options, std::unique_ptr<TaskAction> task) {
        _tasks.emplace_back(options, std::move(task));
    }

   private:
    std::vector<TaskDescription> _tasks;
};

}  // namespace tasks

#endif  // __TASKS_H__