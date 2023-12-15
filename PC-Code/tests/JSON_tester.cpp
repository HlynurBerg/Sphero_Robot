#include "catch2/catch_test_macros.hpp"
#include "communications/client.hpp"


TEST_CASE("TCPHandler parses valid JSON with all fields correctly", "[TCPHandler]") {
    TCPHandler TCPHandler("10.25.46.49", 6003);
    std::string validJSON = R"({"Battery":{"percentage":75.5},"Distance":1200,"Speed":{"Velocity":{"Y":5.5}}})";

    TCPHandler.ParseData(validJSON);

    REQUIRE(TCPHandler.GetBatteryPercentage() == 75.5);
    REQUIRE(TCPHandler.GetDistanceMm() == 1200);
    REQUIRE(TCPHandler.GetSpeedY() == 5.5);
}

TEST_CASE("TCPHandler handles JSON with missing fields", "[TCPHandler]") {
    TCPHandler TCPHandler("10.25.46.49", 6003);
    std::string jsonMissingFields = R"({"Battery":{"percentage":50.0}})";

    TCPHandler.ParseData(jsonMissingFields);

    REQUIRE(TCPHandler.GetBatteryPercentage() == 50.0);
    REQUIRE(TCPHandler.GetDistanceMm() == 0);
    REQUIRE(TCPHandler.GetSpeedY() == 0);
}

TEST_CASE("TCPHandler handles malformed JSON", "[TCPHandler]") {
    TCPHandler TCPHandler("10.25.46.49", 6003);
    std::string malformedJSON = R"({"Battery":75.5, "Distance": "incorrect"})";

    TCPHandler.ParseData(malformedJSON);

    REQUIRE(TCPHandler.GetBatteryPercentage() == 0);
    REQUIRE(TCPHandler.GetDistanceMm() == 0);
    REQUIRE(TCPHandler.GetSpeedY() == 0);
}

TEST_CASE("TCPHandler handles JSON with unexpected values", "[TCPHandler]") {
    TCPHandler TCPHandler("10.25.46.49", 6003);
    std::string jsonUnexpectedValues = R"({"Battery":{"percentage":-20},"Distance":-500,"Speed":{"Velocity":{"Y":-10}}})";

    TCPHandler.ParseData(jsonUnexpectedValues);

    REQUIRE(TCPHandler.GetBatteryPercentage() == -20);
    REQUIRE(TCPHandler.GetDistanceMm() == -500);
    REQUIRE(TCPHandler.GetSpeedY() == -10);
}
