#define _USE_MATH_DEFINES

#include <cmath>
#include <functional>
#include <sstream>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_templated.hpp>

#include "../src/utils/collision_detector.h"

namespace Catch {
template<>
struct StringMaker<collision_detector::GatheringEvent> {
    static std::string convert(collision_detector::GatheringEvent const& value) {
        std::ostringstream tmp;
        tmp << "(" << value.gatherer_id << value.item_id << value.sq_distance << value.time << ")";

        return tmp.str();
    }
};
}  // namespace Catch

namespace {

template <typename Range, typename Predicate>
struct EqualsRangeMatcher : Catch::Matchers::MatcherGenericBase {
    EqualsRangeMatcher(Range const& range, Predicate predicate)
        : range_{range}
        , predicate_{predicate} {
    }

    template <typename OtherRange>
    bool match(const OtherRange& other) const {
        using std::begin;
        using std::end;

        return std::equal(begin(range_), end(range_), begin(other), end(other), predicate_);
    }

    std::string describe() const override {
        return "Equals: " + Catch::rangeToString(range_);
    }

private:
    const Range& range_;
    Predicate predicate_;
};

template <typename Range, typename Predicate>
auto EqualsRange(const Range& range, Predicate prediate) {
    return EqualsRangeMatcher<Range, Predicate>{range, prediate};
}

}

SCENARIO("Collision detection") {
    WHEN("no items") {
        collision_detector::VectorItemGathererProvider provider{
            {}, {{{1, 2}, {4, 2}, 5.}, {{0, 0}, {10, 10}, 5.}, {{-5, 0}, {10, 5}, 5.}}};
        THEN("No events") {
            auto events = collision_detector::FindGatherEvents(provider);
            CHECK(events.empty());
        }
    }
    WHEN("no gatherers") {
        collision_detector::VectorItemGathererProvider provider{
            {{{1, 2}, 5.}, {{0, 0}, 5.}, {{-5, 0}, 5.}}, {}};
        THEN("No events") {
            auto events = collision_detector::FindGatherEvents(provider);
            CHECK(events.empty());
        }
    }
    WHEN("multiple items on a way of gatherer") {
        collision_detector::VectorItemGathererProvider provider{{
            {{9, 0.27}, .1},
            {{8, 0.24}, .1},
            {{7, 0.21}, .1},
            {{6, 0.18}, .1},
            {{5, 0.15}, .1},
            {{4, 0.12}, .1},
            {{3, 0.09}, .1},
            {{2, 0.06}, .1},
            {{1, 0.03}, .1},
            {{0, 0.0}, .1},
            {{-1, 0}, .1},
            }, {
            {{0, 0}, {10, 0}, 0.1},
        }};
        THEN("Gathered items in right order") {
            auto events = collision_detector::FindGatherEvents(provider);
            CHECK_THAT(
                events,
                EqualsRange(std::vector{
                    collision_detector::GatheringEvent{9, 0,0.*0., 0.0},
                    collision_detector::GatheringEvent{8, 0,0.03*0.03, 0.1},
                    collision_detector::GatheringEvent{7, 0,0.06*0.06, 0.2},
                    collision_detector::GatheringEvent{6, 0,0.09*0.09, 0.3},
                    collision_detector::GatheringEvent{5, 0,0.12*0.12, 0.4},
                    collision_detector::GatheringEvent{4, 0,0.15*0.15, 0.5},
                    collision_detector::GatheringEvent{3, 0,0.18*0.18, 0.6},
                }, collision_detector::CompareEvents()));
        }
    }
    WHEN("multiple gatherers and one item") {
        collision_detector::VectorItemGathererProvider provider{{
                                                {{0, 0}, 0.},
                                            },
                                            {
                                                {{-5, 0}, {5, 0}, 1.},
                                                {{0, 1}, {0, -1}, 1.},
                                                {{-10, 10}, {101, -100}, 0.5}, // <-- that one
                                                {{-100, 100}, {10, -10}, 0.5},
                                            }
        };
        THEN("Item gathered by faster gatherer") {
            auto events = collision_detector::FindGatherEvents(provider);
            CHECK(events.front().gatherer_id == 2);
        }
    }
    WHEN("Gatherers stay put") {
        collision_detector::VectorItemGathererProvider provider{{
                                                {{0, 0}, 10.},
                                            },
                                            {
                                                {{-5, 0}, {-5, 0}, 1.},
                                                {{0, 0}, {0, 0}, 1.},
                                                {{-10, 10}, {-10, 10}, 100}
                                            }
        };
        THEN("No events detected") {
            auto events = collision_detector::FindGatherEvents(provider);

            CHECK(events.empty());
        }
    }
}