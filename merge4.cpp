#include <iostream>
#include <vector>
#include <array>
#include <algorithm>
#include <climits>
#include <fstream>
#include <cstdlib>
#include <ctime>
using namespace std;
#define DEPTH 4

const int dx[] = {-1, 0, 1, -1, 1, -1, 0, 1};
const int dy[] = {1, 1, 1, 0, 0, -1, -1, -1};
const int SIZE = 15;

#define min(a,b) a>b?b:a
#define max(a,b) a>b?a:b

enum SPOT_STATE {
    EMPTY = 0,
    BLACK = 1,
    WHITE = 2
};

typedef array<array<int, SIZE>, SIZE> Board;
bool point_on_center = true;

struct Point {
    int x, y;
	Point() : Point(0, 0) {}
	Point(float x, float y) : x(x), y(y) {}
	bool operator==(const Point& rhs) const {
		return x == rhs.x && y == rhs.y;
	}
	bool operator!=(const Point& rhs) const {
		return !operator==(rhs);
	}
	Point operator+(const Point& rhs) const {
		return Point(x + rhs.x, y + rhs.y);
	}
	Point operator-(const Point& rhs) const {
		return Point(x - rhs.x, y - rhs.y);
	}
};

class State{
    public:
        Board board;
        int state_value;
        int player;
        bool leaves = false;
        vector<Point> how_to_move;

        State(){};
        State(Board board, int player) :board(board), player(player){
            this->search_how_to_move();
        }
        State* nextState(Point nextstep);

        void search_how_to_move();
        void check_node_state();     
        void set_state_value(int curplayer);   
};

vector<Point> tomove;
void getboard_tomove(){
    for(int i=0;i<SIZE;i++){
        for(int j=0;j<SIZE;j++)
            tomove.push_back(Point(i,j));
    }
}

//There are several terms in Gomuku
//term |    description                     |   weights     | bitset description
//連五 | 五個同色的棋子連在一起(無法防守)     |      7        |     11111
//活四 | 有兩個連五點(無法防守)               |     6         |    xx1111
//沖四 | 有一個連五點                        |      5        |    1x111、11x11、111x1、x11110、01111x
//活三 | 可以連成活四的三                    |      4        |     x111x、1x11、11x1
//眠三 | 可以連成沖四的三                    |      3        |    xx1110、0111xx、x1x110、011x1x、
                                                            //  x11x10、01x11x、1xx11、1x1x1、0x111x0
//活二 | 可以連成活三的二                    |      2        |    xx11xx、x1x1x、1xx1
//眠二 | 可以連成眠三的二                    |      1        |    xxx110、011xxx、xx1x10、01x1xx、
                                                            //  1xx10、01xx1、1xxx1
//We can use the way to calculate state value

//連五
inline int linefive(const Board& board, int i, int j, int player){
    if(i<0 || i>=SIZE || j<0 || j>=SIZE)    return 0;
    int amo = 0;
    //horizontally
    if(j<SIZE-4){
        if(board[i][j] == board[i][j+1] 
            && board[i][j+1] == board[i][j+2]
            && board[i][j+2] == board[i][j+3]
            && board[i][j+3] == board[i][j+4]
            && board[i][j+4] == player)
               amo++;
    }

    //vertically
    if(i<SIZE-4){
        if(board[i][j] == board[i+1][j]
            && board[i+1][j] == board[i+2][j]
            && board[i+2][j] == board[i+3][j]
            && board[i+3][j] == board[i+4][j]
            && board[i+4][j] == player)
                amo++;
    }

    //cross(left-right)
    if(i<SIZE-4 && j<SIZE-4){
        if(board[i][j] == board[i+1][j+1]
            && board[i+1][j+1] == board[i+2][j+2]
            && board[i+2][j+2] == board[i+3][j+3]
            && board[i+3][j+3] == board[i+4][j+4]
            && board[i+4][j+4] == player)
                amo++;
    }

    //cross(right-left)
    if(i<SIZE-4 && j>=4){
        if(board[i][j] == board[i+1][j-1]
            && board[i+1][j-1] == board[i+2][j-2]
            && board[i+2][j-2] == board[i+3][j-3]
            && board[i+3][j-3] == board[i+4][j-4]
            && board[i+4][j-4] == player)
                amo++;
    }
    return amo;
}

//活四
inline int livefour(const Board& board, int i, int j, int player){
    if(i<0 || i>=SIZE || j<0 || j>=SIZE)    return 0;
    int amo = 0;
    //vertically
    if(i<SIZE-5){
        if(board[i][j] == board[i+5][j]
            && board[i+5][j] == EMPTY
            && board[i+1][j] == board[i+2][j]
            && board[i+2][j] == board[i+3][j]
            && board[i+3][j] == board[i+4][j]
            && board[i+4][j] == player) 
                amo++;      
    }   
    
    //horizontally
    if(j<SIZE-5){
        if(board[i][j] == board[i][j+5]
            && board[i][j+5] == EMPTY
            && board[i][j+1] == board[i][j+2]
            && board[i][j+2] == board[i][j+3]
            && board[i][j+3] == board[i][j+4]
            && board[i][j+4] == player)
                amo++;
    }

    //cross(left-right)
    if(i<SIZE-5 && j<SIZE-5){
        if(board[i][j] == board[i+5][j+5]
            && board[i+5][j+5] == EMPTY
            && board[i+1][j+1] == board[i+2][j+2]
            && board[i+2][j+2] == board[i+3][j+3]
            && board[i+3][j+3] == board[i+4][j+4]
            && board[i+4][j+4] == player)
                amo++;
    }

    //cross(right-left)
    if(i<SIZE-5 && j>=5){
        if(board[i][j] == board[i+5][j-5]
            && board[i+5][j-5] == EMPTY
            && board[i+1][j-1] == board[i+2][j-2]
            && board[i+2][j-2] == board[i+3][j-3]
            && board[i+3][j-3] == board[i+4][j-4]
            && board[i+4][j-4] == player)
                amo++;
    }
    return amo;
}

