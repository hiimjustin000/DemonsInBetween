#include <Geode/utils/string.hpp>
#include "DemonsInBetween.hpp"

#define GDDL_URL "https://docs.google.com/spreadsheets/d/1qKlWKpDkOpU1ZF6V6xGfutDY2NvcA8MNPnsv6GBkKPQ/gviz/tq?tqx=out:csv&sheet=GDDL"

void DemonsInBetween::tryLoadCache() {
    if (!std::filesystem::exists(CACHE_PATH)) {
        log::error("GDDL cache does not exist, loading from spreadsheet");
        loadGDDL();
        return;
    }

    std::ifstream file(CACHE_PATH);
    if (file.peek() == std::ifstream::traits_type::eof()) {
        log::error("GDDL cache is empty, loading from spreadsheet");
        loadGDDL();
        return;
    }
    
    std::stringstream bufferStream;
    bufferStream << file.rdbuf();

    auto error = std::string();
    auto json = matjson::parse(bufferStream.str(), error);
    if (!error.empty()) {
        log::error("Failed to parse GDDL cache, loading from spreadsheet");
        loadGDDL();
        return;
    }
    
    auto cache = json.value();
    if (!cache.contains("cached") || !cache["cached"].is_number() || !cache.contains("list") || !cache["list"].is_array()) {
        log::error("GDDL cache is corrupted, loading from spreadsheet");
        loadGDDL();
        return;
    }

    log::info("Successfully loaded GDDL cache");
    if (time(0) > cache["cached"].as_int() + 604800) {
        log::warn("GDDL cache is outdated, loading from spreadsheet");
        loadGDDL();
        return;
    }

    initGDDL(cache["list"].as_array());
}

void DemonsInBetween::loadGDDL() {
    static std::optional<web::WebTask> task = std::nullopt;
    task = web::WebRequest().get(GDDL_URL).map([](web::WebResponse* res) {
        if (res->ok()) {
            log::info("Successfully loaded GDDL spreadsheet with status code {}", res->code());
            initGDDL(parseGDDL(res->string().value()), true);
        }
        else {
            log::error("Failed to load GDDL spreadsheet with status code {}", res->code());
            Notification::create("Failed to load GDDL", NotificationIcon::Error)->show();
        }

        task = std::nullopt;
        return *res;
    });
}

void DemonsInBetween::initGDDL(matjson::Array const& gddl, bool saveCache) {
    if (saveCache) {
        auto file = std::fstream(CACHE_PATH, std::ios::out);
        file << matjson::Value(matjson::Object({ { "cached", time(0) }, { "list", gddl } })).dump(0);
        file.close();
        log::info("Successfully saved GDDL cache");
    }

    for (auto const& demon : gddl) {
        auto id = demon["ID"].as_int();
        if (id > 100 && !demon["Tier"].is_null()) GDDL.push_back({ id, DIFFICULTY_INDICES[(int)round(demon["Tier"].as_double())] });
    }
}

matjson::Array const& DemonsInBetween::parseGDDL(std::string const& data) {
    // MAKE SURE WE ARE NOT SPLITTING THE COMMAS IN THE QUOTATION MARKS
    auto lines = string::split(data, "\n");
    auto keys = string::split(lines[0].substr(1, lines[0].size() - 2), "\",\"");
    static auto demons = matjson::Array();
    for (size_t i = 1; i < lines.size(); i++) {
        auto values = string::split(lines[i].substr(1, lines[i].size() - 2), "\",\"");
        auto demon = matjson::Object();
        for (size_t j = 0; j < keys.size(); j++) {
            auto key = keys[j];
            auto value = values[j];
            if (key == "ID") demon[key] = std::stoi(value);
            else if (key == "Tier" || key == "Enjoyment") demon[key] = value != "" ? std::stod(value) : matjson::Value(nullptr);
            else demon[key] = value;
        }
        demons.push_back(demon);
    }

    return demons;
}

LadderDemon const& DemonsInBetween::demonForLevel(GJGameLevel* level) {
    static LadderDemon defaultDemon = { 0, 0 };
    auto demon = std::find_if(GDDL.begin(), GDDL.end(), [level](auto const& d) {
        return d.id == level->m_levelID.value();
    });
    return demon == GDDL.end() ? defaultDemon : *demon;
}

CCSprite* DemonsInBetween::spriteForDifficulty(GJDifficultySprite* difficultySprite, int difficulty, GJDifficultyName name, GJFeatureState state) {
    auto glow = state == GJFeatureState::Legendary ? "_4" : "";
    auto sprite = CCSprite::createWithSpriteFrameName((name == GJDifficultyName::Long ?
        fmt::format("DIB_{:02d}{}_btn2_001.png"_spr, difficulty, glow) : fmt::format("DIB_{:02d}{}_btn_001.png"_spr, difficulty, glow)).c_str());
    sprite->setPosition(difficultySprite->getPosition() + (name == GJDifficultyName::Long ?
        LONG_OFFSETS[(size_t)difficulty - 1] : SHORT_OFFSETS[(size_t)difficulty - 1]));
    sprite->setID("between-difficulty-sprite"_spr);
    return sprite;
}

GJFeatureState DemonsInBetween::stateForLevel(GJGameLevel* level) {
    return Loader::get()->isModLoaded("adyagd.godlikefaces") ? level->m_featured ? (GJFeatureState)(level->m_isEpic + 1) : GJFeatureState::None : GJFeatureState::None;
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
