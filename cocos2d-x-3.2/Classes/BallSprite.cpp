//
//  BallSprite.cpp
//  Sotsuken
//
//  Created by 神保貴昭 on 2014/11/17.
//
//

#include "BallSprite.h"

USING_NS_CC;

//コンストラクタ
BallSprite::BallSprite()
: _removedNo(0)
, _checkedX(false)
, _checkedY(false)
, _fallCount(0)
, _positionIndex(0, 0)
{
}

//インスタンス生成
BallSprite* BallSprite::create(BallType type, bool visible)
{
    BallSprite *pRet = new BallSprite();
    if (pRet && pRet->init(type, visible))
    {
        pRet->autorelease();
        return pRet;
    }
    else
    {
        delete pRet;
        pRet = nullptr;
        return nullptr;
    }
}

//初期化
bool BallSprite::init(BallType type, bool visible)
{
    if (!Sprite::initWithFile(getBallImageFilePath(type)))
        return false;
    
    _ballType = type;
    
    setVisible(visible);
    
    return true;
}

//パラメータのリセット
void BallSprite::resetParams()
{
    _removedNo = 0;
    _checkedX = false;
    _checkedY = false;
    _fallCount = 0;
}

//初期位置へ移動
void BallSprite::resetPosition()
{
    setPosition(getPositionForPositionIndex(_positionIndex));//位置を変更する
}

//位置インデックスを返す
BallSprite::PositionIndex BallSprite::getPositionIndex()
{
    return _positionIndex;   //位置インデックスを返す
}

//位置インデックスとタグを変更する
void BallSprite::setPositionIndex(PositionIndex positionIndex)
{
    _positionIndex = positionIndex;    //位置インデックスを保持する
    setTag(generateTag(_positionIndex));     //タグをセットする
}

//インデックスとタグと位置を変更する
void BallSprite::setPositionIndexAndChangePosition(PositionIndex positionIndex)
{
    setPositionIndex(positionIndex);    //インデックスとタグを変更する
    resetPosition();     //位置を変更する
}

//消去アニメーションと落下アニメーション
void BallSprite::removingAndFallingAnimation(int maxRemovedNo)
{
    removingAnimation(maxRemovedNo);
    fallingAnimation(maxRemovedNo);
}

//ボール削除アニメーション
void BallSprite::removingAnimation(int maxRemovedNo)
{
    if (_removedNo > 0)
    {
        auto delay1 = DelayTime::create(ONE_ACTION_TIME * (_removedNo - 1));
        auto fade = FadeTo::create(ONE_ACTION_TIME, 0);
        auto delay2 = DelayTime::create(ONE_ACTION_TIME
                                        *(maxRemovedNo - _removedNo));
        auto removeSelf = RemoveSelf::create(false);
        runAction(Sequence::create(delay1, fade, delay2, removeSelf, nullptr));
    }
}

//ボール落下アニメーション
void BallSprite::fallingAnimation(int maxRemovedNo)
{
    if (_fallCount > 0)
    {
        setPositionIndex(PositionIndex(_positionIndex.x,
                                       _positionIndex.y - _fallCount));
        auto delay = DelayTime::create(ONE_ACTION_TIME * maxRemovedNo);
        auto show = Show::create();
        auto move = MoveTo::create(ONE_ACTION_TIME,
                                   getPositionForPositionIndex(getPositionIndex()));
        runAction(Sequence::create(delay, show, move, nullptr));
    }
}

//ボールの色によって画像名を返す
std::string BallSprite::getBallImageFilePath(BallType type)
{
    switch (type)
    {
        case BallType::Red: return "red.png"; //赤
        case BallType::Blue: return "blue.png"; //青
        case BallType::Yellow: return "yellow.png"; //黄
        case BallType::Green: return "green.png"; //緑
        default: return "dummy.png"; //黒
    }
}

//ボール座標の作成
Point BallSprite::getPositionForPositionIndex(PositionIndex positionIndex)
{
    return Point(BALL_SIZE * (positionIndex.x - 0.5),
                 BALL_SIZE * (positionIndex.y - 0.5) );
}

//11から77まででタグの作成
int BallSprite::generateTag(PositionIndex positionIndex)
{
    return positionIndex.x * 10 + positionIndex.y;
}

