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

        if (level->m_stars.value() < 10 || !Mod::get()->getSettingValue<bool>("enable-difficulties")) return;

        auto difficultyContainer = m_mainLayer->getChildByID("difficulty-container");
        if (!difficultyContainer) difficultyContainer = m_mainLayer->getChildByID("grd-demon-icon-layer");
        if (!difficultyContainer) return;

        auto difficultySprite = static_cast<GJDifficultySprite*>(difficultyContainer->getChildByID("difficulty-sprite"));
        if (!difficultySprite || !difficultySprite->isVisible()) return; // If invisible, we're just going to assume it's Grandpa Demon

        auto gddpDifficulty = difficultyContainer->getChildByID("gddp-difficulty");
        if (gddpDifficulty && !Mod::get()->getSettingValue<bool>("gddp-integration-override")) return;
        else if (gddpDifficulty) gddpDifficulty->setVisible(false);

        auto levelID = level->m_levelID.value();
        auto demon = DemonsInBetween::demonForLevel(levelID, false);
        if (demon.id != 0 && demon.difficulty != 0) {
            createUI(demon, difficultyContainer, difficultySprite);
            return;
        }

        difficultyContainer->addChild(DemonsInBetween::spriteForDifficulty(
            difficultySprite, DemonsInBetween::difficultyForDemonDifficulty(level->m_demonDifficulty),
            GJDifficultyName::Short, DemonsInBetween::stateForLevel(level)
        ), 3);
        difficultySprite->setOpacity(0);

        DemonsInBetween::loadDemonForLevel(std::move(m_fields->m_listener), levelID, false, [this, difficultyContainer, difficultySprite](LadderDemon& demon) {
            createUI(demon, difficultyContainer, difficultySprite);
            release();
            difficultyContainer->release();
            difficultySprite->release();
        }, [this, difficultyContainer, difficultySprite] {
            retain();
            difficultyContainer->retain();
            difficultySprite->retain();
        });
    }

    void createUI(LadderDemon const& demon, CCNode* difficultyContainer, GJDifficultySprite* difficultySprite) {
        if (auto betweenDifficultySprite = static_cast<CCSprite*>(difficultyContainer->getChildByID("between-difficulty-sprite"_spr))) {
            betweenDifficultySprite->setDisplayFrame(
                DemonsInBetween::spriteFrameForDifficulty(demon.difficulty, GJDifficultyName::Short, DemonsInBetween::stateForLevel(m_level)));
            betweenDifficultySprite->setPosition(difficultySprite->getPosition() + DemonsInBetween::SHORT_OFFSETS[(size_t)demon.difficulty - 1]);
        } else {
            difficultyContainer->addChild(DemonsInBetween::spriteForDifficulty(difficultySprite,
                demon.difficulty, GJDifficultyName::Short, DemonsInBetween::stateForLevel(m_level)), 3);
            difficultySprite->setOpacity(0);
        }
    }
};
