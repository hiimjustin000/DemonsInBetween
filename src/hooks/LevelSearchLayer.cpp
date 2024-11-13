#include "../classes/DIBSearchPopup.hpp"
#include "../DemonsInBetween.hpp"

using namespace geode::prelude;

#include <Geode/modify/LevelSearchLayer.hpp>
class $modify(DIBLevelSearchLayer, LevelSearchLayer) {
    struct Fields {
        CCMenuItemSpriteExtra* m_quickSearchButton;
    };

    bool init(int searchType) {
        if (!LevelSearchLayer::init(searchType)) return false;

        m_demonTypeButton->setPositionY(m_demonTypeButton->getPositionY() + 12.0f);
        auto quickSearchSprite = CCSprite::createWithSpriteFrameName("GJ_plusBtn_001.png");
        quickSearchSprite->setScale(0.45f);
        auto f = m_fields.self();
        f->m_quickSearchButton = CCMenuItemSpriteExtra::create(quickSearchSprite, this, menu_selector(DIBLevelSearchLayer::onQuickSearch));
        f->m_quickSearchButton->setPosition(m_demonTypeButton->getPosition() - CCPoint { 0.0f, 23.0f });
        f->m_quickSearchButton->setID("quick-search-button"_spr);
        f->m_quickSearchButton->setVisible(m_demonTypeButton->isVisible());
        m_demonTypeButton->getParent()->addChild(f->m_quickSearchButton);

        return true;
    }

    void onQuickSearch(CCObject* sender) {
        DIBSearchPopup::create()->show();
    }

    void toggleDifficulty(CCObject* sender) {
        LevelSearchLayer::toggleDifficulty(sender);

        m_fields->m_quickSearchButton->setVisible(m_demonTypeButton->isVisible());
    }
};
