#include <Geode/utils/web.hpp>

struct LadderDemon {
    int id;
    double tier;
    double enjoyment;
    int difficulty;
};

class DemonsInBetween {
public:
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
    inline static std::vector<std::pair<int, int>> TIER_BOUNDS = {
        { 0, 0 }, { 1, 1 }, { 2, 2 }, { 3, 3 }, { 4, 4 }, { 5, 5 }, { 6, 6 }, { 7, 7 }, { 8, 8 }, { 9, 9 }, { 10, 10 },
        { 11, 12 }, { 13, 13 }, { 14, 14 }, { 15, 16 }, { 17, 18 }, { 19, 19 }, { 20, 20 }, { 21, 21 }, { 22, 22 }, { 23, 35 }
    };

    inline static std::vector<LadderDemon> GDDL = {};
    inline static std::vector<LadderDemon> GDDL_MAIN = {};
    inline static std::set<int> LEVELS_LOADED = {};
    inline static int MAX_PAGE = 0;
    inline static int SEARCH_SIZE = 0;
    inline static std::unordered_map<int, int> SEARCH_SIZES = {};
    inline static int DIFFICULTY = 0;
    inline static bool SEARCHING = false;

    static LadderDemon& demonForLevel(int, bool);
    static cocos2d::CCSpriteFrame* spriteFrameForDifficulty(int, GJDifficultyName, GJFeatureState);
    static cocos2d::CCSprite* spriteForDifficulty(GJDifficultySprite*, int, GJDifficultyName, GJFeatureState);
    static int difficultyForDemonDifficulty(int);
    static GJFeatureState stateForLevel(GJGameLevel*);
    static void loadDemonForLevel(
        geode::EventListener<geode::utils::web::WebTask>&&, int, bool,
        std::function<void(LadderDemon&)> const&, std::function<void()> const&
    );
    static void searchObjectForPage(
        geode::EventListener<geode::utils::web::WebTask>&&, int, bool,
        std::function<void(GJSearchObject*)> const&, std::function<void()> const&
    );
};
