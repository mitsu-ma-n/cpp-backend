#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <optional>
#include <set>

#include "tagged.h"
#include "utils.h"

namespace model {

using DynamicDimension = double;    // Расстояние игровых единицах
using DynamicCoord = DynamicDimension;
using StartEndCoord = std::pair<DynamicDimension,DynamicDimension>;

using TimeType = DynamicDimension;  // Время в секундах

struct Position {
    DynamicCoord x, y;
};

struct Speed {
    DynamicDimension ux, uy;
};

Position operator*(Speed speed, TimeType dt);
Position operator+(Position a, Position b);
bool operator==(Position a, Position b);
bool operator!=(Position a, Position b);

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

    const Point& GetStart() const noexcept {
        return start_;
    }

    const Point& GetEnd() const noexcept {
        return end_;
    }

    StartEndCoord GetOxProjection() const noexcept {
        return {std::min(start_.x, end_.x)-d_width, std::max(start_.x, end_.x)+d_width};
    }

    StartEndCoord GetOyProjection() const noexcept {
        return {std::min(start_.y, end_.y)-d_width, std::max(start_.y, end_.y)+d_width};
    }

public:
    // Половина ширины дороги или расстояние, на которое можно отойти от оси дороги
    static constexpr DynamicDimension d_width = 0.4;

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
        , position_{std::move(position)}
        , offset_{std::move(offset)} {
    }

    const Id& GetId() const noexcept {
        return id_;
    }

    const Point& GetPosition() const noexcept {
        return position_;
    }

    const Offset& GetOffset() const noexcept {
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
        : id_{std::move(id)}
        , name_{std::move(name)}
        , dog_speed_{std::nullopt} {
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

    std::optional<DynamicDimension> GetDogSpeed() const noexcept {
        return dog_speed_;
    }

    void SetDogSpeed(DynamicDimension dog_speed) noexcept {
        dog_speed_ = dog_speed;
    }

    void AddOffice(const Office& office);

public:

private:
    using OfficeIdToIndex = std::unordered_map<Office::Id, size_t, util::TaggedHasher<Office::Id>>;

    Id id_;
    std::string name_;
    std::optional<DynamicDimension> dog_speed_;

    Roads roads_;
    Buildings buildings_;

    OfficeIdToIndex warehouse_id_to_index_;
    Offices offices_;
};

enum class Direction {
    NORTH = 'U',
    SOUTH = 'D',
    WEST = 'L',
    EAST = 'R'
};

class Dog {
public:
    using Id = util::Tagged<std::uint32_t, Dog>;
    using Name = util::Tagged<std::string, Dog>;

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

    void SetSpeed(DynamicDimension speed, std::optional<Direction> direction) noexcept {
        if (!direction.has_value()) {
            speed_.ux = speed_.uy = 0.0;
            return;
        }

        direction_ = direction.value();
        switch (direction_) {
            case Direction::NORTH: {
                speed_.ux = 0.0;
                speed_.uy = -speed;
                break;
            }
            case Direction::SOUTH: {
                speed_.ux = 0.0;
                speed_.uy = speed;
                break;
            }
            case Direction::WEST: {
                speed_.ux = -speed;
                speed_.uy = 0.0;
                break;
            }
            case Direction::EAST: {
                speed_.ux = speed;
                speed_.uy = 0.0;
                break;
            }
        }
    }

    std::string GetDirectionAsString() const noexcept {
        return {(char)direction_};
    }

    const Direction& GetDirection() const noexcept {
        return direction_;
    }

private:
    Id id_;
    Name name_;
    Position position_;
    Speed speed_;
    Direction direction_;
};

class GameSession {
public:
    using Dogs = std::vector<Dog*>;

    GameSession(const Map& map) noexcept
        : map_{&map} {
    }

    ~GameSession() {
        for (auto dog : dogs_) {
            delete dog;
        }
    }

    Dog* AddDog(Position pos, const Dog::Name& name);

    const Dogs& GetDogs() const noexcept {
        return dogs_;
    }

    const Map& GetMap() const noexcept {
        return *map_;
    }

    void Tick(TimeType dt) noexcept {
        // Для всех собак сессии
        for (auto dog : dogs_) {
            // Находим новые координаты
            auto pos = MoveDog(*dog, dt);
            // и устанавливаем их
            dog->SetPosition(pos);
        }
    }

    const Dog* FindDog(const Dog::Id& id) const noexcept {
        if (auto it = dog_id_to_index_.find(id); it != dog_id_to_index_.end()) {
            return dogs_.at(it->second);
        }
        return nullptr;
    }

private:
    Position MoveDog(Dog& dog, TimeType dt) noexcept;

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
    using GameSessions = std::vector<GameSession*>;

    Game(DynamicDimension dogSpeed = 1.0) noexcept
        : defuault_dog_speed_{dogSpeed} {
    }

    ~Game() {
        for (auto session : sessions_) {
            delete session;
        }
    }

    void AddMap(const Map& map);
    GameSession* CreateSession(const Map::Id& map_id);

    const Maps& GetMaps() const noexcept {
        return maps_;
    }

    const GameSessions& GetSessions() const noexcept {
        return sessions_;
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
            return sessions_.at(it->second);
        } else {
            // Если не нашли сессию для игрока, пробуем создать новую
            return CreateSession(id);
        }
    }

    const DynamicDimension GetDefaultDogSpeed() const noexcept {
        return defuault_dog_speed_;
    }

    void SetDefaultDogSpeed(DynamicDimension dog_speed) noexcept {
        defuault_dog_speed_ = dog_speed;
    }

private:
    using MapIdHasher = util::TaggedHasher<Map::Id>;
    using MapIdToIndex = std::unordered_map<Map::Id, size_t, MapIdHasher>;

    DynamicDimension defuault_dog_speed_;

    Maps maps_;
    MapIdToIndex map_id_to_map_index_;

    GameSessions sessions_;
    MapIdToIndex map_id_to_session_index_;
};

}  // namespace model
