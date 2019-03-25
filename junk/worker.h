//problema con l'ordine. TAG
#include "./safe_queue.h"
#include<thread>
#include<iostream>
#include<atomic>
//T sarà un tipo funzione

template <class T> class Worker: public SafeQueue<T>{
    private:
        static constexpr std::atomic<int> w_id{0};
        std::thread::id t_id;
        SafeQueue<T>* masterQueue;

    public:
        Worker(int my_size, SafeQueue<T> &masterQueue): SafeQueue<T>(my_size){
            this->masterQueue = masterQueue;
            this.w_id.operator++;
        };

    std::thread::id getTid(){ //or kill directly
        return t_id;
    }

    static int getWid(){
        return w_id.load();
    }

    T popQueuesLogic(){
        
    }

    void run(){ //if questa safeQueue è empty e se quella sopra uguale prendi dagli altri
        std::cout << "Start Worker";
        while(true){
            T item = SafeQueue<T>::safe_pop();
            std::thread t(item);
            this->t_id = t.get_id();
            t.join();
        }
    }


};