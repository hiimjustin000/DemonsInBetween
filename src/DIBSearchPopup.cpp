#include "DIBSearchPopup.hpp"

DIBSearchPopup* DIBSearchPopup::create() {
    auto ret = new DIBSearchPopup();
    if (ret->initAnchored(350.0f, 280.0f)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool DIBSearchPopup::setup() {
    setTitle("Quick Search");
    m_noElasticity = true;

    auto menuRow1 = CCMenu::create();
    menuRow1->setLayout(RowLayout::create()->setAxisAlignment(AxisAlignment::Even));
    menuRow1->setPosition(175.0f, 220.0f);
    menuRow1->setContentSize({ 350.0f, 60.0f });
    m_mainLayer->addChild(menuRow1);

    createDifficultyButton(menuRow1, 1);
    createDifficultyButton(menuRow1, 2);
    createDifficultyButton(menuRow1, 3);
    createDifficultyButton(menuRow1, 4);
    createDifficultyButton(menuRow1, 5);

    menuRow1->updateLayout();

    auto menuRow2 = CCMenu::create();
    menuRow2->setLayout(RowLayout::create()->setAxisAlignment(AxisAlignment::Even));
    menuRow2->setPosition(175.0f, 160.0f);
    menuRow2->setContentSize({ 350.0f, 60.0f });
    m_mainLayer->addChild(menuRow2);

    createDifficultyButton(menuRow2, 6);
    createDifficultyButton(menuRow2, 7);
    createDifficultyButton(menuRow2, 8);
    createDifficultyButton(menuRow2, 9);
    createDifficultyButton(menuRow2, 10);

    menuRow2->updateLayout();

    auto menuRow3 = CCMenu::create();
    menuRow3->setLayout(RowLayout::create()->setAxisAlignment(AxisAlignment::Even));
    menuRow3->setPosition(175.0f, 100.0f);
    menuRow3->setContentSize({ 350.0f, 60.0f });
    m_mainLayer->addChild(menuRow3);

    createDifficultyButton(menuRow3, 11);
    createDifficultyButton(menuRow3, 12);
    createDifficultyButton(menuRow3, 13);
    createDifficultyButton(menuRow3, 14);
    createDifficultyButton(menuRow3, 15);

    menuRow3->updateLayout();

    auto menuRow4 = CCMenu::create();
    menuRow4->setLayout(RowLayout::create()->setAxisAlignment(AxisAlignment::Even));
    menuRow4->setPosition(175.0f, 40.0f);
    menuRow4->setContentSize({ 350.0f, 60.0f });
    m_mainLayer->addChild(menuRow4);

    createDifficultyButton(menuRow4, 16);
    createDifficultyButton(menuRow4, 17);
    createDifficultyButton(menuRow4, 18);
    createDifficultyButton(menuRow4, 19);
    createDifficultyButton(menuRow4, 20);

    menuRow4->updateLayout();

    return true;
}

void DIBSearchPopup::createDifficultyButton(CCMenu* menu, int difficulty) {
    auto button = CCMenuItemExt::createSpriteExtraWithFrameName(fmt::format("DIB_{:02d}_btn2_001.png"_spr, difficulty).c_str(), 1.0f, [this, difficulty](auto) {
        DemonsInBetween::DIFFICULTY = difficulty;
        DemonsInBetween::SEARCHING = true;
        CCDirector::sharedDirector()->pushScene(CCTransitionFade::create(0.5f, LevelBrowserLayer::scene(DemonsInBetween::searchObjectForPage(0))));
    });
    button->setTag(difficulty);
    menu->addChild(button);
}
