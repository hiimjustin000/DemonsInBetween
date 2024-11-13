#include "../DemonsInBetween.hpp"

using namespace geode::prelude;

#include <Geode/modify/LevelPage.hpp>
class $modify(DIBLevelPage, LevelPage) {
    void updateDynamicPage(GJGameLevel* level) {
        LevelPage::updateDynamicPage(level);

        if (auto betweenDifficulty = m_levelDisplay->getChildByID("between-difficulty-sprite"_spr)) {
            betweenDifficulty->removeFromParent();
            m_difficultySprite->setVisible(true);
        }

        if (level->m_levelID.value() < 1 || GameStatsManager::get()->getStat("8") < m_level->m_requiredCoins) return;

        auto demon = DemonsInBetween::demonForLevel(level, true);
        if (demon.id == 0 || demon.difficulty == 0) return;

        auto demonSprite = CCSprite::createWithSpriteFrameName(fmt::format("DIB_{:02d}_001.png"_spr, demon.difficulty).c_str());
        demonSprite->setPosition(m_difficultySprite->getPosition());
        demonSprite->setScale(1.1f);
        demonSprite->setID("between-difficulty-sprite"_spr);
        m_levelDisplay->addChild(demonSprite);
        m_difficultySprite->setVisible(false);
    }
};
