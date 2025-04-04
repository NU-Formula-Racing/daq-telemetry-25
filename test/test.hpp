#ifndef __TEST_H__
#define __TEST_H__

#include <unity.h>

#include <define.hpp>
#include <functional>
#include <map>

#include "test_debug.hpp"

class Tests {
   public:
    static Tests& instance() {
        static Tests t;
        return t;
    }

    void addTest(const char* testName, void (*testFunc)()) {
        _registry[testName] = testFunc;
    }

    void runTests() {
        UNITY_BEGIN();

        for (auto& test : _registry) {
            TEST_DEBUG_PRINT("**** RUNNING TEST %s ****\n", test.first);
            UnityDefaultTestRun(test.second, test.first, __LINE__);
        }

        UNITY_END();
    }

   private:
    Tests() {}
    std::map<const char*, void (*)()> _registry;
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