//沖四
inline int rushfour(const Board& board, int i, int j, int player){
    if(i<0 || i>=SIZE || j<0 || j>=SIZE)    return 0;
    int amo = 0;
    //horizontally
    if(i<SIZE-5){
        if( (board[i+1][j] == board[i+2][j]
            && board[i+2][j] == board[i+3][j]
            && board[i+3][j] == board[i+4][j]
            && board[i+4][j] == player) && (
            (board[i][j] == EMPTY && board[i+5][j] == 3-player)
            || (board[i][j] == 3-player && board[i+5][j] == EMPTY)))
                amo++;
    }
    
    if(i<SIZE-4){
        if(board[i][j] == board[i+4][j]
            && board[i+4][j] == player && (
            (board[i+1][j] == board[i+2][j] && board[i+2][j] == player && board[i+3][j] == EMPTY) ||
            (board[i+1][j] == board[i+3][j] && board[i+3][j] == player && board[i+2][j] == EMPTY) ||
            (board[i+2][j] == board[i+3][j] && board[i+3][j] == player && board[i+1][j] == EMPTY)))
               amo++;
    }
    //vertically
    if(j<SIZE-5){
        if( (board[i][j+1] == board[i][j+2]
            && board[i][j+2] == board[i][j+3]
            && board[i][j+3] == board[i][j+4]
            && board[i][j+4] == player) && (
            (board[i][j] == EMPTY && board[i][j+5] == 3-player)
            || (board[i][j] == 3-player && board[i][j+5] == EMPTY)))
                amo++;
    }

    if(j<SIZE-4){
        if(board[i][j] == board[i][j+4]
            && board[i][j+4] == player && (
            (board[i][j+1] == board[i][j+2] && board[i][j+2] == player && board[i][j+3] == EMPTY) ||
            (board[i][j+1] == board[i][j+3] && board[i][j+3] == player && board[i][j+2] == EMPTY) ||
            (board[i][j+2] == board[i][j+3] && board[i][j+3] == player && board[i][j+1] == EMPTY)))
                amo++;
    }

    //cross(left-right)
    if(i<SIZE-5 && j<SIZE-5){
        if( (board[i+1][j+1] == board[i+2][j+2]
            && board[i+2][j+2] == board[i+3][j+3]
            && board[i+3][j+3] == board[i+4][j+4]
            && board[i+4][j+4] == player) && (
            (board[i][j] == EMPTY && board[i+5][j+5] == 3-player)|| 
            (board[i][j] == 3-player && board[i+5][j+5] == EMPTY)))
                amo++;
    }

    if(i<SIZE-4 && j<SIZE-4){
        if(board[i][j] == board[i+4][j+4]
            && board[i+4][j+4] == player && (
            (board[i+1][j+1] == board[i+2][j+2] && board[i+2][j+2] == player && board[i+3][j+3] == EMPTY) ||
            (board[i+1][j+1] == board[i+3][j+3] && board[i+3][j+3] == player && board[i+2][j+2] == EMPTY) ||
            (board[i+2][j+2] == board[i+3][j+3] && board[i+3][j+3] == player && board[i+1][j+1] == EMPTY)))
                amo++;
    }

    //cross(right-left)
    if(i<SIZE-5 && j>=5){
        if( (board[i+1][j-1] == board[i+2][j-2]
            && board[i+2][j-2] == board[i+3][j-3]
            && board[i+3][j-3] == board[i+4][j-4]
            && board[i+4][j-4] == player) && (
            (board[i][j] == EMPTY && board[i+5][j-5] == 3-player) ||
            (board[i][j] == 3-player && board[i+5][j-5] == EMPTY)))
                amo++;
    }

    if(i<SIZE-4 && j>=4){
        if(board[i][j] == board[i+4][j-4]
            && board[i+4][j-4] == player && (
            (board[i+1][j-1] == board[i+2][j-2] && board[i+2][j-2] == player && board[i+3][j-3] == EMPTY) ||
            (board[i+1][j-1] == board[i+3][j-3] && board[i+3][j-3] == player && board[i+2][j-2] == EMPTY) ||
            (board[i+2][j-2] == board[i+3][j-3] && board[i+3][j-3] == player && board[i+1][j-1] == EMPTY)))
                amo++;
    }
    return amo;
}

