//
//  GameLayer.h
//  Sotsuken
//
//  Created by 神保貴昭 on 2014/11/17.
//
//
#ifndef __PuzzleGame__GameLayer__
#define __PuzzleGame__GameLayer__

#include "cocos2d.h"
#include <random>
#include "BallSprite.h"
#include "TitleLayer.h"

class GameLayer : public cocos2d::Layer
{
protected:
    //ボールチェック方向
    enum class Direction
    {
        x,
        y,
    };
    
    //Zオーダー
    enum ZOrder
    {
        BgForCharacter = 0,
        BgForPuzzle,
        Character,
        Life,
        Score,
        Ball,
        Time,
        Level,
        Result,
        
    };
    
    std::default_random_engine _engine; //乱数生成エンジン
    std::discrete_distribution<int> _distForBall; //乱数の分布
    BallSprite* _movingBall; //動かしているボール
    bool _movedBall; //他のボールとの接触有無
    bool _touchable; //タップの可否
    int _maxRemovedNo; //一度に削除される最大連鎖の番号
    int _chainNumber; //連鎖数のカウント
    int _remaining; //残機数
    int score1; //消すべき赤の数
    int score2; //消すべき青の数
    int score3; //消すべき緑の数
    float _gametime; //ゲーム時間
    std::vector<std::map<BallSprite::BallType, int>> _removeNumbers; //消去するボールのカウント
 
    cocos2d::Sprite* _PC; //キャラクター画像
    cocos2d::Sprite* _life; //残機画像
    cocos2d::Sprite* _score1; //課題における赤の画像
    cocos2d::Sprite* _score2; //課題における青の画像
    cocos2d::Sprite* _score3; //課題における緑の画像
    cocos2d::Vector<cocos2d::Sprite*> allLife; //残機の画像の配列
    
    int _level; //現在のレベル
    int _nextLevel; //次のレベル
    float _clearTime; //クリア時間
    
    void initBackground(); //背景の初期化
    void initBalls(); //ボールの初期表示
    BallSprite* newBalls(BallSprite::PositionIndex positionIndex, bool visible); //新規ボール作成
    BallSprite* getTouchBall(cocos2d::Point touchPos, BallSprite::PositionIndex withoutPosIndex = BallSprite::PositionIndex()); //タッチしたボールを取得
    void movedBall(); //タップ操作によるボールの移動完了時処理
    void checksLinedBalls(); //一列に並んだボールがあるかチェックする
    bool existsLinedBalls(); //3個以上並んだボールの存在チェック
    cocos2d::Map<int, BallSprite*> getAllBalls(); //全てのボールのBallTypeを取得
    void initBallParams(); //ボールのパラメータを初期化する
    void checkedBall(BallSprite::PositionIndex current, Direction direction); //全ボールに対してボールの並びをチェックする
    bool isSameBallType(BallSprite::PositionIndex current, Direction direction); //指定方向のボールと同じ色かチェックする
    void removeAndGenerateBalls(); //ボールの削除とボールの生成
    void generateBalls(int xLineNum, int fallCount); //ボールを生成する
    void animationBalls(); //ボールの消去と落下アニメーション
    void showHighScore(); //ハイスコアの表示
    void showTimerLabel(); //ゲーム時間の表示
    void initPC(); //敵の表示
    void initLife(); //残機の可視化
    void initScore(); //課題の設定
    void calculateScore(); //消すべき数の計算
    void updateScore(); //消すべき数の更新
    void damageCharacter(); //残機数が減るときの処理やアニメーション
    void endAnimation(); //アニメーション終了時処理
    void initLevelLayer(); //レベル表示レイヤーの表示
    void removeLevelLayer(float dt); //レベル表示レイヤーの削除
    void winAnimation(); //Winアニメーション
    void loseAnimation(); //Loseアニメーション
    void nextScene(float dt); //次のシーンへ遷移
    void titleScene(float dt); //タイトル画面へ遷移
    
public:
    GameLayer(); //コンストラクタ
    virtual bool init(int level, float clearTime); //初期化
    static GameLayer* create(int level, float clearTime); //create関数生成
    static cocos2d::Scene* createScene(int level = 1, float clearTime = 0.0); //シーン生成(最初はレベル１、０秒)
    void update(float dt) override; //時間を更新
    //シングルタップイベント
    virtual bool onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* unused_event);
    virtual void onTouchMoved(cocos2d::Touch* touch, cocos2d::Event* unused_event);
    virtual void onTouchEnded(cocos2d::Touch* touch, cocos2d::Event* unused_event);
    virtual void onTouchCancelled(cocos2d::Touch* touch, cocos2d::Event* unused_event);
};

#endif /* defined(__PuzzleGame__GameLayer__) */
