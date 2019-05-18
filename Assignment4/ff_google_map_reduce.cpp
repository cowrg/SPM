#include <iostream>
#include <math.h>
#include <vector>
#include <map>
#include <ff/ff.hpp>

using namespace ff;

/////////////////////////////////////////////////////////////////////
template<typename T>
std::pair<T,long> map(T item){
	return std::make_pair(item, 1);
}

template<typename T>
std::pair<T,long> reduce(std::pair<T, long> x, std::pair<T, long> y){
	return std::make_pair(x.first, x.second+y.second);
}
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////
std::vector<int> int_generator(int size){
	std::vector<int> v(size);
	for(auto i = 0; i < size; i++)
		v[i] = rand()%23;
	return v;
}
/////////////////////////////////////////////
struct task {
	task(const long start, const long end, long nw, std::vector<int> *input_vector): start(start), end(end), nw(nw), input_vector(input_vector){}
	long start;
	long end;
	long nw;	
	std::vector<int>* input_vector;
 };     

template<class T>
struct Master: ff_monode_t<task>{
	Master(std::vector<T> input_vector, long nw): input_vector(input_vector), nw(nw){}
	task *svc(task *){
		long chunk = ceil(input_vector.size()/nw), wid = 0, start = 0, end = 0;
		while(wid < nw){
			end = (wid == nw-1) ? input_vector.size()-1 : start+chunk-1;
			task * t = new task(start, end, nw, &input_vector);
			ff_send_out_to(t, wid);
			start=end+1;
			wid++;

		}
		return EOS;
	}
	long nw;
	std::vector<T> input_vector;
};

template<class T>
struct MapWorker: ff_monode_t<task, std::pair<T,long>>{
	MapWorker(std::function<std::pair<T, long> (T)> map_function): map_function(map_function){}

	long select_wid_reduce(T value, int module){
		long wid_assigned = std::hash<T>()(value)%module;
		std::cout << "Hash: wid " << wid_assigned << " value " << value << std::endl;
		return wid_assigned;
	}

	std::pair<T,long> *svc(task *in){
		task &t =  *in; 
		std::pair<T,long> pair;
		for(auto i = t.start; i <= t.end; i++){
			pair = map_function((*t.input_vector)[i]); //map	
			hashmap[pair.first] += pair.second; //local_reduce
		}
		for(auto& entry : hashmap){
			this->ff_send_out_to(&entry, select_wid_reduce(entry.first, t.nw));
			std::cout << "Hash: wid " << entry.first << std::endl;
		}
		return this->GO_ON;
	}
	
	std::map<T,long> hashmap;
	std::function<std::pair<T, long> (T)> map_function;
};


template<class T>
struct MultiInputHelper1: ff_minode_t<task>{
	task *svc(task *in){
		return in;
	}
};

template<class T>
struct MultiInputHelper2: ff_minode_t<std::pair<T,long>>{
	std::pair<T,long> *svc(std::pair<T,long> *in){
		return in;
	}
};


template<class T>
struct ReduceWorker: ff_node_t<std::pair<T,long>, std::vector<std::pair<T,long>>>{
	ReduceWorker(std::function<std::pair<T, long> (std::pair<T, long>, std::pair<T, long>)> reduce_function): reduce_function(reduce_function){}
	std::vector<std::pair<T,long>> *svc(std::pair<T, long> * in){
		std::pair<T,long> &p = *in;
		hashmap[p.first] += p.second;
		for(auto &entry : hashmap)
			partial_out.push_back(entry);
		this->ff_send_out(&partial_out);	
		return this->GO_ON;
	}

	void svc_end(){
		for(auto &item: hashmap)
			std::cout << "<#####> " << item.first << std::endl;
	}

	std::map<T,long> hashmap;
	std::vector<std::pair<T,long>> partial_out;
	std::function<std::pair<T, long> (std::pair<T, long>, std::pair<T, long>)> reduce_function;
};


template<class T>
struct Gatherer: ff_minode_t<std::vector<std::pair<T,long>>>{

	int svc_init(){
		complete_output.reserve(20);
		return 0;
	}

	std::vector<std::pair<T,long>> *svc(std::vector<std::pair<T,long>> *in){
		std::vector<std::pair<T,long>> &v = *in;
		complete_output.resize(complete_output.size()+v.size());
		complete_output.insert(complete_output.end(), v.begin(), v.end());
		return this->GO_ON;
	}
	
	void svc_end(){
		for(auto &item : complete_output)
			std::cout << "Key: " << item.first << " Value: " << item.second << std::endl;
	}

	std::vector<std::pair<T,long>> complete_output;
};

int main(int argc, char* argv[]){
	long nw, size;
	std::vector<std::pair<std::string, long>> string_out;
	std::vector<std::pair<int, long>> int_out;
	if(argc < 2){
		std::cerr << "Usage: " << argv[0] << " nw size" << std::endl;
		return 1;
	}
	nw = atoi(argv[1]);
	size = atoi(argv[2]);

	std::vector<int> ciao{1,3,1,3,1};
	Master<int> master(int_generator(size), nw);
	MapWorker<int> mw(map<int>);
	ReduceWorker<int> rw(reduce<int>);
	MultiInputHelper1<int> h1;
	MultiInputHelper2<int> h2;
	Gatherer<int> gatherer;


	std::vector<ff_node*> LW;
	for(auto i = 0; i < 2; i++)
		LW.push_back(new ff_comb(h1,mw));


	
	std::vector<ff_node*> RW;
	for(auto i = 0; i < 4; i++)
		RW.push_back(new ff_comb(h2,rw));

	ff_a2a a2a;
	a2a.add_firstset(LW,0,true);
	a2a.add_secondset(RW,true);

	ff_Pipe<> gmr(master, a2a);
	
	std::cout << "cidsi" << std::endl;
	if(gmr.run_and_wait_end()<0)
		error("running gmr");


	return 0;
}



















