#include "DemonsInBetween.hpp"

void DemonsInBetween::loadGDDL() {
    static std::optional<web::WebTask> task = std::nullopt;
    task = web::WebRequest().get("https://gdladder.com/api/theList").map([](web::WebResponse* res) {
        if (res->ok()) {
            for (auto const& level : res->json().value().as_array()) {
                auto levelID = level["ID"].as_int();
                if (levelID > 100 && !level["Rating"].is_null()) GDDL.push_back({ levelID, DIFFICULTIES[(size_t)round(level["Rating"].as_double())] });
            }
        }
        else Notification::create("Failed to load GDDL", NotificationIcon::Error)->show();

        task = std::nullopt;
        return *res;
    });
}

CCSprite* DemonsInBetween::spriteForDifficulty(GJDifficultySprite* difficultySprite, int difficulty, GJDifficultyName name) {
    auto sprite = CCSprite::createWithSpriteFrameName((name == GJDifficultyName::Long ?
        fmt::format("DIB_{:02d}_btn2_001.png"_spr, difficulty) : fmt::format("DIB_{:02d}_btn_001.png"_spr, difficulty)).c_str());
    sprite->setPosition(difficultySprite->getPosition() + (name == GJDifficultyName::Long ?
        LONG_OFFSETS[(size_t)(difficulty - 1)] : SHORT_OFFSETS[(size_t)(difficulty - 1)]));
    sprite->setID("between-difficulty-sprite"_spr);
    return sprite;
}
