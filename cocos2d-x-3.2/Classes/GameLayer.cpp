//
//  GameLayer.cpp
//  Sotsuken
//
//  Created by 神保貴昭 on 2014/11/17.
//
//
#include "GameLayer.h"

#define BALL_NUM_X 7
#define BALL_NUM_Y 7
#define WINSIZE Director::getInstance()->getWinSize()
#define TAG_LEVEL_LAYER 10000
#define TAG_TIMER_LABEL 1000
#define TAG_HIGHSCORE_LABEL 2000
#define TAG_SCORE1_LABEL 3000
#define TAG_SCORE2_LABEL 4000
#define TAG_SCORE3_LABEL 5000
USING_NS_CC;

//コンストラクタ
GameLayer::GameLayer()
: _movingBall(nullptr)
, _movedBall(false)
, _touchable(true)
, _maxRemovedNo(0)
, _chainNumber(0)
, _level(0)
,_remaining(0)
,_gametime(0)
, _nextLevel(0)

{
    //乱数初期化および各ボールの出現の重みを指定
    std::random_device device;
    _engine = std::default_random_engine(device());
    _distForBall = std::discrete_distribution<int>{30, 30, 30, 20, 20};
}

//シーン生成
Scene* GameLayer::createScene(int level, float clearTime)
{
    auto scene = Scene::create();
    auto layer = GameLayer::create(level, clearTime);
    scene->addChild(layer);
    
    return scene;
}

//インスタンス生成
GameLayer* GameLayer::create(int level, float clearTime)
{
    GameLayer *pRet = new GameLayer();
    pRet->init(level, clearTime);
    pRet->autorelease();
    return pRet;
}

//初期化
bool GameLayer::init(int level, float clearTime)
{
    if (!Layer::init())
        return false;
    
    // シングルタップイベントの取得
    auto touchListener = EventListenerTouchOneByOne::create();
    touchListener->setSwallowTouches(_swallowsTouches);
    touchListener->onTouchBegan = CC_CALLBACK_2(GameLayer::onTouchBegan, this);
    touchListener->onTouchMoved = CC_CALLBACK_2(GameLayer::onTouchMoved, this);
    touchListener->onTouchEnded = CC_CALLBACK_2(GameLayer::onTouchEnded, this);
    touchListener->onTouchCancelled = CC_CALLBACK_2(GameLayer::onTouchCancelled, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, this);
    
    _level = level; //レベルの保持
    _clearTime = clearTime; //クリアタイムの保持
    _remaining = 10; //残機数の設定
    initBackground(); //背景の初期化
    initBalls(); //ボールの初期表示
    initPC(); //キャラクターの表示
    initScore(); //課題の表示
    initLife(); //残機数の設定（画像など）
    initLevelLayer(); //レベル表示レイヤーの表示
    showTimerLabel(); //ゲーム時間表示
    showHighScore(); //ハイスコア表示
    scheduleUpdate(); //毎フレームごとに時間が更新される
    
    return true;
}

//背景の初期化
void GameLayer::initBackground()
{
    // 上画面の背景
    auto bgForCharacter = Sprite::create("Background1.png");
    bgForCharacter->setAnchorPoint(Point(0, 1));
    bgForCharacter->setPosition(Point(0, WINSIZE.height));
    addChild(bgForCharacter, ZOrder::BgForCharacter);
  
    //パズル部の背景
    auto bgForPuzzle = Sprite::create("Background2.png");
    bgForPuzzle->setAnchorPoint(Point::ZERO);
    bgForPuzzle->setPosition(Point::ZERO);
    addChild(bgForPuzzle, ZOrder::BgForPuzzle);
}

//ボールの初期表示
void GameLayer::initBalls()
{
    for (int x = 1; x <= BALL_NUM_X; x++)
    {
        for (int y = 1; y <= BALL_NUM_Y; y++)
        {
            //ボールを生成する
            newBalls(BallSprite::PositionIndex(x, y), true);
        }
    }
}

