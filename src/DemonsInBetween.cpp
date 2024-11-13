#include "DemonsInBetween.hpp"

using namespace geode::prelude;

#define GDDL_URL "https://docs.google.com/spreadsheets/d/1qKlWKpDkOpU1ZF6V6xGfutDY2NvcA8MNPnsv6GBkKPQ/gviz/tq?tqx=out:csv&sheet=GDDL"

$on_mod(Loaded) {
    DemonsInBetween::CACHE_PATH = (Mod::get()->getSaveDir() / "gddl.json").string();
}

$on_mod(DataSaved) {
    if (DemonsInBetween::GDDL_CACHE_CHANGED) DemonsInBetween::saveGDDL();
}

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

    std::string error;
    auto json = matjson::parse(bufferStream.str());
    if (!json.isOk()) {
        log::error("Failed to parse GDDL cache, loading from spreadsheet");
        loadGDDL();
        return;
    }

    auto cache = json.unwrap();
    if (!cache.contains("cached") || !cache["cached"].isNumber() || !cache.contains("list") || !cache["list"].isArray()) {
        log::error("GDDL cache is corrupted, loading from spreadsheet");
        loadGDDL();
        return;
    }

    log::info("Successfully loaded GDDL cache");
    if (time(0) > cache["cached"].asInt().unwrap() + 172800) {
        log::warn("GDDL cache is outdated, loading from spreadsheet");
        loadGDDL();
        return;
    }

    GDDL_CACHE = cache;
    initGDDL(cache["list"].asArray().unwrap());
}

void DemonsInBetween::loadGDDL() {
    static std::optional<web::WebTask> task = std::nullopt;
    task = web::WebRequest().get(GDDL_URL).map([](web::WebResponse* res) {
        if (res->ok() && res->string().isOk()) {
            log::info("Successfully loaded GDDL spreadsheet with status code {}", res->code());
            initGDDL(parseGDDL(res->string().unwrap()), true);
        }
        else {
            log::error("Failed to load GDDL spreadsheet with status code {}", res->code());
            Notification::create("Failed to load GDDL", NotificationIcon::Error)->show();
        }

        task = std::nullopt;
        return *res;
    });
}

