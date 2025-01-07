#include "../DemonsInBetween.hpp"

using namespace geode::prelude;

#include <Geode/modify/LevelInfoLayer.hpp>
class $modify(DIBLevelInfoLayer, LevelInfoLayer) {
    struct Fields {
        EventListener<web::WebTask> m_listener;
        bool m_createDemon;
        CCMenuItemSpriteExtra* m_demonInfoButton;
    };

    static void onModify(auto& self) {
        (void)self.setHookPriority("LevelInfoLayer::init", -50); // gddp integration is -42 D:
    }

    bool init(GJGameLevel* level, bool challenge) {
        if (!LevelInfoLayer::init(level, challenge)) return false;

        if (level->m_stars.value() < 10) return true;

        auto f = m_fields.self();
        f->m_createDemon = true;

        if (getChildByID("grd-difficulty") || !Mod::get()->getSettingValue<bool>("enable-difficulties")) f->m_createDemon = false;

        auto gddpDifficulty = getChildByID("gddp-difficulty");
        if (gddpDifficulty && !Mod::get()->getSettingValue<bool>("gddp-integration-override")) f->m_createDemon = false;
        else if (gddpDifficulty) gddpDifficulty->setVisible(false);

        auto levelID = level->m_levelID.value();
        auto demon = DemonsInBetween::demonForLevel(levelID, false);
        if (demon.id != 0 && demon.difficulty != 0) {
            createUI(demon);
            return true;
        }

        addChild(DemonsInBetween::spriteForDifficulty(
            m_difficultySprite, DemonsInBetween::difficultyForDemonDifficulty(level->m_demonDifficulty),
            GJDifficultyName::Long, DemonsInBetween::stateForLevel(level)
        ), 3);
        m_difficultySprite->setOpacity(0);

        DemonsInBetween::loadDemonForLevel(std::move(m_fields->m_listener), levelID, false, [this](LadderDemon& demon) {
            createUI(demon);
            release();
        }, [this] { retain(); });

        return true;
    }

    void onDemonInfo(CCObject* sender) {
        auto demon = DemonsInBetween::demonForLevel(m_level->m_levelID.value(), false);
        if (demon.id == 0 || demon.difficulty == 0) return;

        auto difficulty = "Unknown Demon";
        switch (demon.difficulty) {
            case 1: difficulty = "Free Demon"; break;
            case 2: difficulty = "Peaceful Demon"; break;
            case 3: difficulty = "Simple Demon"; break;
            case 4: difficulty = "Easy Demon"; break;
            case 5: difficulty = "Casual Demon"; break;
            case 6: difficulty = "Mild Demon"; break;
            case 7: difficulty = "Medium Demon"; break;
            case 8: difficulty = "Normal Demon"; break;
            case 9: difficulty = "Moderate Demon"; break;
            case 10: difficulty = "Tricky Demon"; break;
            case 11: difficulty = "Hard Demon"; break;
            case 12: difficulty = "Harder Demon"; break;
            case 13: difficulty = "Tough Demon"; break;
            case 14: difficulty = "Wild Demon"; break;
            case 15: difficulty = "Insane Demon"; break;
            case 16: difficulty = "Cruel Demon"; break;
            case 17: difficulty = "Crazy Demon"; break;
            case 18: difficulty = "Bizarre Demon"; break;
            case 19: difficulty = "Brutal Demon"; break;
            case 20: difficulty = "Extreme Demon"; break;
        }

        auto originalDifficulty = "Unknown Demon";
        switch (m_level->m_demonDifficulty) {
            case 0: originalDifficulty = "Hard Demon"; break;
            case 3: originalDifficulty = "Easy Demon"; break;
            case 4: originalDifficulty = "Medium Demon"; break;
            case 5: originalDifficulty = "Insane Demon"; break;
            case 6: originalDifficulty = "Extreme Demon"; break;
        }

        createQuickPopup("Demon Info", fmt::format(
            "<cy>{}</c>\n"
            "<cg>Tier</c>: {}\n"
            "<cl>Enjoyment</c>: {}\n"
            "<cp>Difficulty</c>: {}\n"
            "<co>Original Difficulty</c>: {}",
            std::string(m_level->m_levelName),
            demon.tier,
            demon.enjoyment >= 0.0 ? fmt::format("{}", demon.enjoyment) : "N/A",
            difficulty,
            originalDifficulty
        ), "OK", "Refresh", [this](auto, bool btn2) {
            if (!btn2) return;
            auto levelID = m_level->m_levelID.value();
            DemonsInBetween::LEVELS_LOADED.erase(levelID);
            DemonsInBetween::loadDemonForLevel(std::move(m_fields->m_listener), levelID, false, [this](LadderDemon& demon) {
                createUI(demon);
                release();
            }, [this] { retain(); });
        });
    }

    void createUI(LadderDemon const& demon) {
        auto f = m_fields.self();
        if (f->m_demonInfoButton) f->m_demonInfoButton->removeFromParent();

        f->m_demonInfoButton = CCMenuItemSpriteExtra::create(
            CircleButtonSprite::createWithSpriteFrameName(fmt::format("DIB_{:02d}_001.png"_spr, demon.difficulty).c_str()),
            this, menu_selector(DIBLevelInfoLayer::onDemonInfo)
        );
        f->m_demonInfoButton->setID("demon-info-button"_spr);
        auto leftSideMenu = getChildByID("left-side-menu");
        leftSideMenu->addChild(f->m_demonInfoButton);
        leftSideMenu->updateLayout();

        if (!f->m_createDemon) return;

        if (auto betweenDifficultySprite = static_cast<CCSprite*>(getChildByID("between-difficulty-sprite"_spr))) {
            betweenDifficultySprite->setDisplayFrame(
                DemonsInBetween::spriteFrameForDifficulty(demon.difficulty, GJDifficultyName::Long, DemonsInBetween::stateForLevel(m_level)));
            betweenDifficultySprite->setPosition(m_difficultySprite->getPosition() + DemonsInBetween::LONG_OFFSETS[(size_t)demon.difficulty - 1]);
        }
        else {
            addChild(DemonsInBetween::spriteForDifficulty(m_difficultySprite, demon.difficulty, GJDifficultyName::Long, DemonsInBetween::stateForLevel(m_level)), 3);
            m_difficultySprite->setOpacity(0);
        }
    }
};