//新規ボール作成
BallSprite* GameLayer::newBalls(BallSprite::PositionIndex positionIndex, bool visible)
{
    //現在のタグを取得
    int currentTag = BallSprite::generateTag(positionIndex);
    
    //乱数を元に、ランダムでタイプを取得
    int ballType;
    while (true)
    {
        ballType = _distForBall(_engine);
        
        if (!visible)
            break;
        
        //妥当性のチェック（ボールが隣り合わせにならないようにする）
        
        //左隣のボール
        auto ballX1Tag = currentTag - 10; //1つ左隣は10引いた値
        auto ballX2Tag = currentTag - 20; //2つ左隣は20引いた値
        auto ballX1 = (BallSprite*)(getChildByTag(ballX1Tag));
        auto ballX2 = (BallSprite*)(getChildByTag(ballX2Tag));
        
        //現在のボールが、1つ左隣と2つ左隣のボールと同じだとNG
        if (!(ballX1 && ballType == (int)ballX1->getBallType()) ||
            !(ballX2 && ballType == (int)ballX2->getBallType()))
        {
            //下隣のボール
            auto ballY1Tag = currentTag - 1; //1つ下隣は1引いた値
            auto ballY2Tag = currentTag - 2; //2つ下隣は2引いた値
            auto ballY1 = (BallSprite*)(getChildByTag(ballY1Tag));
            auto ballY2 = (BallSprite*)(getChildByTag(ballY2Tag));
            
            //現在のボールが、1つ下隣と2つ下隣のボールと同じだとNG
            if (!(ballY1 && ballType == (int)ballY1->getBallType()) ||
                !(ballY2 && ballType == (int)ballY2->getBallType()))
            {
                //左隣と下隣が揃わない場合は、ループを抜ける
                break;
            }
        }
    }
    
    //ボールの表示
    auto ball = BallSprite::create((BallSprite::BallType)ballType, visible);
    ball->setPositionIndexAndChangePosition(positionIndex);
    addChild(ball, ZOrder::Ball);
    
    return ball;
}

bool GameLayer::onTouchBegan(Touch* touch, Event* unused_event)
{
    if (!_touchable)
        return false;
    
    _movedBall = false;
    _movingBall = getTouchBall(touch->getLocation());
    
    if (_movingBall)
        return true;
    else
        return false;
}

void GameLayer::onTouchMoved(Touch* touch, Event* unused_event)
{
    //スワイプとともにボールを移動する
    _movingBall->setPosition(_movingBall->getPosition() + touch->getDelta());
    
    auto touchBall = getTouchBall(touch->getLocation(), _movingBall->getPositionIndex());
    if (touchBall && _movingBall != touchBall)
    {
        //移動しているボールが、別のボールの位置に移動
        _movedBall = true;
        
        //別のボールの位置インデックスを取得
        auto touchBallPositionIndex = touchBall->getPositionIndex();
        
        //別のボールを移動しているボールの元の位置へ移動する
        touchBall->setPositionIndexAndChangePosition(_movingBall->getPositionIndex());
        
        //移動しているボールの情報を変更
        _movingBall->setPositionIndex(touchBallPositionIndex);
    }
}

void GameLayer::onTouchEnded(Touch* touch, Event* unused_event)
{
    //タップ操作によるボールの移動完了時処理
    movedBall();
}

void GameLayer::onTouchCancelled(Touch* touch, Event* unused_event)
{
    onTouchEnded(touch, unused_event);
}

//タップした位置のチェック
BallSprite* GameLayer::getTouchBall(Point touchPos, BallSprite::PositionIndex withoutPosIndex)
{
    for (int x = 1; x <= BALL_NUM_X; x++)
    {
        for (int y = 1; y <= BALL_NUM_Y; y++)
        {
            if (x == withoutPosIndex.x && y == withoutPosIndex.y)
            {
                //指定位置のボールの場合は、以下の処理を行わない
                continue;
            }
            
            //タップ位置にあるボールかどうかを判断する
            int tag = BallSprite::generateTag(BallSprite::PositionIndex(x, y));
            auto ball = (BallSprite*)(getChildByTag(tag));
            if (ball)
            {
                //2点間の距離を求める
                float distance = ball->getPosition().getDistance(touchPos);
                
                //ボールの当たり判定は円形。つまりボールの中心からの半径で判断する
                if (distance <= BALL_SIZE / 2)
                {
                    //タップした位置にボールが存在する
                    return ball;
                }
            }
        }
    }
    
    return nullptr;
}

