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
static const unsigned long nPages = long((ram_size * pow(2, 30))/(4 * pow(2,10)));
//static const unsigned long nPages = 400000;

// Size after which to move nodes from inactive queue to free queue
static const unsigned long moveSize = long(nPages* 0.05);

// Empty structure for page frame
struct page_t {
    bool s;
};

// List which stores pages in UVM 
struct node {
    page_t *page_address;
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
        long size;
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
        void enqueue(page_t * tmp) {
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
        page_t * dequeue() {
            
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
                return NULL;
            }
        }
};

// Class for BSD memory allocation queues
class BSDStructure {
    private:
        Queue activeQueue, inactiveQueue, freeQueue;

    public:
        unsigned long freeSize(){
            return freeQueue.size;
        }

        unsigned long activeSize(){
            return activeQueue.size;
        }

        unsigned long inactiveSize(){
            return inactiveQueue.size;
        }

        // Active queue operation
        void addActive(const int n);
        void removeActive(const int n);

        // Free queue operations
        void addFree(std::vector<page_t *> pageLocation);
        std::vector<page_t *> removeFree(const int n);

        // Inactive queue operations
        void addInactive(std::vector<page_t *> pageLocation);
        std::vector<page_t *> removeInactive(const int n);
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
void BSDStructure::addInactive(std::vector<page_t *> pageLocation){
    for(int i=0;i<pageLocation.size();i++){
        this->inactiveQueue.enqueue(pageLocation[i]);
    }
}

// Removes page from inactive list
std::vector<page_t *> BSDStructure::removeInactive(const int n){
    std::vector<page_t *> tp(n);
    for(int i=0;i<n;i++){
        tp[i] = this->inactiveQueue.dequeue();
    }

    return tp;
}

// Takes page and adds to free list
void BSDStructure::addFree(std::vector<page_t *> pageLocation){
    for(int i=0;i<pageLocation.size();i++){
        this->freeQueue.enqueue(pageLocation[i]);
    }
}

// Removes page from free list
std::vector<page_t *> BSDStructure::removeFree(const int n){
    std::vector<page_t *> tp(n);
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
    std::vector<page_t> memory(nPages);
    for(int outerI=0;outerI<25;outerI++){
        // Declare mockup of ram memory
        BSDStructure B;
        ptime initStart, initStop, allocate1Start, allocate1Stop, allocate2Start, allocate2Stop, free1Start, free1Stop, free2Start, free2Stop;

        initStart = microsec_clock::universal_time();
        // init free list
        for(unsigned long i =0; i<nPages; i++){
            std::vector<page_t *> tp(1);
            tp[0] = &memory[i];
            B.addFree(tp);
        }
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

        boost::posix_time::time_duration inittimed = -(initstart-initstop);
        boost::posix_time::time_duration allocate1timed = -(allocate1start - allocate1stop);
        boost::posix_time::time_duration allocate2timed = -(allocate2start - allocate2stop);
        boost::posix_time::time_duration free1timed = -(free1start - free1stop);
        boost::posix_time::time_duration free2timed = -(free2start - free2stop);
        
        inittime.push_back(inittimed.total_nanoseconds());
        allocate1time.push_back(allocate1timed.total_nanoseconds());
        allocate2time.push_back(allocate2timed.total_nanoseconds());
        free1time.push_back(free1timed.total_nanoseconds());
        free2time.push_back(free2timed.total_nanoseconds());
    }
    std::cout<<"\nTime Taken";
    std::cout<<"\n----------";
    std::cout<<"\nInit time: "<<avg(initTime);
    std::cout<<"\nAllocating 3/4th memory: "<<avg(allocate1Time);
    std::cout<<"\nAllocating from 1/2 memory to 3/4th memory: "<<avg(allocate2Time);
    std::cout<<"\nFreeing memory from 3/4 memory to 1/2: "<<avg(free1Time);
    std::cout<<"\nFreeing memory from 3/4 memory to empty: "<<avg(free2Time)<<std::endl;

}

