#include<iostream>
#include<vector>
#include "math.h"

using namespace std; 

// Sets page size in KB
static const int page_size = 4;

// Ram size in GB
static const int ram_size = 16;

// Number of frames
static const int n_pages = int((ram_size * pow(2, 30))/(4 * pow(2,10)));

// Size after which to move nodes from inactive queue to free queue
static const int move_size = int(n_pages* 0.05);

// Object to mock page frames
// Each page is of size 8KB
struct page_t {
    char s;
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
        int size;
        queue() {
            this->head = NULL;
            this->tail = NULL;
            this->size = 0;
        }

        // Is empty
        bool isempty(){
            if(head == tail == NULL)
                return true;

            return false;
        }

        // Add elements to queue, given a node
        void enqueue(struct node * tmp) {
            tmp->next = this->tail;
            this->tail = tmp;

            // Check for empty queue condition
            if(this->head == NULL) {
               this->head = this->tail; 
            }

            // Increment size
            this->size++;
        }

        // Remove element from queue
        struct node dequeue() {
            
            // Check for empty queue condition
            if(this->head == NULL){
                cout<<"Attempt to dequeue an empty queue";
                exit(0);
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
                cout<<"Page element dequeued is NULL";
                return NULL;
            }
        }
};

// Class for BSD memory allocation queues
class BSDStructure {
    private:
        Queue activeQueue, inactiveQueue, freeQueue;

    public:
        // Active queue operation
        void addActive();
        void removeActive();

        // Free queue operations
        void addFree(page_t * pageLocation);
        page_t * removeFree();

        // Inactive queue operations
        void addInactive(page_t * pageLocation);
        page_t * removeInactive();
};

// Takes page from free list and adds to active list
void BSDStructure::addActive() {
    this->activeQueue.enqueue(this->freeQueue.dequeue());

    if(this->freeQueue.size < move_size){
        while(!inactiveQueue.isempty()){
            addFree(removeInactive());
        }
    }
}

// Removes page from active list and adds to inactive list
void BSDStructure::removeActive(){
    this->inactiveQueue.enqueue(this->activeQueue.dequeue());
}

// Takes page and adds to inactive queue
void BSDStructure::addInactive(page_t * pageLocation){
    this->inactiveQueue.enqueue(pageLocation);
}

// Removes page from inactive list
page_t * removeInactive(){
    return this->inactiveQueue.dequeue();
}

// Takes page and adds to free list
void BSDStructure::addActive(page_t * pageLocation){
    this->activeQueue.enqueue(pageLocation);
}

// Removes page from free list
void BSDStructure::removeFree(){
    return this->freeQueue.dequeue();
}


int main() {
    // Declare mockup of ram memory
    std::vector<page_t> memory(n_pages);
    BSDStructure B;

    // init free list
    for(int i =0; i<n_pages; i++){
        B.addFree(&memory[i]);
    }
}