//タップ操作によるボールの移動完了時処理
void GameLayer::movedBall()
{
    //移動しているボールを本来の位置に戻す
    _movingBall->resetPosition();
    _movingBall = nullptr;
    
    //一列に並んだボールがあるかチェックする
    _chainNumber = 0;
    _removeNumbers.clear();
    checksLinedBalls();
}

//一列に並んだボールがあるかチェックする
void GameLayer::checksLinedBalls()
{
    //画面をタップ不可とする
    _touchable = false;
    
    if (existsLinedBalls())
    {
        //3個以上並んだボールの存在する場合
        
        //連鎖カウントアップ
        _chainNumber++;
        
        //ボールの削除と生成
        removeAndGenerateBalls();
        
        //アニメーション後に再チェック
        auto delay = DelayTime::create(ONE_ACTION_TIME * (_maxRemovedNo + 1));
        auto func = CallFunc::create(CC_CALLBACK_0(GameLayer::checksLinedBalls, this));
        auto seq = Sequence::create(delay, func, nullptr);
        runAction(seq);
    }
    else
    {
        //スコアの計算
        calculateScore();
        //スコアが全部0ならゲームクリア
        bool clearGame = false;
        if(score1 == 0 && score2 == 0 && score3 == 0)
        {
            clearGame = true;
        }
        //ゲームクリアが真であるか偽であるかで分岐
        CallFunc* func;
        if (clearGame)
            func = CallFunc::create(CC_CALLBACK_0(GameLayer::winAnimation, this));
        else
            func = CallFunc::create(CC_CALLBACK_0(GameLayer::damageCharacter, this));
        
        runAction(Sequence::create(DelayTime::create(0.5), func, nullptr));
    }
}

//3個以上並んだボールの存在チェック
bool GameLayer::existsLinedBalls()
{
    //ボールのパラメータを初期化する
    initBallParams();
    
    //消去される順番の初期化
    _maxRemovedNo = 0;
    
    for (int x = 1; x <= BALL_NUM_X; x++)
    {
        for (int y = 1; y <= BALL_NUM_Y; y++)
        {
            //x方向のボールをチェック
            checkedBall(BallSprite::PositionIndex(x, y), Direction::x);
            
            //y方向のボールをチェック
            checkedBall(BallSprite::PositionIndex(x, y), Direction::y);
        }
    }
    
    //戻り値の決定
    return _maxRemovedNo > 0;
}

//全てのボールのBallTypeを取得
Map<int, BallSprite*> GameLayer::getAllBalls()
{
    auto balls = Map<int, BallSprite*>();
    
    for (auto object : getChildren())
    {
        auto ball = dynamic_cast<BallSprite*>(object);
        if (ball)
            balls.insert(ball->getTag(), ball);
    }
    
    return balls;
}

//指定方向のボールと同じ色かチェックする
bool GameLayer::isSameBallType(BallSprite::PositionIndex current, Direction direction)
{
    //全てのボールのBallTypeを取得
    auto allBalls = getAllBalls();
    
    if (direction == Direction::x)
    {
        if (current.x + 1 > BALL_NUM_X)
            //列が存在しない場合は抜ける
            return false;
    }
    else
    {
        if (current.y + 1 > BALL_NUM_Y)
            //行が存在しない場合は抜ける
            return false;
    }
    
    //現在のボールを取得
    int currentTag = BallSprite::generateTag(BallSprite::PositionIndex(current.x, current.y));
    BallSprite* currentBall = allBalls.at(currentTag);
    
    //次のボールを取得
    int nextTag;
    if (direction == Direction::x)
        nextTag = BallSprite::generateTag(BallSprite::PositionIndex(current.x + 1, current.y));
    else
        nextTag = BallSprite::generateTag(BallSprite::PositionIndex(current.x, current.y + 1));
    auto nextBall = allBalls.at(nextTag);
    
    if (currentBall->getBallType() == nextBall->getBallType())
        //次のボールが同じBallTypeである
        return true;
    
    return false;
}

