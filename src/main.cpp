#include "DIBSearchPopup.hpp"

$on_mod(DataSaved) {
    if (DemonsInBetween::GDDL_CACHE_CHANGED) DemonsInBetween::saveGDDL();
}

#include <Geode/modify/MenuLayer.hpp>
class $modify(DIBMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;

        if (DemonsInBetween::TRIED_LOADING) return true;
        DemonsInBetween::TRIED_LOADING = true;

        DemonsInBetween::tryLoadCache();

        return true;
    }
};

#include <Geode/modify/LevelSearchLayer.hpp>
class $modify(DIBLevelSearchLayer, LevelSearchLayer) {
    struct Fields {
        CCMenuItemSpriteExtra* m_quickSearchButton;
    };

    bool init(int searchType) {
        if (!LevelSearchLayer::init(searchType)) return false;

        m_demonTypeButton->setPositionY(m_demonTypeButton->getPositionY() + 12.0f);
        m_fields->m_quickSearchButton = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_plusBtn_001.png", 0.45f, [](auto) {
            DIBSearchPopup::create()->show();
        });
        m_fields->m_quickSearchButton->setPosition(m_demonTypeButton->getPosition() - CCPoint { 0.0f, 23.0f });
        m_fields->m_quickSearchButton->setID("quick-search-button"_spr);
        m_fields->m_quickSearchButton->setVisible(m_demonTypeButton->isVisible());
        m_demonTypeButton->getParent()->addChild(m_fields->m_quickSearchButton);

        return true;
    }

    void toggleDifficulty(CCObject* sender) {
        LevelSearchLayer::toggleDifficulty(sender);

        m_fields->m_quickSearchButton->setVisible(m_demonTypeButton->isVisible());
    }
};

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

        if (getChildByID("grd-difficulty") || getChildByID("gddp-difficulty")) {
            m_fields->m_disabled = true;
            return true;
        }

        auto& demon = DemonsInBetween::demonForLevel(level);
        if (demon.id == 0) return true;

        createDemonSprite(demon);

        return true;
    }

    void createDemonSprite(LadderDemon const& demon) {
        if (m_fields->m_disabled) return;

        if (auto existingDifficulty = getChildByID("between-difficulty-sprite"_spr)) existingDifficulty->removeFromParentAndCleanup(true);
        addChild(DemonsInBetween::spriteForDifficulty(m_difficultySprite, demon.difficulty, GJDifficultyName::Long, DemonsInBetween::stateForLevel(m_level)), 3);
        m_difficultySprite->setOpacity(0);
    }

    void onUpdate(CCObject* sender) {
        LevelInfoLayer::onUpdate(sender);
        if (m_fields->m_disabled) return;

        if (!m_isBusy && GameLevelManager::sharedState()->isTimeValid(std::to_string(m_level->m_levelID.value()).c_str(), 3600.0f))
            DemonsInBetween::refreshDemonForLevel(std::move(m_fields->m_listener), m_level, [this](LadderDemon const& demon) { createDemonSprite(demon); });
    }

    void levelUpdateFinished(GJGameLevel* level, UpdateResponse response) override {
        LevelInfoLayer::levelUpdateFinished(level, response);
        if (m_fields->m_disabled) return;

        DemonsInBetween::refreshDemonForLevel(std::move(m_fields->m_listener), level, [this](LadderDemon const& demon) { createDemonSprite(demon); });
    }
};

#include <Geode/modify/LevelCell.hpp>
class $modify(DIBLevelCell, LevelCell) {
    void loadFromLevel(GJGameLevel* level) {
        LevelCell::loadFromLevel(level);

        auto& demon = DemonsInBetween::demonForLevel(level);
        if (demon.id == 0) return;

        if (auto difficultyContainer = m_mainLayer->getChildByID("difficulty-container")) {
            if (difficultyContainer->getChildByID("grd-difficulty") || difficultyContainer->getChildByID("gddp-difficulty")) return;

            auto difficultySprite = static_cast<GJDifficultySprite*>(difficultyContainer->getChildByID("difficulty-sprite"));
            difficultyContainer->addChild(DemonsInBetween::spriteForDifficulty(difficultySprite,
                demon.difficulty, GJDifficultyName::Short, DemonsInBetween::stateForLevel(level)), 3);
            difficultySprite->setOpacity(0);
        }
    }
};

