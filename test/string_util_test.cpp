#include "string_util.h"

#include <string>
#include <queue>
#include "catch.hpp"

using string = std::string;

TEST_CASE("lTrim", "[string_util]") {
    REQUIRE(lTrim("token") == "token");
    REQUIRE(lTrim("token  ") == "token  ");
    REQUIRE(lTrim("t o  ken") == "t o  ken");
    REQUIRE(lTrim("  token") == "token");
    REQUIRE(lTrim("  token  ") == "token  ");
    REQUIRE(lTrim("  t o  ken") == "t o  ken");

    REQUIRE(lTrim(" ") == "");
    REQUIRE(lTrim("     ") == "");
}

TEST_CASE("basic tokenize", "[string_util]") {
    std::queue<string> expected({ "a", "b", "c" });

    REQUIRE(tokenize("a b c") == expected);
    REQUIRE(tokenize(" a b c") == expected);
    REQUIRE(tokenize("  a b c") == expected);
    REQUIRE(tokenize("a b c ") == expected);
    REQUIRE(tokenize("  a  b      c    ") == expected);
}

TEST_CASE("tokenize empty", "[string_util]") {
    std::queue<string> expected;

    REQUIRE(tokenize("") == expected);
    REQUIRE(tokenize(" ") == expected);
    REQUIRE(tokenize(" \t ") == expected);
}
