//
//  MainLayer.cpp
//  CocosTest2
//
//  Created by arvin on 2017/8/21.
//
//

#include "MainLayer.h"
#include "ui/UIButton.h"
#include "ui/UICheckBox.h"
#include <spine/spine-cocos2dx.h>
#include "spine/spine.h"
#include "HelloWorldScene.h"
#include "NavigationController.h"
#include "DrawViewTest.h"
#include "TableViewTest.h"
#include "ActionTest.h"
#include "ClippingTest.h"
#include "NumberInTenCompareLayer.h"
#include "NumberInTenSingularLayer.h"
#include "NumberInTenComBineLayer.h"
#include "NumberInTenFlyLayer.h"
#include "NumberInTwentySequence.h"
#include "SimpleAudioEngine.h"
#include "LoadDialog.h"
#include "ElectronicBoardsLayer.h"

USING_NS_CC;
using namespace ui;
using namespace std;
using namespace CocosDenshion;
using namespace spine;

#define V_WIDTH  Director::getInstance()->getVisibleSize().width
#define V_HEIGHT Director::getInstance()->getVisibleSize().height

const string names[] = {"画板", "GridView", "动作", "节点裁剪", "10_Compare", "10_Singular", "10_Combine", "10_Fly", "20_Sequence", "电子钉板"};

inline int getMRow(int index, int col) {
    return index / col;
}

inline int getMCol(int index, int col) {
    return index % col;
}

MainLayer::MainLayer() {

}

MainLayer::~MainLayer() {

}

bool MainLayer::init() {
    
    if(!Layer::init()) {
        return false;
    }
    
    float paddingLeft = 20;
    float paddingTop = 20;
    float width = 150;
    float height = 54;
    int column = 6;
    float startX = (V_WIDTH - column * width - (column - 1) * paddingLeft) / 2;
    float startY = V_HEIGHT - 50;
    
    for(int i = 0; i < 10; i++) {
        Button* btn = Button::create("mian_button_01_125x54.png");
        btn->setAnchorPoint(Vec2::ANCHOR_TOP_LEFT);
        btn->setContentSize(Size(150, 54));
        btn->setScale9Enabled(true);
        btn->setPosition(Vec2(startX + (getMCol(i, column) * (width + paddingLeft)),
                              startY - (getMRow(i, column) * (height + paddingTop))));
        btn->setTitleFontSize(24);
        btn->setTitleText(names[i]);
        btn->addClickEventListener(CC_CALLBACK_1(MainLayer::onBtnClick, this));
        btn->setTag(i);
        addChild(btn);
    }
    
    CheckBox* audioBtn = CheckBox::create("CloseNormal.png", "CloseSelected.png");
    audioBtn->setPosition(Vec2(50, 50));
    audioBtn->setSelected(true);
    audioBtn->addEventListener([this](Ref* pSender, CheckBox::EventType eventType) {
        if(eventType == CheckBox::EventType::SELECTED) {
            SimpleAudioEngine::getInstance()->playBackgroundMusic("background.mp3", true); //播放音乐
        } else {
            SimpleAudioEngine::getInstance()->stopBackgroundMusic(); //停止音乐
        }
    });
    addChild(audioBtn);
    
    SimpleAudioEngine::getInstance()->preloadBackgroundMusic("background.mp3"); //预加载音乐
//    scheduleOnce([](float dt){
//        SimpleAudioEngine::getInstance()->playBackgroundMusic("background.mp3", true); //播放音乐
//    }, 2.0f, "Audio");
    
    SimpleAudioEngine::getInstance()->preloadEffect("pew-pew-lei.wav"); //预加载音效

    LoadDialog* loadDialog = LoadDialog::create();
    loadDialog->setText("正在登录系统，请稍后...");
    loadDialog->setCancelable(true);
    loadDialog->show(this);
    
//    spine::SkeletonAnimation* skeleton = spine::SkeletonAnimation::createWithJsonFile("tank.json", "tank.atlas");
//    skeleton->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
//    skeleton->setPosition(Vec2(V_WIDTH - 100, V_HEIGHT / 2 - 200));
//    skeleton->setScale(0.3);
//    this->addChild(skeleton);
//    
//    skeleton->setAnimation(0, "drive", true);
    
    return true;
}

HelloWorld* MainLayer::getMainScene() {
    return dynamic_cast<HelloWorld*>(Director::getInstance()->getRunningScene());
}

void MainLayer::onBtnClick(Ref* pSender) {
    
    SimpleAudioEngine::getInstance()->playEffect("pew-pew-lei.wav"); //播放音效
    
    Button* btn = static_cast<Button*>(pSender);
    if(btn) {
        int tag = btn->getTag();
        switch (tag) {
            case 0: {
                DrawViewTest* drawViewTest = DrawViewTest::create();
                getMainScene()->getRootLayer()->controller->pushView(drawViewTest);
                break;
            }
            case 1: {
                TableViewTest* tableViewTest = TableViewTest::create();
                getMainScene()->getRootLayer()->controller->pushView(tableViewTest);
                break;
            }
            case 2: {
                ActionTest* actionTest = ActionTest::create();
                getMainScene()->getRootLayer()->controller->pushView(actionTest);
                break;
            }
            case 3: {
                ClippingTest* clipplingTest = ClippingTest::create();
                getMainScene()->getRootLayer()->controller->pushView(clipplingTest);
                break;
            }
            case 4: {
                BaseNumberInTenDragLayer* baseDragLayer = NumberInTenCompareLayer::create();
                getMainScene()->getRootLayer()->controller->pushView(baseDragLayer);
                break;
            }
            case 5: {
                BaseNumberInTenDragLayer* baseDragLayer = NumberInTenSingularLayer::create();
                getMainScene()->getRootLayer()->controller->pushView(baseDragLayer);
                break;
            }
            case 6: {
                BaseNumberInTenDragLayer* baseDragLayer = NumberInTenCombineLayer::create();
                getMainScene()->getRootLayer()->controller->pushView(baseDragLayer);
                break;
            }
            case 7: {
                NumberInTenFlyLayer* flyLayer = NumberInTenFlyLayer::create();
                getMainScene()->getRootLayer()->controller->pushView(flyLayer);
                break;
            }
            case 8: {
                NumberInTwentySequenceLayer* sequenceLayer = NumberInTwentySequenceLayer::create();
                getMainScene()->getRootLayer()->controller->pushView(sequenceLayer);
                break;
            }
            case 9: {
                ElectronicBoardsLayer* boardsLayer = ElectronicBoardsLayer::create();
                getMainScene()->getRootLayer()->controller->pushView(boardsLayer);
                break;
            }
                
            default:
                break;
        }
    }
}
