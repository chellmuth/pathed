#include "string_util.h"

#include "catch.hpp"

TEST_CASE("lTrim", "[string_util]") {
    REQUIRE(lTrim("token") == "token");
    REQUIRE(lTrim("token  ") == "token  ");
    REQUIRE(lTrim("t o  ken") == "t o  ken");
    REQUIRE(lTrim("  token") == "token");
    REQUIRE(lTrim("  token  ") == "token  ");
    REQUIRE(lTrim("  t o  ken") == "t o  ken");
}

