#include <boost/serialization/vector.hpp>

#include "game.h"

namespace model {

template <typename Archive>
void serialize(Archive& ar, model::Point& point, [[maybe_unused]] const unsigned version) {
    ar & point.x;
    ar & point.y;
}

template <typename Archive>
void serialize(Archive& ar, model::Position& position, [[maybe_unused]] const unsigned version) {
    ar & position.x;
    ar & position.y;    
}

template <typename Archive>
void serialize(Archive& ar, model::Speed& speed, [[maybe_unused]] const unsigned version) {
    ar & speed.ux;
    ar & speed.uy;
}

template <typename Archive>
void serialize(Archive& ar, Item& item, [[maybe_unused]] const unsigned version) {
    ar & (*item.GetId());
    ar & (item.GetType());
    ar & (item.GetPosition());
}

}  // namespace model

namespace serialization {

// DogRepr (DogRepresentation) - сериализованное представление класса Dog
class DogRepr {
public:
    DogRepr() = default;

    explicit DogRepr(const model::Dog& dog)
    : id_{*dog.GetId()}
    , name_{*dog.GetName()}
    , pos_{dog.GetPosition()}
    , speed_{dog.GetSpeed()}
    , direction_{dog.GetDirection()}
    , score_{dog.GetScore()}
    , bag_content_{dog.GetBag()} {
    }

    [[nodiscard]] model::Dog Restore() const {
        model::Dog dog{id_, name_, pos_, speed_, direction_};
        dog.AddScore(score_);
        for (const auto& item : bag_content_) {
            dog.TakeItem(item);
        }
        return dog;
    }

    template <typename Archive>
    void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
        ar &* id_;
        ar & name_;
        ar & pos_;
        ar & bag_capacity_;
        ar & speed_;
        ar & direction_;
        ar & score_;
        ar & bag_content_;
    }

private:
    model::Dog::Id id_ = model::Dog::Id{0u};
    model::Dog::Name name_ = model::Dog::Name{""};
    model::Position pos_;
    size_t bag_capacity_ = 0;
    model::Speed speed_;
    model::Direction direction_ = model::Direction::NORTH;
    model::Dog::Bag bag_content_;
    int score_ = 0;
};

}  // namespace serialization
