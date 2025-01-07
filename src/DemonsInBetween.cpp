#include "DemonsInBetween.hpp"

using namespace geode::prelude;

LadderDemon& DemonsInBetween::demonForLevel(int levelID, bool main) {
    static LadderDemon empty = { 0, 0.0, 0.0, 0 };
    auto& gddl = main ? GDDL_MAIN : GDDL;
    auto demon = std::find_if(gddl.begin(), gddl.end(), [levelID, main](auto const& d) {
        return d.id == levelID;
    });
    return demon == gddl.end() ? empty : *demon;
}

CCSpriteFrame* DemonsInBetween::spriteFrameForDifficulty(int difficulty, GJDifficultyName name, GJFeatureState state) {
    return CCSpriteFrameCache::get()->spriteFrameByName(fmt::format(
        "DIB_{:02d}{}_btn{}_001.png"_spr,
        difficulty,
        state == GJFeatureState::Legendary ? "_4" : state == GJFeatureState::Mythic ? "_5" : "",
        name == GJDifficultyName::Long ? "2" : ""
    ).c_str());
}

CCSprite* DemonsInBetween::spriteForDifficulty(GJDifficultySprite* difficultySprite, int difficulty, GJDifficultyName name, GJFeatureState state) {
    auto sprite = CCSprite::createWithSpriteFrame(spriteFrameForDifficulty(difficulty, name, state));
    sprite->setPosition(difficultySprite->getPosition() + (name == GJDifficultyName::Long ?
        LONG_OFFSETS[(size_t)difficulty - 1] : SHORT_OFFSETS[(size_t)difficulty - 1]));
    sprite->setID("between-difficulty-sprite"_spr);
    return sprite;
}

int DemonsInBetween::difficultyForDemonDifficulty(int demonDifficulty) {
    switch (demonDifficulty) {
        case 0: return 11;
        case 3: return 4;
        case 4: return 7;
        case 5: return 15;
        case 6: return 20;
        default: return 0;
    }
}

GJFeatureState DemonsInBetween::stateForLevel(GJGameLevel* level) {
    auto state = level->m_featured ? (GJFeatureState)(level->m_isEpic + 1) : GJFeatureState::None;
    if (state == GJFeatureState::Legendary && !Mod::get()->getSettingValue<bool>("enable-legendary")) state = GJFeatureState::None;
    else if (state == GJFeatureState::Mythic && !Mod::get()->getSettingValue<bool>("enable-mythic")) state = GJFeatureState::None;
    return state;
}

void DemonsInBetween::loadDemonForLevel(
    EventListener<web::WebTask>&& listenerRef, int levelID, bool main,
    std::function<void(LadderDemon&)> const& callback, std::function<void()> const& preCallback
) {
    if (LEVELS_LOADED.contains(levelID)) return;

    auto&& listener = std::move(listenerRef);
    listener.bind([callback, levelID, main, preCallback](web::WebTask::Event* e) {
        if (auto res = e->getValue()) {
            LEVELS_LOADED.insert(levelID);

            if (!res->ok()) return;

            auto json = res->json().unwrapOr(matjson::Value());
            if (!json.contains("Rating") || json["Rating"].isNull()) return;

            auto rating = round(json["Rating"].asDouble().unwrapOr(0.0) * 100) / 100;
            auto difficulty = DemonsInBetween::DIFFICULTY_INDICES[(int)round(rating)];
            auto enjoyment = !json.contains("Enjoyment") || json["Enjoyment"].isNull() ? -999.0 : round(json["Enjoyment"].asDouble().unwrapOr(-999.0) * 100) / 100;

            auto& gddl = main ? GDDL_MAIN : GDDL;
            gddl.push_back({ levelID, rating, enjoyment, difficulty });
            preCallback();
            queueInMainThread([callback, levelID, main] { callback(demonForLevel(levelID, main)); });
        }
    });

    listener.setFilter(web::WebRequest().get(fmt::format("https://gdladder.com/api/level/{}", levelID)));
}

void DemonsInBetween::searchObjectForPage(
    EventListener<web::WebTask>&& listenerRef, int page, bool refresh,
    std::function<void(GJSearchObject*)> const& callback, std::function<void()> const& preCallback
) {
    auto glm = GameLevelManager::get();
    auto searchCache = static_cast<CCDictionary*>(glm->getUserObject("search-cache"_spr));
    if (!searchCache) {
        searchCache = CCDictionary::create();
        glm->setUserObject("search-cache"_spr, searchCache);
    }

    auto searchKey = fmt::format("{}_{}", DIFFICULTY, page);
    if (!refresh && searchCache->objectForKey(searchKey)) {
        SEARCH_SIZE = SEARCH_SIZES[DIFFICULTY];
        MAX_PAGE = (SEARCH_SIZE - 1) / 10;
        preCallback();
        callback(static_cast<GJSearchObject*>(searchCache->objectForKey(searchKey)));
        return;
    }

    auto tierBound = TIER_BOUNDS[DIFFICULTY];

    auto&& listener = std::move(listenerRef);
    listener.bind([callback, page, preCallback](web::WebTask::Event* e) {
        if (auto res = e->getValue()) {
            if (!res->ok()) return;

            auto json = res->json().unwrapOr(matjson::Value());
            SEARCH_SIZE = json["total"].asInt().unwrapOr(0);
            SEARCH_SIZES[DIFFICULTY] = SEARCH_SIZE;
            MAX_PAGE = (SEARCH_SIZE - 1) / 10;

            std::vector<std::string> levels;
            if (json.contains("levels") && json["levels"].isArray()) {
                for (auto const& level : json["levels"].asArray().unwrap()) {
                    if (level.contains("ID") && level["ID"].isNumber()) levels.push_back(std::to_string(level["ID"].asInt().unwrap()));
                }
            }

            preCallback();
            queueInMainThread([callback, levels, page] {
                auto searchObject = GJSearchObject::create(SearchType::MapPackOnClick, string::join(levels, ","));
                if (auto searchCache = static_cast<CCDictionary*>(GameLevelManager::get()->getUserObject("search-cache"_spr)))
                    searchCache->setObject(searchObject, fmt::format("{}_{}", DIFFICULTY, page));
                callback(searchObject);
            });
        }
    });

    listener.setFilter(web::WebRequest().get(fmt::format("https://gdladder.com/api/level/search/?lowTier={}&highTier={}&chunk=10&page={}",
        tierBound.first, tierBound.second, page + 1)));
}
