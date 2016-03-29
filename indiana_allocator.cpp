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
static const unsigned long nPages = long((ramSize * pow(2, 30))/(4 * pow(2,10)));
//static const unsigned long nPages = 

// Size after which to move nodes from inactive queue to free queue
static const unsigned long moveSize = long(nPages* 0.05);

// Empty structure for page frame
struct page_t {
    bool s;
};

// Frame size
static const int frameSize = sizeof(struct page_t);

// Memseg datastructure
struct memseg {
    page_t * pages;
    page_t * epages;
    struct memseg * next;
    struct memseg * lnext;
    memseg(){
        pages = NULL;
        epages = NULL;
        next = NULL;
        lnext = NULL;
    }
};

//memsegSize
static const int memsegSize = sizeof(struct memseg);

// struct to store page address used in mock-up for active list
struct node {
    page_t *page_address;
    struct node* next;
    node() {
        next = NULL;
        page_address = NULL;
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

        friend class IndianaStructure;
        //friend void addActiveQ(Queue activeQ, MemsegQueue freeQ, const int n);
        //friend void removeActiveQ(Queue activeQ, MemsegQueue freeQ, const int n);
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
    void enqueue(page_t * pages, page_t * epages) {
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
        this->size += (epages - pages + frameSize)/frameSize;
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
        if(tmp != NULL)
            return tmp;
        else{
            std::cout<<"Page element dequeued is NULL";
            return NULL;
        }
    }
    friend class IndianaStructure;
    /*friend void addFreeQ(MemsegQueue freeQ, page_t * page);*/
    //friend std::vector<page_t *> removeFreeQ(MemsegQueue freeQ, const int n);

    //friend void addInactiveQ(MemsegQueue inactiveQ, page_t * page);
    //friend std::vector<page_t *> removeInactiveQ(MemsegQueue inactiveQ, const int n);
};

// Avgs a double vecor
double avg(std::vector<double> d){
    double sum = 0;

    for(int i=0; i<d.size();i++){
        sum += d[i];
    }

    if(d.size() > 0)
        return sum/(float)d.size();
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

        void addActiveQ(const int n);
        void removeActiveQ(const int n);

        void initFreeQ(page_t *page, page_t *epage) {
            freeQ.enqueue(page, epage);
        }
        void addFreeQ(page_t * page);
        std::vector<page_t *> removeFreeQ(const int n);

        void addInactiveQ(page_t * page);
        std::vector<page_t *> removeInactiveQ(const int n);
};

void IndianaStructure::addActiveQ(const int n) {
    // Check if freeQ has enough meory to allocate n pages to activeQ
    if(n > freeQ.size){
        cout<<"\nNot enough memory in FreeQ"<<"\n n: "<<n<<"\n free queue size: "<<freeQ.size;
        exit(EXIT_FAILURE);
    }
    // Get n pages from freeQ
    std::vector<page_t *> freePages = removeFreeQ(n);
    //cout<<"\nRemoved from freeQ successfully";

    // Add to activeQ
    for(int i=0; i<freePages.size();i++){
        this->activeQ.enqueue(freePages[i]);
    }

    // Check if need to reallocate from inactiveQ to freeQ
    /*
     *if(freeQ.size < moveSize){
     *    //cout<<"\n Reallocating from inactiveQ to freeQ";
     *    // Check if inactiveQ is nonempty
     *    if(inactiveQ.size >2){
     *        std::vector<page_t *> removedPages = removeInactiveQ(inactiveQ, inactiveQ.size -1);
     *        for(int i=0; i<removedPages.size(); i++){
     *            addFreeQ(freeQ, removedPages[i]);
     *        }
     *    }
     *}
     */
}

void IndianaStructure::removeActiveQ(const int n){
    for(int i=0; i<n; i++){
        if(this->activeQ.isempty()){
            break;
        }
        this->addFreeQ(this->activeQ.dequeue());
    }
}

void IndianaStructure::addInactiveQ(page_t * page){
    struct memseg * tmp = inactiveQ.head;
    do{
        if( (tmp->pages - frameSize) == page){
            tmp->pages = page;
            this->inactiveQ.size++;
            return;
        }
        else if ( (tmp->epages + frameSize) == page){
            tmp->epages = page;
            this->inactiveQ.size++;
            return;
        }
        tmp = tmp->next;
    }while(tmp!=this->inactiveQ.tail && tmp !=NULL);

    // When page is not contiguous to any existing blocks 
    tmp = new memseg();
    tmp->pages = page;
    tmp->epages = page;
    inactiveQ.tail->next = tmp;
    inactiveQ.tail = tmp;
    this->inactiveQ.size++;
}

void IndianaStructure::addFreeQ(page_t * page){
    struct memseg * tmp = this->freeQ.head;
    do{
        if((tmp->pages - frameSize) == page){
            tmp->pages = page;
            this->freeQ.size++;
            return;
        }
        else if((tmp->epages + frameSize) == page){
            tmp->epages = page;
            this->freeQ.size++;
            return;
        }
        tmp = tmp->next;
    }while(tmp!=this->freeQ.tail && tmp !=NULL);

    // When page is not contiguous to any existing blocks 
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

std::vector<page_t *> IndianaStructure::removeFreeQ(int n){
    std::vector<page_t *> removedPages;
    memseg * tp = freeQ.head;
    // Flag to check if current memseg under operation is empty or not
    bool segEmpty = false;
    while(n > 0){
        const int blockLength = (tp->epages - tp->pages)/frameSize;
        // Allocate memory from freeQ
        for(int i=0; i<min(n,blockLength);i++){
            removedPages.push_back(tp->pages + frameSize*i);
        }
        if(n >= blockLength){
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

std::vector<page_t *> IndianaStructure::removeInactiveQ(int n){
    std::vector<page_t *> removedPages;
    memseg * tp = inactiveQ.head;
    // Flag to check if current memseg under operation is empty or not
    bool segEmpty = false;
    while(n > 0){
        const int blockLength = (tp->epages - tp->pages)/frameSize;
        // Allocate memory from freeQ
        for(int i=0; i<min(n,blockLength);i++){
            removedPages.push_back(tp->pages + frameSize*i);
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
    std::vector<page_t> memory(nPages);

    for(int outerI=0;outerI<1;outerI++){
        // Timestamps
        ptime initStart, initStop, allocate1Start, allocate1Stop, allocate2Start, allocate2Stop, free1Start, free1Stop, free2Start, free2Stop;
        // Queues 
        //MemsegQueue freeQ, inactiveQ;
        //Queue activeQ;
        IndianaStructure I;

        initStart = microsec_clock::universal_time();
        // init free memory
        I.initFreeQ(&memory[0], &memory[nPages-1]);
        initStop = microsec_clock::universal_time();

        cout<<"\nFreeQ size: "<<I.sizeFreeQ();

        // Set random operations
        std::random_device rd; // obtain a random number from hardware
        std::mt19937 eng(rd()); // seed the generator
        std::uniform_int_distribution<> distr(2, 1000); // define the range

        allocate1Start = microsec_clock::universal_time();
        // Allocate 3/4 memory in random size amounts
        while(I.sizeActiveQ() < 0.75*nPages){
            const int step = int(distr(eng));
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
            const int step = int(distr(eng));
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
    std::cout<<"\nTime Taken";
    std::cout<<"\n----------";
    std::cout<<"\nInit time: "<<avg(initTime);
    std::cout<<"\nAllocating 3/4th memory: "<<avg(allocate1Time);
    std::cout<<"\nAllocating from 1/2 memory to 3/4th memory: "<<avg(allocate2Time);
    std::cout<<"\nFreeing memory from 3/4 memory to 1/2: "<<avg(free1Time);
    std::cout<<"\nFreeing memory from 3/4 memory to empty: "<<avg(free2Time)<<std::endl;
}


