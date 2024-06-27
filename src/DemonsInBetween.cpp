#include <Geode/utils/string.hpp>
#include "DemonsInBetween.hpp"

void DemonsInBetween::loadGDDL() {
    static std::optional<web::WebTask> task = std::nullopt;
    task = web::WebRequest().get("https://gdladder.com/api/theList").map([](web::WebResponse* res) {
        if (res->ok()) {
            for (auto const& level : res->json().value().as_array()) {
                auto levelID = level["ID"].as_int();
                if (levelID > 100 && !level["Rating"].is_null()) GDDL.push_back({
                    levelID,
                    DIFFICULTY_INDICES[(size_t)round(level["Rating"].as_double())],
                    level["Enjoyment"].is_null() ? 0.0 : level["Enjoyment"].as_double()
                });
            }
        }
        else Notification::create("Failed to load GDDL", NotificationIcon::Error)->show();

        task = std::nullopt;
        return *res;
    });
}

LadderDemon const& DemonsInBetween::demonForLevel(GJGameLevel* level) {
    static LadderDemon defaultDemon = { 0, 0, 0.0 };
    auto demon = std::find_if(GDDL.begin(), GDDL.end(), [level](auto const& d) {
        return d.id == level->m_levelID.value();
    });
    return demon == GDDL.end() ? defaultDemon : *demon;
}

CCSprite* DemonsInBetween::spriteForDifficulty(GJDifficultySprite* difficultySprite, int difficulty, GJDifficultyName name) {
    auto sprite = CCSprite::createWithSpriteFrameName((name == GJDifficultyName::Long ?
        fmt::format("DIB_{:02d}_btn2_001.png"_spr, difficulty) : fmt::format("DIB_{:02d}_btn_001.png"_spr, difficulty)).c_str());
    sprite->setPosition(difficultySprite->getPosition() + (name == GJDifficultyName::Long ?
        LONG_OFFSETS[(size_t)difficulty - 1] : SHORT_OFFSETS[(size_t)difficulty - 1]));
    sprite->setID("between-difficulty-sprite"_spr);
    return sprite;
}

GJSearchObject* DemonsInBetween::searchObjectForPage(int page) {
    SEARCH_RESULTS.clear();

    for (size_t i = 0; i < GDDL.size(); i++) {
        if (GDDL[i].difficulty == DIFFICULTY) SEARCH_RESULTS.push_back(std::to_string(GDDL[i].id));
    }

    auto size = (int)SEARCH_RESULTS.size();
    MAX_PAGE = (size % 10 == 0 ? size : size + (10 - (size % 10))) / 10 - 1;

    return GJSearchObject::create(SearchType::MapPackOnClick, string::join(std::vector<std::string>(
        SEARCH_RESULTS.begin() + page * 10,
        SEARCH_RESULTS.begin() + std::min((page + 1) * 10, size)
    ), ","));
}
