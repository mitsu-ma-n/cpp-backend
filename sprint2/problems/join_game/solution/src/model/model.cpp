#include "model.h"

#include <stdexcept>

namespace model {
using namespace std::literals;

void Map::AddOffice(Office office) {
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

Dog* GameSession::AddDog(Point pos, Dog::Name name) {
    const size_t index = dogs_.size();  // Получаем незанятый индекс
    // Здесь должна быть генерация уникального Id собаки. Пока берём просто индекс.
    // Пробуем добавить
    if (auto [it, inserted] = dog_id_to_index_.emplace(index, index); !inserted) {
        throw std::invalid_argument("Dog with id "s + std::to_string(index) + " already exists"s);
    } else {
        // Создаём на основе индекса и имени экземпляр собаки
        Dog* new_dog = new Dog(model::Dog::Id(index),name);
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

void Game::AddMap(Map map) {
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

GameSession* Game::CreateSession(Map::Id map_id) {
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
