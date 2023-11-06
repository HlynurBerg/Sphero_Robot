#include <catch2/catch_test_macros.hpp>
#include "comms/communicationhandler.hpp"

TEST_CASE("Test network helper [Big endian]") {

    for (int i = -100; i < 100; i++) {
        auto bytes = int_to_bytes(i, byte_order::BIG);
        CHECK(i == bytes_to_int(bytes, byte_order::BIG));
    }
}

TEST_CASE("Test network helper [Little endian]") {

    for (int i = -100; i < 100; i++) {
        auto bytes = int_to_bytes(i, byte_order::LITTLE);
        CHECK(i == bytes_to_int(bytes, byte_order::LITTLE));
    }
}