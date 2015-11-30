#include "Character.h"

USING_NS_CC;



//インスタンス生成
Character* Character::create()
{
    Character *pRet = new Character();
    pRet->autorelease();
    
    return pRet;
}

//攻撃ターン数取得
int Character::getTurnCount()
{
    return _turnCount;
}

//攻撃ターン数設定
void Character::setTurnCount(int turnCount)
{
    _turnCount = turnCount;
    _remainingTurn = _turnCount;
}


