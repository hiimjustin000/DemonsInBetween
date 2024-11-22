#include "../DemonsInBetween.hpp"

using namespace geode::prelude;

#include <Geode/modify/LevelSelectLayer.hpp>
class $modify(DIBLevelSelectLayer, LevelSelectLayer) {
    struct Fields {
        EventListener<web::WebTask> m_listener1;
        EventListener<web::WebTask> m_listener2;
        EventListener<web::WebTask> m_listener3;
    };

    bool init(int page) {
        if (!LevelSelectLayer::init(page)) return false;

        auto f = m_fields.self();

        auto ladderDemon1 = DemonsInBetween::demonForLevel(14, true);
        if (ladderDemon1.id == 0 || ladderDemon1.difficulty == 0)
            DemonsInBetween::loadDemonForLevel(std::move(f->m_listener1), 1, true, [this](LadderDemon& demon) { demon.id = 14; });

        auto ladderDemon2 = DemonsInBetween::demonForLevel(18, true);
        if (ladderDemon2.id == 0 || ladderDemon2.difficulty == 0)
            DemonsInBetween::loadDemonForLevel(std::move(f->m_listener2), 2, true, [this](LadderDemon& demon) { demon.id = 18; });

        auto ladderDemon3 = DemonsInBetween::demonForLevel(20, true);
        if (ladderDemon3.id == 0 || ladderDemon3.difficulty == 0)
            DemonsInBetween::loadDemonForLevel(std::move(f->m_listener3), 3, true, [this](LadderDemon& demon) { demon.id = 20; });

        return true;
    }
};
