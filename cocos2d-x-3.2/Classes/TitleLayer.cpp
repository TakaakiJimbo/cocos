//
//  TitleLayer.cpp
//  Sotsuken
//
//  Created by 神保貴昭 on 2014/11/29.
//
//

#include "TitleLayer.h"
#include "GameLayer.h"

USING_NS_CC;

Scene *TitleLayer::createScene()
{
    auto scene = Scene::create();
    auto layer = TitleLayer::create();
    scene->addChild(layer);
    return scene;
}

bool TitleLayer::init()//タイトル画面の初期化
{
    if (! Layer::init()) return false; //初期画面でないならば偽を返す
    makeBackground(); //背景およびキャラクターの配置
    auto dispacher = Director::getInstance()->getEventDispatcher();
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan = CC_CALLBACK_2(TitleLayer::onTouchBegan, this);
    dispacher->addEventListenerWithSceneGraphPriority(listener, this);
    return true;
}
void TitleLayer::makeBackground()//背景およびキャラクターの配置
{
    Size winSize = Director::getInstance()->getWinSize();
    auto tiBG = Sprite::create("TitleLayer.png"); //背景画像の取得
    tiBG->setPosition(Point(winSize.width/2, winSize.height/2));
    addChild(tiBG,ZOrder::BgForTitle);
    _PC = Sprite::create("PC.png");//キャラクター画像の取得
    _PC->setPosition(Point(winSize.width/2, winSize.height*0.25));
    addChild(_PC,ZOrder::Character);
}
bool TitleLayer::onTouchBegan(Touch *touches, Event *event)
//タップ開始
{
    auto seq = MoveBy::create(0.5,Point(0,1000));
    _PC->runAction(seq);//0.5秒でキャラクターを1000ピクセル上へ
    auto scene = GameLayer::createScene(); //ゲーム画面作成
    auto transition = TransitionFade::create(2.0, scene); //2秒後に移動
    Director::getInstance()->replaceScene(transition);//上記２つの実行
    return false;
}