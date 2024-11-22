#include <Geode/utils/web.hpp>

struct CachedLadderDemon {
    int id;
    int difficulty;
};

class DIBInfoPopup : public geode::Popup<> {
protected:
    geode::EventListener<geode::utils::web::WebTask> m_listener;
    cocos2d::CCArray* m_demonSprites;
    cocos2d::CCArray* m_demonClassicLabels;
    cocos2d::CCArray* m_demonPlatformerLabels;
    LoadingCircle* m_loadingCircle;
    int m_page;
    bool m_loaded;
    std::vector<int> m_completionCountClassic = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    std::vector<int> m_completionCountPlatformer = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

    bool setup() override;
    void setupDemonInfo();
    void loadPage(int);
    void onClose(CCObject*) override;
    void keyDown(cocos2d::enumKeyCodes) override;
public:
    inline static std::vector<CachedLadderDemon> CACHED_DEMONS = {};

    static DIBInfoPopup* create();

    ~DIBInfoPopup() override;
};
