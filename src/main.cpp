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
            auto pos = CCPoint { 0.0f, 0.0f };
            switch (index) {
                case 1: pos.x = 0.0f; pos.y = -5.0f; break;
                case 2: pos.x = 0.125f; pos.y = -5.0f; break;
                case 3: pos.x = 0.0f; pos.y = -5.0f; break;
                case 4: pos.x = 0.0f; pos.y = -5.125f; break;
                case 5: pos.x = 0.25f; pos.y = -5.0f; break;
                case 6: pos.x = 0.125f; pos.y = -4.75f; break;
                case 7: pos.x = 0.0f; pos.y = -5.0f; break;
                case 8: pos.x = 0.0f; pos.y = -4.125f; break;
                case 9: pos.x = -0.125f; pos.y = -4.125f; break;
                case 10: pos.x = 0.0f; pos.y = -3.75f; break;
                case 11: pos.x = -0.125f; pos.y = -4.125f; break;
                case 12: pos.x = 0.0f; pos.y = -4.125f; break;
                case 13: pos.x = 0.125f; pos.y = -4.125f; break;
                case 14: pos.x = 0.0f; pos.y = -4.125f; break;
                case 15: pos.x = 0.0f; pos.y = -4.125f; break;
                case 16: pos.x = 0.0f; pos.y = -3.625f; break;
                case 17: pos.x = 0.0f; pos.y = -3.625f; break;
                case 18: pos.x = 0.0f; pos.y = -3.625f; break;
                case 19: pos.x = 0.0f; pos.y = -3.5f; break;
                case 20: pos.x = 0.0f; pos.y = -3.5f; break;
            }
            betweenDifficultySprite->setPosition(m_difficultySprite->getPosition() + pos);
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
                auto pos = CCPoint { 0.0f, 0.0f };
                switch (index) {
                    case 1: pos.x = -0.125f; pos.y = -0.25f; break;
                    case 2: pos.x = -0.125f; pos.y = -0.25f; break;
                    case 3: pos.x = -0.125f; pos.y = -0.25f; break;
                    case 4: pos.x = -0.125f; pos.y = -0.375f; break;
                    case 5: pos.x = -0.125f; pos.y = -0.25f; break;
                    case 6: pos.x = -0.125f; pos.y = -0.25f; break;
                    case 7: pos.x = -0.125f; pos.y = -0.375f; break;
                    case 8: pos.x = -0.125f; pos.y = 0.5f; break;
                    case 9: pos.x = -0.125f; pos.y = 0.5f; break;
                    case 10: pos.x = -0.125f; pos.y = 0.25f; break;
                    case 11: pos.x = -0.125f; pos.y = 0.5f; break;
                    case 12: pos.x = 0.125f; pos.y = 0.5f; break;
                    case 13: pos.x = 0.125f; pos.y = 0.5f; break;
                    case 14: pos.x = 0.125f; pos.y = 0.5f; break;
                    case 15: pos.x = 0.0f; pos.y = 0.5f; break;
                    case 16: pos.x = 0.0f; pos.y = 1.25f; break;
                    case 17: pos.x = 0.0f; pos.y = 1.25f; break;
                    case 18: pos.x = 0.0f; pos.y = 1.125f; break;
                    case 19: pos.x = 0.0f; pos.y = 1.125f; break;
                    case 20: pos.x = 0.0f; pos.y = 1.125f; break;
                }
                betweenDifficultySprite->setPosition(difficultySprite->getPosition() + pos);
                betweenDifficultySprite->setID("between-difficulty-sprite"_spr);
                difficultyContainer->addChild(betweenDifficultySprite, 3);
                difficultySprite->setOpacity(0);
            }
        }
    }
};