//活三
inline int livethree(const Board& board, int i, int j, int player){
    if(i<0 || i>=SIZE || j<0 || j>=SIZE)    return 0;
    int amo = 0;
    //horizontally
    if(i<SIZE-4){
        if(board[i][j] == board[i+4][j]
            && board[i+4][j] == EMPTY
            && board[i+1][j] == board[i+2][j]
            && board[i+2][j] == board[i+3][j]
            && board[i+3][j] == player)
                amo++;
    }

    if(i<SIZE-5){
        if((board[i+1][j] == board[i+4][j]
            && board[i+4][j] == player
            && board[i][j] == board[i+5][j]
            && board[i+5][j] == EMPTY) && (
            (board[i+2][j] == player && board[i+3][j] == EMPTY) ||
            (board[i+3][j] == player && board[i+2][j] == EMPTY)))
                amo++;
    }

    //vertically
    if(j<SIZE-4){
        if(board[i][j] == board[i][j+4]
            && board[i][j+4] == EMPTY
            && board[i][j+1] == board[i][j+2]
            && board[i][j+2] == board[i][j+3]
            && board[i][j+3] == player)
                amo++;
    }

    if(j<SIZE-5){
        if((board[i][j+1] == board[i][j+4]
            && board[i][j+4] == player
            && board[i][j] == board[i][j+5]
            && board[i][j+5] == EMPTY) && (
            (board[i][j+2] == player && board[i][j+3] == EMPTY) ||
            (board[i][j+3] == player && board[i][j+2] == EMPTY)))
                amo++;
    }

    //cross(left-right)
    if(i<SIZE-4 && j<SIZE-4){
        if(board[i][j] == board[i+4][j+4]
            && board[i+4][j+4] == EMPTY
            && board[i+1][j+1] == board[i+2][j+2]
            && board[i+2][j+2] == board[i+3][j+3]
            && board[i+3][j+3] == player)
                amo++;
    }

    if(i<SIZE-5 && j<SIZE-5){
        if((board[i+1][j+1] == board[i+4][j+4]
            && board[i+4][j+4] == player
            && board[i][j] == board[i+5][j+5]
            && board[i+5][j+5] == EMPTY) && (
            (board[i+2][j+2] == player && board[i+3][j+3] == EMPTY) ||
            (board[i+3][j+3] == player && board[i+2][j+2] == EMPTY)
            ))
                amo++;
    }

    //cross(right-left)
    if(i<SIZE-5 && j>=5){
        if(board[i][j] == board[i+4][j-4]
            && board[i+4][j-4] == EMPTY
            && board[i+1][j-1] == board[i+2][j-2]
            && board[i+2][j-2] == board[i+3][j-3]
            && board[i+3][j-3] == player)
                amo++;
    }

    if(i<SIZE-4 && j>=4){
        if((board[i+1][j-1] == board[i+4][j-4]
            && board[i+4][j-4] == player
            && board[i][j] == board[i+5][j-5]
            && board[i+5][j-5] == EMPTY) && (
            (board[i+2][j-2] == player && board[i+3][j-3] == EMPTY) ||
            (board[i+3][j-3] == player && board[i+2][j-2] == EMPTY)
            ))
                amo++;
    }
    return amo;
}

