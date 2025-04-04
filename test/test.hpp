#ifndef __TEST_H__
#define __TEST_H__

#include <define.hpp>
#include "test_debug.hpp"

#include <unity.h>

#include <functional>
#include <map>

class Tests {
   public:
    static Tests& instance() {
        static Tests t;
        return t;
    }

    void addTest(const char* testName, std::function<void()> testFunc) {
        _registry[testName] = testFunc;
    }

    void runTests() {
        UNITY_BEGIN();

        for (auto &test : _registry){
            TEST_DEBUG_PRINT("**** RUNNING TEST %s ****\n", test.first);
            // test.second();
        }

        UNITY_END();
    }

   private:
    Tests() {}
    std::map<const char*, std::function<void()>> _registry;
};

#define TEST_FUNC(func)                                        \
    namespace {                                                \
    struct __##func {                                          \
        __##func() {                                           \
            TEST_DEBUG_PRINT("Registering test: %s\n", #func); \
            Tests::instance().addTest(#func, func);            \
        }                                                      \
    };                                                         \
    static __##func __inst_##func;                             \
    }


#endif  // __TEST_H__