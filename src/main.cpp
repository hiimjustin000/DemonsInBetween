#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>

using namespace geode::prelude;

std::map<int, int> TIERS = {};
bool TIERS_TRIED_LOADING = false;

int tierToIndex(int tier) {
    switch (tier) {
        case 1: return 1;
        case 2: return 2;
        case 3: return 3;
        case 4: return 4;
        case 5: return 5;
        case 6: return 6;
        case 7: return 7;
        case 8: return 8;
        case 9: return 9;
        case 10: return 10;
        case 11: case 12: return 11;
        case 13: return 12;
        case 14: return 13;
        case 15: case 16: return 14;
        case 17: case 18: return 15;
        case 19: return 16;
        case 20: return 17;
        case 21: return 18;
        case 22: return 19;
        default: return 20;
    }
}

#include <Geode/modify/MenuLayer.hpp>
class $modify(DIBMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;

        if (!TIERS_TRIED_LOADING) {
            TIERS_TRIED_LOADING = true;
            web::AsyncWebRequest()
                .get("https://gdladder.com/api/theList")
                .json()
                .then([](matjson::Value const& json) {
                    for (auto const& level : json.as_array()) {
                        auto levelID = level["ID"].as_int();
                        if (levelID > 100 && !level["Rating"].is_null()) TIERS[levelID] = (int)round(level["Rating"].as_double());
                    }
                })
                .expect([](std::string const& error) {
                    Notification::create("Failed to load GDDL", NotificationIcon::Error)->show();
                });
        }

        return true;
    }
};

#include <Geode/modify/LevelInfoLayer.hpp>
class $modify(DIBLevelInfoLayer, LevelInfoLayer) {
    bool init(GJGameLevel* level, bool challenge) {
        if (!LevelInfoLayer::init(level, challenge)) return false;

        auto levelID = level->m_levelID.value();
        if (!getChildByID("grd-difficulty") && TIERS.find(levelID) != TIERS.end()) {
            auto index = tierToIndex(TIERS[levelID]);
            auto betweenDifficultySprite = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(fmt::format("DIB_{:02d}_btn2_001.png", index).c_str()));
            betweenDifficultySprite->setPosition({
                m_difficultySprite->getPositionX(),
                m_difficultySprite->getPositionY() + (index < 16 ? index < 8 ? -5.0f : -4.25f : -3.75f)
            });
            betweenDifficultySprite->setID("between-difficulty-sprite"_spr);
            addChild(betweenDifficultySprite, 3);
            m_difficultySprite->setOpacity(0);
        }

        return true;
    }
};

#include <Geode/modify/LevelCell.hpp>
class $modify(DIBLevelCell, LevelCell) {
    void loadFromLevel(GJGameLevel* level) {
        LevelCell::loadFromLevel(level);

        auto levelID = level->m_levelID.value();
        if (TIERS.find(levelID) != TIERS.end()) {
            auto index = tierToIndex(TIERS[levelID]);
            if (auto difficultyContainer = m_mainLayer->getChildByID("difficulty-container")) {
                auto difficultySprite = static_cast<GJDifficultySprite*>(difficultyContainer->getChildByID("difficulty-sprite"));
                auto betweenDifficultySprite = CCSprite::createWithSpriteFrameName(Mod::get()->expandSpriteName(fmt::format("DIB_{:02d}_btn_001.png", index).c_str()));
                betweenDifficultySprite->setPosition({
                    difficultySprite->getPositionX() + (index > 12 ? -0.125f : 0.0f),
                    difficultySprite->getPositionY() + (index < 16 ? index == 4 || index == 7 ? -0.5f : index < 8 ? -0.25f : 0.5f : 1.0f)
                });
                betweenDifficultySprite->setID("between-difficulty-sprite"_spr);
                difficultyContainer->addChild(betweenDifficultySprite, 3);
                difficultySprite->setOpacity(0);
            }
        }
    }
};