//ボールのパラメータを初期化する
void GameLayer::initBallParams()
{
    //全てのボールのBallTypeを取得
    auto allBalls = getAllBalls();
    
    for (auto ball : allBalls)
    {
        ball.second->resetParams();
    }
}

//全ボールに対してボールの並びをチェックする
void GameLayer::checkedBall(BallSprite::PositionIndex current, Direction direction)
{
    //全てのボールのBallTypeを取得
    auto allBalls = getAllBalls();
    
    //検索するタグの生成
    int tag = BallSprite::generateTag(BallSprite::PositionIndex(current.x, current.y));
    BallSprite* ball = allBalls.at(tag);
    
    //指定方向のチェック済みフラグを取得
    bool checked;
    if (direction == Direction::x)
        checked = ball->getCheckedX();
    else
        checked = ball->getCheckedY();
    
    if (!checked)
    {
        int num = 0;
        
        while (true)
        {
            //検索位置を取得
            BallSprite::PositionIndex searchPosition;
            if (direction == Direction::x)
                searchPosition = BallSprite::PositionIndex(current.x + num, current.y);
            else
                searchPosition = BallSprite::PositionIndex(current.x, current.y + num);
            
            //次のボールと同じballTypeかチェックする
            if (isSameBallType(searchPosition, direction))
            {
                //次のボールと同じballType
                int nextTag = BallSprite::generateTag(searchPosition);
                auto nextBall = allBalls.at(nextTag);
                
                //チェックしたボールのチェック済みフラグを立てる
                if (direction == Direction::x)
                    nextBall->setCheckedX(true);
                else
                    nextBall->setCheckedY(true);
                
                num++;
            }
            else
            {
                //次のボールが異なるballType
                
                if (num >= 2)
                {
                    int removedNo = 0;
                    
                    //消去するボールのカウント
                    if (_removeNumbers.size() <= _chainNumber)
                    {
                        //配列が存在しない場合は追加する
                        std::map<BallSprite::BallType, int> removeNumber;
                        _removeNumbers.push_back(removeNumber);
                    }
                    _removeNumbers[_chainNumber][ball->getBallType()] += num + 1;
                    
                    //すでにRemovedNoがあるものが存在するかチェック
                    for (int i = 0; i <= num; i++)
                    {
                        BallSprite::PositionIndex linedPosition;
                        if (direction == Direction::x)
                            linedPosition = BallSprite::PositionIndex(current.x + i, current.y);
                        else
                            linedPosition = BallSprite::PositionIndex(current.x, current.y + i);
                        
                        int linedBallTag = BallSprite::generateTag(linedPosition);
                        auto linedBall = allBalls.at(linedBallTag);
                        if (linedBall->getRemovedNo() > 0)
                        {
                            //すでにRemovedNoがあるものが存在するので、removedNoを取得し次の処理を行う
                            removedNo = linedBall->getRemovedNo();
                            break;
                        }
                    }
                    
                    //消去する順番のカウントアップ
                    if (removedNo == 0)
                        removedNo = ++_maxRemovedNo;
                    
                    //3個以上並んでいた場合は、removedNoをセットする
                    for (int i = 0; i <= num; i++)
                    {
                        BallSprite::PositionIndex linedPosition;
                        if (direction == Direction::x)
                            linedPosition = BallSprite::PositionIndex(current.x + i, current.y);
                        else
                            linedPosition = BallSprite::PositionIndex(current.x, current.y + i);
                        
                        int linedBallTag = BallSprite::generateTag(linedPosition);
                        auto linedBall = allBalls.at(linedBallTag);
                        linedBall->setRemovedNo(removedNo);
                    }
                }
                
                break;
            }
        };
        
        //指定方向をチェック済みとする
        if (direction == Direction::x)
            ball->setCheckedX(true);
        else
            ball->setCheckedY(true);
    }
}