#include <Geode/modify/LevelBrowserLayer.hpp>
class $modify(DIBLevelBrowserLayer, LevelBrowserLayer) {
    struct Fields {
        int m_currentPage;
    };

    void loadLevelsFinished(CCArray* levels, const char* key, int type) override {
        LevelBrowserLayer::loadLevelsFinished(levels, key, type);

        if (DemonsInBetween::SEARCHING) {
            m_leftArrow->setVisible(m_fields->m_currentPage > 0);
            m_rightArrow->setVisible(m_fields->m_currentPage < DemonsInBetween::MAX_PAGE);
            m_pageBtn->setVisible(true);
        }
    }

    void setupPageInfo(gd::string pageInfo, const char* key) override {
        LevelBrowserLayer::setupPageInfo(pageInfo, key);

        if (DemonsInBetween::SEARCHING) {
            auto size = (int)DemonsInBetween::SEARCH_RESULTS.size();
            m_countText->setString(fmt::format("{} to {} of {}", m_fields->m_currentPage * 10 + 1, std::min(size, (m_fields->m_currentPage + 1) * 10), size).c_str());
            m_countText->limitLabelWidth(100.0f, 0.6f, 0.0f);
            m_pageText->setString(std::to_string(m_fields->m_currentPage + 1).c_str());
            m_pageText->limitLabelWidth(32.0f, 0.8f, 0.0f);
        }
    }

    void onBack(CCObject* sender) override {
        LevelBrowserLayer::onBack(sender);

        if (DemonsInBetween::SEARCHING) {
            DemonsInBetween::SEARCHING = false;
            DemonsInBetween::DIFFICULTY = 0;
        }
    }

    void setIDPopupClosed(SetIDPopup* popup, int page) override {
        LevelBrowserLayer::setIDPopupClosed(popup, page);

        if (DemonsInBetween::SEARCHING && popup->getTag() != 4) {
            m_fields->m_currentPage = std::min(page - 1, DemonsInBetween::MAX_PAGE);
            loadPage(DemonsInBetween::searchObjectForPage(m_fields->m_currentPage));
        }
    }

    void onGoToPage(CCObject* sender) {
        LevelBrowserLayer::onGoToPage(sender);

        if (DemonsInBetween::SEARCHING) {
            if (auto popup = getChildOfType<SetIDPopup>(CCDirector::sharedDirector()->getRunningScene(), 0)) {
                popup->m_value = m_fields->m_currentPage + 1;
                popup->updateTextInputLabel();
            }
        }
    }

    void onNextPage(CCObject* sender) {
        LevelBrowserLayer::onNextPage(sender);

        if (DemonsInBetween::SEARCHING) loadPage(DemonsInBetween::searchObjectForPage(++m_fields->m_currentPage));
    }

    void onPrevPage(CCObject* sender) {
        LevelBrowserLayer::onPrevPage(sender);

        if (DemonsInBetween::SEARCHING) loadPage(DemonsInBetween::searchObjectForPage(--m_fields->m_currentPage));
    }
#ifdef GEODE_IS_MACOS
    void keyDown(enumKeyCodes key) override {
        if (DemonsInBetween::SEARCHING) {
            switch (key) {
                case KEY_Left: case CONTROLLER_Left:
                    if (m_leftArrow && m_leftArrow->isVisible()) loadPage(DemonsInBetween::searchObjectForPage(--m_fields->m_currentPage));
                    break;
                case KEY_Right: case CONTROLLER_Right:
                    if (m_rightArrow && m_rightArrow->isVisible()) loadPage(DemonsInBetween::searchObjectForPage(++m_fields->m_currentPage));
                    break;
                default:
                    LevelBrowserLayer::keyDown(key);
                    break;
            }
        }
        else LevelBrowserLayer::keyDown(key);
    }
#endif
};