void DemonsInBetween::initGDDL(std::vector<matjson::Value> const& gddl, bool saveCache) {
    if (saveCache) {
        GDDL_CACHE = matjson::makeObject({ { "cached", time(0) }, { "list", gddl } });
        saveGDDL();
    }

    for (auto const& demon : gddl) {
        int id = demon["ID"].asInt().unwrapOr(0);
        switch (id) {
            case 1: id = 14; break; // Clubstep
            case 2: id = 18; break; // Theory of Everything 2
            case 3: id = 20; break; // Deadlocked
        }
        if (!demon["Tier"].isNull()) {
            auto tier = demon["Tier"].asDouble().unwrapOr(0.0);
            (id < 100 ? GDDL_MAIN : GDDL).push_back({
                id,
                tier,
                !demon["Enjoyment"].isNull() ? demon["Enjoyment"].asDouble().unwrapOr(0.0) : -999.0,
                DIFFICULTY_INDICES[(int)round(tier)]
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

std::vector<matjson::Value> DemonsInBetween::parseGDDL(std::string const& data) {
    // MAKE SURE WE ARE NOT SPLITTING THE COMMAS IN THE QUOTATION MARKS
    auto lines = string::split(data, "\n");
    auto keys = string::split(lines[0].substr(1, lines[0].size() - 2), "\",\"");
    std::vector<matjson::Value> demons;
    for (size_t i = 1; i < lines.size(); i++) {
        auto values = string::split(lines[i].substr(1, lines[i].size() - 2), "\",\"");
        matjson::Value demon;
        for (size_t j = 0; j < keys.size(); j++) {
            auto key = keys[j];
            auto value = values[j];
            if (key == "ID") demon[key] = numFromString<int>(value).unwrapOr(-1);
            else if (key == "Tier") demon[key] = value != "" ? round(numFromString<double>(value).unwrapOr(0.0) * 100) / 100 : matjson::Value(nullptr);
            else if (key == "Enjoyment") demon[key] = value != "" ? round(numFromString<double>(value).unwrapOr(-999.0) * 100) / 100 : matjson::Value(nullptr);
            else demon[key] = value;
        }
        demons.push_back(demon);
    }

    return demons;
}

LadderDemon DemonsInBetween::demonForLevel(GJGameLevel* level, bool main) {
    auto& gddl = main ? GDDL_MAIN : GDDL;
    auto demon = std::find_if(gddl.begin(), gddl.end(), [level, main](auto const& d) {
        return d.id == level->m_levelID.value();
    });
    return demon == gddl.end() ? LadderDemon { 0, 0.0, 0.0, 0 } : *demon;
}

void DemonsInBetween::refreshDemonForLevel(
    EventListener<web::WebTask>&& listenerRef, GJGameLevel* level, std::function<void(LadderDemon const&)> const& callback, bool showPopup
) {
    auto levelID = level->m_levelID.value();
    auto demon = std::find_if(GDDL.begin(), GDDL.end(), [levelID](auto const& d) {
        return d.id == levelID;
    });
    if (demon == GDDL.end()) return;

    auto&& listener = std::move(listenerRef);
    listener.bind([callback, demon, showPopup](web::WebTask::Event* e) {
        if (auto res = e->getValue()) {
            REFRESHED_DEMONS.insert(demon->id);

            if (!res->ok()) {
                if (showPopup) queueInMainThread([] {
                    FLAlertLayer::create("Refresh Failed", "Failed to refresh difficulty. Please try again later.", "OK")->show();
                });
                log::error("Failed to update demon with ID {}", demon->id);
                return;
            }

            auto json = res->json().unwrapOr(matjson::Value());
            if (json["Rating"].isNull()) {
                if (showPopup) queueInMainThread([] {
                    FLAlertLayer::create("Refresh Failed", "Failed to refresh difficulty. Please try again later.", "OK")->show();
                });
                log::error("Failed to update demon with ID {}", demon->id);
                return;
            }

            demon->difficulty = DIFFICULTY_INDICES[(int)round(json["Rating"].asDouble().unwrapOr(0.0))];
            demon->enjoyment = !json["Enjoyment"].isNull() ? round(json["Enjoyment"].asDouble().unwrapOr(-999.0) * 100) / 100 : -999.0;

            if (!GDDL_CACHE["list"].isArray()) {
                if (showPopup) queueInMainThread([] {
                    FLAlertLayer::create("Refresh Failed", "Failed to refresh difficulty. Please try again later.", "OK")->show();
                });
                log::error("Failed to update demon with ID {}", demon->id);
                return;
            }

            auto cacheArray = GDDL_CACHE["list"].isArray() ? GDDL_CACHE["list"].asArray().unwrap() : std::vector<matjson::Value>();
            auto it = std::find_if(cacheArray.begin(), cacheArray.end(), [demon](matjson::Value const& d) {
                return d["ID"].asInt().unwrapOr(0) == demon->id;
            });
            auto tier = round(json["Rating"].asDouble().unwrapOr(0.0) * 100) / 100;
            auto enjoyment = !json["Enjoyment"].isNull() ? round(json["Enjoyment"].asDouble().unwrapOr(-999.0) * 100) / 100 : matjson::Value(nullptr);
            if (it != cacheArray.end()) {
                auto cacheDemon = *it;
                cacheDemon["Tier"] = tier;
                cacheDemon["Enjoyment"] = enjoyment;
                GDDL_CACHE_CHANGED = true;
            } else {
                auto cacheDemon = matjson::makeObject({
                    { "ID", demon->id },
                    { "Tier", tier },
                    { "Enjoyment", enjoyment }
                });

                auto lower = std::find_if(cacheArray.rbegin(), cacheArray.rend(), [demon](matjson::Value const& d) {
                    return d["ID"].asInt().unwrapOr(0) < demon->id;
                });
                auto upper = std::find_if(cacheArray.begin(), cacheArray.end(), [demon](matjson::Value const& d) {
                    return d["ID"].asInt().unwrapOr(0) > demon->id;
                });
                if (lower != cacheArray.rend()) cacheArray.insert(lower.base() + 1, cacheDemon);
                else if (upper != cacheArray.end()) cacheArray.insert(upper, cacheDemon);
                else cacheArray.push_back(cacheDemon);

                GDDL_CACHE_CHANGED = true;
            }

            if (showPopup) queueInMainThread([] {
                FLAlertLayer::create("Refresh Success", "Successfully refreshed difficulty.", "OK")->show();
            });
            log::info("Updated demon with ID {}", demon->id);

            if (callback) callback(*demon);
        }
    });

    listener.setFilter(web::WebRequest().get(fmt::format("https://gdladder.com/api/level/{}", demon->id).c_str()));
}

std::string DemonsInBetween::infoForLevel(GJGameLevel* level, LadderDemon const& demon) {
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

    return fmt::format(
        "<cy>{}</c>\n"
        "<cg>Tier</c>: {}\n"
        "<cl>Enjoyment</c>: {}\n"
        "<cp>Difficulty</c>: {}\n"
        "<co>Original Difficulty</c>: {}",
        std::string(level->m_levelName),
        demon.tier,
        demon.enjoyment >= 0.0 ? fmt::format("{}", demon.enjoyment) : "N/A",
        difficulty,
        originalDifficulty
    );
}

CCSprite* DemonsInBetween::spriteForDifficulty(GJDifficultySprite* difficultySprite, int difficulty, GJDifficultyName name, GJFeatureState state) {
    auto glow = state == GJFeatureState::Legendary ? "_4" : state == GJFeatureState::Mythic ? "_5" : "";
    auto sprite = CCSprite::createWithSpriteFrameName((name == GJDifficultyName::Long ?
        fmt::format("DIB_{:02d}{}_btn2_001.png"_spr, difficulty, glow) : fmt::format("DIB_{:02d}{}_btn_001.png"_spr, difficulty, glow)).c_str());
    sprite->setPosition(difficultySprite->getPosition() + (name == GJDifficultyName::Long ?
        LONG_OFFSETS[(size_t)difficulty - 1] : SHORT_OFFSETS[(size_t)difficulty - 1]));
    sprite->setID("between-difficulty-sprite"_spr);
    return sprite;
}

GJFeatureState DemonsInBetween::stateForLevel(GJGameLevel* level) {
    auto state = level->m_featured ? (GJFeatureState)(level->m_isEpic + 1) : GJFeatureState::None;
    if (state == GJFeatureState::Legendary && !Mod::get()->getSettingValue<bool>("enable-legendary")) state = GJFeatureState::None;
    else if (state == GJFeatureState::Mythic && !Mod::get()->getSettingValue<bool>("enable-mythic")) state = GJFeatureState::None;
    return state;
}

GJSearchObject* DemonsInBetween::searchObjectForPage(int page) {
    SEARCH_RESULTS.clear();

    for (size_t i = 0; i < GDDL.size(); i++) {
        if (GDDL[i].difficulty == DIFFICULTY) SEARCH_RESULTS.push_back(std::to_string(GDDL[i].id));
    }

    int size = SEARCH_RESULTS.size();
    MAX_PAGE = (size % 10 == 0 ? size : size + (10 - (size % 10))) / 10 - 1;

    return GJSearchObject::create(SearchType::MapPackOnClick, string::join(std::vector<std::string>(
        SEARCH_RESULTS.begin() + page * 10,
        SEARCH_RESULTS.begin() + std::min((page + 1) * 10, size)
    ), ","));
}
