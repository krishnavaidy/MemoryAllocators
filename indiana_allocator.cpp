#include<iostream>
#include<vector>
#include<cstdlib>
#include "random"
#include "math.h"
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace boost::posix_time;
using namespace std;

// Sets page size in KB
static const int pageSize = 4;

// Ram size in GB
static const int ramSize = 16;

// Number of frames
static uint64_t nPages = ((ramSize * pow(2, 30))/(4 * pow(2,10)));

// Size after which to move nodes from inactive queue to free queue
static uint64_t moveSize = long(nPages* 0.05);

// Memseg datastructure
struct memseg {
    uint64_t pages;
    uint64_t epages;
    struct memseg * next;
    struct memseg * lnext;
    memseg(){
        pages = -1;
        epages = -1;
        next = NULL;
        lnext = NULL;
    }
};

//memsegSize
static const int memsegSize = sizeof(struct memseg);

// struct to store page address used in mock-up for active list
struct node {
    uint64_t page_address;
    struct node* next;
    node() {
        next = NULL;
        page_address = -1;
    }
};

// Declare classes
class IndianaStructure;
class Queue;
class MemsegQueue;

// Class which implements Queue
// TODO: Refactor to implement templates
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
            if(tmp != NULL){
                uint64_t tmp_page_address = tmp->page_address;
                free(tmp);
                return tmp_page_address;
            }
            else{
                std::cout<<"Page element dequeued is NULL";
                free(tmp);
                return -1;
            }
        }

        friend class IndianaStructure;
};

class MemsegQueue {
    private:
    struct memseg* head;
    struct memseg* tail;

    public:
    long size;
    MemsegQueue() {
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
    void enqueue(uint64_t pages, uint64_t epages) {
        struct memseg* tmpNode = new struct memseg();
        tmpNode->pages = pages;
        tmpNode->epages = epages;

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
        this->size += epages - pages;
    }

    // Remove element from queue
    struct memseg * dequeue() {

        // Check for empty queue condition
        if(this->head == NULL){
            std::cout<<"\nAttempt to dequeue an empty queue";
            exit(EXIT_FAILURE);
        }

        struct memseg * tmp = NULL;

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
        if(tmp != NULL){
            free(tmp);
            return NULL;
        }
        else{
            std::cout<<"Page element dequeued is NULL";
            return NULL;
        }
    }
    friend class IndianaStructure;
};

// Avgs a double vecor
double avg(std::vector<double> d){
    double sum = 0;

    for(int i=0; i<d.size();i++){
        sum += d[i];
    }

    if(d.size() > 0)
        return sum/(double)d.size();
    return 0;
}
class IndianaStructure{
    private:
        Queue activeQ;
        MemsegQueue inactiveQ, freeQ;

    public:
        int sizeActiveQ(){
            return activeQ.size;
        }

        int sizeInactiveQ(){
            return inactiveQ.size;
        }

        int sizeFreeQ(){
            return freeQ.size;
        }

        void addActiveQ(const uint64_t n);
        void removeActiveQ(const uint64_t n);

        void initFreeQ(uint64_t page, uint64_t epage) {
            freeQ.enqueue(page, epage);
        }
        void addFreeQ(const uint64_t page);
        std::vector<uint64_t> removeFreeQ(uint64_t n);

