#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Qpainter"
#include "QPixmap"
#include "QLabel"
#include "QCursor"
#include "iostream"
#include "QMouseEvent"
#include <stdio.h>
#include <string>
#include <vector>
#include <math.h>
#include <time.h>
#include <queue>
#include <stack>

using namespace std;
double COMPUTER_PARA = 0.02;
double PLAYER_PARA = 0.01;
bool mousedown = false;
int mousex;
int mousey;
BitBoard* mainBoard;


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    init();
    mainBoard = new BitBoard();
    mainBoard->init();
    ui->time->setText("60");
    turn = "computer";
    if (turn.compare("player") == 0) {
        playerTile = BLACK;
        computerTile = WHITE;
    }
    else {
        playerTile = WHITE;
        computerTile = BLACK;
    }
    btnGroup = new QButtonGroup(this);
    btnGroup->addButton(ui->ai,0);
    btnGroup->addButton(ui->human,1);

    background.load(":/rsc/board.png");
    black.load(":/rsc/black.png");
    white.load(":/rsc/white.png");
    hintwhite.load(":/rsc/whitepotential.png");
    hintblack.load(":/rsc/blackpotential.png");
    hintred.load(":/rsc/redpotential.png");

    connect(ui->pushButton, SIGNAL(clicked()),this,SLOT(ClickButton()));
}

void MainWindow::start(){
    if(isGameStart == true && !gameOver){
        if(turn.compare("computer") == 0){
            while(turn.compare("computer") == 0)
            {
                cout << "Turn in start: " << turn << endl;
                if (turn.compare("computer") == 0 && bisGameOver(mainBoard) == false&&!gameOver) {
                    int next = getComputerMove(mainBoard, computerTile, COMPUTER_PARA); //运行MCTS，失败返回-1
                    cout << "next: " << next << endl;
                    if (next!= -1 ) { //如果computer有子可下，就makeMove（实际是MCTS运行成功）
                        bmakeMove(mainBoard, computerTile, next);
                        Last = next;
                        printReport();
                        cout << "AI GO: " << next<< endl;
                        ui->AIGO->setText(QString::number(next));
                    }
                    else {
                        if (bgetValidMove(mainBoard, playerTile).size() == 0) { //如果computer和player都无子可下
                            gameOver = true;
                        }
                    }
                    if (bgetValidMove(mainBoard, playerTile).size() != 0) { //如果player一直无子可下，computer就一直下下去
                        turn = "player";
                    }
                    cout << "---------------------------------------" << endl;
                }
                printReport();
                repaint();
            }
        }
    }
}

void MainWindow::ClickButton(){
    if(isGameStart == true){
        Last = -1;
        isGameStart = false;
        mainBoard->init();
        int originpos = 64;
        delete root;
        root = new MCTS(originpos,mainBoard, BLACK);
        ui->pushButton->setText("Start");
        Score score = bgetScoreofBoard(mainBoard);
        if (score.black_score == 0 || score.white_score == 0 || bisGameOver(mainBoard)) {
            gameOver = true;
        }
        ui->white->setText(QString::number(score.white_score,10));
        ui->black->setText(QString::number(score.black_score,10));
        repaint();
    }

    else if(isGameStart == false){
        ui->pushButton->setText("Reset");
        QString time = ui->time->text();
        this->TIME = time.toDouble();
        switch(btnGroup->checkedId())
        {
        case 0:
            turn = "computer";
            isGameStart = true;
            gameOver = false;
            break;
        case 1:
            turn = "player";
            isGameStart = true;
            gameOver = false;
            break;
        }
        if (turn.compare("player") == 0) {
            playerTile = BLACK;
            computerTile = WHITE;
        }
        else {
            playerTile = WHITE;
            computerTile = BLACK;
        }
        start();
    }
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    this->resize(600,400);
    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.drawPixmap(0,0,400,400,background);

    int tile;
    if(turn.compare("computer") == 0){
        tile = computerTile;
    }
    else{
        tile = playerTile;
    }

    int markHaveDraw[8][8];
    for(int i=0;i<8;i++){
        for(int j=0;j<8;j++){
            markHaveDraw[i][j] = -1;
        }
    }
    vector<StateNode> nodes;
        nodes = bgetValidMove(mainBoard,tile);
        for(unsigned int i=0;i < nodes.size();i++){
            int x = nodes[i].position%8;
            int y = nodes[i].position/8;
            markHaveDraw[x][y] = tile;
            //cout<<nodes[i]<<endl;
        }
    int count = 0;
    for(int j=0;j<8;j++){
        for(int i=0;i<8;i++){
            if(mainBoard->get(count,WHITE)){
                painter.drawPixmap(0+50*i, 0+50*j, 50, 50, white);
            }else if(mainBoard->get(count,BLACK)){
                painter.drawPixmap(0+50*i, 0+50*j, 50, 50, black);
            }

            if(markHaveDraw[i][j] == BLACK){
                painter.drawPixmap(0+50*i, 0+50*j, 50, 50, hintblack);
            }
            if(markHaveDraw[i][j] == WHITE){
                painter.drawPixmap(0+50*i, 0+50*j, 50, 50, hintwhite);
            }
            count++;
        }
    }
    if(Last != -1){
        painter.drawPixmap(0+50*(Last%8), 0+50*(Last/8), 50, 50, hintred);
    }
}

