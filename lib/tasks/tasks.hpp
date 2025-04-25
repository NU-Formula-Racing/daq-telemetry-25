#ifndef __TASKS_H__
#define __TASKS_H__

#include <freertos/FreeRTOS.h>
#include <stdint.h>

#include <initializer_list>
#include <memory>
#include <vector>

namespace tasks {

enum ESPCore : BaseType_t {
    ESPC_0 = 0,                // run on PRO CPU
    ESPC_1 = 1,                // run on APP CPU
    ESPC_ANY = tskNO_AFFINITY  // let scheduler pick
};

enum TaskComplexity : configSTACK_DEPTH_TYPE {
    // very tiny tasks: interrupts, LED blink, etc.
    TC_VERY_LOW = configMINIMAL_STACK_SIZE / 2,  // ~384 words (~1.5 KiB)

    // simple tasks: sensor reads, stateless handlers
    TC_LOW = configMINIMAL_STACK_SIZE,  // ~768 words (~3 KiB)

    // “normal” tasks: control loops, shallow state machines
    TC_STANDARD = configMINIMAL_STACK_SIZE * 2,  // ~1 536 words (~6 KiB)

    // heavier tasks: logging, moderate buffers, queues
    TC_HIGH = configMINIMAL_STACK_SIZE * 4,  // ~3 072 words (~12 KiB)

    // really heavy: wireless stacks, big static buffers
    TC_VERY_HIGH = configMINIMAL_STACK_SIZE * 8  // ~6 144 words (~24 KiB)
};

enum TaskPriority : UBaseType_t {
    // The idle task runs when no one else needs the CPU.
    TP_IDLE = tskIDLE_PRIORITY,

    // Background/housekeeping: logging, low‐freq sensors, diagnostics
    TP_LOW = TP_IDLE + 1,

    // “Normal” work: control loops, non‐critical state machines
    TP_NORMAL = (configMAX_PRIORITIES - 1) / 2,

    // Important tasks: telemetry, comms stacks, real‐time control
    TP_HIGH = configMAX_PRIORITIES - 2,

    // Critical tasks: ISR wrappers, watchdog kickers, safety routines
    TP_CRITICAL = configMAX_PRIORITIES - 1
};

struct TaskOptions {
    const char* name;
    uint32_t intervalTime;
    TaskComplexity complexity;
    TaskPriority priority;
    ESPCore core;
};

class TaskAction {
   public:
    virtual bool initialize() {}
    virtual void run() {}
    virtual void end() {}

    template <typename T, typename... Args>
    static std::unique_ptr<T> make(Args&&... args) {
        return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }
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
        std::cout << "Adding task: " << options.name << "\n";
        _tasks.emplace_back(options, std::move(task));
    }

    void start() {
        // prevent vector from moving elements once we've got pointers into it
        _tasks.reserve(_tasks.size());

        for (auto& td : _tasks) {
            // pass &td (pointer to the element in the vector)
            std::cout << &td << "\n";

            xTaskCreatePinnedToCore(
                // entry function
                [](void* param) {
                    std::cout << "Beginning task!\n";

                    std::cout << param << "\n";

                    auto* desc = static_cast<TaskDescription*>(param);

                    // one‐shot init
                    if (!desc->action->initialize()) {
                        // initialization failed — delete this task if needed
                        vTaskDelete(nullptr);
                        return;
                    }
                    TickType_t lastWake = xTaskGetTickCount();
                    TickType_t period = pdMS_TO_TICKS(desc->options.intervalTime);
                    for (;;) {
                        desc->action->run();
                        vTaskDelayUntil(&lastWake, period);
                    }

                    // never actually reached, but for completeness:
                    desc->action->end();
                    vTaskDelete(nullptr);
                },
                td.options.name,        // task name
                td.options.complexity,  // stack depth
                &td,                    // pvParameters
                td.options.priority,    // priority
                nullptr,                // returned task handle (unused)
                td.options.core);
        }
    }

   private:
    std::vector<TaskDescription> _tasks;
};

}  // namespace tasks

#endif  // __TASKS_H__