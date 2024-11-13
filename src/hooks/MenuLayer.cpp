#include "../DemonsInBetween.hpp"

using namespace geode::prelude;

#include <Geode/modify/MenuLayer.hpp>
class $modify(DIBMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;

        if (DemonsInBetween::TRIED_LOADING) return true;
        DemonsInBetween::TRIED_LOADING = true;

        DemonsInBetween::tryLoadCache();

        return true;
    }
};
