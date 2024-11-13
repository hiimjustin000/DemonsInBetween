#include "../DemonsInBetween.hpp"

using namespace geode::prelude;

#include <Geode/modify/LevelBrowserLayer.hpp>
class $modify(DIBLevelBrowserLayer, LevelBrowserLayer) {
    struct Fields {
        int m_currentPage;
    };

    void loadLevelsFinished(CCArray* levels, const char* key, int type) override {
        LevelBrowserLayer::loadLevelsFinished(levels, key, type);

        if (DemonsInBetween::SEARCHING) {
            auto f = m_fields.self();
            m_leftArrow->setVisible(f->m_currentPage > 0);
            m_rightArrow->setVisible(f->m_currentPage < DemonsInBetween::MAX_PAGE);
            m_pageBtn->setVisible(true);
        }
    }

    void setupPageInfo(gd::string pageInfo, const char* key) override {
        LevelBrowserLayer::setupPageInfo(pageInfo, key);

        if (DemonsInBetween::SEARCHING) {
            int size = DemonsInBetween::SEARCH_RESULTS.size();
            auto f = m_fields.self();
            m_countText->setString(fmt::format("{} to {} of {}", f->m_currentPage * 10 + 1, std::min(size, (f->m_currentPage + 1) * 10), size).c_str());
            m_countText->limitLabelWidth(100.0f, 0.6f, 0.0f);
            m_pageText->setString(std::to_string(f->m_currentPage + 1).c_str());
            m_pageText->limitLabelWidth(32.0f, 0.8f, 0.0f);
        }
    }

    void onBack(CCObject* sender) override {
        LevelBrowserLayer::onBack(sender);

        if (DemonsInBetween::SEARCHING) {
            DemonsInBetween::SEARCHING = false;
            DemonsInBetween::DIFFICULTY = 0;
        }
    }

    void setIDPopupClosed(SetIDPopup* popup, int page) override {
        LevelBrowserLayer::setIDPopupClosed(popup, page);

        if (DemonsInBetween::SEARCHING && popup->getTag() != 4) {
            auto f = m_fields.self();
            f->m_currentPage = std::min(page - 1, DemonsInBetween::MAX_PAGE);
            loadPage(DemonsInBetween::searchObjectForPage(f->m_currentPage));
        }
    }

    void onGoToPage(CCObject* sender) {
        LevelBrowserLayer::onGoToPage(sender);

        if (DemonsInBetween::SEARCHING) {
            if (auto popup = CCScene::get()->getChildByType<SetIDPopup>(0)) {
                popup->m_value = m_fields->m_currentPage + 1;
                popup->updateTextInputLabel();
            }
        }
    }

    void onNextPage(CCObject* sender) {
        LevelBrowserLayer::onNextPage(sender);

        if (DemonsInBetween::SEARCHING) loadPage(DemonsInBetween::searchObjectForPage(++m_fields->m_currentPage));
    }

    void onPrevPage(CCObject* sender) {
        LevelBrowserLayer::onPrevPage(sender);

        if (DemonsInBetween::SEARCHING) loadPage(DemonsInBetween::searchObjectForPage(--m_fields->m_currentPage));
    }
#ifdef GEODE_IS_MACOS // In the Geometry Dash binary for macOS, onNextPage and onPrevPage are inlined into keyDown
    void keyDown(enumKeyCodes key) override {
        LevelBrowserLayer::keyDown(key);

        if (DemonsInBetween::SEARCHING) {
            switch (key) {
                case KEY_Left: case CONTROLLER_Left:
                    if (m_leftArrow && m_leftArrow->isVisible() && m_leftArrow->isEnabled())
                        loadPage(DemonsInBetween::searchObjectForPage(--m_fields->m_currentPage));
                    break;
                case KEY_Right: case CONTROLLER_Right:
                    if (m_rightArrow && m_rightArrow->isVisible() && m_rightArrow->isEnabled())
                        loadPage(DemonsInBetween::searchObjectForPage(++m_fields->m_currentPage));
                    break;
                default: break;
            }
        }
    }
#endif
};
