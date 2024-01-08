#pragma once

#include "model_geom.h"
#include "tagged.h"
#include "item.h"
#include "serializer.h"

#include <optional>

namespace model {

class Dog {
public:
    using Id = util::Tagged<std::uint32_t, Dog>;
    using Name = util::Tagged<std::string, Dog>;
    using Bag = std::vector<Item>;

    Dog(Id id, Name name, Position position = {0.0, 0.0}, Speed speed = {0.0, 0.0}, Direction direction = Direction::NORTH ) noexcept
        : id_{std::move(id)}
        , name_{std::move(name)}
        , position_{position}
        , speed_{speed}
        , direction_{direction} {
    }

    const Id& GetId() const noexcept {
        return id_;
    }

    const Name& GetName() const noexcept {
        return name_;
    }

    const Position& GetPosition() const noexcept {
        return position_;
    }

    void SetPosition(const Position& pos) noexcept {
        position_ = pos;
    }

    const Speed& GetSpeed() const noexcept {
        return speed_;
    }

    void SetSpeed(DynamicDimension speed, std::optional<Direction> direction) noexcept;

    std::string GetDirectionAsString() const noexcept {
        return {(char)direction_};
    }

    const Direction& GetDirection() const noexcept {
        return direction_;
    }

    double GetWidth() const noexcept {
        return width_;
    }

    const Bag& GetBag() const {
        return bag_;
    }

    size_t GetBagSize() const noexcept {
        return bag_.size();
    }

    void TakeItem(const model::Item& item) {
        bag_.push_back(item);
    }

    void ClearBag() {
        bag_.clear();
    }

    int GetScore() const noexcept {
        return score_;
    }

    void SetScore(int score) noexcept {
        score_ = score;
    }

    void AddScore(int score) noexcept {
        score_ += score;
    }

    void SaveBag() {
        for (auto item : bag_) {
            score_ += item.GetValue();
        }
        ClearBag();
    }

    serializer::SerDog GetSerDog() const
    {
        serializer::SerDog serDog;
        serDog.id = *id_;
        serDog.name = *name_;
        serDog.position = position_;
        serDog.speed = speed_;
        serDog.direction = direction_;
        for (auto item : bag_) {
            serDog.bag.push_back(item.GetSerItem());
        }
        serDog.score = score_;
        return serDog;
    }

private:
    Id id_;
    Name name_;
    Position position_;
    Speed speed_;
    Direction direction_;
    Bag bag_;
    int score_ = 0;

    static constexpr double width_ = 0.6;
};

}  // namespace model
