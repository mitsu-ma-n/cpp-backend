#include <catch2/catch_test_macros.hpp>

#include "../src/model/model.h"

using namespace std::literals;

SCENARIO("Dog") {
    using namespace model;
    GIVEN("a dog") {
        Dog dog{Dog::Id{0}, Dog::Name{"Sharik"}, Position{0.0, 0.0}, Speed{0.0, 0.0}};
    }
}
