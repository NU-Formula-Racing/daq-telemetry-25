#include <unity.h>
#include "test.hpp"

void setUp() {}
void tearDown() {}

int main(int argc, char** argv) {
    Tests::instance().runTests();
}