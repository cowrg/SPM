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
#include <mutex>
#include <condition_variable>

//Provando a fare tutto senza sincronizzazione, serve un vettore le cui posizioni sono accedute solamente da un unico thread. Ciò significa
//che avremmo tante posizioni quanti i thread (si pensi a 128 thread) molte delle quali potrebbero essere a 0, occupando posto e appesantendo l'ultima reduce.
//Nel caso volessimo sincronizzare la struttura, sempre utilizzando una map che permette di prepare il workspace per il thread reduce, dovremmo associare una
//lock a ciascuna possibile chiave, con lock tutte diverse tra loro (ciascuna per chiave) e utilizzando la funzione push_back di vector.
//Potremmo mettere i risultati ottenuti dalla local_reduce in un unico vettore (es <p,1> <r,5> <a,4>) ma poi dovremmo (dopo il join dei map_phase thread) fare un sort e dividere il vettore
//in chunk omogenei (ovvero memorizzare start, end) ma questo implica che dovremmo scorrere l'intero vettore dei risultati e quindi, in realtà, mentre scorriamo con il for avremmo potuto
//già fare l'ultima reduce (al netto di ottimizzazioni che permetterebbo di individuare sub-array in tempo logaritmico).
//
template<class T> class GoogleMapReduce{
	private:
		long nw, input_size;
		std::vector<T> input;
		std::vector<std::pair<T,long>> output;
		std::map<long, std::map<T, std::vector<long>>> dispatcher; //wid -> map of that wid
		std::function<std::pair<T, long> (T)> map_function;
		std::function<std::pair<T, long> (std::pair<T, long>, std::pair<T, long>)> reduce_function;
		std::vector<std::thread*> vthreads;
		std::mutex mutex;
		std::condition_variable condition;

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
				std::vector<long> *res_vect = &dispatcher[wid_assigned][t.first];
				res_vect->resize(this->nw);
				wid_assigned = select_wid_reduce(t.first);
				std::cout << " size " << res_vect->size() << std::endl;
				(*res_vect)[actual_wid] = t.second;
			}
			for(auto &it : dispatcher[0]){
				for(auto &p : it.second){
					std::cout << "item: " << p << " "; 
				}
				
			}
			return;
		}

		void reduce_phase(long actual_wid){
			std::map<T, std::vector<long>> *wid_workspace = &dispatcher[actual_wid];
			std::pair<T, long> sum, temp;
			for(auto &item : *wid_workspace){
				sum.first = item.first;
				sum.second = 0;
				for(auto i = 0; i < item.second.size(); i++){ //abbiamo taroccato la reduce, sommando sequenzialmente, dato che utilizziamo un singolo thread per eseguire la reduce. Questa scelta è una conseguenza di non aver voluto utilizzare meccanismi di sicronizzazioni. 
					temp = std::pair(item.first, item.second[i]);
					std::cout << "bello nvim " << item.second[i] << std::endl;
					sum = this->reduce_function(sum, temp);
				}
				std::unique_lock<std::mutex> lock(mutex);
				condition.wait(lock, [=]{return true;});
				output.push_back(sum);
				condition.notify_one();
			}

			return;
		}

		long select_wid_reduce(T value){
			long wid_assigned = std::hash<T>()(value)%this->nw;
			std::cout << "Hash: wid " << wid_assigned << " value " << value << std::endl;
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


		std::vector<std::pair<T,long>> run(){
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

			return output;
		}
};


