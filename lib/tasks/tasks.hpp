#ifndef __TASKS_H__
#define __TASKS_H__

#include <freertos/FreeRTOS.h>
#include <stdint.h>

#include <initializer_list>
#include <memory>
#include <vector>

#include "tasks_debug.hpp"
#include "util_debug.hpp"

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
    TC_VERY_HIGH = configMINIMAL_STACK_SIZE * 8,  // ~6 144 words (~24 KiB)

    // really heavy: wireless stacks, big static buffers
    TC_EXTREME = configMINIMAL_STACK_SIZE * 20  // ~6 144 words (~24 KiB)
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
    virtual bool initialize() { return true; }
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

    TaskDescription(const TaskOptions& opt, std::unique_ptr<TaskAction>&& act) noexcept
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
        if (_hasStarted) {
            TASKS_DEBUG_PRINT_ERRORLN("Cannot add tasks once the scheduler has started!");
            return;
        }

        TASKS_DEBUG_PRINT("Adding task: %s\n", options.name);
        _tasks.emplace_back(options, std::move(task));
    }

    void start() {
        if (_hasStarted) {
            TASKS_DEBUG_PRINT_ERRORLN("Cannot start scheduler again!");
            return;
        }

        _hasStarted = true;

        // prevent vector from moving elements once we've got pointers into it
        _tasks.reserve(_tasks.size());

        for (auto& td : _tasks) {
            xTaskCreatePinnedToCore(
                [](void* param) {
                    auto* desc = static_cast<TaskDescription*>(param);

                    // One-shot init
                    TASKS_DEBUG_PRINT("Starting task %s\n", desc->options.name);
                    if (!desc->action->initialize()) {
                        TASKS_DEBUG_PRINT_ERROR("Failed to init %s\n", desc->options.name);
                        vTaskDelete(nullptr);
                        return;
                    }

                    // Prepare for a periodic loop
                    TickType_t lastWake = xTaskGetTickCount();
                    const TickType_t period = pdMS_TO_TICKS(desc->options.intervalTime);

                    for (;;) {
                        TASKS_DEBUG_PRINT("Running task %s\n", desc->options.name);
                        desc->action->run();
                        UBaseType_t hwm = uxTaskGetStackHighWaterMark(nullptr);
                        UTIL_DEBUG_PRINT("%s task high water (words): %u\n", desc->options.name, hwm);

                        // <- This _blocks_ the task until exactly 'period' has elapsed since
                        // lastWake
                        vTaskDelayUntil(&lastWake, period);
                    }

                    // unreachable
                    desc->action->end();
                    vTaskDelete(nullptr);
                },
                td.options.name,        // name
                td.options.complexity,  // stack depth
                &td,                    // pvParameters
                td.options.priority,    // priority
                nullptr,                // handle
                td.options.core         // core
            );
        }
    }

    /// Block the calling task for `ms` milliseconds.
    static void delayMs(uint32_t ms) { vTaskDelay(pdMS_TO_TICKS(ms)); }

    /// Block the calling task for `ticks` FreeRTOS ticks.
    static void delayTicks(TickType_t ticks) { vTaskDelay(ticks); }

    /// Yield to another ready task of equal priority.
    static void yield() { taskYIELD(); }

    /// Return the current tick count.
    static TickType_t getTickCount() { return xTaskGetTickCount(); }

    /// Globally disable interrupts (careful—no nesting tracking!).
    static void disableInterrupts() { portDISABLE_INTERRUPTS(); }

    /// Globally enable interrupts.
    static void enableInterrupts() { portENABLE_INTERRUPTS(); }

   private:
    std::vector<TaskDescription> _tasks;
    bool _hasStarted = false;
};

}  // namespace tasks

#endif  // __TASKS_H__