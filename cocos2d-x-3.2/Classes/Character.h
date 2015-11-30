#ifndef __PuzzleGame__Character__
#define __PuzzleGame__Character__

#include "cocos2d.h"

class Character : public cocos2d::Ref
{
public:
    //キャラクター属性
    enum class Element
    {
        Fire, //火
        Water, //水
        Wind, //風
        Holy, //光
        None, //なし
    };
    
protected:
    int _remainingTurn; //攻撃するまでの残りターン
    CC_SYNTHESIZE(Element, _element, Element); //属性
    CC_PROPERTY(int, _turnCount, TurnCount); //攻撃ターン数（敵の場合）
    
public:
    Character(); //コンストラクタ
    static Character* create(); //インスタンス生成
};

#endif /* defined(__PuzzleGame__Character__) */