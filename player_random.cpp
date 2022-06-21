#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <array>

enum SPOT_STATE {
    EMPTY = 0,
    BLACK = 1,
    WHITE = 2
};

int player;
const int SIZE = 15;
std::array<std::array<int, SIZE>, SIZE> board;

void read_board(std::ifstream& fin) {
    fin >> player;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            fin >> board[i][j];
        }
    }
}

void write_valid_spot(std::ofstream& fout) {
    srand(time(NULL));          //取得時間序列
    int x, y;
    // Keep updating the output until getting killed.
    while(true) {
        // Choose a random spot.
        int x = (rand() % SIZE);
        int y = (rand() % SIZE);
        //int x = 1;
        //int y = 1;
        if (board[x][y] == EMPTY) {
            //fout << "domo" << std::endl;
            fout << x << " " << y << std::endl;
            // Remember to flush the output to ensure the last action is written to file.
            fout.flush();
        }
    }
}


int main(int, char** argv) {
    std::ifstream fin(argv[1]);             //用fin讀入文件
    std::ofstream fout(argv[2]);            //用fout輸出文件
    read_board(fin);                        
    write_valid_spot(fout);
    fin.close();                            //關起fin和fout使文件變回可訪問的
    fout.close();
    return 0;
}
