#pragma once
#include <string>
#include <unordered_map>
#include <vector>

#include "tagged.h"

namespace model {

using Dimension = int;
using Coord = Dimension;

struct Point {
    Coord x, y;
};

struct Size {
    Dimension width, height;
};

struct Rectangle {
    Point position;
    Size size;
};

struct Offset {
    Dimension dx, dy;
};

class Road {
    struct HorizontalTag {
        explicit HorizontalTag() = default;
    };

    struct VerticalTag {
        explicit VerticalTag() = default;
    };

public:
    constexpr static HorizontalTag HORIZONTAL{};
    constexpr static VerticalTag VERTICAL{};

    Road(HorizontalTag, Point start, Coord end_x) noexcept
        : start_{start}
        , end_{end_x, start.y} {
    }

    Road(VerticalTag, Point start, Coord end_y) noexcept
        : start_{start}
        , end_{start.x, end_y} {
    }

    bool IsHorizontal() const noexcept {
        return start_.y == end_.y;
    }

    bool IsVertical() const noexcept {
        return start_.x == end_.x;
    }

    Point GetStart() const noexcept {
        return start_;
    }

    Point GetEnd() const noexcept {
        return end_;
    }

private:
    Point start_;
    Point end_;
};

class Building {
public:
    explicit Building(Rectangle bounds) noexcept
        : bounds_{bounds} {
    }

    const Rectangle& GetBounds() const noexcept {
        return bounds_;
    }

private:
    Rectangle bounds_;
};

class Office {
public:
    using Id = util::Tagged<std::string, Office>;

    Office(Id id, Point position, Offset offset) noexcept
        : id_{std::move(id)}
        , position_{position}
        , offset_{offset} {
    }

    const Id& GetId() const noexcept {
        return id_;
    }

    Point GetPosition() const noexcept {
        return position_;
    }

    Offset GetOffset() const noexcept {
        return offset_;
    }

private:
    Id id_;
    Point position_;
    Offset offset_;
};

class Map {
public:
    using Id = util::Tagged<std::string, Map>;
    using Roads = std::vector<Road>;
    using Buildings = std::vector<Building>;
    using Offices = std::vector<Office>;

    Map(Id id, std::string name) noexcept
        : id_(std::move(id))
        , name_(std::move(name)) {
    }

    const Id& GetId() const noexcept {
        return id_;
    }

    const std::string& GetName() const noexcept {
        return name_;
    }

    const Buildings& GetBuildings() const noexcept {
        return buildings_;
    }

    const Roads& GetRoads() const noexcept {
        return roads_;
    }

    const Offices& GetOffices() const noexcept {
        return offices_;
    }

    void AddRoad(const Road& road) {
        roads_.emplace_back(road);
    }

    void AddBuilding(const Building& building) {
        buildings_.emplace_back(building);
    }

    void AddOffice(Office office);

private:
    using OfficeIdToIndex = std::unordered_map<Office::Id, size_t, util::TaggedHasher<Office::Id>>;

    Id id_;
    std::string name_;
    Roads roads_;
    Buildings buildings_;

    OfficeIdToIndex warehouse_id_to_index_;
    Offices offices_;
};

class Dog {
public:
    using Id = util::Tagged<std::uint32_t, Dog>;
    using Name = std::string;

    Dog(Id id, Name name) noexcept
        : id_{std::move(id)}
        , name_{std::move(name)} {
    }

    const Id& GetId() const noexcept {
        return id_;
    }

    const Name& GetName() const noexcept {
        return name_;
    }

private:
    Id id_;
    Name name_;
};

class GameSession {
public:
    using Dogs = std::vector<Dog>;

    GameSession(const Map& map) noexcept
        : map_{&map} {
    }

    Dog* AddDog(Point pos, Dog::Name name);

    const Dogs& GetDogs() const noexcept {
        return dogs_;
    }

    const Dog* FindDog(const Dog::Id& id) const noexcept {
        if (auto it = dog_id_to_index_.find(id); it != dog_id_to_index_.end()) {
            return &dogs_.at(it->second);
        }
        return nullptr;
    }

private:
    using DogIdHasher = util::TaggedHasher<Dog::Id>;
    using DogIdToIndex = std::unordered_map<Dog::Id, size_t, DogIdHasher>;

    Dogs dogs_;
    const Map* map_;
    DogIdToIndex dog_id_to_index_;
};

class Game {
public:
    using Maps = std::vector<Map>;

    void AddMap(Map map);
    GameSession* CreateSession(Map::Id map_id);

    const Maps& GetMaps() const noexcept {
        return maps_;
    }

    const Map* FindMap(const Map::Id& id) const noexcept {
        if (auto it = map_id_to_map_index_.find(id); it != map_id_to_map_index_.end()) {
            return &maps_.at(it->second);
        }
        return nullptr;
    }

    // Возвращает указатель на игровую сесссию с возможностью изменения
    GameSession* FindSession(const Map::Id& id) {
        // В данной реализации одна карта (MapId) соответсвует одной сессии
        // При необходимости можно будет переделать, чтобы на одной карте было несколько сессий.
        // Для этого нужно будет добавить критерий "переполнения" сессии и создания новой, например, по количеству 
        // игроков в текущей найденной сессии
        if (auto it = map_id_to_session_index_.find(id); it != map_id_to_session_index_.end()) {
            return &sessions_.at(it->second);
        } else {
            // Если не нашли сессию для игрока, пробуем создать новую
            return CreateSession(id);
        }
    }

private:
    using MapIdHasher = util::TaggedHasher<Map::Id>;
    using MapIdToIndex = std::unordered_map<Map::Id, size_t, MapIdHasher>;

    std::vector<Map> maps_;
    MapIdToIndex map_id_to_map_index_;

    std::vector<GameSession> sessions_;
    MapIdToIndex map_id_to_session_index_;
};

}  // namespace model
