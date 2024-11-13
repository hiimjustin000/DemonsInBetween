#include <Geode/utils/web.hpp>

struct LadderDemon {
    int id;
    double tier;
    double enjoyment;
    int difficulty;
};

class DemonsInBetween {
private:
    inline static std::vector<cocos2d::CCPoint> LONG_OFFSETS = {
        { 0.0f, -5.0f }, { 0.125f, -5.0f }, { 0.0f, -5.0f }, { 0.0f, -5.125f }, { 0.25f, -5.0f },
        { 0.125f, -4.75f }, { 0.0f, -5.0f }, { 0.0f, -4.125f }, { -0.125f, -4.125f }, { 0.0f, -4.0f },
        { -0.125f, -4.125f }, { 0.0f, -4.125f }, { 0.125f, -4.125f }, { 0.0f, -4.125f }, { 0.0f, -4.125f },
        { 0.0f, -3.625f }, { 0.0f, -3.625f }, { 0.0f, -3.5f }, { 0.0f, -3.5f }, { 0.0f, -3.5f }
    };
    inline static std::vector<cocos2d::CCPoint> SHORT_OFFSETS = {
        { -0.125f, -0.25f }, { -0.125f, -0.25f }, { -0.125f, -0.25f }, { -0.125f, -0.375f }, { -0.125f, -0.25f },
        { -0.125f, -0.25f }, { -0.125f, -0.375f }, { -0.125f, 0.5f }, { -0.125f, 0.5f }, { -0.125f, 0.25f },
        { -0.125f, 0.5f }, { 0.125f, 0.5f }, { 0.125f, 0.5f }, { 0.125f, 0.5f }, { 0.0f, 0.5f },
        { 0.0f, 1.25f }, { 0.0f, 1.25f }, { 0.0f, 1.125f }, { 0.0f, 1.125f }, { 0.0f, 1.125f }
    };
    inline static std::vector<int> DIFFICULTY_INDICES = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 11, 12, 13, 14, 14, 15, 15,
        16, 17, 18, 19, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20
    };
public:
    inline static std::vector<LadderDemon> GDDL = {};
    inline static std::vector<LadderDemon> GDDL_MAIN = {};
    inline static matjson::Value GDDL_CACHE = {};
    inline static std::set<int> REFRESHED_DEMONS = {};
    inline static bool GDDL_CACHE_CHANGED = false;
    inline static bool TRIED_LOADING = false;
    inline static int MAX_PAGE = 0;
    inline static int DIFFICULTY = 0;
    inline static bool SEARCHING = false;
    inline static std::vector<std::string> SEARCH_RESULTS = {};
    inline static std::string CACHE_PATH = "";

    static void tryLoadCache();
    static void loadGDDL();
    static void initGDDL(std::vector<matjson::Value> const&, bool saveCache = false);
    static void saveGDDL();
    static std::vector<matjson::Value> parseGDDL(std::string const&);
    static LadderDemon demonForLevel(GJGameLevel*, bool);
    static void refreshDemonForLevel(geode::EventListener<geode::utils::web::WebTask>&&, GJGameLevel*, std::function<void(LadderDemon const&)> const&, bool);
    static std::string infoForLevel(GJGameLevel*, LadderDemon const&);
    static cocos2d::CCSprite* spriteForDifficulty(GJDifficultySprite*, int, GJDifficultyName, GJFeatureState);
    static GJFeatureState stateForLevel(GJGameLevel*);
    static GJSearchObject* searchObjectForPage(int);
};
