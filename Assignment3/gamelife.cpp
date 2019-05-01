#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <math.h>

#include "./utimer.hpp"
#include "./CImg-2.6.0/CImg.h"                                                                                                    
using namespace std;
using namespace cimg_library;

// notable constants and types 
using board_t = cimg_library::CImg<unsigned char>;
const unsigned int alive = 255;
const unsigned int dead  = 000;

//synchronization variable
int counter = 0;
std::mutex d_mutex;
std::condition_variable s_condition;        //square 
std::condition_variable b_condition;        //board	
 

struct square{int x1, y1, x2, y2;};

long mod(long a, long b){return (a%b+b)%b;}

int find_div(int val, int sq){
	while(sq%val!=0)
		val++;
	return val;
}

unsigned int rules(int count, unsigned int current){
	if(current == alive)
		return (count < 2 || count > 3) ? dead : alive;
	else if(current == dead)
		return (count == 3) ? alive : dead;
	cout << "Something gone wrong" << endl;
}


int count_neighbors(int i, int j, int n, board_t* board){
	int counter_alive = 0;
	for(int x = i-1; x < i+2; x++)
		for(int y = j-1; y < j+2; y++)
			if(x == i && y == j)
				continue;
			else
				counter_alive += (*board)(mod(x,n), mod(y,n), 0, 0) == alive ? 1 : 0;
	return counter_alive;
}


void update_square(board_t *new_board, board_t* old_board, square s, int N, int n){
	int iter = 0;
	while(iter < N){
		for(int i = s.x1; i <= s.x2; i++)
			for(int j = s.y1; j <= s.y2; j++)
					(*new_board)(i, j, 0, 0) = rules(count_neighbors(i, j, n, old_board), (*old_board)(i, j, 0, 0));
		iter++;
		std::unique_lock<std::mutex> lock(d_mutex);
		counter++;
		b_condition.notify_one();
		s_condition.wait(lock);
	}
	return;
}

void render(board_t *new_board, board_t* old_board, CImgDisplay* main_displ, int N, int nw){
	board_t temp;
	int iter = 0;
	while(iter < N){
		std::unique_lock<std::mutex> lock(d_mutex);
		b_condition.wait(lock, [=]{return counter == nw;});
		(*main_displ).display(*new_board);
		temp = *old_board;
		*old_board = *new_board;
		*new_board = temp;
		counter = 0;
		s_condition.notify_all();
		iter++;
	}
	return; 	
}

void board_partitions(std::vector<square>* squares, int n, int nw, int max_size){ 
	int x_size, y_size, tot_size = n*n/nw;
	y_size = tot_size / (x_size = find_div(ceil(sqrt(tot_size)), tot_size));
	int per_rows = n/x_size;
	int r = (n-x_size*per_rows)/per_rows;
	int rr = (n-x_size*per_rows)-(r*per_rows); 
	std::cout << "x_size: " << x_size << std::endl;
	std::cout << "y_size: " << y_size << std::endl;
	std::cout << "How many per row: " << per_rows << std::endl;
	int x1=0, y1=0, x2=x_size-1+r+rr, y2=y_size-1;
	int count_row = 1;
	for(int wid = 0; wid < nw; wid++){	
		(*squares)[wid].x1 = x1; 
		(*squares)[wid].y1 = y1; 
		(*squares)[wid].x2 = x2; 
		(*squares)[wid].y2 = y2;
		if(count_row < per_rows){
			x1 += x_size;
			x2 += x_size+r;
			count_row++;	
		}
		else{
			x1 = 0; x2 = x_size-1+r+rr;
			y1 += (y1 + y_size < n) ? y_size : n-y1-1;
			y2 += (y2 + y_size < n) ? y_size : n-y2-1;
			count_row = 1;
		}
	}
	return;
}

int main(int argc, char * argv[]) {
	if(argc < 4) {
		cout << "Usage is: " << argv[0] << " board_size iters nworkers seed" << endl;
		return(0);
	}

	int n = atoi(argv[1]);
	int N = atoi(argv[2]);
	int nw = atoi(argv[3]);
	int seed = atoi(argv[4]);
	int cacheL1_size = 32768;

	cout << "Board size: " << n << "x" << n << "\nworkers: " << nw << endl;

	// create an empty board
	board_t old_board(n,n,1,1,0);
	board_t new_board(n,n,1,1,0);

	//vector of threads
	std::vector<std::thread> threads(nw);

	//vector of squares 
	std::vector<square> squares(nw); 
	board_partitions(&squares, n, nw, cacheL1_size);

	for(int i = 0; i < squares.size(); i++)
		std::cout << squares[i].x1 << "," << squares[i].y1 << "||" << squares[i].x2 << "," << squares[i].y2 << std::endl;


	// initialize it // randomly
	srand(seed);
	for(int i=0; i<n; i++)
		for(int j=0;  j<n; j++)
			old_board(i,j,0,0) = (rand() % 32 == 0 ? alive : dead);

	CImgDisplay main_displ(old_board,"Game of Life");
	sleep(1);

	std::thread renderer = std::thread(render, &new_board, &old_board, &main_displ, N, nw); 
	for(int i = 0; i < nw; i++)
		threads[i] = std::thread(update_square, &new_board, &old_board, squares[i], N, n);

	for(int i = 0; i < nw; i++)
		threads[i].join();
	renderer.join();

	
	return(0);
}
