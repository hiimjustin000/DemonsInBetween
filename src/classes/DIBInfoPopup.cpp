#include "../DemonsInBetween.hpp"
#include "DIBInfoPopup.hpp"

using namespace geode::prelude;

#define GDDL_URL "https://docs.google.com/spreadsheets/d/1qKlWKpDkOpU1ZF6V6xGfutDY2NvcA8MNPnsv6GBkKPQ/gviz/tq?tqx=out:csv&sheet=GDDL"

DIBInfoPopup* DIBInfoPopup::create() {
    auto ret = new DIBInfoPopup();
    if (ret->initAnchored(380.0f, 210.0f, "GJ_square02.png")) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool DIBInfoPopup::setup() {
    m_demonSprites = CCArray::create();
    m_demonSprites->retain();

    m_demonClassicLabels = CCArray::create();
    m_demonClassicLabels->retain();

    m_demonPlatformerLabels = CCArray::create();
    m_demonPlatformerLabels->retain();

    m_loadingCircle = LoadingCircle::create();
    m_loadingCircle->setContentSize({ 380.0f, 210.0f });
    m_loadingCircle->setParentLayer(m_mainLayer);
    m_loadingCircle->m_sprite->setPosition({ 190.0f, 105.0f });
    m_loadingCircle->show();

    auto okButton = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("OK", 0, 0, 1.0f, false, "goldFont.fnt", "GJ_button_01.png", 0.0f),
        this, menu_selector(DIBInfoPopup::onClose)
    );
    okButton->setPosition({ 190.0f, 25.0f });
    m_buttonMenu->addChild(okButton);

    m_closeBtn->setVisible(false);

    setKeyboardEnabled(true);

    if (!CACHED_DEMONS.empty()) {
        setupDemonInfo();
        return true;
    }

    m_listener.bind([this](web::WebTask::Event* e) {
        if (auto res = e->getValue()) {
            if (!res->ok()) return;

            auto data = res->string().unwrapOr("");
            auto lines = string::split(data, "\n");
            auto keys = string::split(lines[0].substr(1, lines[0].size() - 2), "\",\"");
            for (size_t i = 1; i < lines.size(); i++) {
                auto values = string::split(lines[i].substr(1, lines[i].size() - 2), "\",\"");
                CachedLadderDemon demon;
                for (size_t j = 0; j < keys.size(); j++) {
                    auto key = keys[j];
                    auto value = values[j];
                    if (key == "ID") demon.id = numFromString<int>(value).unwrapOr(-1);
                    else if (key == "Tier") demon.difficulty = value != "" ?
                        DemonsInBetween::DIFFICULTY_INDICES[(int)round(numFromString<double>(value).unwrapOr(0.0))] : 0;
                }
                CACHED_DEMONS.push_back(demon);
            }

            queueInMainThread([this] { setupDemonInfo(); });
        }
    });

    m_listener.setFilter(web::WebRequest().get(GDDL_URL));

    return true;
}

