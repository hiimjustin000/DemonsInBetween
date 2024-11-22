#include <random>
#include "../DemonsInBetween.hpp"

using namespace geode::prelude;

#include <Geode/modify/LevelBrowserLayer.hpp>
class $modify(DIBLevelBrowserLayer, LevelBrowserLayer) {
    struct Fields {
        EventListener<web::WebTask> m_listener;
        int m_currentPage;
        bool m_loadingPage;
    };

    static void onModify(auto& self) {
        (void)self.setHookPriority("LevelBrowserLayer::init", -1); // betterinfo is 0 D:
        (void)self.setHookPriority("LevelBrowserLayer::setupPageInfo", -1); // betterinfo is 0 D:
        (void)self.setHookPriority("LevelBrowserLayer::loadPage", -1); // betterinfo is 0 D:
    }

    bool init(GJSearchObject* searchObject) {
        if (!LevelBrowserLayer::init(searchObject)) return false;

        if (auto pageMenu = getChildByID("page-menu")) {
            if (auto randomButton = static_cast<CCMenuItemSpriteExtra*>(pageMenu->getChildByID("cvolton.betterinfo/random-button")))
                randomButton->m_pfnSelector = menu_selector(DIBLevelBrowserLayer::onBetterInfoRandom);
            if (auto lastButton = static_cast<CCMenuItemSpriteExtra*>(pageMenu->getChildByID("cvolton.betterinfo/last-button")))
                lastButton->m_pfnSelector = menu_selector(DIBLevelBrowserLayer::onBetterInfoLast);
        }
        if (auto searchMenu = getChildByID("search-menu")) {
            if (auto firstButton = static_cast<CCMenuItemSpriteExtra*>(searchMenu->getChildByID("cvolton.betterinfo/first-button")))
                firstButton->m_pfnSelector = menu_selector(DIBLevelBrowserLayer::onBetterInfoFirst);
        }

        updatePageButtons();

        return true;
    }

    void onBetterInfoRandom(CCObject* sender) {
        static std::mt19937 mt(std::random_device{}());
        m_fields->m_currentPage = std::uniform_int_distribution<int>(1, DemonsInBetween::MAX_PAGE)(mt);
        switchToPage();
    }

    void onBetterInfoLast(CCObject* sender) {
        m_fields->m_currentPage = DemonsInBetween::MAX_PAGE;
        switchToPage();
    }

    void onBetterInfoFirst(CCObject* sender) {
        m_fields->m_currentPage = 0;
        switchToPage();
    }

    void switchToPage(bool refresh = false) {
        if (!DemonsInBetween::SEARCHING) return;

        auto f = m_fields.self();
        f->m_loadingPage = true;
        DemonsInBetween::searchObjectForPage(std::move(f->m_listener), f->m_currentPage, refresh, [this](GJSearchObject* obj) {
            m_fields->m_loadingPage = false;
            loadPage(obj);
        });
    }

    void updatePageButtons() {
        auto f = m_fields.self();

        m_leftArrow->setVisible(f->m_currentPage > 0);
        m_rightArrow->setVisible(f->m_currentPage < DemonsInBetween::MAX_PAGE);
        m_pageBtn->setVisible(DemonsInBetween::SEARCH_SIZE > 10);

        if (auto pageMenu = getChildByID("page-menu")) {
            if (auto randomButton = pageMenu->getChildByID("cvolton.betterinfo/random-button"))
                randomButton->setVisible(DemonsInBetween::SEARCH_SIZE > 10);
            if (auto lastButton = pageMenu->getChildByID("cvolton.betterinfo/last-button"))
                lastButton->setVisible(f->m_currentPage < DemonsInBetween::MAX_PAGE);
        }
        if (auto searchMenu = getChildByID("search-menu")) {
            if (auto firstButton = searchMenu->getChildByID("cvolton.betterinfo/first-button"))
                firstButton->setVisible(f->m_currentPage > 0);
        }
    }

