#include <iostream>
#include <string>
#include <stdlib.h> 
#include <vector>
#include <map>
#include <thread>
#include <math.h>
#include <chrono>
#include <functional>
#include <typeinfo>




template<class T> class GoogleMapReduce{
	private:
		long nw, input_size;
		std::vector<T> input;
		std::map<long, std::map<T, std::vector<long>>> dispatcher; //wid -> map of that wid
		std::function<std::pair<T, long> (T)> map_function;
		std::function<std::pair<T, long> (std::pair<T, long>, std::pair<T, long>)> reduce_function;
		std::vector<std::thread*> vthreads;

		void map_phase(long start, long end, long actual_wid){
//			std::cout << "-----------" << actual_wid << " " << start << " " << end << std::endl;
			long wid_assigned = 0;
			std::map<T,long> hashmap;
			std::pair<T,long> pair;
			for(auto i = start; i <= end; i++){
				pair = this->map_function(this->input[i]); //map	
				hashmap[pair.first] += pair.second; //local_reduce
			}
			for(auto& t : hashmap){
				std::cout << t.first << " - " << t.second << std::endl;
				wid_assigned = select_wid_reduce(t.first);
				(dispatcher[wid_assigned][t.first])[actual_wid] = t.second;
				std::cout << "########## " << (dispatcher[wid_assigned][t.first])[actual_wid] << std::endl; 
			}
			return;
		}

		void reduce_phase(long actual_wid){
			std::map<T, std::vector<long>>* wid_workspace = &dispatcher[actual_wid];
			for(auto &item : *wid_workspace)
				std::cout << "################ " << item.first << std::endl;
			return;
		}

		long select_wid_reduce(T value){
			long wid_assigned = std::hash<T>()(value)%this->nw;
			std::cout << "wid " << wid_assigned << " value " << value << std::endl;
			return wid_assigned;
		}

	public:
		GoogleMapReduce(long nw, 
				std::vector<T> input, //Should I pass the pointer?
				std::function<std::pair<T, long> (T)> map_function,
				std::function<std::pair<T, long> (std::pair<T, long>, std::pair<T, long>)> reduce_function){
			this->nw = nw;
			this->input = input;
			this->input_size = input.size();
			this->map_function = map_function;
			this->reduce_function = reduce_function;
			this->vthreads = std::vector<std::thread*>(nw);
		}


		void run(){
			long chunk = ceil(this->input_size/this->nw); //Should be computed on instantiation?
			std::cout << "Chunk_size: " << chunk << std::endl;
			long counter = 0, start = 0, end = 0;
			while(counter < this->nw){
				end = (counter == this->nw-1) ? input_size-1 : start+chunk-1;
//				std::cout << "Start: "<< start << std::endl;
//				std::cout << "End: " << end << std::endl;
				vthreads[counter] = new std::thread([=]{map_phase(start, end, counter);});
				start=end+1;
				counter++;
			}

			for(auto i = 0; i < vthreads.size(); i++)
				vthreads[i]->join();

			std::cout << "------------------------" << std::endl;
			for(const auto &item : dispatcher)
				for(const auto &i : item.second)
					std::cout << i.first << std::endl;
			//FIX
			std::vector<std::thread*> rthreads(nw);
			for(auto i = 0; i < rthreads.size(); i++)
				rthreads[i] = new std::thread([=]{reduce_phase(i);});

			for(auto i = 0; i < vthreads.size(); i++)
				rthreads[i]->join();


		}
};


