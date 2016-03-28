#include<iostream>
#include<vector>

using namespace std; 

// Sets page size
static const int page_size = 4000;

// Ram size
static const int ram_size = 800000

// Size after which to move nodes from inactive queue to free queue
static const int move_size = int(ram_size * 0.1)

// Object to mock page frames
// Each page is of size 8KB
struct page_t {
    std::vector<char> s(page_size);
};

// Zero a page
void zero(page_t p){

    // For loop to manually zero contents of a page
    for(int i =0; i<page_size; i++) {
        p.s[i]=0;
    }
}

// List which stores pages in UVM 

struct node {
    page_t *page_address;
    struct node* next;
    node() {
        next = NULL;
    }
};

class Queue {
    private:
        struct node* head;
        struct node* tail;
        int size;

    public:
        queue() {
            head = NULL;
            tail = NULL;
            size = 0;
        }

        friend void add_active(page_t *page_location);
        friend void remove_active(page_t *page_location)
        void remove();
};

// Add pages to active queue
void add_active(Queue *q, page_t * page_location) {
    struct node tmp;
    tmp.page_address = page_location;
    tmp.next = q->tail;
    q->tail = tmp;

    // Check when queue is empty
    if(q->head == NULL){
        q->head = q->tail;
    }

    size++;
    // TODO: when active queue size goes beyond a certain point
    if(size > move_size){
        // TODO
    }
}

// Remove page from active queue to inactive queue
void remove_active(Queue q, page_t *page_location) {
    struct node * tmp = q->head;
}

// initialize active_list

int main() {
    // Declare mockup of ram memory
    std::vector<page_t> memory(ram_size);
    struct Queue active_queue, inactive_queue, free_queue;

}