void DIBInfoPopup::setupDemonInfo() {
    m_loadingCircle->setVisible(false);
    m_loadingCircle->fadeAndRemove();
    m_loadingCircle->release();
    m_loadingCircle = nullptr;

    auto glm = GameLevelManager::get();
    auto gsm = GameStatsManager::get();
    auto classicCompleted = 0;
    auto platformerCompleted = 0;
     for (auto [onlineID, level] : CCDictionaryExt<std::string, GJGameLevel*>(glm->m_onlineLevels)) {
        if (level->m_stars.value() < 10 || level->m_normalPercent.value() < 100 || !gsm->hasCompletedLevel(level)) continue;

        auto isPlatformer = level->m_levelLength == 5;
        if (isPlatformer) platformerCompleted++;
        else classicCompleted++;

        auto levelID = level->m_levelID.value();
        auto demon = std::find_if(CACHED_DEMONS.begin(), CACHED_DEMONS.end(), [levelID](auto const& d) {
            return d.id == levelID;
        });
        if (demon == CACHED_DEMONS.end() || demon->id == 0 || demon->difficulty == 0) {
            auto& completionCount = isPlatformer ? m_completionCountPlatformer : m_completionCountClassic;
            auto difficulty = DemonsInBetween::difficultyForDemonDifficulty(level->m_demonDifficulty);
            if (difficulty > 0) completionCount[difficulty - 1]++;
            continue;
        }

        auto& completionCount = isPlatformer ? m_completionCountPlatformer : m_completionCountClassic;
        completionCount[demon->difficulty - 1]++;
    }

    auto classicLabel = CCLabelBMFont::create(fmt::format("Classic: {}", classicCompleted).c_str(), "goldFont.fnt");
    classicLabel->setScale(0.7f);
    m_mainLayer->addChild(classicLabel);

    auto platformerLabel = CCLabelBMFont::create(fmt::format("Platformer: {}", platformerCompleted).c_str(), "goldFont.fnt");
    platformerLabel->setScale(0.7f);
    platformerLabel->setColor({ 255, 200, 255 });
    m_mainLayer->addChild(platformerLabel);

    auto titleLabels = CCArray::create();
    titleLabels->addObject(classicLabel);
    titleLabels->addObject(platformerLabel);

    GameToolbox::alignItemsHorisontally(titleLabels, 30.0f, { 190.0f, 190.0f }, false);

    auto bottomRightMenu = CCMenu::create();
    bottomRightMenu->setPosition({ 368.0f, 14.0f });
    bottomRightMenu->setAnchorPoint({ 1.0f, 0.0f });
    bottomRightMenu->setContentSize({ 40.0f, 100.0f });
    bottomRightMenu->setScale(0.4f);
    bottomRightMenu->setLayout(
        ColumnLayout::create()
            ->setAxisAlignment(AxisAlignment::Start)
            ->setCrossAxisLineAlignment(AxisAlignment::End)
            ->setGap(10.0f)
    );
    m_mainLayer->addChild(bottomRightMenu, 1);

    auto weeklyCompleted = 0;
    auto eventCompleted = 0;
    auto gauntletCompleted = 0;

    for (auto [dailyID, dailyLevel] : CCDictionaryExt<std::string, GJGameLevel*>(glm->m_dailyLevels)) {
        if (dailyLevel->m_stars.value() < 10 || dailyLevel->m_normalPercent.value() < 100 || !gsm->hasCompletedLevel(dailyLevel)) continue;
        if (dailyLevel->m_dailyID >= 200000) eventCompleted++;
        else if (dailyLevel->m_dailyID >= 100000) weeklyCompleted++;
    }

    for (auto [levelID, gauntletLevel] : CCDictionaryExt<std::string, GJGameLevel*>(glm->m_gauntletLevels)) {
        if (gauntletLevel->m_stars.value() < 10 || gauntletLevel->m_normalPercent.value() < 100 || !gsm->hasCompletedLevel(gauntletLevel)) continue;
        gauntletCompleted++;
    }

    auto gauntletLabel = CCLabelBMFont::create(fmt::format("Gauntlet: {}", gauntletCompleted).c_str(), "goldFont.fnt");
    gauntletLabel->setAnchorPoint({ 1.0f, 0.0f });
    bottomRightMenu->addChild(gauntletLabel);

    auto eventLabel = CCLabelBMFont::create(fmt::format("Event: {}", eventCompleted).c_str(), "goldFont.fnt");
    eventLabel->setAnchorPoint({ 1.0f, 0.0f });
    bottomRightMenu->addChild(eventLabel);

    auto weeklyLabel = CCLabelBMFont::create(fmt::format("Weekly: {}", weeklyCompleted).c_str(), "goldFont.fnt");
    weeklyLabel->setAnchorPoint({ 1.0f, 0.0f });
    bottomRightMenu->addChild(weeklyLabel);

    bottomRightMenu->updateLayout();

    auto prevButton = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_arrow_01_001.png", 1.0f, [this](auto) {
        loadPage(m_page - 1);
    });
    prevButton->setPosition({ -20.0f, 105.0f });
    m_buttonMenu->addChild(prevButton);

    auto nextButtonSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
    nextButtonSprite->setFlipX(true);
    auto nextButton = CCMenuItemExt::createSpriteExtra(nextButtonSprite, [this](auto) {
        loadPage(m_page + 1);
    });
    nextButton->setPosition({ 400.0f, 105.0f });
    m_buttonMenu->addChild(nextButton);

    m_loaded = true;

    loadPage(0);
}