//ボールの削除とボールの生成
void GameLayer::removeAndGenerateBalls()
{
    //全てのボールのBallTypeを取得
    auto allBalls = getAllBalls();
    
    int maxRemovedNo = 0;
    
    for (int x = 1; x <= BALL_NUM_X; x++)
    {
        int fallCount = 0;
        
        for (int y = 1; y <= BALL_NUM_Y; y++)
        {
            int tag = BallSprite::generateTag(BallSprite::PositionIndex(x, y));
            auto ball = allBalls.at(tag);
            
            if (ball) {
                int removedNoForBall = ball->getRemovedNo();
                
                if (removedNoForBall > 0)
                {
                    //落ちる段数をカウント
                    fallCount++;
                    
                    if (removedNoForBall > maxRemovedNo)
                        maxRemovedNo = removedNoForBall;
                }
                else
                {
                    //落ちる段数をセット
                    ball->setFallCount(fallCount);
                }
            }
        }
        
        //ボールを生成する
        generateBalls(x, fallCount);
    }
    
    //ボールの消去＆落下アニメーション
    animationBalls();
}

//ボールを生成する
void GameLayer::generateBalls(int xLineNum, int fallCount)
{
    for (int i = 1; i <= fallCount; i++)
    {
        //ボールを生成する
        auto positionIndex = BallSprite::PositionIndex(xLineNum, BALL_NUM_Y + i);
        auto ball = newBalls(positionIndex, false);
        ball->setFallCount(fallCount);
    }
}

//ボールの消去と落下アニメーション
void GameLayer::animationBalls()
{
    //全てのボールのBallTypeを取得
    auto allBalls = getAllBalls();
    
    for (auto ball : allBalls)
    {
        //ボールのアニメーションを実行する
        ball.second->removingAndFallingAnimation(_maxRemovedNo);
    }
}

//ハイスコアを表示する
void GameLayer::showHighScore()
{
    UserDefault* userDefault =UserDefault::sharedUserDefault();
    const char* highScoreKey = "highscore";
    float _highscore = userDefault->getFloatForKey(highScoreKey, 999.9);
    //HIGHSCOREのラベルを表示する
    auto highscoreLabel0 = LabelTTF::create("HIGHSCORE", "Arial", 28);
    highscoreLabel0->setPosition(BALL_SIZE * (BALL_NUM_X - 3.5) , BALL_SIZE * (BALL_NUM_Y+ 0.73));
    this->addChild(highscoreLabel0);
    //level5をクリアしたときにハイスコアが更新されるかどうか判定
    if(_level == 5 && score1 == 0 && score2 == 0 && score3 == 0)
    {
        if(_gametime > _highscore)
        {
            return;
        }
        else
        {
            //ハイスコアが更新されたとき、ゲームを終了した後もハイスコアを保存する
            _highscore = _gametime;
            userDefault->setFloatForKey(highScoreKey, _highscore);
            userDefault->flush();
            
        }
    }
    
    auto highscoreLabel = (Label*)getChildByTag(TAG_HIGHSCORE_LABEL);
    if (!highscoreLabel)
    {
        //ハイスコアのラベルを表示する
        highscoreLabel = Label::createWithSystemFont("", "Arial", 48);
        highscoreLabel->setPosition(BALL_SIZE * (BALL_NUM_X - 3.5), BALL_SIZE * (BALL_NUM_Y +0.3));
        highscoreLabel->setTag(TAG_HIGHSCORE_LABEL);
        addChild(highscoreLabel);
    }
    
    highscoreLabel->setString(StringUtils::format("%0.2fs", _highscore));
}

