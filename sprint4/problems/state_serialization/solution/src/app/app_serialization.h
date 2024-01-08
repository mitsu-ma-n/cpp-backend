#include <boost/serialization/vector.hpp>

#include "players.h"

namespace serialization {

// PlayersRepr (PlayersRepresentation) - сериализованное представление класса Players
class PlayersRepr {
public:
    PlayersRepr() = default;

    explicit PlayersRepr(const app::Players& players) {

    }
};

}  // namespace serialization
