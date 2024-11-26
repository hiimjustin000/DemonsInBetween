#include "../DemonsInBetween.hpp"
#include "DIBSearchPopup.hpp"

using namespace geode::prelude;

TableNode* TableNode::create(int columns, int rows) {
    auto ret = new TableNode();
    if (ret->init(columns, rows)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool TableNode::init(int columns, int rows) {
    if (!CCNode::init()) return false;

    setAnchorPoint({ 0.5f, 0.5f });
    m_menus = CCArray::create();
    m_menus->retain();
    m_columns = columns;
    m_rows = rows;

    return true;
}

void TableNode::setColumnLayout(AxisLayout* columnLayout) {
    m_columnLayout = columnLayout;
    setLayout(m_columnLayout);
}

void TableNode::setRowLayout(AxisLayout* rowLayout) {
    m_rowLayout = rowLayout;
    for (auto menu : CCArrayExt<CCMenu*>(m_menus)) {
        menu->setLayout(m_rowLayout);
    }
}

void TableNode::setRowHeight(float rowHeight) {
    m_rowHeight = rowHeight;
    for (auto menu : CCArrayExt<CCMenu*>(m_menus)) {
        menu->setContentSize({ m_obContentSize.width, rowHeight });
    }
}

void TableNode::updateAllLayouts() {
    for (auto menu : CCArrayExt<CCMenu*>(m_menus)) {
        menu->updateLayout();
    }
    updateLayout();
}

void TableNode::addButton(CCMenuItemSpriteExtra* button) {
    CCMenu* menu = nullptr;
    if (m_menus->count() <= 0 || static_cast<CCMenu*>(m_menus->objectAtIndex(m_menus->count() - 1))->getChildrenCount() >= m_columns) {
        menu = CCMenu::create();
        menu->setContentSize({ m_obContentSize.width, m_rowHeight });
        menu->setLayout(m_rowLayout);
        addChild(menu);
        m_menus->addObject(menu);
    } else menu = static_cast<CCMenu*>(m_menus->objectAtIndex(m_menus->count() - 1));

    menu->addChild(button);
}

TableNode::~TableNode() {
    CC_SAFE_RELEASE(m_menus);
}

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

    auto table = TableNode::create(5, 4);
    table->setContentSize({ 350.0f, 240.0f });
    table->setColumnLayout(ColumnLayout::create()->setAxisReverse(true));
    table->setRowLayout(RowLayout::create()->setAxisAlignment(AxisAlignment::Even));
    table->setRowHeight(60.0f);
    table->setPosition({ 175.0f, 130.0f });
    m_mainLayer->addChild(table);

    for (int i = 1; i < 21; i++) {
        table->addButton(CCMenuItemExt::createSpriteExtraWithFrameName(fmt::format("DIB_{:02d}_btn2_001.png"_spr, i).c_str(), 1.0f, [this, i](auto) {
            if (m_isBusy) return;
            m_isBusy = true;
            DemonsInBetween::DIFFICULTY = i;
            DemonsInBetween::SEARCHING = true;
            DemonsInBetween::searchObjectForPage(std::move(m_listener), 0, false, [this](GJSearchObject* obj) {
                CCDirector::get()->pushScene(CCTransitionFade::create(0.5f, LevelBrowserLayer::scene(obj)));
                m_isBusy = false;
                release();
            }, [this] { retain(); });
        }));
    }

    table->updateAllLayouts();

    return true;
}
