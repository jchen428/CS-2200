#include <stdlib.h>
#include <stdio.h>
#include "tlb.h"
#include "pagetable.h"
#include "global.h" /* for tlb_size */
#include "statistics.h"

/*******************************************************************************
 * Looks up an address in the TLB. If no entry is found, attempts to access the
 * current page table via cpu_pagetable_lookup().
 *
 * @param vpn The virtual page number to lookup.
 * @param write If the access is a write, this is 1. Otherwise, it is 0.
 * @return The physical frame number of the page we are accessing.
 */
pfn_t tlb_lookup(vpn_t vpn, int write) {
    pfn_t pfn;

    /* 
     * FIX ME : Step 6
     */

    /* 
     * Search the TLB for the given VPN. Make sure to increment count_tlbhits if
     * it was a hit!
     */
    tlbe_t *entry = NULL;
    for (int i = 0; i < tlb_size; i++) {
        if (tlb[i].vpn == vpn && tlb[i].valid) {
            entry = &tlb[i];
            pfn = entry->pfn;
            //tlb[i].dirty = write;
            //tlb[i].used = 1;
            count_tlbhits++;
            //return tlb[i].pfn;
            break;
        }
    }

    /* If it does not exist (it was not a hit), call the page table reader */
    if (!entry) {
        pfn = pagetable_lookup(vpn, write);

        /* 
         * Replace an entry in the TLB if we missed. Pick invalid entries first,
         * then do a clock-sweep to find a victim.
         */
        for (int i = 0; i < tlb_size; i++) {
            if (!tlb[i].valid) {
                entry = &tlb[i];
                break;
            }
        }

        unsigned char swept = 0;
        for (int i = 0; i < tlb_size; i++) {
            if (tlb[i].used) {
                tlb[i].used = 0;
            } else {
                entry = &tlb[i];
                break;
            }

            if (!swept && i == tlb_size - 1) {
                swept = 1;
                i = 0;
            }
        }
    }

    //if (!entry)
        //entry = tlb[0];

    /*
     * Perform TLB house keeping. This means marking the found TLB entry as
     * accessed and if we had a write, dirty. We also need to update the page
     * table in memory with the same data.
     *
     * We'll assume that this write is scheduled and the CPU doesn't actually
     * have to wait for it to finish (there wouldn't be much point to a TLB if
     * we didn't!).
     */
    entry->vpn = vpn;
    entry->pfn = pfn;
    entry->valid = 1;
    entry->dirty = write;
    entry->used = 1;

    current_pagetable[vpn].pfn = pfn;
    current_pagetable[vpn].valid = 1;
    current_pagetable[vpn].dirty = write;
    current_pagetable[vpn].used = 1;

    return pfn;
}

