#include <queue>
#include <math.h>
#include <math.h>
#include <iostream>
#include <ff/ff.hpp>
#include <ff/farm.hpp>

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

struct pair_t {
	pair_t(const size_t task, const size_t res): task(task), res(res){}
	size_t task;
	size_t res;
};

struct emitter: ff_node_t<size_t>{
	emitter(const size_t n_tasks):n_tasks(n_tasks){}
	size_t* svc(int*){
		for(size_t i = 0; i < n_tasks; i++)
			ff_send_out(new size_t(i));
		return EOS;
	}
	const size_t n_tasks;
};


struct worker: ff_node_t<pair_t>{
	pair_t svc(size_t* task){ //should be pair_t*
		size_t &t = *task;
		size_t sum = 0;
		for(size_t i = 2; i < t; i++)
			sum+=isPrime(i);
		return pair_t(t, sum);
	}
};

struct collector: ff_node_t<int>{
	int* svc(pair_t* p){
		res.push(*p);
		return GO_ON;
	}
	void svc_end(){
		pair_t val(0,0);
		while(!res.empty()){
			val = res.front();
			res.pop();
			std::cout << "task: " << val.task << "res: " << val.res << std::endl;
		}
		std::cout << "End\n";
	}
	std::queue<pair_t> res;
};




int main(int argc, char* argv[]){
	if(argc < 3){
		std::cerr << "use: " << argv[0] << "nworkers ntasks\n";
		return -1;
	}
	const size_t nworkers = std::stol(argv[1]);
	const size_t n_tasks = std::stol(argv[2]);

	//Farm
	//unique_ptr: la proprietà della risrsa viene trasferita a un altro unique_ptr e l'oggetto unique_ptr
	//originale non ne è più il proprietario della risorsa
	emitter e(n_tasks);
	collector c;
	std::vector<std::unique_ptr<ff_node>> W;
	for(size_t i = 0; i < nworkers; i++)
		W.push_back(make_unique<worker>());
	ff_Farm<int> farm(std::move(W), e, c);

	ffTime(START_TIME);
	if(farm.run_and_wait_end()<0){
		error("running farm");
		return -1;
	}
	ffTime(STOP_TIME);
	std::cout << "Time: " << ffTime(GET_TIME) << std::endl;
	return 0;
}