//眠三
inline int sleepthree(const Board& board, int i, int j, int player){
    if(i<0 || i>=SIZE || j<0 || j>=SIZE)    return 0;
    int amo = 0;
    //horizontally
    if(i<SIZE-4){
        if(board[i][j] == board[i+4][j]
            && board[i+4][j] == player&&(
            (board[i+1][j] == board[i+2][j] && board[i+2][j] == EMPTY && board[i+3][j] == player) ||
            (board[i+1][j] == player && board[i+2][j] == board[i+3][j] && board[i+3][j] == EMPTY) ||
            (board[i+1][j] == board[i+3][j] && board[i+3][j] == EMPTY && board[i+2][j] == player)))
                amo++;
    }

    if(i<SIZE-5){
        if(board[i+2][j] == board[i+3][j]
            && board[i+3][j] == player &&(
            (board[i][j] == board[i+1][j] && board[i+1][j] == EMPTY && board[i+4][j] == player && board[i+5][j]==3-player)||
            (board[i][j] == 3-player && board[i+1][j] == player && board[i+4][j] == board[i+5][j] && board[i+5][j]==EMPTY)))
                amo++;

        if(board[i+1][j] == board[i+3][j]
            && board[i+3][j] == board[i+4][j]
            && board[i+4][j] == player &&(
            (board[i][j] == board[i+2][j] && board[i+2][j] == EMPTY && board[i+5][j] == 3-player) ||
            (board[i][j] == 3-player && board[i+2][j] == board[i+5][j]  && board[i+5][j] == EMPTY)))
                amo++;
        
        if(board[i+1][j] == board[i+2][j]
            && board[i+2][j] == board[i+4][j]
            && board[i+4][j] == player &&(
            (board[i][j] == board[i+3][j] && board[i+3][j] == EMPTY && board[i+5][j] == 3-player) ||
            (board[i][j] == 3-player && board[i+2][j] == board[i+5][j]  && board[i+5][j] == EMPTY)))
                amo++;
    }

    if(i<SIZE-6){
        if(board[i][j] == board[i+6][j]
            && board[i+6][j] == 3-player
            && board[i+1][j] == board[i+5][j]
            && board[i+5][j] == EMPTY
            && board[i+2][j] == board[i+3][j]
            && board[i+3][j] == board[i+4][j]
            && board[i+4][j] == player)
                amo++;
    }

    //vertically
    if(j<SIZE-4){
        if(board[i][j] == board[i][j+4]
            && board[i][j+4] == player&&(
            (board[i][j+1] == board[i][j+2] && board[i][j+2] == EMPTY && board[i][j+3] == player) ||
            (board[i][j+1] == player && board[i][j+2] == board[i][j+3] && board[i][j+3] == EMPTY) ||
            (board[i][j+1] == board[i][j+3] && board[i][j+3] == EMPTY && board[i][j+2] == player)))
                amo++;
    }

    if(j<SIZE-5){
        if(board[i][j+2] == board[i][j+3]
            && board[i][j+3] == player &&(
            (board[i][j] == board[i][j+1] && board[i][j+1] == EMPTY && board[i][j+4] == player && board[i][j+5]==3-player)||
            (board[i][j] == 3-player && board[i][j+1] == player && board[i][j+4] == board[i][j+5] && board[i][j+5]==EMPTY)))
                amo++;

        if(board[i][j+1] == board[i][j+3]
            && board[i][j+3] == board[i][j+4]
            && board[i][j+4] == player &&(
            (board[i][j] == board[i][j+2] && board[i][j+2] == EMPTY && board[i][j+5] == 3-player) ||
            (board[i][j] == 3-player && board[i][j+2] == board[i][j+5]  && board[i][j+5] == EMPTY)))
                amo++;
        
        if(board[i][j+1] == board[i][j+2]
            && board[i][j+2] == board[i][j+4]
            && board[i][j+4] == player &&(
            (board[i][j] == board[i][j+3] && board[i][j+3] == EMPTY && board[i][j+5] == 3-player) ||
            (board[i][j] == 3-player && board[i][j+2] == board[i][j+5]  && board[i][j+5] == EMPTY)))
                amo++;
    }

    if(j<SIZE-6){
        if(board[i][j] == board[i][j+6]
            && board[i][j+6] == 3-player
            && board[i][j+1] == board[i][j+5]
            && board[i][j+5] == EMPTY
            && board[i][j+2] == board[i][j+3]
            && board[i][j+3] == board[i][j+4]
            && board[i][j+4] == player)
                amo++;
    }

    //cross(left-right)
    if(i<SIZE-4 && j<SIZE-4){
        if(board[i][j] == board[i+4][j+4]
            && board[i+4][j+4] == player&&(
            (board[i+1][j+1] == board[i+2][j+2] && board[i+2][j+2] == EMPTY && board[i+3][j+3] == player) ||
            (board[i+1][j+1] == player && board[i+2][j+2] == board[i+3][j+3] && board[i+3][j+3] == EMPTY) ||
            (board[i+1][j+1] == board[i+3][j+3] && board[i+3][j+3] == EMPTY && board[i+2][j+2] == player)))
                amo++;
    }

    if(i<SIZE-5 && j<SIZE-5){
        if(board[i+2][j+2] == board[i+3][j+3]
            && board[i+3][j+3] == player &&(
            (board[i][j] == board[i+1][j+1] && board[i+1][j+1] == EMPTY && board[i+4][j+4] == player && board[i+5][j+5]==3-player)||
            (board[i][j] == 3-player && board[i+1][j+1] == player && board[i+4][j+4] == board[i+5][j+5] && board[i+5][j+5]==EMPTY)))
                amo++;

        if(board[i+1][j+1] == board[i+3][j+3]
            && board[i+3][j+3] == board[i+4][j+4]
            && board[i+4][j+4] == player &&(
            (board[i][j] == board[i+2][j+2] && board[i+2][j+2] == EMPTY && board[i+5][j+5] == 3-player) ||
            (board[i][j] == 3-player && board[i+2][j+2] == board[i+5][j+5]  && board[i+5][j+5] == EMPTY)))
                amo++;
        
        if(board[i+1][j+1] == board[i+2][j+2]
            && board[i+2][j+2] == board[i+4][j+4]
            && board[i+4][j+4] == player &&(
            (board[i][j] == board[i+3][j+3] && board[i+3][j+3] == EMPTY && board[i+5][j+5] == 3-player) ||
            (board[i][j] == 3-player && board[i+3][j+3] == board[i+5][j+5]  && board[i+5][j+5] == EMPTY)))
                amo++;
    }

    if(i<SIZE-6 && j<SIZE-6){
        if(board[i][j] == board[i+6][j+6]
            && board[i+6][j+6] == 3-player
            && board[i+1][j+1] == board[i+5][j+5]
            && board[i+5][j+5] == EMPTY
            && board[i+2][j+2] == board[i+3][j+3]
            && board[i+3][j+3] == board[i+4][j+4]
            && board[i+4][j+4] == player)
                amo++;
    }

    //cross(right-left)
    if(i<SIZE-4 && j>=4){
        if(board[i][j] == board[i+4][j-4]
            && board[i+4][j-4] == player&&(
            (board[i+1][j-1] == board[i+2][j-2] && board[i+2][j-2] == EMPTY && board[i+3][j-3] == player) ||
            (board[i+1][j-1] == player && board[i+2][j-2] == board[i+3][j-3] && board[i+3][j-3] == EMPTY) ||
            (board[i+1][j-1] == board[i+3][j-3] && board[i+3][j-3] == EMPTY && board[i+2][j-2] == player)))
                amo++;
    }

    if(i<SIZE-5 && j>=5){
        if(board[i+2][j-2] == board[i+3][j-3]
            && board[i+3][j-3] == player &&(
            (board[i][j] == board[i+1][j-1] && board[i+1][j-1] == EMPTY && board[i+4][j-4] == player && board[i+5][j-5]==3-player)||
            (board[i][j] == 3-player && board[i+1][j-1] == player && board[i+4][j-4] == board[i+5][j-5] && board[i+5][j-5]==EMPTY)))
                amo++;

        if(board[i+1][j-1] == board[i+3][j-3]
            && board[i+3][j-3] == board[i+4][j-4]
            && board[i+4][j-4] == player &&(
            (board[i][j] == board[i+2][j-2] && board[i+2][j-2] == EMPTY && board[i+5][j-5] == 3-player) ||
            (board[i][j] == 3-player && board[i+2][j-2] == board[i+5][j-5]  && board[i+5][j-5] == EMPTY)))
                amo++;
        
        if(board[i+1][j-1] == board[i+2][j-2]
            && board[i+2][j-2] == board[i+4][j-4]
            && board[i+4][j-4] == player &&(
            (board[i][j] == board[i+3][j-3] && board[i+3][j-3] == EMPTY && board[i+5][j-5] == 3-player) ||
            (board[i][j] == 3-player && board[i+3][j-3] == board[i+5][j-5]  && board[i+5][j-5] == EMPTY)))
                amo++;
    }

    if(i<SIZE-6 && j>=6){
        if(board[i][j] == board[i+6][j-6]
            && board[i+6][j-6] == 3-player
            && board[i+1][j-1] == board[i+5][j-5]
            && board[i+5][j-5] == EMPTY
            && board[i+2][j-2] == board[i+3][j-3]
            && board[i+3][j-3] == board[i+4][j-4]
            && board[i+4][j-4] == player)
                amo++;
    }
    return amo;
}

