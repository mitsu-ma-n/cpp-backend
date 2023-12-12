#include "model.h"

#include <stdexcept>

namespace model {
using namespace std::literals;

Position operator*(Speed speed, TimeType dt) {
    return Position{speed.ux * dt, speed.uy * dt};
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

Position GameSession::MoveDog(Dog& dog, TimeType dt) noexcept {
    // Направление движения собаки
    Direction dog_direction = dog.GetDirection();
    // Переводим координаты на прямую в направлении движения собаки
    auto NormalizeCoord = [](Direction dog_direction, Position pos) {
        switch (dog_direction) {
            case Direction::NORTH: return -pos.y;
            case Direction::SOUTH: return  pos.y;
            case Direction::WEST:  return -pos.x;
            case Direction::EAST:  return  pos.x;
        }
        // По идее, недостижимое условие
        return DynamicCoord{0.0};
    };

    auto dog_start_pos = dog.GetPosition();
    // Сначала находим конечную позицию в предположении, что туда можно попасть
    auto dog_end_pos = dog.GetPosition()+dog.GetSpeed()*dt;

    //StartEndCoord dog_movement = {dog_start_pos, dog_end_pos};
    Direction dir = dog.GetDirection();
    bool isHorizontalMove = dir == Direction::WEST || dir == Direction::EAST;

    // Проектируем дороги на вектор движения
    std::set<DynamicDimension> maximums;
    for ( auto road : map_->GetRoads() ) {
        auto ox_projection = road.GetOxProjection();
        auto oy_projection = road.GetOyProjection();
        StartEndCoord projection;
        // Проверяем, что вектор движения пересекает дорогу
        if ( isHorizontalMove ) {
            if ( road.IsHorizontal() && !utils::geometry::IsInInterval(dog_start_pos.y, oy_projection) ) {
                continue;   // Дорога вне вектора движения
            }
            if (dir == Direction::WEST) {
                projection = {-ox_projection.first, -ox_projection.second};
            } else {
                projection = {ox_projection.first, ox_projection.second};
            }
            // Запоминаем проекцию на вектор движения
            projection = {std::minmax(projection.first, projection.second)};
        } else {
            if ( road.IsVertical() && !utils::geometry::IsInInterval(dog_start_pos.x, ox_projection) ) {
                continue;   // Дорога вне вектора движения
            }
            if (dir == Direction::NORTH) {
                projection = {-oy_projection.first, -oy_projection.second};
            } else {
                projection = {oy_projection.first, oy_projection.second};
            }
            projection = {std::minmax(projection.first, projection.second)};
        }
        // Дорога пересекает вектор движения. Ищем максимум пересечения в направлении движения
        auto res = utils::geometry::GetMaxMoveOnSegment(NormalizeCoord(dog_direction, dog_start_pos), 
            NormalizeCoord(dog_direction, dog_end_pos), projection.first, projection.second);

        if ( res ) {
            maximums.insert(*res);
        }
    }

    DynamicDimension result = (dir == Direction::WEST || dir == Direction::NORTH) ? -*maximums.rbegin() : *maximums.rbegin();

    // Перед возвратом проверим, что мы не врезались в границу дороги.
    // Если врезались, нужно занулить скорость

    Position res;
    if (isHorizontalMove) {
        res = {result, dog_start_pos.y};
    } else {
        res = {dog_start_pos.x, result};
    }

    if( dog_end_pos != res ) {
        dog.SetSpeed(0.0, std::nullopt);
    }

    return res;
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
        GameSession* new_session = new GameSession(*map);
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
