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
    if (time(0) > cache["cached"].as_int() + 172800) {
        log::warn("GDDL cache is outdated, loading from spreadsheet");
        loadGDDL();
        return;
    }

    GDDL_CACHE = cache;
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
        GDDL_CACHE = matjson::Object({ { "cached", time(0) }, { "list", gddl } });
        saveGDDL();
    }

    for (auto const& demon : gddl) {
        auto id = demon["ID"].as_int();
        if (id > 100 && !demon["Tier"].is_null()) {
            auto tier = (int)round(demon["Tier"].as_double());
            GDDL.push_back({
                id,
                tier,
                !demon["Enjoyment"].is_null() ? (int)round(demon["Enjoyment"].as_double()) : 0,
                DIFFICULTY_INDICES[tier]
            });
        }
    }
}

void DemonsInBetween::saveGDDL() {
    auto file = std::fstream(CACHE_PATH, std::ios::out);
    file << GDDL_CACHE.dump(0);
    file.close();
    log::info("Successfully saved GDDL cache");
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
            else if (key == "Tier" || key == "Enjoyment") demon[key] = value != "" ? round(std::stod(value) * 100) / 100 : matjson::Value(nullptr);
            else demon[key] = value;
        }
        demons.push_back(demon);
    }

    return demons;
}

LadderDemon const& DemonsInBetween::demonForLevel(GJGameLevel* level) {
    static LadderDemon defaultDemon = { 0, 0, 0, 0 };
    auto demon = std::find_if(GDDL.begin(), GDDL.end(), [level](auto const& d) {
        return d.id == level->m_levelID.value();
    });
    return demon == GDDL.end() ? defaultDemon : *demon;
}

void DemonsInBetween::refreshDemonForLevel(EventListener<web::WebTask>&& listenerRef, GJGameLevel* level, MiniFunction<void(LadderDemon const&)> callback) {
    auto levelID = level->m_levelID.value();
    auto demon = std::find_if(GDDL.begin(), GDDL.end(), [levelID](auto const& d) {
        return d.id == levelID;
    });
    if (demon == GDDL.end()) return;

    auto&& listener = std::move(listenerRef);
    listener.bind([callback, demon](web::WebTask::Event* e) {
        if (auto res = e->getValue()) {
            if (res->ok()) {
                auto json = res->json().value();
                if (json["Rating"].is_null()) return;

                demon->difficulty = DIFFICULTY_INDICES[(int)round(json["Rating"].as_double())];

                auto& cacheArray = GDDL_CACHE["list"].as_array();
                auto cacheDemon = std::find_if(cacheArray.begin(), cacheArray.end(), [demon](matjson::Value const& d) {
                    return d["ID"].as_int() == demon->id;
                });
                if (cacheDemon != cacheArray.end()) {
                    cacheDemon->operator[]("Tier") = round(json["Rating"].as_double() * 100) / 100;
                    cacheDemon->operator[]("Enjoyment") = !json["Enjoyment"].is_null() ? round(json["Enjoyment"].as_double() * 100) / 100 : matjson::Value(nullptr);
                    GDDL_CACHE_CHANGED = true;
                    FLAlertLayer::create("Refresh Success", "Successfully refreshed difficulty.", "OK")->show();
                    log::info("Updated demon with ID {}", demon->id);
                } else {
                    FLAlertLayer::create("Refresh Failed", "Failed to refresh difficulty. Please try again later.", "OK")->show();
                    log::error("Failed to update demon with ID {}", demon->id);
                }

                if (callback) callback(*demon);
            }
        }
    });

    listener.setFilter(web::WebRequest().get(fmt::format("https://gdladder.com/api/level/{}", demon->id).c_str()));
}

std::string const& DemonsInBetween::infoForLevel(GJGameLevel* level, LadderDemon const& demon) {
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
    switch (level->m_demonDifficulty) {
        case 0: originalDifficulty = "Hard Demon"; break;
        case 3: originalDifficulty = "Easy Demon"; break;
        case 4: originalDifficulty = "Medium Demon"; break;
        case 5: originalDifficulty = "Insane Demon"; break;
        case 6: originalDifficulty = "Extreme Demon"; break;
    }

    static std::string info;
    info = fmt::format("<cy>{}</c>\n<cg>Tier</c>: {}\n<cl>Enjoyment</c>: {}\n<cp>Difficulty</c>: {}\n<co>Original Difficulty</c>: {}",
        level->m_levelName, demon.tier, demon.enjoyment, difficulty, originalDifficulty);
    return info;
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