//活二
inline int livetwo(const Board& board, int i, int j, int player){
    if(i<0 || i>=SIZE || j<0 || j>=SIZE)    return 0;
    int amo = 0;
    //horizontally
    if(i<SIZE-3){
        if(board[i][j] == board[i+3][j]
            && board[i+3][j] == player
            && board[i+1][j] == board[i+2][j]
            && board[i+2][j] == EMPTY)
            amo++;
    }

    if(i<SIZE-4){
        if(board[i][j] == board[i+2][j]
            && board[i+2][j] == board[i+4][j]
            && board[i+4][j] == EMPTY
            && board[i+1][j] == board[i+3][j]
            && board[i+3][j] == player)
            amo++;
    }

    if(i<SIZE-5){
        if(board[i+2][j] == board[i+3][j]
            && board[i+3][j] == player
            && board[i][j] == board[i+1][j]
            && board[i+1][j] == board[i+4][j]
            && board[i+4][j] == board[i+5][j]
            && board[i+5][j] == EMPTY)
            amo++;
    }

    //vertically
    if(j<SIZE-3){
        if(board[i][j] == board[i][j+3]
            && board[i][j+3] == player
            && board[i][j+1] == board[i][j+2]
            && board[i][j+2] == EMPTY)
            amo++;
    }

    if(j<SIZE-4){
        if(board[i][j] == board[i][j+2]
            && board[i][j+2] == board[i][j+4]
            && board[i][j+4] == EMPTY
            && board[i][j+1] == board[i][j+3]
            && board[i][j+3] == player)
            amo++;
    }

    if(j<SIZE-5){
        if(board[i][j+2] == board[i][j+3]
            && board[i][j+3] == player
            && board[i][j] == board[i][j+1]
            && board[i][j+1] == board[i][j+4]
            && board[i][j+4] == board[i][j+5]
            && board[i][j+5] == EMPTY)
            amo++;
    }

    //cross(left-right)
    if(i<SIZE-3 && j<SIZE-3){
        if(board[i][j] == board[i+3][j+3]
            && board[i+3][j+3] == player
            && board[i+1][j+1] == board[i+2][j+2]
            && board[i+2][j+2] == EMPTY)
            amo++;
    }

    if(i<SIZE-4 && j<SIZE-4){
        if(board[i][j] == board[i+2][j+2]
            && board[i+2][j+2] == board[i+4][j+4]
            && board[i+4][j+4] == EMPTY
            && board[i+1][j+1] == board[i+3][j+3]
            && board[i+3][j+3] == player)
            amo++;
    }
    
    if(i<SIZE-5 && j<SIZE-5){
        if(board[i+2][j+2] == board[i+3][j+3]
            && board[i+3][j+3] == player
            && board[i][j] == board[i+1][j+1]
            && board[i+1][j+1] == board[i+4][j+4]
            && board[i+4][j+4] == board[i+5][j+5]
            && board[i+5][j+5] == EMPTY)
            amo++;
    } 

    //cross(right-left)
    if(i<SIZE-3 && j>=3){
        if(board[i][j] == board[i+3][j-3]
            && board[i+3][j-3] == player
            && board[i+1][j-1] == board[i+2][j-2]
            && board[i+2][j-2] == EMPTY)
            amo++;
    }

    if(i<SIZE-4 && j>=4){
        if(board[i][j] == board[i+2][j-2]
            && board[i+2][j-2] == board[i+4][j-4]
            && board[i+4][j-4] == EMPTY
            && board[i+1][j-1] == board[i+3][j-3]
            && board[i+3][j-3] == player)
            amo++;
    }

    if(i<SIZE-5 && j>=5){
        if(board[i+2][j-2] == board[i+3][j-3]
            && board[i+3][j-3] == player
            && board[i][j] == board[i+1][j-1]
            && board[i+1][j-1] == board[i+4][j-4]
            && board[i+4][j-4] == board[i+5][j-5]
            && board[i+5][j-5] == EMPTY)
            amo++;
    } 
    return amo;
}