        void addInactiveQ(const uint64_t page);
        std::vector<uint64_t> removeInactiveQ(uint64_t n);
};

void IndianaStructure::addActiveQ(const uint64_t n) {
    // Check if freeQ has enough meory to allocate n pages to activeQ
    if(n > freeQ.size){
        cout<<"\nNot enough memory in FreeQ"<<"\n n: "<<n<<"\n free queue size: "<<freeQ.size;
        exit(EXIT_FAILURE);
    }
    // Get n pages from freeQ
    std::vector<uint64_t> freePages = removeFreeQ(n);
    //cout<<"\nRemoved from freeQ successfully";

    // Add to activeQ
    for(int i=0; i<freePages.size();i++){
        this->activeQ.enqueue(freePages[i]);
    }
}

void IndianaStructure::removeActiveQ(const uint64_t n){
    for(int i=0; i<n; i++){
        if(this->activeQ.isempty()){
            break;
        }
        this->addFreeQ(this->activeQ.dequeue());
    }
}

void IndianaStructure::addInactiveQ(uint64_t page){
    struct memseg * tmp = inactiveQ.head;

    // Add page to head of list
    tmp = new memseg();
    tmp->pages = page;
    tmp->epages = page;
    inactiveQ.tail->next = tmp;
    inactiveQ.tail = tmp;
    this->inactiveQ.size++;
}

void IndianaStructure::addFreeQ(uint64_t page){
    struct memseg * tmp = this->freeQ.head;

    // Add page to head of list
    tmp = new memseg();
    tmp->pages = page;
    tmp->epages = page;
    freeQ.tail->next = tmp;
    freeQ.tail = tmp;
    this->freeQ.size++;
}

int min(const int x, const int y){
    if(x < y)
        return x;
    return y;
}

std::vector<uint64_t>  IndianaStructure::removeFreeQ(uint64_t n){
    std::vector<uint64_t> removedPages;
    memseg * tp = freeQ.head;
    // Flag to check if current memseg under operation is empty or not
    bool segEmpty = false;
    while(n > 0){
        const int blockLength = (tp->epages - tp->pages);
        // Allocate memory from freeQ
        for(int i=0; i<min(n,blockLength);i++){
            removedPages.push_back(tp->pages + i);
        }
        if(n >= blockLength){
            // FIXME: Use functions and not touch Memseg private members 
            // directly
            freeQ.head = tp->next;
            free(tp);
            freeQ.size -= blockLength;
            // When all required memory is allocated
            if(n==blockLength)
                return removedPages;
            n -= blockLength;
        }
        else{
            freeQ.size -= n;
            n=0;
        }
    }
    return removedPages;
}

std::vector<uint64_t> IndianaStructure::removeInactiveQ(uint64_t n){
    std::vector<uint64_t> removedPages;
    memseg * tp = inactiveQ.head;
    // Flag to check if current memseg under operation is empty or not
    bool segEmpty = false;
    while(n > 0){
        const int blockLength = (tp->epages - tp->pages);
        // Allocate memory from freeQ
        for(int i=0; i<min(n,blockLength);i++){
            removedPages.push_back(tp->pages + i);
        }
        if(n >= blockLength){
            inactiveQ.head = tp->next;
            free(tp);
            // When all required memory is allocated
            if(n==blockLength)
                return removedPages;
            n -= blockLength;
        }
    }
    return removedPages;
}

int main() {

    std::vector<double> initTime, free1Time, free2Time, allocate1Time, allocate2Time;
    // Memory 

    for(int outerI=0;outerI<10;outerI++){
        // Timestamps
        ptime initStart, initStop, allocate1Start, allocate1Stop, allocate2Start, allocate2Stop, free1Start, free1Stop, free2Start, free2Stop;
        // Queues 
        IndianaStructure I;

        initStart = microsec_clock::universal_time();
        // init free memory
        I.initFreeQ(0, nPages -1);
        initStop = microsec_clock::universal_time();

        //cout<<"\nFreeQ size: "<<I.sizeFreeQ();

        // Set random operations

        allocate1Start = microsec_clock::universal_time();
        // Allocate 3/4 memory in random size amounts
        while(I.sizeActiveQ() < 0.75*nPages){
            const int step = 1;
            I.addActiveQ(step);
            //cout<<endl<<step<<"\nactive size: "<<I.sizeActiveQ();
        }
        allocate1Stop = microsec_clock::universal_time();
        //cout<<"\nAllocated 3/4 of memory";

        // Free down to 1/2 of memory
        free1Start = microsec_clock::universal_time();
        I.removeActiveQ(nPages/4);
        free1Stop = microsec_clock::universal_time();
        //cout<<"\nFreed down to 1/2 of memory";

        allocate2Start = microsec_clock::universal_time();
        // Randomly allocate upto 3/4 memory from 1/2
        while(I.sizeActiveQ() < 0.75*nPages){
            const int step = 1; 
            I.addActiveQ(step);
        }
        allocate2Stop = microsec_clock::universal_time();
        //cout<<"\nAllocate again upto 3/4 of memory";

        free2Start = microsec_clock::universal_time();
        // Free all memory
        I.removeActiveQ(0.75*nPages);
        free2Stop = microsec_clock::universal_time();
        //cout<<"\nFreed all memory";

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
    std::cout<<"\nTime Taken - OpenIndiana (ns)";
    std::cout<<"\n-----------------------------";
    std::cout<<"\nInit time: "<<avg(initTime);
    std::cout<<"\nAvg time for allocating a single frame while allocating 3/4th memory: "<<avg(allocate1Time)/(0.75*nPages);
    std::cout<<"\nAvg time for allocating a single frame while alllocating from 1/2 memory to 3/4th memory: "<<avg(allocate2Time)/(0.25*nPages);
    std::cout<<"\nFreeing memory from 3/4 memory to 1/2: "<<avg(free1Time)/(nPages/4);
    std::cout<<"\nFreeing memory from 3/4 memory to empty: "<<avg(free2Time)/(nPages*0.75)<<std::endl;
}


