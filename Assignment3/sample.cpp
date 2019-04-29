#include <iostream>
#include <vector>
#include <thread>

#include "/home/alessandro/Desktop/github/SPM/Assignment3/utimer.hpp"
#include "./CImg-2.6.0/CImg.h"                                                                                                    
using namespace std;
using namespace cimg_library;

// notable constants and types 
using board_t = cimg_library::CImg<unsigned char>;
const unsigned int alive = 000;
const unsigned int dead  = 255;

struct square{
  int x1;
  int y1;
  int x2;
  int y2;
};

long mod(long a, long b){
  return (a%b+b)%b;
}


int count_neighbors(int i, int j, int n, board_t board){
  int counter_alive = 0;
  counter_alive += board( mod(i-1,n), j, 0, 0) == alive ? 1 : 0;
  counter_alive += board( mod(i+1,n), j, 0, 0) == alive ? 1 : 0;
  counter_alive += board( mod(i-1,n),  mod(j-1,n), 0, 0) == alive ? 1 : 0;
  counter_alive += board( mod(i-1,n), mod(j+1,n), 0, 0) == alive ? 1 : 0;
  counter_alive += board(i,mod(j-1,n), 0, 0) == alive ? 1 : 0;
  counter_alive += board(i, mod(j-1,n), 0, 0) == alive ? 1 : 0;
  counter_alive += board( mod(i+1,n), mod(j-1,n), 0, 0) == alive ? 1 : 0;
  counter_alive += board( mod(i+1,n), mod(j+1,n), 0, 0) == alive ? 1 : 0;
  return counter_alive;
}

unsigned int rule(int count, unsigned int current){
  if(count < 2 || count > 3)
    return dead;
  if(count == 3 && current == dead )
    return alive;
  return current;
}

void update(board_t *new_board, board_t old_board, square s, int n){
  //lock
  for(int i = s.x1; i < s.x2; i++){
    for(int j = s.y1; j < s.y2; j++){
      int count = count_neighbors(i, j, n, old_board);
      (*new_board)(i, j, 0, 0) = rule(count, old_board(i, j, 0, 0));
    }
  }

  return;
}

std::vector<square> divide_and_assing(int n, int nw){ //da passaare puntatore
  std::vector<square> squares(nw);
  int size = n/nw;
  std::cout << "size " << size << std::endl;
  square s;
  for(int i = 0; i < nw; i++){
    if(i==0){
      squares[i].x1 = 0;
      squares[i].y1 = 0;  
    }else{
      squares[i].x1 = (i*size)+1;
      squares[i].y1 = (i*size)+1;
    }
    if( i == nw-1){
      squares[i].x2 = n-1;
      squares[i].y2 = n-1;    
    }else{
      squares[i].x2 = (i+1)*size;
      squares[i].y2 = (i+1)*size;    
    }
  }
  return squares;
}

int main(int argc, char * argv[]) {

  if(argc == 1) {
    cout << "Usage is: life n m seed iter init-no nw" << endl;
    return(0);
  }
  // get matrix dimensions from the command line
  int n = atoi(argv[1]);
  int m = n;
  int seed = atoi(argv[2]);
  int nw = 8;

  cout << "Using " << n << "x" << m << " board " << endl;
  
  // create an empty board
  board_t old_board(n,m,1,1,0);

  std::vector<square> squares = divide_and_assing(n, nw);
  for(int i = 0; i < squares.size(); i++)
    std::cout << squares[i].x1 << "," << squares[i].y1 << "," << squares[i].x2 << "," << squares[i].y2 << "," << std::endl;


  // initialize it // randomly
  srand(seed);
  for(int i=0; i<n; i++)
    for(int j=0;  j<m; j++)
      old_board(i,j,0,0) = (rand() % 32 == 0 ? alive : dead);
  
  board_t new_board(n,m,1,1,0);
  new_board = old_board;
  CImgDisplay main_displ(new_board,"Init");
  sleep(2);
  std::vector<std::thread> threads(nw);
  for(int j = 0; j < 100; j++){
    for(int i = 0; i < nw; i++){
      threads[i] = std::thread(update, &new_board, old_board, squares[i], n);
    }
  
    for(int i = 0; i < nw; i++)
      threads[i].join();


    main_displ.display(new_board); 
    old_board = new_board;
    board_t new_board(n,m,1,1,0);

    // and redisplay
  }
  
    
  return(0);
  
}
