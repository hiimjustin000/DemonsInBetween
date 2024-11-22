#include "../DemonsInBetween.hpp"

using namespace geode::prelude;

#include <Geode/modify/LevelCell.hpp>
class $modify(DIBLevelCell, LevelCell) {
    struct Fields {
        EventListener<web::WebTask> m_listener;
    };

    static void onModify(auto& self) {
        (void)self.setHookPriority("LevelCell::loadFromLevel", -1); // grandpa demon and gddp integration are 0 D:
    }

    void loadFromLevel(GJGameLevel* level) {
        LevelCell::loadFromLevel(level);

        if (!Mod::get()->getSettingValue<bool>("enable-difficulties")) return;

        auto levelID = level->m_levelID.value();
        auto demon = DemonsInBetween::demonForLevel(levelID, false);
        if (demon.id != 0 && demon.difficulty != 0) {
            createUI(demon);
            return;
        }

        DemonsInBetween::loadDemonForLevel(std::move(m_fields->m_listener), levelID, false, [this](LadderDemon& demon) { createUI(demon); });
    }

    void createUI(LadderDemon const& demon) {
        auto difficultyContainer = m_mainLayer->getChildByID("difficulty-container");
        if (!difficultyContainer) difficultyContainer = m_mainLayer->getChildByID("grd-demon-icon-layer");
        if (!difficultyContainer) return;

        auto difficultySprite = static_cast<GJDifficultySprite*>(difficultyContainer->getChildByID("difficulty-sprite"));
        if (!difficultySprite->isVisible()) return; // We're just going to assume it's Grandpa Demon

        auto gddpDifficulty = difficultyContainer->getChildByID("gddp-difficulty");
        if (gddpDifficulty && !Mod::get()->getSettingValue<bool>("gddp-integration-override")) return;
        else if (gddpDifficulty) gddpDifficulty->setVisible(false);

        difficultyContainer->addChild(DemonsInBetween::spriteForDifficulty(difficultySprite,
            demon.difficulty, GJDifficultyName::Short, DemonsInBetween::stateForLevel(m_level)), 3);
        difficultySprite->setOpacity(0);
    }
};
