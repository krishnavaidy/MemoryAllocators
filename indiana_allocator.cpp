#include<iostream>
#include<vector>
#include<cstdlib>
#include "random"
#include "math.h"
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace boost::posix_time;
using namespace std;

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

// Memseg datastructure
struct memseg {
    page_t *pages, epages;
    struct memseg * next;
    struct memseg * lnext;
};

// struct to store page address used in mock-up for active list
struct node {
    page_t *page_address;
    struct node* next;
    node() {
        next = NULL;
    }
};

// Declare classes
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

        friend void addActiveQ(Queue activeQ, MemsegQueue freeQ, const int n);
        friend removeActiveQ(Queue activeQ, MemsegQueue inactiveQ, const int n);
};

class MemsegQueue {
    private:
    struct memseg* head;
    struct memseg* tail;

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
    void enqueue(page_t * pages, page_t * epages) {
        struct memseg* tmpNode = new struct memseg();
        tmpNode->pages = pages;
        tmpNode->epages = epages;
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

    friend void addFreeQ(MemsegQueue freeQ, page_t * page);
    friend std::vector<page_t *> * removeFreeQ(MemsegQueue freeQ, const int n);

    friend void addInactiveQ(MemsegQueue inactiveQ, page_t * page);
    friend std::vector<page_t *> removeInactiveQ(MemsegQueue inactiveQ, const int n);
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

void addActiveQ(Queue activeQ, MemsegQueue freeQ, const int n){
    // Check if freeQ has enough meory to allocate n pages to activeQ
    if(n > freeQ.size){
        cout<<"\nNot enough memory in FreeQ";
        exit(EXIT_FAILURE);
    }
    // Get n pages from freeQ
    std::vector<page_t *> * freePages = removeFree(freeQ, n);

    // Add to activeQ
    for(int i=0; i<*freePages.size();i++){
        activeQ.enqueue(&(*freePages[i]));
    }
}

void removeActiveQ(Queue activeQ, MemsegQueue inactiveQ, const int n){
    for(int i=0; i<n; i++){
        if(activeQ.isempty()){
            break;
        }
        addInactiveQ(inactiveQ, activeQ.dequeue());
    }
}

void addInactiveQ(MemsegQueue inactiveQ, page_t * page){
    struct memseg * tmp = inactiveQ.head;
    const int memsegSize= sizeof(page_t);
    do{
        if( (tmp->pages - memsegSize) == page){
            tmp->pages = page;
            return;
        }
        else if ( (tmp->epages + memsegSize) == page){
            tmp->epages = page;
            return;
        }
    }while{tmp!=inactiveQ.tail};

    // When page is not contiguous to any existing blocks 
    tmp = new memseg();
    tmp->pages = page;
    tmp->epages = page;
    inactiveQ.tail->next = tmp;
    inactiveQ.tail = tmp;
}

void addFreeQ(MemsegQueue freeQ, page_t * page){
    struct memseg * tmp = freeQ.head;
    const int memsegSize= sizeof(page_t);
    do{
        if( (tmp->pages - memsegSize) == page){
            tmp->pages = page;
            return;
        }
        else if ( (tmp->epages + memsegSize) == page){
            tmp->epages = page;
            return;
        }
    }while{tmp!=this->tail};

    // When page is not contiguous to any existing blocks 
    tmp = new memseg();
    tmp->pages = page;
    tmp->epages = page;
    freeQ.tail->next = tmp;
    freeQ.tail = tmp;
}

int main() {

    std::vector<double> initTime, free1Time, free2Time, allocate1Time, allocate2Time;
    std::vector<page_t> memory(nPages);

    for(int outerI=0;i<outerI<25;outerI++){
        // init free memory
        MemsegQueue freeQ, inactiveQ;
        Queue activeQ;
        activeQ.enque(&memory[0], &memory[nPages-1]);
    }
}


