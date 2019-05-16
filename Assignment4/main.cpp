#include "./google_map_reduce.h"
/////////////////////////////////////////////////////////////////////
template<typename T>
std::pair<T,long> map(T item){
	return std::make_pair(item, 1);
}

template<typename T>
std::pair<T,long> reduce(std::pair<T, long> x, std::pair<T, long> y){
	if(x.first != y.first){ //to be removed
		std::cout << "Not equals" << std::endl;
		exit(EXIT_FAILURE);
	}
	return std::make_pair(x.first, x.second+y.second);
}
/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////
std::vector<int> generator(int size){
	std::vector<int> v(size);
	for(auto i = 0; i < size; i++)
		v[i] = i%2;
	for(auto i : v)
		std::cout << i << std::endl;
	return v;
}
/////////////////////////////////////////////

int main(int argc, char**argv){
	long nw = 8, map_nw = 4, reduce_nw = 4, size = 16;
	if(argc < 2){
		std::cout << "Usage: " << argv[0] << " nw size" << std::endl;
		return 1;
	}
	nw = atoi(argv[1]);
	size = atoi(argv[2]);
//	std::function<std::pair<int, long> (int)> map_function = map<int>;
//	std::function<std::pair<int, long> (std::pair<int, long>, std::pair<int, long>)> reduce_function = reduce<int>;
//	GoogleMapReduce<int> gmr(nw, generator(size), map_function, reduce_function); 
	std::function<std::pair<std::string, long> (std::string)> map_function = map<std::string>;
	std::function<std::pair<std::string, long> (std::pair<std::string, long>, std::pair<std::string, long>)> reduce_function = reduce<std::string>;
	std::vector<std::string> vector = {"ciao", "come", "butta", "la", "la", "vita", "ciao", "ciao"};
	GoogleMapReduce<std::string> gmr(nw, vector, map_function, reduce_function); 
	std::vector<std::pair<std::string, long>> out = gmr.run();
	for(auto &item : out)
		std::cout << "Key: " << item.first << "--- Value: " << item.second << std::endl;
	return 0;
}
