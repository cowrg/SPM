#include<iostream>
#include<thread>
#include<unistd.h>
#include "./safe_queue.h"

#define EOS -1



template<typename T> 
struct Worker
{
    SafeQueue<T> safeQueue;
    std::thread* t;
};

enum Functions
{
   Increase,
   Square,
   Decrease,
};


struct Item{
    int (*fun) (int);
    int data;
    Functions function;
};


SafeQueue<Item> masterQueue(40000);
std::mutex print_mutex;


void print(int v){
    std::cout << v << std::endl;
}

int decrease(int v){
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    return --v;
}

int square(int v){
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    return (v*v);
}

int increase(int v){
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    return ++v;
}

void streamInt(int m){
    for(int i = 0; i < m; i++){
        Item item = {increase, i, Increase};
        masterQueue.safe_push(item);
    }
    //masterQueue.safe_push(EOS); //EOS = -1
}


void body_worker(){
    SafeQueue<int> safeQueue(40);
    while(!masterQueue.isEmpty()){
        Item item = masterQueue.safe_pop();

        int res = item.fun(item.data); //execut fun

        switch (item.function)
        {
            case Increase:{
                Item item = {square, res, Square};
                masterQueue.safe_push(item);
                break;
            }
            case Square:{
                Item item = {decrease, res, Decrease};
                masterQueue.safe_push(item);
                break;
            }
            case Decrease:{
                //std::unique_lock<std::mutex> lock(print_mutex);
                std::cout << res << std::endl;
                //lock.unlock();
                break;
            }
            default:{
                break;
            }
        }
    }
}




int main(int argc, const char** argv){
    int m = atoi(argv[1]);
    int Num_Threads = std::thread::hardware_concurrency();
    std::cout << Num_Threads << std::endl;

    std::thread master(streamInt,m);
    std::vector<std::thread*> threads;
    threads.resize(0);

    for(int i = 0; i < 4; i++)
        threads.push_back(new std::thread(body_worker));
    
    for(int i = 0; i < 4; i++){
        threads.at(i)->join();
    }
    master.join();
    
    return 0;
}