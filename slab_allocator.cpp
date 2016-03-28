#include<iostream>
#include<vector>

using namespace std; 

// Sets page size
static const int page_size = 8000;

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

// memseg datastructure which stores free frames
struct memseg {
    page_t *pages, *epages;
    struct memseg *next;
};

int main() {
}