//眠二
inline int sleeptwo(const Board& board, int i, int j, int player){
    if(i<0 || i>=SIZE || j<0 || j>=SIZE)    return 0;
    int amo = 0;
    //horizontally
    if(i<SIZE-4){
        if(board[i][j] == board[i+4][j]
            && board[i+4][j] == player
            && board[i+1][j] == board[i+2][j]
            && board[i+2][j] == board[i+3][j]
            && board[i+3][j] == EMPTY)
            amo++;
    }

    if(i<SIZE-5){
        if(board[i+1][j] == board[i+4][j]
            && board[i+4][j] == player
            && board[i+2][j] == board[i+3][j]
            && board[i+3][j] == EMPTY &&(
            (board[i][j] == EMPTY && board[i+5][j] == 3-player) ||
            (board[i][j] == 3-player && board[i+5][j] == EMPTY)))
            amo++;

        if(board[i][j] == 3-player
            && board[i+1][j] == player
            && board[i+4][j] == board[i+5][j]
            && board[i+5][j] == EMPTY && (
            (board[i+2][j] == player && board[i+3][j] == EMPTY) ||
            (board[i+2][j] == EMPTY && board[i+3][j] == player)))
            amo++;

        if(board[i][j] == board[i+1][j]
            && board[i+1][j] == EMPTY
            && board[i+5][j] == 3-player
            && board[i+4][j] == player && (
            (board[i+2][j] == EMPTY && board[i+3][j] == player) ||
            (board[i+2][j] == player && board[i+3][j] == EMPTY)))
            amo++;
    }

    //vertically
    if(j<SIZE-4){
        if(board[i][j] == board[i][j+4]
            && board[i][j+4] == player
            && board[i][j+1] == board[i][j+2]
            && board[i][j+2] == board[i][j+3]
            && board[i][j+3] == EMPTY)
            amo++;
    }

    if(j<SIZE-5){
        if(board[i][j+1] == board[i][j+4]
            && board[i][j+4] == player
            && board[i][j+2] == board[i][j+3]
            && board[i][j+3] == EMPTY &&(
            (board[i][j] == EMPTY && board[i][j+5] == 3-player) ||
            (board[i][j] == 3-player && board[i][j+5] == EMPTY)))
            amo++;

        if(board[i][j] == 3-player
            && board[i][j+1] == player
            && board[i][j+4] == board[i][j+5]
            && board[i][j+5] == EMPTY && (
            (board[i][j+2] == player && board[i][j+3] == EMPTY) ||
            (board[i][j+2] == EMPTY && board[i][j+3] == player)))
            amo++;

        if(board[i][j] == board[i][j+1]
            && board[i][j+1] == EMPTY
            && board[i][j+5] == 3-player
            && board[i][j+4] == player && (
            (board[i][j+2] == EMPTY && board[i][j+3] == player) ||
            (board[i][j+2] == player && board[i][j+3] == EMPTY)))
            amo++;
    }

    //cross(left-right)
    if(i<SIZE-4 && j<SIZE-4){
        if(board[i][j] == board[i+4][j+4]
            && board[i+4][j+4] == player
            && board[i+1][j+1] == board[i+2][j+2]
            && board[i+2][j+2] == board[i+3][j+3]
            && board[i+3][j+3] == EMPTY)
            amo++;
    }

    if(i<SIZE-5 && j<SIZE-5){
        if(board[i+1][j] == board[i+4][j+4]
            && board[i+4][j+4] == player
            && board[i+2][j+2] == board[i+3][j+3]
            && board[i+3][j+3] == EMPTY &&(
            (board[i][j] == EMPTY && board[i+5][j+5] == 3-player) ||
            (board[i][j] == 3-player && board[i+5][j+5] == EMPTY)))
            amo++;

        if(board[i][j] == 3-player
            && board[i+1][j+1] == player
            && board[i+4][j+4] == board[i+5][j]
            && board[i+5][j+5] == EMPTY && (
            (board[i+2][j+2] == player && board[i+3][j+3] == EMPTY) ||
            (board[i+2][j+2] == EMPTY && board[i+3][j+3] == player)))
            amo++;

        if(board[i][j] == board[i+1][j+1]
            && board[i+1][j+1] == EMPTY
            && board[i+5][j+5] == 3-player
            && board[i+4][j+4] == player && (
            (board[i+2][j+2] == EMPTY && board[i+3][j+3] == player) ||
            (board[i+2][j+2] == player && board[i+3][j+3] == EMPTY)))
            amo++;
    }

    //cross(right-left)
    if(i<SIZE-4 && j>=4){
        if(board[i][j] == board[i+4][j-4]
            && board[i+4][j-4] == player
            && board[i+1][j-1] == board[i+2][j-2]
            && board[i+2][j-2] == board[i+3][j-3]
            && board[i+3][j-3] == EMPTY)
            amo++;
    }

    if(i<SIZE-5 && j>=5){
        if(board[i+1][j-1] == board[i+4][j-4]
            && board[i+4][j-4] == player
            && board[i+2][j-2] == board[i+3][j-3]
            && board[i+3][j-3] == EMPTY &&(
            (board[i][j] == EMPTY && board[i+5][j-5] == 3-player) ||
            (board[i][j] == 3-player && board[i+5][j-5] == EMPTY)))
            amo++;

        if(board[i][j] == 3-player
            && board[i+1][j-1] == player
            && board[i+4][j-4] == board[i+5][j-5]
            && board[i+5][j-5] == EMPTY && (
            (board[i+2][j-2] == player && board[i+3][j-3] == EMPTY) ||
            (board[i+2][j-2] == EMPTY && board[i+3][j-3] == player)))
            amo++;

        if(board[i][j] == board[i+1][j-1]
            && board[i+1][j-1] == EMPTY
            && board[i+5][j-5] == 3-player
            && board[i+4][j-4] == player && (
            (board[i+2][j-2] == EMPTY && board[i+3][j-3] == player) ||
            (board[i+2][j-2] == player && board[i+3][j-3] == EMPTY)))
            amo++;
    }
    return amo;
}

