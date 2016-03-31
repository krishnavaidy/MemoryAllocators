#ifndef _HELPER_H
#define _HELPER_H

#include "types.h"
#include "list.h"

#define page_to_pfn(page) ((unsigned long)((page) - mem_map_base))

/**
 * Returns a pointer to the container of this list element.
 *
 * Example:
 * struct foo* f;
 * f = container_of(&foo->entry, struct foo, entry);
 * assert(f == foo);
 *
 * @param ptr Pointer to the struct list_head.
 * @param type Data type of the list element.
 * @param member Member name of the struct list_head field in the list element.
 * @return A pointer to the data struct containing the list head.
 */
#ifndef container_of
#define container_of(ptr, type, member)                   \
  (type *)((char *)(ptr) - (char *) &((type *)0)->member)
#endif

#define set_page_private(page, v) ((page)->_private = (v))

static inline void __SetPageBuddy(struct page *page)
{
  page->_mapcount = PAGE_BUDDY_MAPCOUNT_VALUE;
}

static inline void __ClearPageBuddy(struct page *page)
{
  page->_mapcount = -1;
}

static inline void set_page_order(struct page *page, int order)
{
  set_page_private(page, order);
  __SetPageBuddy(page);
}

static inline void rmv_page_order(struct page *page)
{
  __ClearPageBuddy(page);
  set_page_private(page, 0);
}

static inline void expand(struct page *page,
                          int low, int high, struct free_area *area)
{
  unsigned long size = 1 << high;
  while (high > low) {
    area--;
    high--;
    size >>= 1;
    list_add(&page[size].lru, &area->free_list);
    /* area->nr_free++; */
    set_page_order(&page[size], high);
  }
}

/*
 * PageBuddy() indicate that the page is free and in the buddy system
 * (see mm/page_alloc.c).
 *
 * PAGE_BUDDY_MAPCOUNT_VALUE must be <= -2 but better not too close to
 * -2 so that an underflow of the page_mapcount() won't be mistaken
 * for a genuine PAGE_BUDDY_MAPCOUNT_VALUE. -128 can be created very
 * efficiently by most CPU architectures.
 */

static inline int PageBuddy(struct page *page)
{
  return page->_mapcount == PAGE_BUDDY_MAPCOUNT_VALUE;
}

static inline int page_order(struct page *page)
{
  return page->_private;
}

/*
 * This function checks whether a page is free && is the buddy
 * we can do coalesce a page and its buddy if
 * (a) the buddy is not in a hole && (deleted)
 * (b) the buddy is in the buddy system && (deleted)
 * (c) a page and its buddy have the same order &&
 * (d) a page and its buddy are in the same zone.
 *
 * For recording whether a page is in the buddy system, we set ->_mapcount
 * PAGE_BUDDY_MAPCOUNT_VALUE.
 * Setting, clearing, and testing _mapcount PAGE_BUDDY_MAPCOUNT_VALUE is
 * serialized by zone->lock.
 *
 * For recording page's order, we use page_private(page).
 */
static inline int page_is_buddy(struct page *buddy,
							unsigned int order)
{
	if (PageBuddy(buddy) && page_order(buddy) == (int)order) {
		return 1;
	}
	return 0;
}

/*
 * Locate the struct page for both the matching buddy in our
 * pair (buddy1) and the combined O(n+1) page they form (page).
 *
 * 1) Any buddy B1 will have an order O twin B2 which satisfies
 * the following equation:
 *     B2 = B1 ^ (1 << O)
 * For example, if the starting buddy (buddy2) is #8 its order
 * 1 buddy is #10:
 *     B2 = 8 ^ (1 << 1) = 8 ^ 2 = 10
 *
 * 2) Any buddy B will have an order O+1 parent P which
 * satisfies the following equation:
 *     P = B & ~(1 << O)
 *
 * Assumption: *_mem_map is contiguous at least up to MAX_ORDER
 */
static inline unsigned long
__find_buddy_index(unsigned long page_idx, unsigned int order)
{
	return page_idx ^ (1 << order);
}


#endif
