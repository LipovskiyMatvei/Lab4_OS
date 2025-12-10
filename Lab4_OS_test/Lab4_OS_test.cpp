
#define CATCH_CONFIG_MAIN 
#include "catch.hpp"
#include "Common.h"


TEST_CASE("Constants and Structures check", "[setup]") {

    REQUIRE(MSG_SIZE == 20);
    REQUIRE(sizeof(QueueHeader) == 16); 
}

TEST_CASE("Offset Calculation", "[math]") {
    int headerSize = sizeof(QueueHeader);

    SECTION("First element offset") {
        REQUIRE(getOffset(0) == headerSize);
    }

    SECTION("Second element offset") {
        REQUIRE(getOffset(1) == headerSize + 20);
    }
}


TEST_CASE("Circular Buffer Logic", "[logic]") {
    int max_size = 5;
    int tail = 4;


    int next = (tail + 1) % max_size;

    REQUIRE(next == 0);
}