void MainWindow::mousePressEvent(QMouseEvent *e){
    //cout << "start" << mainBoard->pieces[BLACK]<< " " <<  mainBoard->pieces[WHITE]<< endl;
    if(isGameStart == false || gameOver == true){
        cout << "return: " << isGameStart<<gameOver<< endl;
        return;
    }
    if (turn.compare("player") == 0 && bisGameOver(mainBoard) == false && !gameOver ) {
        //cout << e->x() << " " << e->y() << endl;
        mousex = e->x()/50;
        mousey = e->y()/50;
        //cout << "(mousex,mousey):" << mousex << " " << mousey << endl;
        int mouse_pos = 8*mousey + mousex;
        //cout << "mouse_pos:" << mouse_pos << endl;
        if (bmakeMove(mainBoard, playerTile, mouse_pos) == true) {
            Last = mouse_pos;
            //cout<< playerTile << endl;
            //cout << "last！" <<dec<<Last << endl;
            //cout << "这里是" <<hex<< mainBoard->pieces[BLACK]<< " " <<  mainBoard->pieces[WHITE]<< endl;
            if (bgetValidMove(mainBoard, computerTile).size() != 0) {
                turn = "computer";
                //cout << "conputer的turn了！" << endl;
            }
        }
        else {
            cout << "Reinput: " << endl;
        }
    }
    repaint();
    //printBoard(mainBoard);

    Score score = bgetScoreofBoard(mainBoard);

    if (score.black_score == 0 || score.white_score == 0 || bisGameOver(mainBoard)) {
        gameOver = true;
    }
    //cout << "score: " << score.black_score<< " " <<  score.white_score<< endl;
    while(turn.compare("computer") == 0 && !gameOver)
    {
        cout << "Turn: " << turn << endl;
        if (turn.compare("computer") == 0 && bisGameOver(mainBoard) == false&&!gameOver) {
            int next = getComputerMove(mainBoard, computerTile,COMPUTER_PARA);
            if (next != -1 ) {
                bmakeMove(mainBoard, computerTile, next);
                Last = next;
                printReport();
                cout << "AI GO: " << next << endl;
                ui->AIGO->setText(QString::number(next));
            }
            else {
                if (bgetValidMove(mainBoard, playerTile).size() == 0) {
                    gameOver = true;
                }
            }
            if (bgetValidMove(mainBoard, playerTile).size() != 0) {
                turn = "player";
            }
            cout << "---------------------------------------" << endl;
        }
        //printBoard(mainBoard);
        repaint();
    }
    score = bgetScoreofBoard(mainBoard);
    if (score.black_score == 0 || score.white_score == 0 || bisGameOver(mainBoard)) {
        gameOver = true;
    }
    ui->white->setText(QString::number(score.white_score,10));
    ui->black->setText(QString::number(score.black_score,10));

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::init() {
    int originpos;
    originpos  = 64;
    root = new MCTS(originpos, mainBoard, BLACK);
    Last = -1;

}

void MainWindow::printReport(){

    for(int i=0;i<root->child.size();i++){
        cout << "[" << root->child[i]->position << "]  ";
        double winvalue = root->child[i]->score*1.0/root->child[i]->visit;
        double randomvalue = root->child[i]->C * sqrt(2 * double(root->child[i]->father->visit / root->child[i]->visit));
        printf("%.4f, %.4f, sum=%.4f, real=%.4f\n", winvalue,randomvalue, winvalue+randomvalue, root->child[i]->realscore);
    }
}

vector<int> MainWindow::bisValidMove(BitBoard* board, int tile, int pos) {
    vector<int> path;
//    if (board->getmy(pos) || board->getop(pos) || pos >= 64 || pos < 0) {
//        return false;
//    } //非法的move
/* 1.邻居是不是对手颜色 2.邻居的后面第一个非对手颜色的格子是否为自己颜色*/
    int temp;
    int optile = getEnemyTile(tile);

    if(pos%8 < 7){
        temp=pos+1;
        while( temp%8){
            if(board->pieces[optile] & mainBoard->mask[temp]){
                path.push_back(temp);
                temp++;
            }
            else if(board->pieces[tile] & mainBoard->mask[temp]){
                if(path.size())
                    return path;
                else break;
            }else
                break;
        }
    }
    path.clear();

    if(pos%8 >0){
        temp=pos-1;
        while(temp%8 >=0){
            if(board->pieces[optile] & mainBoard->mask[temp]){
                path.push_back(temp);
                temp--;
             }
            else if(board->pieces[tile] & mainBoard->mask[temp]){
                if(path.size()) return path;
                else break;
            }else
                break;
        }
    }
    path.clear();

    if(pos < 56){
        temp=pos+8;
        while(temp < 64){
            if(board->pieces[optile] & mainBoard->mask[temp]){
                path.push_back(temp);
                temp += 8;
                }
            else if(board->pieces[tile] & mainBoard->mask[temp]){
                if(path.size()) return path;
                 else break;
            }else
                break;
        }
    }
    path.clear();

    if(pos > 7){
        temp=pos-8;
        while(temp){
            if(board->pieces[optile] & mainBoard->mask[temp]){
                path.push_back(temp);
                temp -= 8;
                }
            else if(board->pieces[tile] & mainBoard->mask[temp]){
                if(path.size()) return path;
                 else break;
            }else
                break;
        }
    }
    path.clear();

    if(pos > 7 && (pos+1)%8){
        temp=pos-7;
        while(temp/8 >= 0 && temp%8 >=0){
            if(board->pieces[optile] & mainBoard->mask[temp]){
                path.push_back(temp);
                temp-=7;
                }
            else if(board->pieces[tile] & mainBoard->mask[temp]){
                if(path.size()) return path;
                 else break;
            }else
                break;
        }
    }

    path.clear();
    if(pos <56 && pos%8 ){
         temp=pos+7;
         while(temp/8 < 8 && temp%8){
             if(board->pieces[optile] & mainBoard->mask[temp]){
                 path.push_back(temp);
                 temp+=7;
                 }
             else if(board->pieces[tile] & mainBoard->mask[temp]){
                 if(path.size()) return path;
                  else break;
             }else
                 break;
         }
    }
    path.clear();

    if(pos >7 && pos%8){
        temp=pos-9;
        while(temp/8 >= 0 && temp%8 >= 0){
            if(board->pieces[optile] & mainBoard->mask[temp]){
                path.push_back(temp);
                temp-=9;
                }
            else if(board->pieces[tile] & mainBoard->mask[temp]){
                if(path.size()) return path;
                 else break;
            }else
                break;
        }
    }
    path.clear();

    if(pos < 56 && (pos+1)%8){
        temp=pos+9;
        while(temp/8 < 8 && temp%8){
            if(board->pieces[optile] & mainBoard->mask[temp]){
                path.push_back(temp);
                temp+=9;
                }
            else if(board->pieces[tile] & mainBoard->mask[temp]){
                if(path.size()) return path;
                 else break;
            }else
                break;
        }
    }
    path.clear();

    return path;
}


bool MainWindow::bmakeMove(BitBoard* board, int tile, int pos) {
    vector<int> path = bisValidMove(board, tile, pos);
    int optile = getEnemyTile(tile);
//    for(int i=0; i<path.size();i++){
//        cout<< "应该被吃掉的棋子:"<< path[i] << endl;
//    }
    if (!path.size())
        return false;
    board->pieces[tile] |= mainBoard->mask[pos];
    while (path.size()) {
        int optile_pos = path.back();
        path.pop_back();
        board->pieces[tile] |= mainBoard->mask[optile_pos];
        board->pieces[optile] &= mainBoard->opmask[optile_pos];
    }
   // cout << "makemove结束后board：" <<hex<< board->pieces[BLACK]<< " " <<  board->pieces[WHITE]<< endl;
   // cout << "makemove结束后mainboard：" <<hex<< mainBoard->pieces[BLACK]<< " " <<  mainBoard->pieces[WHITE]<< endl;
    return true;
}

vector<StateNode> MainWindow::bgetValidMove(BitBoard* board, int tile, bool forpolicy) {
    vector<StateNode> validMoves;
    int optile = getEnemyTile(tile);
    /*乞丐版：遍历64个格子，看它们是不是validmove
      浙大版：只关注空的格子 && 邻居&oppieces非空
      清北版：只关注对方棋子的邻居*/
    //unsigned long long occupied = (board->mypieces|board->oppieces) ;
    //cout << "bgetValidMove里的board：" <<hex<< board->pieces[BLACK]<< " " <<  board->pieces[WHITE]<< endl;
    //cout << "bgetValidMove里的tile：" <<dec<< tile<< endl;
    for (int i = 0; i < 64; i++){
        if( ~(board->pieces[tile] | board->pieces[optile] | mainBoard->opmask[i]) ){ //ownneighbor[i]board->validneighbor[i] & board->oppieces
            // cout << "空格子: "<<dec<<i << " ";
            //if(tile == playerTile) cout << "空格子: "<<dec<<i << " ";
            if(bisValidMove(board,tile,i).size()){
                BitBoard * dupboard = getBitBoardCopy(board);
                bmakeMove(dupboard,tile,i);
                StateNode tmpstate(i,*dupboard);
                validMoves.push_back(tmpstate);
                //if(tile == playerTile) cout << "有效格子: "<<dec<<i << " ";
            }
        }
    }
    //cout <<  endl;
    return validMoves;
}

bool MainWindow::bisGameOver(BitBoard* board) {
    if ((board->pieces[BLACK] | board->pieces[WHITE]) == 0 || (board->pieces[BLACK] | board->pieces[WHITE]) == 0x1111111111111111) {
        return true;
    }
    return false;
}

BitBoard* MainWindow::getBitBoardCopy(BitBoard* bitboard) {
    BitBoard* dupBoard = new BitBoard();
    dupBoard->pieces[BLACK] = bitboard->pieces[BLACK];
    dupBoard->pieces[WHITE] = bitboard->pieces[WHITE];
    return dupBoard;
}

Result MainWindow::treePolicy(BitBoard* board, MCTS* state, int tile) {
    /*层级遍历，直到找到一个可扩展的节点，
     * 若遇到未扩展的节点就把它的所有孩子加进树中，并把它返回
     * 若树中的所有节点都已扩展，就把当前节点的bestChild作为result返回*/
    MCTS* expand = nullptr;
    queue<MCTS*> q;
    q.push(state);
    while (!q.empty()) { //树的层级遍历，直到找到一个可扩展的节点
        MCTS* n = q.front();
        q.pop();
        if (n->expandable == false) { //如果pop出来的节点不可扩展，那么把它的孩子全部入队
            for (unsigned int i = 0; i < n->child.size(); i++) {
                q.push(n->child[i]);
            }
        }
        else {
            expand = n;
            break;
        }
    }
    if (expand == nullptr) { //树中没有可扩展的节点
        expand = state->child[state->bestChild()]; //找到当前节点的bestChild
        bmakeMove(board, tile, expand->position);
        tile = getEnemyTile(tile); //取反执子人
    }
    else {   //树中有可扩展的节点
        expand->expandable = false;
        stack<MCTS*> s;
        MCTS* node = expand;
        while (!node->root) { //把它和它的所有祖先push入栈
            s.push(node);
            node = node->father;
        }
        while (!s.empty()) {
            node = s.top();
            bmakeMove(board, tile, node->position);
            s.pop();
            tile = getEnemyTile(tile);
        }                       //从头带尾走一遍直到board变成了现在的state的样子，我觉得可以通过把board存下来优化
        vector<StateNode>nodes = bgetValidMove(board, tile);  //getValidMove并添加到树中去
        for (int i = 0; i < nodes.size(); i++) {
            MCTS* mnode = new MCTS(nodes[i], getEnemyTile(expand->mytile));
            mnode->father = expand;
            expand->child.push_back(mnode);
        }
    }

    return ResultNode(expand, tile);
}

Score MainWindow::bgetScoreofBoard(BitBoard* board) {
    int op = 0;
    int my = 0;
    unsigned long long m,n;
    n = board->pieces[BLACK];
    m = board->pieces[WHITE];
    for (my =0; n; ++my)
        n &= (n -1) ; // 清除最低位的1

    for (op =0; m; ++op)
        m &= (m -1) ; // 清除最低位的1
    return Score(op, my);
}

Score MainWindow::bdefaultPolicy(BitBoard* board, int tile) {
    int flag = 0;
    srand((unsigned)time(NULL));
    while (!bisGameOver(board)) {
        vector<StateNode> nodes = bgetValidMove(board, tile, true);
        if (nodes.size()) {
            flag = 0;
            int random = rand() % (nodes.size());
            int pos = nodes[random].position;
            bmakeMove(board, tile, pos);
        }
        else {
            flag += 1;
            if (flag == 2) {
                return bgetScoreofBoard(board);
            }
        }
    }
    return bgetScoreofBoard(board);
}

void MainWindow::backup(MCTS* bottom, int tile,int computertile, Score score) {
    tile = getEnemyTile(tile);
    int manTile = getEnemyTile(computerTile);
    int deta = score.get(computerTile) - score.get(manTile);

    if (deta >= 0)
        deta = 1;
    else
        deta = 0;

    MCTS* node = bottom;
    while (true) {
        node->score += deta;
        node->visit ++;

        // test
        //if (node->child.size() > 0) {
        //	int tmp = -100000;
        //	for (int i = 0; i < node->child.size(); i++) {
        //		if (node->child[i]->score > tmp) {
        //			tmp = node->child[i]->score;
        //		}
        //	}
        //	node->score = tmp;
        //}

        if (node->root == true)
            break;

        node = node->father;
        tile = getEnemyTile(tile);
    }
}

int MainWindow::uctSearch(MCTS* root, BitBoard* board, int computertile, int DEPTH, double TIME) {
    vector<StateNode> nodes = bgetValidMove(board, computertile);
    cout<< "uctSearch--"<< "computertile: "<< computertile << endl;
    if (nodes.size() == 0) {
        cout<< "return -1" << endl;
        return -1;
    }
    else if (nodes.size() == 1) {
        return nodes[0].position;
    }
    for (unsigned int i = 0; i < nodes.size(); i++) { //把所有的validmove都作为root的孩子，并且把root的访问次数用所有孩子的访问次数之和更新
        //if (isOnCorner(x, y) == true) {
        //	return Postion(x, y);
        //} 先下角落
        MCTS* mnodes = new MCTS(nodes[i], computertile);
        mnodes->father = root; //更新validmove的父亲为root
        root->child.push_back(mnodes);
        root->visit += mnodes->visit;
    }

    clock_t starttime = clock();
    int playtime = 0;
    while (true) {
        playtime++;
        BitBoard* dupBoard = getBitBoardCopy(board);
        Result result = treePolicy(dupBoard, root, computertile); /*层级遍历，直到找到一个可扩展的节点，若遇到未扩展的节点就把它的所有孩子加进树中，并把它返回
                                                         * 若树中的所有节点都已扩展，就把当前节点的bestChild作为result返回*/
        Score score = bdefaultPolicy(dupBoard, result.tile); //score是相对ai来说的, dupBoard在treePolicy期间更新了
        backup(result.state, result.tile, computertile, score);
        delete dupBoard;
        clock_t endtime = clock();
        if ((endtime - starttime)/CLOCKS_PER_SEC >= TIME) { /*修改时间单位的地方，目前单位为1s*/
            cout << playtime << endl;
            break;
        }
    }
    //cout << "Rrootvisit  " << root->visit << endl;
    //for (int i = 0; i < root->child.size(); i++) {
    //	cout << root->child[i]->pos.x << "," << root->child[i]->pos.y << ":" << root->child[i]->visit << " " << root->child[i]->score << endl;
    //}
    int index = root->bestChild();
//    cout << "final " << root->child[index]->score*1.0 / root->child[index]->visit +
//        root->child[index]->C * sqrt(2 * log(root->child[index]->father->visit / root->child[index]->visit))  << endl;
    return root->child[index]->position;

}

int MainWindow::getComputerMove(BitBoard* board,int tile, double newc=0.02) {
    BitBoard* dupBoard = getBitBoardCopy(board);
    cout<< "getComputerMove" << endl;
    root->C = newc;
    root->rootClear();
    return uctSearch(root, dupBoard, tile, DEPTH, TIME);
}


MCTS::MCTS(StateNode state, int mytile) {
    //child = nullptr;
    bestchild = nullptr;
    visit = 0;
    score = 0;
    expandable = true;
    father = nullptr;
    root = false;
    this->position = state.position;
    this->mytile = mytile;
    this->board.pieces[BLACK] = state.board.pieces[BLACK];
    this->board.pieces[WHITE] = state.board.pieces[WHITE];
}

MCTS::MCTS(int position, BitBoard* board, int mytile) {
    //child = nullptr;
    bestchild = nullptr;
    visit = 0;
    score = 0;
    expandable = true;
    father = nullptr;
    root = false;
    this->position = position;
    this->mytile = mytile;
    this->board.pieces[BLACK] = board->pieces[BLACK];
    this->board.pieces[WHITE] = board->pieces[WHITE];
}

void MCTS::rootClear() { //把root变为一个未访问过的，未扩展过的空节点，没有对position动手脚
    while (child.empty() == false) {
        MCTS* ele = child.back();
        child.pop_back();
        delete ele;
    }
    child.clear();
    visit = 0;
    bestchild = nullptr;
    expandable = false;
    root = true;
}

double MCTS::calculate() {
    double sum = this->score*1.0 / this->visit + this->C * sqrt(2 * double(this->father->visit / this->visit)); //公式
    if(mainBoard->pieces[this->mytile] & 0x8100000000000081){
        if(mainBoard->pieces[this->mytile] & 0x4281000000008142)
            sum *= 0.95; //如果(1,0)or(0,1)有子，且最边角是对手的子，sum打85折
        else if(mainBoard->pieces[this->mytile] & 0x0042000000004200)
            sum *= 0.87; //如果(1,1)有子，且最边角是对手的子，sum打87折
    }else if(mainBoard->pieces[this->mytile] & 0x8100000000000081)
         sum *= 1.15; //最边角是自己的子，sum乘1.15倍
    else if(mainBoard->pieces[this->mytile] & 0xFF818181818181FF)
        sum *= 1.1; //是周边但非角落，sum乘1.1倍
    this->realscore = sum;
    return sum;
}

int MCTS::bestChild() {
    double max = child[0]->calculate();
    int best = 0;
    double tmp;
    for (int i = 1; i < child.size(); i++) {
        tmp = child[i]->calculate();
        if (tmp > max) {
            max = tmp;
            best = i;
        }
    }
    return best;
}
