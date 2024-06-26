#include "DemonsInBetween.hpp"

class DIBSearchPopup : public Popup<> {
protected:
    bool setup() override;
    void createDifficultyButton(CCMenu*, int);
public:
    static DIBSearchPopup* create();
};
