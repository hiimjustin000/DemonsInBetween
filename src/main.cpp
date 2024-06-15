#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>

using namespace geode::prelude;

std::vector<CCPoint> LIL_OFFSETS = {
    { 0.0f, -5.0f }, { 0.125f, -5.0f }, { 0.0f, -5.0f }, { 0.0f, -5.125f }, { 0.25f, -5.0f },
    { 0.125f, -4.75f }, { 0.0f, -5.0f }, { 0.0f, -4.125f }, { -0.125f, -4.125f }, { 0.0f, -4.0f },
    { -0.125f, -4.125f }, { 0.0f, -4.125f }, { 0.125f, -4.125f }, { 0.0f, -4.125f }, { 0.0f, -4.125f },
    { 0.0f, -3.625f }, { 0.0f, -3.625f }, { 0.0f, -3.5f }, { 0.0f, -3.5f }, { 0.0f, -3.5f }
};
std::vector<CCPoint> LC_OFFSETS = {
    { -0.125f, -0.25f }, { -0.125f, -0.25f }, { -0.125f, -0.25f }, { -0.125f, -0.375f }, { -0.125f, -0.25f },
    { -0.125f, -0.25f }, { -0.125f, -0.375f }, { -0.125f, 0.5f }, { -0.125f, 0.5f }, { -0.125f, 0.25f },
    { -0.125f, 0.5f }, { 0.125f, 0.5f }, { 0.125f, 0.5f }, { 0.125f, 0.5f }, { 0.0f, 0.5f },
    { 0.0f, 1.25f }, { 0.0f, 1.25f }, { 0.0f, 1.125f }, { 0.0f, 1.125f }, { 0.0f, 1.125f }
};

std::vector<int> INDICES = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 11, 12, 13, 14, 14, 15, 15,
    16, 17, 18, 19, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20
};
std::map<int, int> TIERS = {};
bool TIERS_TRIED_LOADING = false;

#include <Geode/modify/MenuLayer.hpp>
class $modify(DIBMenuLayer, MenuLayer) {
    struct Fields {
        EventListener<web::WebTask> m_listener;
    };

    bool init() {
        if (!MenuLayer::init()) return false;

        if (TIERS_TRIED_LOADING) return true;
        TIERS_TRIED_LOADING = true;

        m_fields->m_listener.bind([](auto e) {
            if (auto res = e->getValue()) {
                if (res->ok()) {
                    for (auto const& level : res->json().value().as_array()) {
                        auto levelID = level["ID"].as_int();
                        if (levelID > 100 && !level["Rating"].is_null()) TIERS[levelID] = (int)round(level["Rating"].as_double());
                    }
                }
                else Notification::create("Failed to load GDDL", NotificationIcon::Error)->show();
            }
        });

        m_fields->m_listener.setFilter(web::WebRequest().get("https://gdladder.com/api/theList"));

        return true;
    }
};

#include <Geode/modify/LevelInfoLayer.hpp>
class $modify(DIBLevelInfoLayer, LevelInfoLayer) {
    static void onModify(auto& self) {
        (void)self.setHookPriority("LevelInfoLayer::init", -50);
    }

    bool init(GJGameLevel* level, bool challenge) {
        if (!LevelInfoLayer::init(level, challenge)) return false;

        auto levelID = level->m_levelID.value();
        if (getChildByID("grd-difficulty") || getChildByID("gddp-difficulty") || TIERS.find(levelID) == TIERS.end()) return true;

        auto index = INDICES[TIERS[levelID]];
        auto betweenDifficultySprite = CCSprite::createWithSpriteFrameName(fmt::format("DIB_{:02d}_btn2_001.png"_spr, index).c_str());
        betweenDifficultySprite->setPosition(m_difficultySprite->getPosition() + LIL_OFFSETS[(size_t)(index - 1)]);
        betweenDifficultySprite->setID("between-difficulty-sprite"_spr);
        addChild(betweenDifficultySprite, 3);
        m_difficultySprite->setOpacity(0);

        return true;
    }
};

#include <Geode/modify/LevelCell.hpp>
class $modify(DIBLevelCell, LevelCell) {
    void loadFromLevel(GJGameLevel* level) {
        LevelCell::loadFromLevel(level);

        auto levelID = level->m_levelID.value();
        if (TIERS.find(levelID) == TIERS.end()) return;

        if (auto difficultyContainer = m_mainLayer->getChildByID("difficulty-container")) {
            if (difficultyContainer->getChildByID("gddp-difficulty")) return;

            auto index = INDICES[TIERS[levelID]];
            auto betweenDifficultySprite = CCSprite::createWithSpriteFrameName(fmt::format("DIB_{:02d}_btn_001.png"_spr, index).c_str());
            auto difficultySprite = static_cast<GJDifficultySprite*>(difficultyContainer->getChildByID("difficulty-sprite"));
            betweenDifficultySprite->setPosition(difficultySprite->getPosition() + LC_OFFSETS[(size_t)(index - 1)]);
            betweenDifficultySprite->setID("between-difficulty-sprite"_spr);
            difficultyContainer->addChild(betweenDifficultySprite, 3);
            difficultySprite->setOpacity(0);
        }
    }
};
