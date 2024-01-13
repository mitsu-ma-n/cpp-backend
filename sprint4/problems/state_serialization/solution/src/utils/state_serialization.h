#pragma once

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <cstdio>
#include <fstream>

#include <boost/serialization/vector.hpp>
#include <vector>

#include "app.h"
#include "game_session.h"
#include "map.h"
#include "players.h"
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

}  // namespace model

namespace serialization {

// ItemRepr (ItemRepresentation) - сериализованное представление класса Item
class ItemRepr {
public:
    ItemRepr() = default;

    explicit ItemRepr(const model::Item& item)
    : id_{item.GetId()}
    , type_{item.GetType()}
    , position_{item.GetPosition()}
    , value_{item.GetValue()} {
    };

    [[nodiscard]] model::Item Restore() const {
        return model::Item{id_, type_, position_, value_};
    }

    template <typename Archive>
    void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
        ar & *id_;
        ar & type_;
        ar & position_;
        ar & value_;
    }

private:
    model::Item::Id id_{0};
    model::Item::Type type_;
    model::Position position_;
    model::Item::Value value_;
};

// DogRepr (DogRepresentation) - сериализованное представление класса Dog
class DogRepr {
public:
    DogRepr() = default;

    explicit DogRepr(const model::Dog& dog)
    : id_{dog.GetId()}
    , name_{dog.GetName()}
    , pos_{dog.GetPosition()}
    , speed_{dog.GetSpeed()}
    , direction_{dog.GetDirection()}
    , score_{dog.GetScore()} {
        for (const auto& item : dog.GetBag()) {
            bag_content_.push_back(ItemRepr{item});
        }
    }

    [[nodiscard]] model::Dog Restore() const {
        model::Dog dog{id_, name_, pos_, speed_, direction_};
        dog.AddScore(score_);
        for (const auto& item : bag_content_) {
            dog.TakeItem(item.Restore());
        }
        return dog;
    }

    template <typename Archive>
    void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
        ar & *id_;
        ar & *name_;
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
    std::vector<ItemRepr> bag_content_;
    int score_ = 0;
};

// GameSessionRepr (GameSessionRepresentation) - сериализованное представление класса GameSession
class GameSessionRepr {
public:
    GameSessionRepr() = default;

    explicit GameSessionRepr(const model::GameSession& session) {
        for (const auto& dog : session.GetDogs()) {
            dogs_.push_back(DogRepr(*dog));
        }
        /*
        for (const auto& item : session.GetItems()) {
            items_.push_back(*item);
        }
        */
        map_id_ = session.GetMap().GetId();
    }

    // Restore нет, потому что для создания объекта игровой сессии требуется
    // вызывать функции игры
    const model::Map::Id& GetMapId() const {
        return map_id_;
    }

    const std::vector<DogRepr>& GetDogs() const {
        return dogs_;
    }

    template <typename Archive>
    void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
        ar & dogs_;
        //ar & items_;
        ar & *map_id_;
    }

private:
    std::vector<DogRepr> dogs_;
    std::vector<model::Item> items_;
    model::Map::Id map_id_ = model::Map::Id{""};
};

// GameRepr (GameRepresentation) - сериализованное представление класса Game
class GameRepr {
public:
    GameRepr() = default;

    explicit GameRepr(const model::Game& game) {
        for (const auto& session : game.GetSessions()) {
            sessions_.push_back(GameSessionRepr(*session));
        }
    }

    template <typename Archive>
    void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
        ar & sessions_;
    }

private:
    std::vector<GameSessionRepr> sessions_;
};


class StateSerializer2 {
public:
    StateSerializer2(model::Game& game, app::Application& app)
    : game_(&game)
    , app_(&app)
    {}

    void Serialize(const std::filesystem::path& path) {
        auto tmp_file = std::string(path) + ".tmp";
        std::ofstream out{tmp_file, std::ios_base::binary};
        //boost::archive::binary_oarchive ar{out};
        boost::archive::text_oarchive ar{out};

        // Сначала сохраняем игру
        ar << game_->GetSessions().size();  // Надо сохранить количество сессий
        for ( const auto& session : game_->GetSessions() ) {
            ar << GameSessionRepr(*session);
        }

        // Сохраняем данные игроков
        for ( const auto& player : app_->GetPlayers() ) {
            
        }


        //ar << "Some data";
        //std::cout << "StateSerializer2::Serialize" << std::endl;

        std::rename(tmp_file.c_str(), path.c_str());
    }

    void Deserialize(const std::filesystem::path& path) {
        std::ifstream in{std::string(path), std::ios_base::binary};
        //boost::archive::binary_oarchive ar{out};
        boost::archive::text_iarchive ar{in};

        // Сначала загружаем игру
        size_t sessions_size;
        ar >> sessions_size;  // Надо загрузить количество сессий
        for ( size_t i = 0; i < sessions_size; ++i ) {
            // Создаём заготовку под представление сессии
            GameSessionRepr session_repr;
            ar >> session_repr;
            auto session = game_->CreateSession(session_repr.GetMapId());
            for ( const auto& dog_repr : session_repr.GetDogs() ) {
                session->AddDog(dog_repr.Restore());
            }
        }


        // ar << *game_.;
        // ar << *app_;

        //ar << "Some data";
        //std::cout << "StateSerializer2::Serialize" << std::endl;
    }

public:
    model::Game* game_;
    app::Application* app_;
};

}  // namespace serialization
