#include <iostream>
#include <stdlib.h> 
#include <vector>
#include <thread>
#include <math.h>
#include <chrono>
#include <functional>





template<class T> class GoogleMapReduce{
	private:
		long nw, input_size;
		std::vector<T> input;
		std::vector<std::pair<T,long>> *out_map_phase;
		std::function<std::pair<T, long> (T)> map_function;
		std::function<std::pair<T, long> (std::pair<T, long>, std::pair<T, long>)> reduce_function;
		std::vector<std::thread*> vthreads;

		void map_phase(long start, long end, long wid){
			std::cout << "-----------" << wid << " " << start << " " << end << std::endl;
			for(auto i = start; i <= end; i++)
				(*out_map_phase)[i] = this->map_function(this->input[i]);	
			return;
		}

		void reduce_phase(){
			return;
		}

	public:
		GoogleMapReduce(long nw, 
				std::vector<T> input, //Should I pass the pointer?
				std::function<std::pair<T, long> (T)> map_function,
				std::function<std::pair<T, long> (std::pair<T, long>, std::pair<T, long>)> reduce_function){
			this->nw = nw;
			this->input = input;
			this->input_size = input.size();
			this->out_map_phase = new std::vector<std::pair<T,long>>(this->input_size);
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
				std::cout << "Start: "<< start << std::endl;
				std::cout << "End: " << end << std::endl;
				vthreads[counter] = new std::thread([=]{map_phase(start, end, counter);});
				start=end+1;
				counter++;
			}

			for(auto i = 0; i < vthreads.size(); i++)
				vthreads[i]->join();

			for(auto i = 0; i < (*out_map_phase).size(); i++)
				std::cout << i << " ---- " << (*out_map_phase)[i].first << " : " << (*out_map_phase)[i].second << std::endl; 
			
		}
};


