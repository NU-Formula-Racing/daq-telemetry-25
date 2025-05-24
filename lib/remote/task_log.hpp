#ifndef __TASK_LOG_H__
#define __TASK_LOG_H__

#include <define.hpp>
#include <iostream>
#include <tasks.hpp>
#include <resources.hpp>

namespace remote {

class LogTask : public tasks::TaskAction {
   public:
    bool initialize() {
        Resources::instance().logger.initialize();
        return true;
    }

    void run() {
        Resources::instance().logger.log(Resources::drive());
    }

    void end() {}

   private:
};

}  // namespace remote
#endif  // __TASK_LOG_H__
