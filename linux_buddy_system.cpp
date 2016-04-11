/***
 * Test the time cost for Linux buddy system
 * to allocate a page and free a page without
 * mimic memory model.
 *
 * Allocating test:
 *   allocate memory one page a time with
 *   page index from 0 to nPages.
 *
 * Freeing test:
 *   free memory one page a time with
 *   page index from nPages to 0.
 */

#include <iostream>
#include <vector>
#include "linux_helper/types.h"
#include "linux_helper/list.h"
#include "linux_helper/helper.h"
#include <boost/date_time/posix_time/posix_time.hpp>

#define MAX_ORDER 11

using namespace std;
using namespace boost::posix_time;

extern struct page * mem_map_base;

// Number of pages, should be relative large
// It also should be larger than or equal to 2^MAX_ORDER
// and it needs to be times of 64, since the buddy system
// free 64 pages a time when initializing.
// here I use the number of frames for 4GB memory in order
// to get a relative stable results
static const unsigned long nPages = 1048576;

// Declare mockup of ram memory
static struct page mem_map[nPages];

class Buddy {
private:
  struct free_area free_area[MAX_ORDER];

public:
  Buddy() {};
  Buddy(struct page* mem_map);

  struct page * alloc_pages(const unsigned int order);
  void free_pages(struct page *page, unsigned int order);
};

Buddy::Buddy(struct page* mem_map) {
  int order;
  unsigned long pfn;
  struct page * page;
  for (order = 0; order < MAX_ORDER; order++) {
    // init empty list
    INIT_LIST_HEAD(&free_area[order].free_list);
  }
  for (pfn = 0; pfn < nPages; pfn += 64) {
    page = &mem_map[pfn];
    free_pages(page, 6);// add page to buddy system
                        // 2^6 = 64, 6 is the number
                        // of bits for long type of 64-bit system
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
    expand(page, order, current_order, area);
    return page;
  }
  return NULL;
}

void Buddy::free_pages(struct page *page, unsigned int order) {
  unsigned long buddy_idx;
  unsigned long page_idx;
  // TODO very confused with the line below
  page_idx = page_to_pfn(page) & ((1 << MAX_ORDER) - 1);
  while (order < MAX_ORDER-1) {
    unsigned long combined_idx;
    // struct free_area * area;
    struct page *buddy;
    buddy_idx = __find_buddy_index(page_idx, order);
    buddy = page + ((page_idx^(1<<order))-page_idx);
    if (!page_is_buddy(buddy, order))
        break;      /* Move the buddy up one level. */
    list_del(&buddy->lru);
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
  struct page * page;

  std::vector<double> initTime, free1Time, allocate1Time;
  ptime initStart, initStop, allocate1Start, allocate1Stop, free1Start, free1Stop;

  Buddy buddy(mem_map);
  for(unsigned int i = 0; i<10; i++) {
    // initStart = microsec_clock::universal_time();
    // Buddy buddy(mem_map);
    // initStop = microsec_clock::universal_time();

    /* allcation test: allocate 1 page each time */
    allocate1Start = microsec_clock::universal_time();
    for (unsigned int j=0; j < nPages; j++) {
      page = buddy.alloc_pages(0); // order == 0
      alloc_page_p.push_back(page);
    }
    allocate1Stop = microsec_clock::universal_time();

    /* free memory test: free 1 page each time */
    free1Start =  microsec_clock::universal_time();
    while (alloc_page_p.size() > 0) {
      buddy.free_pages(alloc_page_p.back(), 0); // order == 0
      alloc_page_p.pop_back();
    }
    free1Stop = microsec_clock::universal_time();

    // boost::posix_time::time_duration initTimeD = -(initStart-initStop);
    boost::posix_time::time_duration allocate1TimeD = -(allocate1Start - allocate1Stop);
    boost::posix_time::time_duration free1TimeD = -(free1Start - free1Stop);

    // initTime.push_back(initTimeD.total_nanoseconds());
    allocate1Time.push_back(allocate1TimeD.total_nanoseconds()/nPages);
    free1Time.push_back(free1TimeD.total_nanoseconds()/nPages);
  }

  std::cout<<"\nTime Taken - Linux (ns)";
  std::cout<<"\n-----------------------";
  // std::cout<<"\nInit time: "<< avg(initTime);
  std::cout<<"\nAllocating 1 page: "<<avg(allocate1Time);
  std::cout<<"\nFreeing 1 page: "<<avg(free1Time);
  std::cout << endl;

  return 0;
}

