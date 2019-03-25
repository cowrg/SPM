#include<iostream>
#include<fstream>
#include<thread>
#include<queue>
#include<mutex>
#include<condition_variable>
#include<vector>
#include<unistd.h>
#include "./safe_queue.h"


#define EOS -1

using namespace std;


std::vector<SafeQueue<int>*> safeQueues;

void streamInt(int m){
    for(int i = 0; i < m; i++){
        safeQueues.at(0)->safe_push(i);
        //safeQueues.at(0)->safe_push(rand() % 10);
    }
    safeQueues.at(0)->safe_push(EOS); //-1 EOF
}

void streamIncrease(){
    int v;
    while( (v = safeQueues.at(0)->safe_pop()) != EOS){
        safeQueues.at(1)->safe_push(++v);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    safeQueues.at(1)->safe_push(EOS);       
}

void streamSquare(){
    int v;
    while( (v = safeQueues.at(1)->safe_pop()) != EOS){
        safeQueues.at(2)->safe_push(v*v);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    safeQueues.at(2)->safe_push(EOS);
}

void streamDecrease(){
    int v;
    while((v = safeQueues.at(2)->safe_pop()) != EOS){
        safeQueues.at(3)->safe_push(--v);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    safeQueues.at(3)->safe_push(EOS);
}

void printAll(){
    int v;
    while ((v = safeQueues.at(3)->safe_pop()) != EOS)
        std::cout << v << " ";
    std::cout << std::endl;
}

int main(int argc, char* argv[]){
    int m = atoi(argv[1]);
    

    safeQueues.resize(0);
    for(int i = 0; i < 4; i++)
        safeQueues.push_back(new SafeQueue<int>(50000));

    std::vector<std::thread*> threads;
    threads.resize(0);
    threads.push_back(new thread(streamInt, m));
    threads.push_back(new thread(streamIncrease));
    threads.push_back(new thread(streamSquare));
    threads.push_back(new thread(streamDecrease));
    threads.push_back(new thread(printAll));

    //for(int i = 0; i < 5; i++)
    //    threads.at(i)->sleep_for(std::chrono::milliseconds(1000));


    for(int i = 0; i < 5; i++)
        threads.at(i)->join();

    return 0;
}

//EOF e dies on read