#include "../DemonsInBetween.hpp"

using namespace geode::prelude;

#include <Geode/modify/LevelInfoLayer.hpp>
class $modify(DIBLevelInfoLayer, LevelInfoLayer) {
    struct Fields {
        EventListener<web::WebTask> m_listener;
        bool m_disabled;
    };

    static void onModify(auto& self) {
        (void)self.setHookPriority("LevelInfoLayer::init", -50); // gddp integration is -42 D:
    }

    bool init(GJGameLevel* level, bool challenge) {
        if (!LevelInfoLayer::init(level, challenge)) return false;

        auto f = m_fields.self();

        if (getChildByID("grd-difficulty") || !Mod::get()->getSettingValue<bool>("enable-difficulties")) f->m_disabled = true;

        auto gddpDifficulty = getChildByID("gddp-difficulty");
        if (gddpDifficulty && !Mod::get()->getSettingValue<bool>("gddp-integration-override")) f->m_disabled = true;
        else if (gddpDifficulty) gddpDifficulty->setVisible(false);

        auto demon = DemonsInBetween::demonForLevel(level, false);
        if (demon.id == 0 || demon.difficulty == 0) return true;

        auto leftSideMenu = getChildByID("left-side-menu");
        leftSideMenu->addChild(CCMenuItemSpriteExtra::create(
            CircleButtonSprite::createWithSpriteFrameName(fmt::format("DIB_{:02d}_001.png"_spr, demon.difficulty).c_str()),
            this, menu_selector(DIBLevelInfoLayer::onDemonInfo)
        ));
        leftSideMenu->updateLayout();

        createDemonSprite(demon);

        if (DemonsInBetween::REFRESHED_DEMONS.find(demon.id) != DemonsInBetween::REFRESHED_DEMONS.end()) return true;

        DemonsInBetween::refreshDemonForLevel(std::move(f->m_listener), m_level, [this](LadderDemon const& demon) {
            createDemonSprite(demon);
        }, false);

        return true;
    }

    void onDemonInfo(CCObject* sender) {
        auto demon = DemonsInBetween::demonForLevel(m_level, false);
        if (demon.id == 0 || demon.difficulty == 0) return;

        createQuickPopup("Demon Info", DemonsInBetween::infoForLevel(m_level, demon), "OK", "Refresh", [this](auto, bool btn2) {
            if (btn2) DemonsInBetween::refreshDemonForLevel(std::move(m_fields->m_listener), m_level, [this](LadderDemon const& demon) {
                createDemonSprite(demon);
            }, true);
        });
    }

    void createDemonSprite(LadderDemon const& demon) {
        if (m_fields->m_disabled) return;

        if (auto existingDifficulty = getChildByID("between-difficulty-sprite"_spr)) existingDifficulty->removeFromParent();
        addChild(DemonsInBetween::spriteForDifficulty(m_difficultySprite, demon.difficulty, GJDifficultyName::Long, DemonsInBetween::stateForLevel(m_level)), 3);
        m_difficultySprite->setOpacity(0);
    }
};
