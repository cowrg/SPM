#include <thread>
#include <vector>
#include "safe_queue.h"
#include <iostream>
#include <math.h>
#define EOS -1

SafeQueue<int> producerQueue;
SafeQueue<int> outputQueue;

void producer(int numTask){
    for(int i=0;i<numTask;i++){
        int tmp = rand()%10000+1;
        //int tmp = 22 ;
        producerQueue.safe_push(tmp);
    }

    producerQueue.safe_push(EOS);
}

bool isPrime(int x){
    if(x == 2)
        return true;
    if(x % 2 == 0 || x == 1){
        return false;}

    int i = 2;
    int sqR =sqrt(x);

    while(i <= sqR){
        if(x % i == 0){
            return false;
        }
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

        int binLen = ceil( (float) extracted / (float) nw );

        int numBin = 0;
        int start = (binLen * numBin) + 1;
        int end = binLen * (numBin + 1);
        
        while(start <= extracted){
                std::cout << "start" << start << "end" << end << std::endl;
                threads[numBin] = std::thread(computePrime, nw, start, end, &partialResult, numBin);
                numBin++;

                if(binLen * (numBin + 1) <= extracted)
                    end = binLen * (numBin + 1);
                else 
                    end = extracted;
                start = (binLen * numBin) + 1;
        }
        for (int i = 0; i < numBin; i++){
            threads[i].join();
        }
        sum = 0;
        for (int i = 0; i < nw; i++){
            sum+=partialResult[i];
        }
        std::cout << sum << std::endl;
        outputQueue.safe_push(sum);
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