//時間を表示する
void GameLayer::showTimerLabel()
{
    _gametime = _clearTime;
    //PLAYTIMEのラベルを表示する
    auto timerLabel0 = LabelTTF::create("PLAYTIME", "Arial", 28);
    timerLabel0->setPosition(BALL_SIZE ,BALL_SIZE * (BALL_NUM_Y+ 0.73) );
    this->addChild(timerLabel0);
    
    auto timerLabel = (Label*)getChildByTag(TAG_TIMER_LABEL);
    if (!timerLabel)
    {
        //ゲーム時間のラベルを表示する
        timerLabel = Label::createWithSystemFont("", "Arial", 48);
        timerLabel->setPosition(BALL_SIZE , BALL_SIZE * (BALL_NUM_Y +0.3));
        timerLabel->setTag(TAG_TIMER_LABEL);
        addChild(timerLabel);
    }
    
    timerLabel->setString(StringUtils::format("%0.2fs", _gametime));
}


//時間を更新
void GameLayer::update(float dt)
{
    _gametime += dt;
    
    auto timerLabel = (Label*)getChildByTag(TAG_TIMER_LABEL);
    if(timerLabel)
    {
        timerLabel->setString(StringUtils::format("%0.2fs", _gametime));
    }
    bool finish = false;
    if(score1 == 0 && score2 == 0 && score3 == 0)
    {
        finish = true;
    }
    if(finish)
    {
        unscheduleUpdate();
    }
}


void GameLayer::initPC()
{

    //キャラクターの表示
    _PC = Sprite::create(StringUtils::format("PC.png"));
    _PC->setPosition(Point(WINSIZE.width * 0.76, WINSIZE.height * 0.86));
    addChild(_PC, ZOrder::Character);
}

//残機の可視化
void GameLayer::initLife()
{
    int j,k;
    for(int i = 0;i < _remaining;i++)
    {
        if(i > 4)
        {
            j = i - 5;
            k = 1;
        }
        else
        {
            j = i;
            k = 0;
            
        }
        _life = Sprite::create("life.png");
        _life->setPosition(Point(BALL_SIZE * (BALL_NUM_X - 2) + 35 * j , BALL_SIZE * (BALL_NUM_Y + 1)- 35 * k - 40));
        addChild(_life, ZOrder::Life);
        allLife.pushBack(_life);
    }
}

//課題の設定
void GameLayer::initScore()
{
    //赤色の課題設定
    _score1 = Sprite::create("red.png");
    _score1->setScale(0.75);
    _score1->setPosition(40,920);
    addChild(_score1, ZOrder::Score);
    score1 = _level * 10;
    auto score1Label = (Label*)getChildByTag(TAG_SCORE1_LABEL);
    score1Label = Label::createWithSystemFont("", "Arial", 48);
    score1Label->setPosition(120,920);
    score1Label->setTag(TAG_SCORE1_LABEL);
    addChild(score1Label);
    score1Label->setString(StringUtils::format("×%0.2d", score1));
    
    //青色の課題設定
    _score2 = Sprite::create("blue.png");
    _score2->setScale(0.75);
    _score2->setPosition(40,850);
    addChild(_score2, ZOrder::Score);
    score2 = _level * 10;
    auto score2Label = (Label*)getChildByTag(TAG_SCORE2_LABEL);
    score2Label = Label::createWithSystemFont("", "Arial", 48);
    score2Label->setPosition(120,850);
    score2Label->setTag(TAG_SCORE2_LABEL);
    addChild(score2Label);
    score2Label->setString(StringUtils::format("×%0.2d", score2));

    //緑色の課題設定
    _score3 = Sprite::create("green.png");
    _score3->setScale(0.75);
    _score3->setPosition(40,780);
    addChild(_score3, ZOrder::Score);
    score3 = _level * 10;
    auto score3Label = (Label*)getChildByTag(TAG_SCORE3_LABEL);
    score3Label = Label::createWithSystemFont("", "Arial", 48);
    score3Label->setPosition(120,780);
    score3Label->setTag(TAG_SCORE3_LABEL);
    addChild(score3Label);
    score3Label->setString(StringUtils::format("×%0.2d", score3));
}


