//
//  TitleLayer.h
//  Sotsuken
//
//  Created by 神保貴昭 on 2014/11/29.
//
//

#ifndef Sotsuken_TitleLayer_h
#define Sotsuken_TitleLayer_h

#include "cocos2d.h"

USING_NS_CC;

class TitleLayer : public cocos2d::Layer
{
public:
    enum ZOrder
    {
        BgForTitle = 0,
        Character,
    };
    
    static Scene *createScene();
    cocos2d::Sprite* _PC; //キャラクター画像
    virtual bool init();
    CREATE_FUNC(TitleLayer);
    
    void makeBackground();
    
    bool onTouchBegan(Touch *touches, Event *event);

};


#endif
