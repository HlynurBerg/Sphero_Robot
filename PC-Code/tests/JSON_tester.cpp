#include "sensors/sensor_processing.hpp"
#include "catch2/catch_test_macros.hpp"
#include <SDL.h>

TEST_CASE("DataReceiver parses valid JSON with all fields correctly", "[DataReceiver]") {
    DataReceiver dataReceiver("10.25.46.49", 6003);
    std::string validJSON = R"({"Battery":{"percentage":75.5},"Distance":1200,"Speed":{"Velocity":{"Y":5.5}}})";

    dataReceiver.ParseData(validJSON);

    REQUIRE(dataReceiver.GetBatteryPercentage() == 75.5);
    REQUIRE(dataReceiver.GetDistanceMm() == 1200);
    REQUIRE(dataReceiver.GetSpeedY() == 5.5);
}


TEST_CASE("DataReceiver handles JSON with missing fields", "[DataReceiver]") {
    DataReceiver dataReceiver("10.25.46.49", 6003);
    std::string jsonMissingFields = R"({"Battery":{"percentage":50.0}})";

    dataReceiver.ParseData(jsonMissingFields);

    REQUIRE(dataReceiver.GetBatteryPercentage() == 50.0);
    REQUIRE(dataReceiver.GetDistanceMm() == 0);
    REQUIRE(dataReceiver.GetSpeedY() == 0);
}


TEST_CASE("DataReceiver handles malformed JSON", "[DataReceiver]") {
    DataReceiver dataReceiver("10.25.46.49", 6003);
    std::string malformedJSON = R"({"Battery":75.5, "Distance": "incorrect"})";

    dataReceiver.ParseData(malformedJSON);

    REQUIRE(dataReceiver.GetBatteryPercentage() == 0);
    REQUIRE(dataReceiver.GetDistanceMm() == 0);
    REQUIRE(dataReceiver.GetSpeedY() == 0);
}

TEST_CASE("DataReceiver handles JSON with unexpected values", "[DataReceiver]") {
    DataReceiver dataReceiver("10.25.46.49", 6003);
    std::string jsonUnexpectedValues = R"({"Battery":{"percentage":-20},"Distance":-500,"Speed":{"Velocity":{"Y":-10}}})";

    dataReceiver.ParseData(jsonUnexpectedValues);

    REQUIRE(dataReceiver.GetBatteryPercentage() == -20);
    REQUIRE(dataReceiver.GetDistanceMm() == -500);
    REQUIRE(dataReceiver.GetSpeedY() == -10);
}

