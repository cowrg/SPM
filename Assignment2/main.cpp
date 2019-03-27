#include <thread>
#include <vector>
#include "safe_queue.h"
#include <iostream>
#include <math.h>
#include <chrono>
#define EOS -1

typedef struct{
    int start;
    int end;
} params;


int nw = (int) std::thread::hardware_concurrency();
SafeQueue<int> producerQueue;
SafeQueue<int> outputQueue;
std::vector<SafeQueue<params*>> paramsVector(nw);
std::vector<SafeQueue<int>> partialResult(nw);

void producer(int numTask){
    for(int i=0;i<numTask;i++)
        producerQueue.safe_push(rand()%10000+8);
    producerQueue.safe_push(EOS);
    return;
}

bool isPrime(int x){
    if(x == 2)
        return true;
    if(x % 2 == 0){
        return false;}
    int i = 2;
    int sqR = sqrt(x);
    while(i <= sqR){
        if(x % i == 0)
            return false;
        i++;
    }
    return true;
}

void computePrime(int wid){
    params* p;
    int sum = 0;
    while( ( p = paramsVector[wid].safe_pop())->start != EOS){
        sum = 0;
        for(int i=p->start; i<=p->end; i++)
            if(isPrime(i))
                sum++;
        partialResult[wid].safe_push(sum);
    }
    return;
}

void reducer(){
    params* p;
    float binLen;
    int extracted = 0, sum = 0, numBin = 0, start = 0, end = 0;
    std::vector<std::thread> threads(nw);
    for(int wid = 0; wid < threads.size(); wid++)
            threads[wid] = std::thread(computePrime, wid);
    while ((extracted = producerQueue.safe_pop()) != EOS){
        sum = 0;
        start = 2;
        binLen = (float) (extracted-start)/nw;  //-2 cuz numbers 0,1 are excluded [2,x]
        for(int wid = 0; wid < nw; wid++){  //da invertire la run --
            if(ceil(binLen) != float(binLen) && wid == nw-1)
                end =  extracted;
            else
                end =  start+(int) binLen-1;
            *p = params{start, end};
            start = start+binLen;
            paramsVector[wid].safe_push(p);
        }

        for (int wid = 0; wid < nw; wid++)
            sum+=partialResult[wid].safe_pop();
        outputQueue.safe_push(sum);
        //std::cout << sum << std::endl;
    }
    for (int wid = 0; wid < paramsVector.size(); wid++){
        *p = params{-1, -1};
        paramsVector[wid].safe_push(p);
    }
    for (int wid = 0; wid < threads.size(); wid++){
        threads[wid].join();
    }
    //delete data structures
    return;
}

int main(int argc, const char ** argv){
    if(argc > 1){
        int numTask = atoi(argv[1]);
        std::thread producerThread = std::thread(producer, numTask);
        reducer();
        producerThread.join();  
    }
    else
        std::cout << "Missing argument: numTasks" << std::endl;
    return 0;
}