// 課題の計算
void GameLayer::calculateScore()
{
    // 消したものと数を繰り返す (*ballIt).firstはボールの色　(*ballIt).secondは個数
    auto removeIt = _removeNumbers.begin();
    while (removeIt != _removeNumbers.end())
    {
        auto ballIt = (*removeIt).begin();
        while(ballIt != (*removeIt).end())
        {
           //赤を消した数だけ画面上の課題から引かれる
            if((*ballIt).first == BallSprite::BallType::Red)
            {
                score1 -= (*ballIt).second;
               //マイナスの値になる場合は0にする（以下同様）
                if(score1 <= 0)
                {
                    score1 = 0;
                }
            }
             //青を消した数だけ画面上の課題から引かれる
            if((*ballIt).first == BallSprite::BallType::Blue)
            {
                score2 -= (*ballIt).second;
                if(score2 <= 0)
                {
                    score2 = 0;
                }
            }
             //緑を消した数だけ画面上の課題から引かれる
            if((*ballIt).first == BallSprite::BallType::Green)
            {
                score3 -= (*ballIt).second;
                if(score3 <= 0)
                {
                    score3 = 0;
                }
            }
            //黄色を消した数だけ画面上の３色の課題から引かれる
            if((*ballIt).first == BallSprite::BallType::Yellow)
            {
                score1 -= (*ballIt).second;
                if(score1 <= 0)
                {
                    score1 = 0;
                }
                score2 -= (*ballIt).second;
                if(score2 <= 0)
                {
                    score2 = 0;
                }
                score3 -= (*ballIt).second;
                if(score3 <= 0)
                {
                    score3 = 0;
                }
            }
            
            ballIt++;
        }
        
        removeIt++;
   }
    updateScore();
    //すべての処理が終わったら、課題を更新する
}
//課題の更新
void GameLayer::updateScore()
{
    auto score1Label = (Label*)getChildByTag(TAG_SCORE1_LABEL);
    score1Label->setString(StringUtils::format("×%0.2d", score1));
    auto score2Label = (Label*)getChildByTag(TAG_SCORE2_LABEL);
    score2Label->setString(StringUtils::format("×%0.2d", score2));
    auto score3Label = (Label*)getChildByTag(TAG_SCORE3_LABEL);
    score3Label->setString(StringUtils::format("×%0.2d", score3));
}


//残機数が減るときの処理やアニメーション
void GameLayer::damageCharacter()
{
    //ダメージをうける
    --_remaining;
    
    //被ダメージアニメーション
    auto seq1 = Sequence::create(TintTo::create(0, 255, 0, 0),
                                 DelayTime::create(0.2),
                                 TintTo::create(0, 255, 255, 255), nullptr);
    _PC->runAction(seq1);
    
    //ライフが減るアニメーション
    allLife.at(_remaining)->runAction(FadeTo::create(0.5, 0));
    //ゲームオーバーのチェック
    bool lifeZero = true;
    
    if (_remaining > 0){
        
        lifeZero = false;
    }
    
    // アニメーション終了時処理
    CallFunc* func;
    if (lifeZero)
        func = CallFunc::create(CC_CALLBACK_0(GameLayer::loseAnimation, this));
    else
        func = CallFunc::create(CC_CALLBACK_0(GameLayer::endAnimation, this));
    
    runAction(Sequence::create(DelayTime::create(0.5), func, nullptr));
}

// アニメーション終了時処理
void GameLayer::endAnimation()
{
    // タップを有効にする
    _touchable = true;
}

