#include <iostream>
#include <vector>
#include "types.h"
#include "list.h"
#include "helper.h"

#include <cstdlib>
#include "random"
#include "math.h"
#include <boost/date_time/posix_time/posix_time.hpp>

#define MAX_ORDER 11

using namespace std;
using namespace boost::posix_time;

extern struct page * mem_map_base;

// Sets page size in KB
static const int page_size = 4;

// Ram size in GB
static const int ram_size = 16;

// Number of frames
// static const unsigned long
// nPages = int((ram_size * pow(2, 30))/(page_size * pow(2,10)));
static const unsigned long nPages = 4194304; // (2^22)

// Declare mockup of ram memory
static struct page mem_map[nPages];

class Buddy {
private:
  struct free_area free_area[MAX_ORDER];

public:
  Buddy() {};
  Buddy(struct page* mem_map);

  unsigned int fix_order(const unsigned int page_num);
  struct page * alloc_pages(const unsigned int order);
  void free_pages(struct page *page, unsigned int order);
};

unsigned int Buddy::fix_order(const unsigned int page_num) {
  //TODO
  return 0;
}

Buddy::Buddy(struct page* mem_map) {
  int order;
  unsigned long pfn;
  struct page * page;
  for (order = 0; order < MAX_ORDER; order++) {
    // init empty list
    INIT_LIST_HEAD(&free_area[order].free_list);
  }
  for (pfn = 0; pfn < nPages; pfn++) {
    page = &mem_map[pfn];
    free_pages(page, 0);// add page to buddy system
  }
};

struct page * Buddy::alloc_pages(const unsigned int order) {
	struct free_area * area;
	unsigned int current_order;
	struct page *page;
	for (current_order = order; current_order < MAX_ORDER;
       ++current_order) {
		area = &free_area[current_order];
		if (list_empty(&area->free_list))
			continue;
		//Get first page of the free_list
		page = list_entry(area->free_list.next, struct page, lru);
		list_del(&page->lru);
    set_page_order(page, 0);
		// area->nr_free--;
		expand(page, order, current_order, area);
		return page;
	}
	return NULL;
}

void Buddy::free_pages(struct page *page, unsigned int order) {
	unsigned long buddy_idx;
  unsigned long page_idx;
	page_idx = page_to_pfn(page) & ((1 << MAX_ORDER) - 1); // ?? very confusing
	while (order < MAX_ORDER-1) {
		unsigned long combined_idx;
		// struct free_area * area;
		struct page *buddy;
    buddy_idx = __find_buddy_index(page_idx, order);
		buddy = page + ((page_idx^(1<<order))-page_idx);
		if (!page_is_buddy(buddy, order))
			break;		/* Move the buddy up one level. */
		list_del(&buddy->lru);
		// area = &free_area[order];
		// area->nr_free--;
		rmv_page_order(buddy);
		// get page_idx after combining
    combined_idx = buddy_idx & page_idx;
		// get first page after combining
		page = page + (combined_idx - page_idx);
		page_idx = combined_idx;
		order++;
	}
	set_page_order(page, order);
	list_add(&page->lru, &free_area[order].free_list);
	// free_area[order].nr_free++;
}

// Avgs a double vector
double avg(std::vector<double> d){
  double sum = 0;

  for(unsigned int i=0; i<d.size();i++){
    sum += d[i];
  }

  if(d.size() > 0)
    return sum/(float)d.size();
  return 0;
}

int main() {
  mem_map_base = (struct page *)&mem_map;
  std::vector<struct page *> alloc_page_p;
  std::vector<int> alloc_page_order;
  struct page * page;
  unsigned int i;

  std::vector<double> initTime, free1Time, free2Time, allocate1Time, allocate2Time;
  ptime initStart, initStop, allocate1Start, allocate1Stop, allocate2Start, allocate2Stop, free1Start, free1Stop, free2Start, free2Stop;

  // Set random operations
  std::random_device rd; // obtain a random number from hardware
  std::mt19937 eng(rd()); // seed the generator
  std::uniform_int_distribution<> distr(1, 10); // define the range

  for(int i; i<100; i++) {
    initStart = microsec_clock::universal_time();
    Buddy buddy(mem_map);
    initStop = microsec_clock::universal_time();

    /* test 1 */
    unsigned long psum = 0;
    allocate1Start = microsec_clock::universal_time();
    while(psum < 0.75*nPages){
      int porder = distr(eng);
      psum += 1 << porder;
      page = buddy.alloc_pages(porder);
      alloc_page_p.push_back(page);
      alloc_page_order.push_back(porder);
      // cout << page << ": " << porder << endl;
    }
    allocate1Stop = microsec_clock::universal_time();

    free1Start =  microsec_clock::universal_time();
    for (i=0; i<alloc_page_p.size() && psum > nPages/2; i++) {
      int porder = alloc_page_order[i];
      psum -= 1 << porder;
      buddy.free_pages(alloc_page_p[i], porder);
    }
    free1Stop = microsec_clock::universal_time();

    /* test 2 */
    allocate2Start = microsec_clock::universal_time();
    while(psum < 0.75*nPages){
      int porder = distr(eng);
      psum += 1 << porder;
      page = buddy.alloc_pages(porder);
      alloc_page_p.push_back(page);
      alloc_page_order.push_back(porder);
      // cout << page << ": " << porder << endl;
    }
    allocate2Stop = microsec_clock::universal_time();
    free2Start = microsec_clock::universal_time();
    for (i=0; i<alloc_page_p.size(); i++) {
      buddy.free_pages(alloc_page_p[i], alloc_page_order[i]);
    }
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

  std::cout<<"\nTime Taken";
  std::cout<<"\n----------";
  std::cout<<"\nInit time: "<< avg(initTime);
  std::cout<<"\nAllocating 3/4th memory: "<<avg(allocate1Time);
  std::cout<<"\nAllocating from 1/2 memory to 3/4th memory: "<<avg(allocate2Time);
  std::cout<<"\nFreeing memory from 3/4 memory to 1/2: "<<avg(free1Time);
  std::cout<<"\nFreeing memory from 3/4 memory to empty: "<<avg(free2Time)<<std::endl;

  return 0;
}

