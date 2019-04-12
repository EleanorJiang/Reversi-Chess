#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include<stdio.h>
#include<string>
#include<iostream>
#include<vector>
#include"QMutex"
#include"QButtonGroup"

using namespace std;
#define BLACK  0
#define WHITE  1

class BitBoard {
public:
    unsigned long long mask[64];
    unsigned long long opmask[64];
    unsigned long long pieces[2]; //black_pieces, white_pieces

//    unsigned long long validneighbor[64];
//    unsigned long long ownneighbor[64];
//    unsigned long long opneighbor[64];
    unsigned long long get(int p, int tile) { // p = 8*i+j
        return pieces[tile] & mask[p];
    }

//    void set(int p, int tile) {
//        pieces[tile] |= mask[p];
//    }

    void init() {
        unsigned long long b = 1;
        for (int c = 0; c < 64; c ++) {
         mask[c] = b << c;
         opmask[c] = ~mask[c];
        } //初始化mask[64]数组
//        validneighbor[63] = 0x80C0000000000000;
//        validneighbor[56] = 0x0203000000000000;
//        validneighbor[7] = 0x000000000000C080;
//        validneighbor[0] = 0x0000000000000302;
//        validneighbor[62] = 0xA0E0000000000000;
//        validneighbor[55] = 0xC080C00000000000;
//        validneighbor[48] = 0x0302030000000000;
//        validneighbor[1] = 0x0000000000000705;
//        for (int i=61;i>56;i--) {
//            validneighbor[i]= validneighbor[i+1] >> 1;
//        }
//        for (int i=2;i<7;i++) {
//            validneighbor[i]= validneighbor[i-1] << 1;
//        }
//        for (int i=47;i>7;i-=8) {
//            validneighbor[i]= validneighbor[i+8] >> 8;
//        }
//        for (int i=40;i>7;i-=8) {
//            validneighbor[i]= validneighbor[i+8] >> 8;
//        }
//        validneighbor[9] = 0x0E0A0E;
//        for (int i=1;i<=6;i++) {
//            if(i != 1)
//                validneighbor[8*i+1] = validneighbor[8*(i-1)+1] << 8;
//            for (int j=2;j<=6;j++){
//                validneighbor[8*i+j]= validneighbor[8*i+j-1] << 1;
//            }
//        }
       pieces[BLACK] = 0x0000000810000000;
       pieces[WHITE] = 0x0000001008000000;
//        for (int i=0;i<64;i++) {
//            ownneighbor[i]=  validneighbor[i] & pieces[BLACK];
//            opneighbor[i]= validneighbor[i] & pieces[WHITE];
//        }
    }
}; //bitboard用2个64位长整数描述，有置某格子，get某格子和初始化的功能

typedef struct StateNode {
    int position;               //the corresponding position on the board
    BitBoard board;
    StateNode(int position, BitBoard board) {
        this->position = position;
        this->board = board;
    }
}State; //某个tile的结果

class MCTS {
public:
    vector<MCTS*> child;         //child nodes of this node 子节点组成的vector
    MCTS* bestchild = nullptr;  //best child of this node   最佳的子节点
    int visit;                     //visit times of this node  该节点的访问次数
    double score;                 //score of this node  该节点的score
    bool expandable = true;     //whether this node is expandable, initialize to be true
    MCTS* father = nullptr;     //father node of this node in MCT
    bool root = false;           //whether this node is dummy root 是否为dummy root
    int position;               //the corresponding position on the board
    BitBoard board;
    int mytile;                   //the corresponding chess tile on the board
    double C = 0.02;            //the constant C in UTC, which can be adjusted
    double realscore;

    MCTS(StateNode state, int mytile);  //constructor
    MCTS(int position, BitBoard* board, int mytile);
    void rootClear();  //function to initialize the dummy root
    double calculate(); //function to calculate for determining best child
    int bestChild();    //function to get the best child
};


typedef struct ResultNode {
    MCTS* state;
    int tile;
    ResultNode(MCTS* state, int tile) {
        this->state = state;
        this->tile = tile;
    }
}Result; //某个tile的结果


typedef struct ScoreNode {
    int white_score;
    int black_score;
    ScoreNode(int whscore, int blkscore) {
        this->white_score = whscore;
        this->black_score = blkscore;
    }
    int get(int tile){
        if(tile == BLACK)
            return black_score;
        else return white_score;
    }
}Score; //包含op的分数和my的分数




namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
private:
    Ui::MainWindow *ui;
    MCTS* root;  //MCTS的根节点
    double TIME = 60;
    int DEPTH = 5; //搜索深度
    bool gameOver = false;
    int playerTile, computerTile;
    QMutex mutex;
    string turn;
    int Last;

    QButtonGroup *btnGroup;
    bool isGameStart = false; //游戏是否开始
    QPixmap background;
    QPixmap white,black,hintwhite,hintblack,hintred;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void init();
    BitBoard* getBitBoardCopy(BitBoard* bitboard);
    bool bmakeMove(BitBoard* mainboard, int playerTile, int pos);
    vector<int> bisValidMove(BitBoard* board, int tile, int pos);
    vector<StateNode>  bgetValidMove(BitBoard* board, int tile,  bool forpolicy=false); //获取有效的行为集合
    void printBoard(BitBoard* board); //命令行打印棋盘
    bool bisGameOver(BitBoard* board); //判定游戏是否结束
    int getComputerMove(BitBoard* board,int tile, double newc); //获取AI做出的动作
    int uctSearch(MCTS* root, BitBoard* board, int tile,int DEPTH, double TIME); //获取当前比分
    Score bdefaultPolicy(BitBoard* board, int tile); //快速出子
    Score bgetScoreofBoard(BitBoard* board);
    Result treePolicy(BitBoard* board, MCTS* state, int tile); //拓展
    int getEnemyTile(int tile){
        return tile ^ 0x1;
    }
    void backup(MCTS* bottom, int tile, int computertile, Score score); //回溯
    void start();
    void printReport();


protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *e);       //--鼠标按下事件

private slots:
    void ClickButton();
};

#endif // MAINWINDOW_H
