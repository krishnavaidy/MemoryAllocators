#include<iostream>
#include<vector>
#include<cstdlib>
#include "random"
#include "math.h"
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace boost::posix_time;

// Sets page size in KB
static const int page_size = 4;

// Ram size in GB
static const int ram_size = 16;

// Number of frames
static const uint64_t nPages = ((ram_size * pow(2, 30))/(4 * pow(2,10)));

// Size after which to move nodes from inactive queue to free queue
static const uint64_t moveSize = (nPages* 0.05);

// List which stores pages in UVM 
struct node {
    uint64_t page_address;
    struct node* next;
    node() {
        next = NULL;
    }
};

// Class which implements Queue
// TODO: Refactor to separate active, inactive, free queue
class Queue {
    private:
        struct node* head;
        struct node* tail;

    public:
        uint64_t size;
        Queue() {
            this->head = NULL;
            this->tail = NULL;
            this->size = 0;
        }

        // Is empty
        bool isempty(){
            if(this->head == this->tail && (this->head == NULL))
                return true;

            return false;
        }

        // Add elements to queue, given a node
        void enqueue(uint64_t tmp) {
            struct node * tmpNode = new struct node();
            tmpNode->page_address = tmp;

            // Check for empty queue
            if(this->tail != NULL){
                (this->tail)->next = tmpNode;
            }
            this->tail = tmpNode;

            // Check for empty queue condition
            if(this->head == NULL) {
               this->head = this->tail; 
            }

            // Increment size
            this->size++;
        }

        // Remove element from queue
        uint64_t dequeue() {
            
            // Check for empty queue condition
            if(this->head == NULL){
                std::cout<<"\nAttempt to dequeue an empty queue";
                exit(EXIT_FAILURE);
            }

            struct node * tmp = NULL;

            if(this->head == this->tail){
                tmp = this->head;
                this->head = NULL;
                this->tail= NULL;
            }
            else{
                tmp = this->head;
                this->head = (this->head)->next;
            }

            // Decrement size
            this->size--;

            // Return deleted element from queue
            if(tmp != NULL)
                return tmp->page_address;
            else{
                std::cout<<"Page element dequeued is NULL";
                return 0;
            }
        }
};

// Class for BSD memory allocation queues
class BSDStructure {
    private:
        Queue activeQueue, inactiveQueue, freeQueue;

    public:
        uint64_t freeSize(){
            return freeQueue.size;
        }

        uint64_t activeSize(){
            return activeQueue.size;
        }

        uint64_t inactiveSize(){
            return inactiveQueue.size;
        }

        // Active queue operation
        void addActive(const int n);
        void removeActive(const int n);

        // Free queue operations
        void addFree(std::vector<uint64_t> pageLocation);
        std::vector<uint64_t> removeFree(const int n);

        // Inactive queue operations
        void addInactive(std::vector<uint64_t> pageLocation);
        std::vector<uint64_t> removeInactive(const int n);
};

// Takes page from free list and adds to active list
void BSDStructure::addActive(const int n) {
    
    // Check for free memory available
    if(n > this->freeQueue.size){
        std::cout<<"Not enough memory in freeMemory"<<"\n n :"<<n<<"\n freeQ size :"<<this->freeQueue.size;
        exit(EXIT_FAILURE);
    }

    for(int i=0;i<n;i++){
        //std::cout<<"\ni: "<<i;
        this->activeQueue.enqueue(this->freeQueue.dequeue());
    }

    if(this->freeQueue.size < moveSize){
        // Check if inactive list has nodes
        if(inactiveQueue.size > 2){
            //std::cout<<"\nReallocating from Inactive list to Free list";
            // Leave one page in inactive list just in case
            addFree(removeInactive(this->inactiveQueue.size - 1));
        }
        else{
            std::cout<<"\nNo free memory in free queue";
        }
    }
}

// Removes page from active list and adds to inactive list
void BSDStructure::removeActive(const int n){
    for(int i=0;i<n;i++){
        this->inactiveQueue.enqueue(this->activeQueue.dequeue());
    }
}

// Takes page and adds to inactive queue
void BSDStructure::addInactive(std::vector<uint64_t> pageLocation){
    for(int i=0;i<pageLocation.size();i++){
        this->inactiveQueue.enqueue(pageLocation[i]);
    }
}

