#include "model.h"

#include <algorithm>
#include <stdexcept>

namespace model {
using namespace std::literals;

Position operator*(Speed speed, TimeType dt) {
    return Position{speed.ux * dt.count() / 1000, speed.uy * dt.count() / 1000};
}

Position operator+(Position a, Position b) {
    return Position{a.x + b.x, a.y + b.y};
}

bool operator==(Position a, Position b) {
    const DynamicDimension eps = 1.e-14;
    return (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y) < eps;
}

bool operator!=(Position a, Position b) {
    return !(a == b);
}

bool operator==(Speed a, Speed b) {
    const DynamicDimension eps = 1.e-14;
    return (a.ux - b.ux) * (a.ux - b.ux) + (a.uy - b.uy) * (a.uy - b.uy) < eps;
}

bool operator!=(Speed a, Speed b) {
    return !(a == b);
}

bool operator==(Point a, Point b) {
    return a.x == b.x && a.y == b.y;
}

bool operator!=(Point a, Point b) {
    return !(a == b);
}

bool operator<(Point a, Point b) {
    return a.x < b.x;
}

bool operator==(Road a, Road b) {
    return a.GetStart() == b.GetStart() && a.GetEnd() == b.GetEnd();
}

bool operator!=(Road a, Road b) {
    return !(a == b);
}

bool operator<(Road a, Road b) {
    return a.GetStart() < b.GetStart();
}

void Map::AddOffice(const Office& office) {
    if (warehouse_id_to_index_.contains(office.GetId())) {
        throw std::invalid_argument("Duplicate warehouse");
    }

    const size_t index = offices_.size();
    Office& o = offices_.emplace_back(std::move(office));
    try {
        warehouse_id_to_index_.emplace(o.GetId(), index);
    } catch (...) {
        // Удаляем офис из вектора, если не удалось вставить в unordered_map
        offices_.pop_back();
        throw;
    }
}

// Получение произвольной точки на дорогах карты
model::Position Map::GetRandomPointOnMap() const {
    // Выбираем случайную дорогу
//    size_t road_index = utils::my_random::GetRandomIndex(0, roads.size()-1);
// Временно для тестрования берём начальную точку первой дороги
    size_t road_index = 0;
    const model::Road& road = roads_[road_index];

    // Выбираем случайную точку на дороге
    model::Point start = road.GetStart();
    model::Point end = road.GetEnd();

    model::Position res;
    res.x = start.x;
    res.y = start.y;
/*
    if ( road.IsHorizontal() ) {
        res.y = start.y;
        res.x = utils::my_random::GetRandomDouble(start.x, end.x);
    } else {
        res.x = start.x;
        res.y = utils::my_random::GetRandomDouble(start.y, end.y);
    }
*/
    return res;
}


Dog* GameSession::AddDog(Position pos, const Dog::Name& name) {
    const size_t index = dogs_.size();  // Получаем незанятый индекс
    // Здесь должна быть генерация уникального Id собаки. Пока берём просто индекс.
    // Пробуем добавить
    if (auto [it, inserted] = dog_id_to_index_.emplace(index, index); !inserted) {
        throw std::invalid_argument("Dog with id "s + std::to_string(index) + " already exists"s);
    } else {
        // Создаём на основе индекса и имени экземпляр собаки
        Dog* new_dog = new Dog(model::Dog::Id(index), name, pos);
        try {
            dogs_.emplace_back(new_dog);
        } catch (...) {
            dog_id_to_index_.erase(it);
            delete new_dog;
            throw;
        }
    }
    return dogs_[index];
}

Item* GameSession::AddItem(Position pos, Item::Type& type) {
    const size_t index = items_.size();  // Получаем незанятый индекс
    // Пробуем добавить
    if (auto [it, inserted] = item_id_to_index_.emplace(index, index); !inserted) {
        throw std::invalid_argument("Item with id "s + std::to_string(index) + " already exists"s);
    } else {
        // Создаём на основе индекса и имени экземпляр собаки
        Item* new_item = new Item(model::Item::Id(index), type, pos);
        try {
            items_.emplace_back(new_item);
        } catch (...) {
            item_id_to_index_.erase(it);
            delete new_item;
            throw;
        }
    }
    return items_[index];
}

bool isPointOnRoad(Position pos, const Road& road) {
    auto ox = road.GetOxProjection();
    auto oy = road.GetOyProjection();
    return pos.x >= ox.first && pos.x <= ox.second && pos.y >= oy.first && pos.y <= oy.second;
}

struct Move {
    Position pos;
    DynamicCoord path;
};

Move GetMoveOnRoad(Position start_pos, Position end_pos, Road road) {
    StartEndCoord oxMove = {std::min(start_pos.x, end_pos.x), std::max(start_pos.x, end_pos.x)};
    StartEndCoord oyMove = {std::min(start_pos.y, end_pos.y), std::max(start_pos.y, end_pos.y)};

    DynamicCoord start, end;
    StartEndCoord roadProjection;

    bool isHorizontal = oxMove.second-oxMove.first > oyMove.second-oyMove.first;
    // Уходим на прямую
    if (isHorizontal) {  // Перемещение по X
        start = start_pos.x;
        end = end_pos.x;
        roadProjection = road.GetOxProjection();
    } else {    // Перемещение по Y
        start = start_pos.y;
        end = end_pos.y;
        roadProjection = road.GetOyProjection();
    }

    DynamicCoord intersect;
    if ( start < end ) { // Движение вправо
        intersect = std::min(end,roadProjection.second);
    } else {    // Движение влево
        intersect = std::max(end,roadProjection.first);
    }

    DynamicCoord len = std::fabs(intersect - start);

    // Начальная точка уже точно на дороге, значит пересечение проверять не нужно

    if (isHorizontal) {  // Перемещение по X
        return {.pos = {intersect, start_pos.y}, .path = len};
    } else {
        return {.pos = {start_pos.x, intersect}, .path = len};
    }
}


Position GameSession::MoveDog(Dog& dog, TimeType dt) noexcept {
    if( dog.GetSpeed() == Speed{0.0, 0.0} ) {
        return dog.GetPosition();
    }

    // Направление движения собаки
    Direction dog_direction = dog.GetDirection();

    auto dog_start_pos = dog.GetPosition();
    // Сначала находим конечную позицию в предположении, что туда можно попасть
    auto dog_end_pos = dog.GetPosition()+dog.GetSpeed()*dt;

    // Дороги, на которых начало
    std::set<Road> withStartRoads;
    for ( auto road : map_->GetRoads() ) {
        if (isPointOnRoad(dog_start_pos, road)) {
            withStartRoads.insert(road);
        }
    }

    // Дороги, на которых конец
    std::set<Road> withEndRoads;
    for ( auto road : map_->GetRoads() ) {
        if (isPointOnRoad(dog_start_pos, road)) {
            withEndRoads.insert(road);
        }
    }

    // Находим дороги, на которых есть обе точки
    std::set<Road> intersect;
    for ( auto road : withStartRoads ) {
        if ( withEndRoads.find(road) != withEndRoads.end() ) {
            intersect.insert(road);
        }
    }
    
    if (!intersect.empty()) {   // Если есть общие, то вычисляем перемещение на них
        withStartRoads = intersect;
    }

    // Нужно найти максимальное перемещение, которое может сделать собака
    Move res(dog_start_pos, 0.0);
    for ( auto road : withStartRoads ) {
        auto move = GetMoveOnRoad(dog_start_pos, dog_end_pos, road);
        if (move.path > res.path) {
            res = move;
        }
    }

    if( dog_end_pos != res.pos ) {
        dog.SetSpeed(0.0, std::nullopt);
    }

    return res.pos;
}

void Game::AddMap(const Map& map) {
    const size_t index = maps_.size();
    if (auto [it, inserted] = map_id_to_map_index_.emplace(map.GetId(), index); !inserted) {
        throw std::invalid_argument("Map with id "s + *map.GetId() + " already exists"s);
    } else {
        try {
            maps_.emplace_back(std::move(map));
        } catch (...) {
            map_id_to_map_index_.erase(it);
            throw;
        }
    }
}

GameSession* Game::CreateSession(const Map::Id& map_id) {
    //  Найдём карту, к которой хотим подключиться
    auto map = FindMap(map_id);
    if( !map ) {
        throw std::invalid_argument("Map with id "s + *map_id + " isn`t exists"s);
    }

    // Карта есть. Пробуем добавить новую сессию
    const size_t index = sessions_.size();
    if (auto [it, inserted] = map_id_to_session_index_.emplace(map_id, index); !inserted) {
        // Сессия есть для карты с таким Id. Заново создавать нельзя!
        throw std::invalid_argument("Session for map with id "s + *map_id + " already exists"s);
    } else {
        // Создаём новую сессию, привязанную к указанной карте
        GameSession* new_session = new GameSession(*map, &loot_generator_);
        try {
            sessions_.emplace_back(new_session);
        } catch (...) {
            // Не получилось. Откатываем изменения в map_id_to_map_index_
            map_id_to_session_index_.erase(it);
            delete new_session;
            throw;
        }
    }
    return sessions_.back();
}

}  // namespace model
