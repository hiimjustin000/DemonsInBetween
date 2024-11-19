#include "../DemonsInBetween.hpp"

using namespace geode::prelude;

#include <Geode/modify/LevelPage.hpp>
class $modify(DIBLevelPage, LevelPage) {
    static void onModify(auto& self) {
        (void)self.setHookPriority("LevelPage::updateDynamicPage", -1); // overcharged main levels is 0 D:
    }

    void updateDynamicPage(GJGameLevel* level) {
        LevelPage::updateDynamicPage(level);

        if (auto betweenDifficulty = m_levelDisplay->getChildByID("between-difficulty-sprite"_spr)) {
            betweenDifficulty->removeFromParent();
            m_difficultySprite->setVisible(true);
        }

        if (level->m_levelID.value() < 1 || GameStatsManager::get()->getStat("8") < m_level->m_requiredCoins) return;

        auto demon = DemonsInBetween::demonForLevel(level, true);
        if (demon.id == 0 || demon.difficulty == 0) return;

        auto overcharged = Loader::get()->getLoadedMod("firee.overchargedlevels");
        auto overchargedEnabled = overcharged && overcharged->getSettingValue<bool>("enabled");
        auto demonSprite = CCSprite::createWithSpriteFrameName(fmt::format("DIB_{:02d}{}_001.png"_spr, demon.difficulty, overchargedEnabled ? "_btn" : "").c_str());
        demonSprite->setPosition(m_difficultySprite->getPosition() +
            (overchargedEnabled ? DemonsInBetween::SHORT_OFFSETS[(size_t)demon.difficulty - 1] * 0.9f : CCPoint { 0.0f, 0.0f }));
        demonSprite->setScale(overchargedEnabled ? 0.9f : 1.1f);
        demonSprite->setID("between-difficulty-sprite"_spr);
        m_levelDisplay->addChild(demonSprite);
        m_difficultySprite->setVisible(false);
    }
};