    void loadPage(GJSearchObject* searchObject) {
        LevelBrowserLayer::loadPage(searchObject);

        if (!DemonsInBetween::SEARCHING || m_fields->m_loadingPage) return;

        updatePageButtons();
    }

    void loadLevelsFinished(CCArray* levels, const char* key, int type) override {
        auto f = m_fields.self();
        if (f->m_loadingPage) return;

        LevelBrowserLayer::loadLevelsFinished(levels, key, type);

        if (!DemonsInBetween::SEARCHING) return;

        updatePageButtons();
    }

    void setupPageInfo(gd::string pageInfo, const char* key) override {
        LevelBrowserLayer::setupPageInfo(pageInfo, key);

        if (!DemonsInBetween::SEARCHING) return;

        auto f = m_fields.self();
        m_countText->setString(fmt::format("{} to {} of {}",
            f->m_currentPage * 10 + 1, std::min(DemonsInBetween::SEARCH_SIZE, (f->m_currentPage + 1) * 10), DemonsInBetween::SEARCH_SIZE).c_str());
        m_countText->limitLabelWidth(100.0f, 0.6f, 0.0f);
        m_pageText->setString(std::to_string(f->m_currentPage + 1).c_str());
        m_pageText->limitLabelWidth(32.0f, 0.8f, 0.0f);

        updatePageButtons();
    }

    void onBack(CCObject* sender) override {
        LevelBrowserLayer::onBack(sender);

        if (!DemonsInBetween::SEARCHING) return;

        DemonsInBetween::SEARCHING = false;
        DemonsInBetween::DIFFICULTY = 0;
    }

    void setIDPopupClosed(SetIDPopup* popup, int page) override {
        LevelBrowserLayer::setIDPopupClosed(popup, page);

        if (!DemonsInBetween::SEARCHING || popup->getTag() == 4) return;

        m_fields->m_currentPage = std::min(page - 1, DemonsInBetween::MAX_PAGE + 1);
        switchToPage();
    }

    void onRefresh(CCObject* sender) {
        LevelBrowserLayer::onRefresh(sender);

        if (!DemonsInBetween::SEARCHING) return;

        switchToPage();
    }

    void onGoToPage(CCObject* sender) {
        LevelBrowserLayer::onGoToPage(sender);

        if (!DemonsInBetween::SEARCHING) return;

        if (auto popup = CCScene::get()->getChildByType<SetIDPopup>(0)) {
            popup->m_value = m_fields->m_currentPage + 1;
            popup->updateTextInputLabel();
        }
    }

    void onNextPage(CCObject* sender) {
        LevelBrowserLayer::onNextPage(sender);

        if (!DemonsInBetween::SEARCHING) return;

        m_fields->m_currentPage++;
        switchToPage();
    }

    void onPrevPage(CCObject* sender) {
        LevelBrowserLayer::onPrevPage(sender);

        if (!DemonsInBetween::SEARCHING) return;

        m_fields->m_currentPage--;
        switchToPage();
    }
#ifdef GEODE_IS_MACOS // In the Geometry Dash binary for macOS, onNextPage and onPrevPage are inlined into keyDown
    void keyDown(enumKeyCodes key) override {
        LevelBrowserLayer::keyDown(key);

        if (!DemonsInBetween::SEARCHING) return;

        switch (key) {
            case KEY_Left: case CONTROLLER_Left:
                if (m_leftArrow && m_leftArrow->isVisible() && m_leftArrow->isEnabled()) {
                    m_fields->m_currentPage--;
                    switchToPage();
                }
                break;
            case KEY_Right: case CONTROLLER_Right:
                if (m_rightArrow && m_rightArrow->isVisible() && m_rightArrow->isEnabled()) {
                    m_fields->m_currentPage++;
                    switchToPage();
                }
                break;
            default:
                break;
        }
    }
#endif
};
