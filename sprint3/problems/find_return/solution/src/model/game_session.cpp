#include "game_session.h"

#include "collision_detector.h"

#include <set>
#include <unordered_set>

namespace model {
using namespace std::string_literals;

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
        // Создаём на основе индекса и имени экземпляр предмета
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

void GameSession::Tick(TimeType dt) noexcept {
    // Список предметов для обработки коллизий
    std::vector<collision_detector::Item> col_items;
    for (auto item : items_) {
        col_items.push_back({ {item->GetPosition().x, item->GetPosition().y}, item->GetWidth() });
    }

    // Список офисов для обработки коллизий
    std::vector<collision_detector::Rect> col_offices;
    for (auto office : map_->GetOffices()) {
        collision_detector::Rect rect(  office.GetPosition().x, office.GetPosition().y, 
                                        office.GetOffset().dx, office.GetOffset().dy);
        col_offices.push_back(rect);
    }

    // Список сборщиков для обработки коллизий
    std::vector<collision_detector::Gatherer> col_gatherers;

    // Перемещаем собак
    for (auto dog : dogs_) {
        // Запоминаем старые координаты для сбора предметов
        auto old_pos = dog->GetPosition();
        // Находим новые координаты
        auto pos = MoveDog(*dog, dt);
        // и устанавливаем их
        dog->SetPosition(pos);
        // Сохраняем "собирателя" 
        col_gatherers.push_back({ {old_pos.x, old_pos.y}, {pos.x, pos.y}, dog->GetWidth() });
    }

    // Обрабатываем коллизии собак и предметов
    collision_detector::VectorItemGathererProvider item_provider(col_items, col_gatherers);
    auto item_collisions = collision_detector::FindGatherEvents(item_provider);
    // Обрабатываем коллизии собак и баз(офисов)
    collision_detector::VectorOfficeSaveProvider office_provider(col_offices, col_gatherers);
    auto office_collisions = collision_detector::FindOfficeSaveEvents(office_provider);

    std::set<size_t> collected_items_ids;
    // Обрабатываем все коллизи вместе хронологическом порядке вместе
    auto items_it = item_collisions.begin();
    auto offices_it = office_collisions.begin();
    while ( items_it != item_collisions.end() && 
            offices_it != office_collisions.end()) {
        if (items_it->time < offices_it->time) {
            // Выполняем работу с предметом
            auto item = items_.at(items_it->item_id);
            auto dog = dogs_.at(items_it->gatherer_id);
            // Если предмет ещё не собран и  помещается в рюкзак
            if (dog->GetBagSize() < map_->GetBagCapacity() && !collected_items_ids.contains(items_it->item_id)) {
                // Убираем предмет в рюкзак
                dog->TakeItem(*item);
                // Очищаем память. Теперь предмет только в рюкзаке
                delete item;
                items_[items_it->item_id] = nullptr;
                // Запоминаем, что мы собрали предмет
                collected_items_ids.insert(items_it->item_id);
            }
        } else {
            // Выполняем работу с офисом
            auto dog = dogs_.at(offices_it->gatherer_id);
            //dog->SaveOffice(*offices_it);
        }
    }


    // Раздаём предметы собакам
    for (auto collision : item_collisions) {
        auto item = items_.at(collision.item_id);
        auto dog = dogs_.at(collision.gatherer_id);
        // Если предмет ещё не собран и  помещается в рюкзак
        if (dog->GetBagSize() < map_->GetBagCapacity() && !collected_items_ids.contains(collision.item_id)) {
            // Убираем предмет в рюкзак
            dog->TakeItem(*item);
            // Очищаем память. Теперь предмет только в рюкзаке
            delete item;
            items_[collision.item_id] = nullptr;
            // Запоминаем, что мы собрали предмет
            collected_items_ids.insert(collision.item_id);
        }
    }

    // Удаляем собранные предметы
    for (int i = 0; i < items_.size(); ++i) {
        if (items_[i] == nullptr) {
            items_[i] = items_.back();
            items_.pop_back();
            // TODO: Работа со списком Id предметов
        }
    }

    // Генерируем новые предметы при необходимости
    size_t n_items = item_id_to_index_.size();
    size_t n_dogs = dog_id_to_index_.size();
    unsigned n_new_items = loot_generator_->Generate(dt, n_items, n_dogs);
    for (unsigned i = 0; i < n_new_items; ++i) {
        //
        Item::Type rand_type = std::rand() % map_->GetNLootTypes();
        Position pos = map_->GetRandomPointOnMap();
        AddItem(pos, rand_type);
    }
}

const Dog* GameSession::FindDog(const Dog::Id& id) const noexcept {
    if (auto it = dog_id_to_index_.find(id); it != dog_id_to_index_.end()) {
        return dogs_.at(it->second);
    }
    return nullptr;
}

// Вспомогательные функции
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
    std::unordered_set<Road*> withStartRoads;
    for ( auto road : map_->GetRoads() ) {
        if (isPointOnRoad(dog_start_pos, road)) {
            withStartRoads.insert(&road);
        }
    }

    // Дороги, на которых конец
    std::unordered_set<Road*> withEndRoads;
    for ( auto road : map_->GetRoads() ) {
        if (isPointOnRoad(dog_start_pos, road)) {
            withEndRoads.insert(&road);
        }
    }

    // Находим дороги, на которых есть обе точки
    std::unordered_set<Road*> intersect;
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
        auto move = GetMoveOnRoad(dog_start_pos, dog_end_pos, *road);
        if (move.path > res.path) {
            res = move;
        }
    }

    if( dog_end_pos != res.pos ) {
        dog.SetSpeed(0.0, std::nullopt);
    }

    return res.pos;
}

}  // namespace model
