#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>

using namespace geode::prelude;

std::map<int, int> INDICES = {
    {1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 5}, {6, 6}, {7, 7}, {8, 8}, {9, 9}, {10, 10},
    {11, 11}, {12, 11}, {13, 12}, {14, 13}, {15, 14}, {16, 14}, {17, 15}, {18, 15},
    {19, 16}, {20, 17}, {21, 18}, {22, 19}, {23, 20}, {24, 20}, {25, 20}, {26, 20},
    {27, 20}, {28, 20}, {29, 20}, {30, 20}, {31, 20}, {32, 20}, {33, 20}, {34, 20},
    {35, 20}
};
std::map<int, int> TIERS = {};
bool TIERS_TRIED_LOADING = false;

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
            auto index = INDICES[TIERS[levelID]];
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
            auto index = INDICES[TIERS[levelID]];
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
