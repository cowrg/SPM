#include <thread>
#include <vector>
#include "safe_queue.h"
#include <iostream>
#include <math.h>
#define EOS -1

SafeQueue<int> producerQueue;

void producer(int numTask){
    for(int i=0;i<numTask;i++){
        //int tmp = rand()%10000+1;
        int tmp = 17 ;
        producerQueue.safe_push(tmp);
    }

    producerQueue.safe_push(EOS);
}

bool isPrime(int x){

    if(x == 2)
        return true;

    if(x % 2 == 0)
        return false;

    int i = 2;
    int sqR =sqrt(x);

    while(i <= sqR){
        if(x % i == 0)
            return false;
        i++;
    }
    
    return true;
}

void computePrime(int nw, int start, int end, std::vector<int> * partialResults, int position){
    
    for(int i=start; i<=end; i++){
        if(isPrime(i)){
            (* partialResults)[position]++;
        }
    }
    
}

void reducer(int nw){
    int extracted = 0;
    int sum = 0;

    while ((extracted = producerQueue.safe_pop()) != -1){

        std::vector<std::thread> threads(nw);
        std::vector<int> partialResult(nw);

        int binLen = (extracted / nw);
        
        for(int i=0;i<nw;i++){
            
            if(i == nw - 1){

                threads[i] = std::thread(computePrime, nw, (binLen * i) +1, binLen * (i + 1), &partialResult, i);
            }
            else{

                threads[i] = std::thread(computePrime, nw, (binLen * i) + 1, binLen * (i + 1), &partialResult, i);
            }
        
        }
        for (int i = 0; i < nw; i++){
            threads[i].join();
        }
        sum = 0;
        for (int i = 0; i < nw; i++){
            sum+=partialResult[i];
        }
        std::cout << sum << std::endl;
    }
}

int main(int argc, const char ** argv){

    if(argc > 1){
        int numTask = atoi(argv[1]);
        std::thread * producerThread = new std::thread(producer, numTask);
        
        int concurentThreadsSupported = (int) std::thread::hardware_concurrency();
        reducer(concurentThreadsSupported);
        
        producerThread->join();

        
    }


    return 0;
}