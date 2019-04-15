#include <thread>
#include <vector>
#include <math.h>
#include <iostream>
#include <ff/ff.hpp>
#include <ff/parallel_for.hpp>


using namespace ff;

int isPrime(int x){
    if(x == 2)
        return 1;
    if(x % 2 == 0)
        return 0;
    int i = 2;
    int sqR = sqrt(x);
    while(i <= sqR){
        if(x % i == 0)
            return 0;
        i++;
    }
    return 1;
}



int main(int argc, const char** argv){
    if(argc < 3){
        std::cerr << "use: " << argv[0] << " task numbers\n";
        return -1;
    }
    int n_tasks = atoi(argv[1]);
    int threads = atoi(argv[2]);

    std::vector<int> tasks(n_tasks);
    for(int i = 0; i < n_tasks; i++)
        tasks[i] = rand()%10000+2;
    
    ParallelForReduce<long> pfr;
    long sum = 0;
    pfr.parallel_reduce(sum, 0, 0, n_tasks, 1, 0, [&tasks](const long i, long &mysum){
            for(long k = 2; k <= tasks[i]; k++)
                mysum += isPrime(k);
        },
        [](long &s, const long e){ s+= e;}
    );

    std::cout << sum << std::endl;

    return 0;
}