// State::State(Board board, int player) :board(board), player(player){
    // this->search_how_to_move();
// }

void State::search_how_to_move(){
    vector<Point> movement;
    Board already_be_push;

    for(auto it:tomove){
        for(int p=0; p<8; p++){
            //已經被下過的周遭
            if(board[it.x][it.y] != EMPTY){
                int x = it.x +dx[p];
                int y = it.y +dy[p];
                if(x<0 || y<0 || x>=SIZE || y>=SIZE || board[x][y] != EMPTY || already_be_push[x][y] == 1)
                    continue;
                movement.push_back(Point(x,y));
                already_be_push[x][y] = 1;
            }
        }
    }

    how_to_move = movement;
}

State* State::nextState(Point nextstep){
    Board next_board = this->board;
    next_board[nextstep.x][nextstep.y] = this->player;    //occupied by the player
    State* next = new State(next_board, 3-this->player);
    
    return next;
}

void State::check_node_state(){
    for(int p=0; p<SIZE;p++){
        for(int q=0; q<SIZE; q++){
            if(linefive(this->board, p, q,this->player) || livefour(this->board, p, q, this->player)){
                this->leaves = true;
                return;
            }
            if(linefive(this->board, p, q, 3-this->player) || livefour(this->board, p, q, 3-this->player)){
                this->leaves = true;
                return;
            }
        }
    }
    if(this->how_to_move.empty())
        this->leaves = true;
}

//temporary weights : 100/ 75/ 25/ 15/ 10/ 8/ 5
//temporary weights : 100000(1000000) / 10000(100000) / 990(9900) / 100(1000) / 80(80) / 50(25) / 10(5)
//temporary weights : 10000(100000) / 5000(50000) / 1000(10000) / 990(9900) / 500(5000) / 100(1000) / 50(500)
//temporary weights : 1000(100000) / 500(50000) / 100(10000) / 99(9900) / 50(5000) / 10(1000) / 5(500)
//temporary weights : 9(90) / 8(80) / 7(70) / 6(60) / 3(30) / 2(20) / 1(10)
//temporary weights : 50(500) / 30(300) / 20(200) / 15(150) / 8(80) / 5(50) / 3(30)
//temporary weights : 15(150) /14(140) / 12(120) / 10(100) / 8(80) / 6(60) / 5(50) / 2(20)
//temporary weights : 100(150) / 70(120) /50(100) / 40(90) / 20(70) / 10(60) / 5(55)
//temporary weights : 1(10) etc 
//temporary weights : 11000(20000) / 16000(18000) / 14000(16000) /13000(15000) / 10000(12000) / 8000(10000)/ 5000(7000)
void State::set_state_value(int curplayer){
    int temp_value = 0;
    int temp_temp;

    for(int p=0; p<SIZE;p++){
        for(int q=0; q<SIZE; q++){
            if(p<SIZE-4 || q<SIZE-4 || q>=4){   
                temp_temp = temp_value;
                temp_value += 15000000*linefive(this->board, p, q, curplayer);
                temp_value -= 200000000*linefive(this->board, p, q, 3-curplayer);
                if(temp_value!=temp_temp) cout << "exist linefive\n";
            }
            if(p<SIZE-5 || q<SIZE-5 || q>=5){
                temp_temp = temp_value;
                temp_value += 2500000*livefour(this->board, p, q, curplayer);
                temp_value -= 20000000*livefour(this->board, p, q, 3-curplayer);
                if(temp_value!=temp_temp) cout << "exist livefour\n";
            }
            if(p<SIZE-5 || p<SIZE-4 || q<SIZE-5 || q<SIZE-4 || q>=5 || q>=4){
                temp_temp = temp_value;
                temp_value += 19000*rushfour(this->board, p, q, curplayer);
                temp_value -= 190000*rushfour(this->board, p, q, 3-curplayer);
                if(temp_value!=temp_temp) cout << "exist rushfour\n";
            }
            if(p<SIZE-4 || p<SIZE-5 || q<SIZE-4 || q<SIZE-5 || q>=4 || q>=5){
                temp_temp = temp_value;
                temp_value += 19500*livethree(this->board, p, q, curplayer);
                temp_value -= 195000*livethree(this->board, p, q, 3-curplayer);
                if(temp_value!=temp_temp)   cout << "exits livethree\n";
            }
            if(p<SIZE-4 || p<SIZE-5 || p<SIZE-6 || q<SIZE-4 || q<SIZE-5 || q<SIZE-6 || q>=4 || q>=5 || q>=6){
                temp_temp = temp_value;
                temp_value += 17000*sleepthree(this->board, p, q, curplayer);
                temp_value -= 170000*sleepthree(this->board, p, q, 3-curplayer);
                if(temp_value!=temp_temp)   cout << "exist sleepthree\n";
            }
            if(p<SIZE-3 || p<SIZE-4 || p<SIZE-5 || q<SIZE-3 || q<SIZE-4 || q<SIZE-5 | q>=3 || q>=4 || q>=5){
                temp_temp = temp_value; 
                temp_value += 1100*livetwo(this->board, p, q, curplayer);
                temp_value -= 1300*livetwo(this->board, p, q, 3-curplayer);
                if(temp_value!=temp_temp)   cout << "exist livetwo\n";
            }
            if(p<SIZE-4 || p<SIZE-5 || q<SIZE-4 || q<SIZE-5 || q>=4 || q>=5){
                temp_temp = temp_value;
                temp_value += 50*sleeptwo(this->board, p, q, curplayer);
                temp_value -= 100*sleeptwo(this->board, p, q, 3-curplayer);
                if(temp_value!=temp_temp)   cout << "exist sleeptwo\n";
            }
        }
    }
    
    this->state_value = temp_value;
    return;
}