//レベル表示レイヤーの表示
void GameLayer::initLevelLayer()
{
    _touchable=false;
    //レベルレイヤーの生成
    auto levelLayer = LayerColor::create(Color4B(0, 0, 0, 191), WINSIZE.width, WINSIZE.height);
    levelLayer->setPosition(Point::ZERO);
    levelLayer->setTag(TAG_LEVEL_LAYER);
    addChild(levelLayer, ZOrder::Level);
    
    auto Llevel = LabelTTF::create("LEVEL", "メイリオ", 150);
    Llevel->setPosition(Point(WINSIZE.width * 0.45, WINSIZE.height * 0.5));
    levelLayer->addChild(Llevel);
    auto levelP = StringUtils::format ("%d", _level);
    auto Nlevel = LabelTTF::create(levelP, "メイリオ", 150);
    Nlevel->setPosition(Point(WINSIZE.width * 0.87, WINSIZE.height * 0.5));
    levelLayer->addChild(Nlevel);

    scheduleOnce(schedule_selector(GameLayer::removeLevelLayer), 1.5);
}

//レベル表示レイヤーの削除
void GameLayer::removeLevelLayer(float dt)
{
    //タップ可能とする
    _touchable = true;
    
    //0.5秒で消えるようにする
    auto levelLayer = getChildByTag(TAG_LEVEL_LAYER);
    levelLayer->runAction(Sequence::create(FadeTo::create(0.5, 0),
                                           RemoveSelf::create(),
                                           nullptr));
    _touchable=true;
}

//Winアニメーション
void GameLayer::winAnimation()
{
    //背景を用意する
    auto Layer = LayerColor::create(Color4B(0, 0, 0, 127), WINSIZE.width, WINSIZE.height);
    Layer->setPosition(Point::ZERO);
    addChild(Layer, ZOrder::Result);
    
    //クリアーの画面を表示する
    auto win = LabelTTF::create("くりあ〜", "ＤＦＰ勘亭流", 180);
    win->setPosition(Point(WINSIZE.width / 2, WINSIZE.height / 2));
    Layer->addChild(win);
    
    auto seq = MoveBy::create(0.5,Point(-1000,0));
    _PC->runAction(seq);
    // 次のレベルを設定（Level5の次はないので、10秒後タイトルに戻る）
    if (_level >= 5)
    {
        Layer->runAction(Sequence::create(FadeTo::create(3.0, 0),
                                        RemoveSelf::create(),
                                        nullptr));
        win->runAction(Sequence::create(FadeTo::create(3.0, 0),
                                               RemoveSelf::create(),
                                               nullptr));
        showHighScore();
        scheduleOnce(schedule_selector(GameLayer::titleScene), 10);
    }
    else
    {
        _nextLevel = _level + 1;
        _clearTime = _gametime;
        //指定秒数後に次のシーンへ
        scheduleOnce(schedule_selector(GameLayer::nextScene), 3);
    }
}

//Loseアニメーション
void GameLayer::loseAnimation()
{
    //ゲーム時間の停止
    unscheduleUpdate();
    //キャラクターが消える
    auto seq=FadeTo::create(1.0, 0);
    _PC->runAction(seq);
    //黒い背景を用意する
    auto Layer = LayerColor::create(Color4B(0, 0, 0, 127), WINSIZE.width, WINSIZE.height);
    Layer->setPosition(Point::ZERO);
    addChild(Layer, ZOrder::Result);
    //ゲームオーバーを表示する
    auto lose = LabelTTF::create("げ〜むお〜ば〜", "ＤＦＰ勘亭流", 96);
    lose->setPosition(Point(WINSIZE.width / 2, WINSIZE.height / 2));
    Layer->addChild(lose);
    //３秒後にタイトルへ
    scheduleOnce(schedule_selector(GameLayer::titleScene), 3);
}

//次のシーンへ遷移
void GameLayer::nextScene(float dt)
{
    // 次のシーンを生成する
    auto scene = GameLayer::createScene(_nextLevel, _clearTime);
    Director::getInstance()->replaceScene(scene);
}

void GameLayer::titleScene(float dt)
{
    // タイトル画面を生成する
    auto scene = TitleLayer::createScene();
    Director::getInstance()->replaceScene(scene);
}