void DIBInfoPopup::loadPage(int page) {
    m_page = (4 + (page % 4)) % 4;

    auto sfc = CCSpriteFrameCache::get();
    for (int i = 0; i < 5; i++) {
        auto demonFrame = fmt::format("DIB_{:02d}_btn2_001.png"_spr, m_page * 5 + i + 1);
        if (m_demonSprites->count() <= i) {
            auto demonSprite = CCSprite::createWithSpriteFrameName(demonFrame.c_str());
            demonSprite->setPosition(CCPoint { 50.0f + i * 70.0f, 140.0f } + DemonsInBetween::LONG_OFFSETS[m_page * 5 + i]);
            m_mainLayer->addChild(demonSprite);
            m_demonSprites->addObject(demonSprite);
        }
        else {
            auto demonSprite = static_cast<CCSprite*>(m_demonSprites->objectAtIndex(i));
            demonSprite->setDisplayFrame(sfc->spriteFrameByName(demonFrame.c_str()));
            demonSprite->setPosition(CCPoint { 50.0f + i * 70.0f, 140.0f } + DemonsInBetween::LONG_OFFSETS[m_page * 5 + i]);
        }

        auto classicString = fmt::format("{}", m_completionCountClassic[m_page * 5 + i]);
        if (m_demonClassicLabels->count() <= i) {
            auto classicLabel = CCLabelBMFont::create(classicString.c_str(), "goldFont.fnt");
            classicLabel->setScale(0.6f);
            classicLabel->setPosition({ 50.0f + i * 70.0f, 90.0f });
            m_mainLayer->addChild(classicLabel);
            m_demonClassicLabels->addObject(classicLabel);
        }
        else static_cast<CCLabelBMFont*>(m_demonClassicLabels->objectAtIndex(i))->setString(classicString.c_str());

        auto platformerString = fmt::format("{}", m_completionCountPlatformer[m_page * 5 + i]);
        if (m_demonPlatformerLabels->count() <= i) {
            auto platformerLabel = CCLabelBMFont::create(platformerString.c_str(), "goldFont.fnt");
            platformerLabel->setScale(0.6f);
            platformerLabel->setPosition({ 50.0f + i * 70.0f, 64.0f });
            platformerLabel->setColor({ 255, 200, 255 });
            m_mainLayer->addChild(platformerLabel);
            m_demonPlatformerLabels->addObject(platformerLabel);
        }
        else static_cast<CCLabelBMFont*>(m_demonPlatformerLabels->objectAtIndex(i))->setString(platformerString.c_str());
    }
}

void DIBInfoPopup::onClose(CCObject*) {
    setKeypadEnabled(false);
    setTouchEnabled(false);
    setKeyboardEnabled(false);
    removeFromParent();
}

void DIBInfoPopup::keyDown(enumKeyCodes key) {
    if (!m_loaded) {
        Popup<>::keyDown(key);
        return;
    }

    switch (key) {
        case KEY_Left: case CONTROLLER_Left:
            loadPage(m_page - 1);
            break;
        case KEY_Right: case CONTROLLER_Right:
            loadPage(m_page + 1);
            break;
        default:
            Popup<>::keyDown(key);
            break;
    }
}

DIBInfoPopup::~DIBInfoPopup() {
    CC_SAFE_RELEASE(m_demonSprites);
    CC_SAFE_RELEASE(m_demonClassicLabels);
    CC_SAFE_RELEASE(m_demonPlatformerLabels);
    CC_SAFE_RELEASE(m_loadingCircle);
}
