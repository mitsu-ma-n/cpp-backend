#pragma once

#include "model_geom.h"
#include "tagged.h"

#include <string>

namespace model {

class Item {
public:
    using Id = util::Tagged<std::uint32_t, Item>;
    using Type = int;

    Item(Id id, Type type, Position position) noexcept
    : id_{std::move(id)} 
    , type_{type}
    , position_{position} {
    }

    const Id& GetId() const noexcept {
        return id_;
    }

    const Type& GetType() const noexcept {
        return type_;
    }

    const Position& GetPosition() const noexcept {
        return position_;
    }

    const std::string GetIdAsString() const noexcept {
        return std::to_string(*id_);
    }

    double GetWidth() const noexcept {
        return width_;
    }


private:
    Id id_;
    Type type_;
    Position position_;

    static constexpr double width_ = 0.0;
};

template <typename T> struct ItemInBag {
    T const& ref;
    ItemInBag(T const& ref) : ref(ref) {}
};

}  // namespace model