// Removes page from inactive list
std::vector<uint64_t> BSDStructure::removeInactive(const int n){
    std::vector<uint64_t> tp(n);
    for(int i=0;i<n;i++){
        tp[i] = this->inactiveQueue.dequeue();
    }

    return tp;
}

// Takes page and adds to free list
void BSDStructure::addFree(std::vector<uint64_t> pageLocation){
    for(int i=0;i<pageLocation.size();i++){
        this->freeQueue.enqueue(pageLocation[i]);
    }
}

// Removes page from free list
std::vector<uint64_t> BSDStructure::removeFree(const int n){
    std::vector<uint64_t> tp(n);
    for(int i=0;i<n;i++){
        tp[i] = this->freeQueue.dequeue();
    }

    return tp;
}

// Avgs a double vector
double avg(std::vector<double> d){
    double sum = 0;

    for(int i=0; i<d.size();i++){
        sum += d[i];
    }

    if(d.size() > 0)
        return sum/(float)d.size();
    return 0;
}

int main() {

    std::vector<double> initTime, free1Time, free2Time, allocate1Time, allocate2Time;
    //std::vector<uint64_t> memory(nPages);
    for(int outerI=0;outerI<25;outerI++){
        // Declare mockup of ram memory
        BSDStructure B;
        ptime initStart, initStop, allocate1Start, allocate1Stop, allocate2Start, allocate2Stop, free1Start, free1Stop, free2Start, free2Stop;

        // init free list
        std::vector<uint64_t> tp;
        for(uint64_t i =0; i<nPages; i++){
            tp.push_back(i);
        }
        initStart = microsec_clock::universal_time();
        B.addFree(tp);
        initStop = microsec_clock::universal_time();

        //std::cout<<"Free Queue size: "<<B.freeSize()<<"\n nPages: "<<nPages;

        // Set random operations
        std::random_device rd; // obtain a random number from hardware
        std::mt19937 eng(rd()); // seed the generator
        std::uniform_int_distribution<> distr(2, 1000); // define the range

        allocate1Start = microsec_clock::universal_time(); 
        // Allocate 3/4th memory in random size amounts
        while(B.activeSize() < 0.75*nPages){
            const int step = int(distr(eng));
            //std::cout<<"\nStep :"<<step;
            B.addActive(step);
        }
        allocate1Stop = microsec_clock::universal_time();

        free1Start =  microsec_clock::universal_time(); 
        // Free down to 1/2 of memory
        B.removeActive(nPages/4);
        free1Stop = microsec_clock::universal_time(); 

        allocate2Start = microsec_clock::universal_time();
        // Randomly allocate again upto 3/4 from 1/2
        while(B.activeSize() < 0.75*nPages){
            const int step = int(distr(eng));
            //std::cout<<"\nStep :"<<step;
            B.addActive(step);
        }
        allocate2Stop = microsec_clock::universal_time(); 

        free2Start = microsec_clock::universal_time(); 
        // Free all memory
        B.removeActive(B.activeSize());
        free2Stop = microsec_clock::universal_time(); 

        boost::posix_time::time_duration initTimeD = -(initStart-initStop);
        boost::posix_time::time_duration allocate1TimeD = -(allocate1Start - allocate1Stop);
        boost::posix_time::time_duration allocate2TimeD = -(allocate2Start - allocate2Stop);
        boost::posix_time::time_duration free1TimeD = -(free1Start - free1Stop);
        boost::posix_time::time_duration free2TimeD = -(free2Start - free2Stop);
        
        initTime.push_back(initTimeD.total_nanoseconds());
        allocate1Time.push_back(allocate1TimeD.total_nanoseconds());
        allocate2Time.push_back(allocate2TimeD.total_nanoseconds());
        free1Time.push_back(free1TimeD.total_nanoseconds());
        free2Time.push_back(free2TimeD.total_nanoseconds());
    }
    std::cout<<"\nTime Taken - BSD (ns)";
    std::cout<<"\n---------------------";
    std::cout<<"\nInit time: "<<avg(initTime);
    std::cout<<"\nAllocating 3/4th memory: "<<avg(allocate1Time);
    std::cout<<"\nAllocating from 1/2 memory to 3/4th memory: "<<avg(allocate2Time);
    std::cout<<"\nFreeing memory from 3/4 memory to 1/2: "<<avg(free1Time);
    std::cout<<"\nFreeing memory from 3/4 memory to empty: "<<avg(free2Time)<<std::endl;

}

