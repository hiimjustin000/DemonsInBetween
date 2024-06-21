#include "DemonsInBetween.hpp"

#include <Geode/modify/MenuLayer.hpp>
class $modify(DIBMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;

        if (DemonsInBetween::TRIED_LOADING) return true;
        DemonsInBetween::TRIED_LOADING = true;

        DemonsInBetween::loadGDDL();

        return true;
    }
};

#include <Geode/modify/LevelInfoLayer.hpp>
class $modify(DIBLevelInfoLayer, LevelInfoLayer) {
    static void onModify(auto& self) {
        (void)self.setHookPriority("LevelInfoLayer::init", -50);
    }

    bool init(GJGameLevel* level, bool challenge) {
        if (!LevelInfoLayer::init(level, challenge)) return false;

        if (getChildByID("grd-difficulty") || getChildByID("gddp-difficulty")) return true;

        auto demon = std::find_if(DemonsInBetween::GDDL.begin(), DemonsInBetween::GDDL.end(), [level](auto const& d) {
            return d.id == level->m_levelID.value();
        });
        if (demon == DemonsInBetween::GDDL.end()) return true;

        addChild(DemonsInBetween::spriteForDifficulty(m_difficultySprite, demon->difficulty, GJDifficultyName::Long), 3);
        m_difficultySprite->setOpacity(0);

        return true;
    }
};

#include <Geode/modify/LevelCell.hpp>
class $modify(DIBLevelCell, LevelCell) {
    void loadFromLevel(GJGameLevel* level) {
        LevelCell::loadFromLevel(level);

        auto demon = std::find_if(DemonsInBetween::GDDL.begin(), DemonsInBetween::GDDL.end(), [level](auto const& d) {
            return d.id == level->m_levelID.value();
        });
        if (demon == DemonsInBetween::GDDL.end()) return;

        if (auto difficultyContainer = m_mainLayer->getChildByID("difficulty-container")) {
            if (difficultyContainer->getChildByID("gddp-difficulty")) return;

            auto difficultySprite = static_cast<GJDifficultySprite*>(difficultyContainer->getChildByID("difficulty-sprite"));
            difficultyContainer->addChild(DemonsInBetween::spriteForDifficulty(difficultySprite, demon->difficulty, GJDifficultyName::Short), 3);
            difficultySprite->setOpacity(0);
        }
    }
};