//for minimax
//player picks the move with the highest score(player's value function)
//but opponent picks the move with the lowest score(player's value function)

//the pseudocode of minimax on the slide
/*
function minimax(node, depth, maximizingPlayer) is
    if(depth==0 || node is terminal node)   then
        return the heuristic value of node
    if(maximizingPlayer)----------------------------------->that is, for black/white
        value = -INT_MAX
        for each child or node do
            value = max(value, minimax(child, depth-1, False))------->False means other one(white/balck)
    else
        value = INT_MAX
        for each child or node do
            value = min(value, minimax(child, depth-1, True))
    return value
*/

bool terminal(State* state){
    return state->leaves;
}

/*int minimax(State* state, int depth, bool player){
    int value;
    if(depth == 0){
        state->set_state_value(BLACK);
        return state->state_value;
    }   
    if(player){
        cout << "true\n";
        value = INT_MIN;
        for(auto it : state->how_to_move){
            State* next = state->nextState(it);
            value = max(value, minimax(next, depth-1, false));
        }
        return value;
    }else{
        cout << "false\n";
        value = INT_MAX;
        for(auto it : state->how_to_move){
            State* next = state->nextState(it);
            value = min(value, minimax(next, depth-1, true));
        }
        return value;
    }
}*/

int curplayer;
std::array<std::array<int, SIZE>, SIZE> board;
State root;

int minimax(State* state, int depth, bool player,int alpha,int beta,std::ofstream& fout){
    int value;
    if(depth == 0){
        state->set_state_value(curplayer);
        return state->state_value;
    }   
    if(player){
        //cout << "true\n";
        alpha = value = INT_MIN;
        for(auto it : state->how_to_move){
            State* next = state->nextState(it);
            int prevalue = minimax(next, depth-1, false, alpha, beta,fout);
            if(prevalue>value){
                value=prevalue;
                fout << it.x << " " <<it.y << std::endl;
            }
            alpha=max(alpha,value);
            if(alpha>=beta){
                break;
            }
        }
        return value;
    }else{
        //cout << "false\n";
        value = INT_MAX;
        for(auto it : state->how_to_move){
            State* next = state->nextState(it);
            value = min(value, minimax(next, depth-1, true,alpha,beta,fout));
        
            beta=min(beta,value);
            if(alpha>=beta)
            {
                break;
            }
        }
        return value;
    }
}

Point implement_move(State* state, int depth, std::ofstream& fout){
    Point best_move;
    int compared_score = -INT_MAX;
    int alpha = INT_MIN;
    int beta = INT_MAX;

    for(Point it: state->how_to_move){
        int score = minimax(state->nextState(it), depth-1, false, alpha, beta, fout);
        cout << "score: " << score << "\n" << "x y" << it.x << " " << it.y << "\n";
        if(score > compared_score){
            best_move = it;
            compared_score = score;
        }
    }
    cout << "BestScore" <<compared_score << "\n";
    cout << "BestMove: " << best_move.x << " " << best_move.y << "\n"; 
    return best_move;
}

void read_board(std::ifstream& fin) {
    fin >> curplayer;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            fin >> board[i][j];
            if(board[i][j]!=EMPTY)
                point_on_center = false;
        }
    }

    root = State(board, curplayer);
}

void write_valid_spot(std::ofstream& fout) {
    // srand(time(NULL));          //取得時間序列
    int x, y;
    int depth;
    // Keep updating the output until getting killed.
    // while(true) {
        if(point_on_center){
            fout << 7 << " " << 7 << std::endl;
            fout.flush();
        }
        depth = 1;
        auto move = implement_move(&root, depth, fout);
        if(move.x>0 && move.y>0 && move.x<SIZE && move.y<SIZE && board[move.x][move.y] == EMPTY && move.x!=-1 && move.y!=-1){
            cout << "DecidedMove: " << move.x << " " << move.y << "\n";
            fout << move.x << " " << move.y << std::endl;
            // Remember to flush the output to ensure the last action is written to file.
            fout.flush();
        }
    // }
}

int main(int, char** argv) {
    std::ifstream fin(argv[1]);             //用fin讀入文件
    std::ofstream fout(argv[2]);            //用fout輸出文件

    getboard_tomove();
    read_board(fin);                        
    write_valid_spot(fout);

    fin.close();                            //關起fin和fout使文件變回可訪問的
    fout.close();
    return 0;
}