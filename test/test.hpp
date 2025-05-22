#ifndef __TEST_H__
#define __TEST_H__

#include <unity.h>
#include <unity_internals.h>

#include <define.hpp>
#include <functional>
#include <map>

#include "test_debug.hpp"

struct TestData {
    const char* testName;
    const char* testFile;
    void (*testFunc)();
};

class Tests {
   public:
    static Tests& instance() {
        static Tests t;
        return t;
    }

    void addTest(const char* testName, const char* testFile, void (*testFunc)()) {
        _registry[testName] = {.testName = testName, .testFile = testFile, .testFunc = testFunc};
    }

    void runTests() {
        UNITY_BEGIN();

        TEST_DEBUG_PRINT("**** RUNNING %d TESTS ****\n", _registry.size());

        for (auto& test : _registry) {
            // TEST_DEBUG_PRINT("**** RUNNING TEST %s ****\n", test.first);
            UnitySetTestFile(test.second.testFile);
            UnityDefaultTestRun(test.second.testFunc, test.first, __LINE__);
        }

        UNITY_END();
    }

   private:
    Tests() {}
    std::map<const char*, TestData> _registry;
};

#define TEST_FUNC(func)                                        \
    namespace {                                                \
    struct __##func {                                          \
        __##func() {                                           \
            TEST_DEBUG_PRINT("Registering test: %s\n", #func); \
            Tests::instance().addTest(#func, __FILE__, func);  \
        }                                                      \
    };                                                         \
    static __##func __inst_##func;                             \
    }

#endif  // __TEST_H__