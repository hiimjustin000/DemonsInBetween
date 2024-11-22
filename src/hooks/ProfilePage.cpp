#include "../classes/DIBInfoPopup.hpp"

using namespace geode::prelude;

#include <Geode/modify/ProfilePage.hpp>
class $modify(DIBProfilePage, ProfilePage) {
    void onStatInfo(CCObject* sender) {
        if (!m_ownProfile || sender->getTag() != 3) {
            ProfilePage::onStatInfo(sender);
            return;
        }

        DIBInfoPopup::create()->show();
    }
};
