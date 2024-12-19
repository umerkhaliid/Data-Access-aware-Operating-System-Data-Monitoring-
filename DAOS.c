#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>

#define NUM_PAGES 100      // Number of pages to monitor
#define INITIAL_REGION_SIZE 10 // Initial pages per region
#define INTERVAL 1         // Sampling interval in seconds
#define ACCESS_THRESHOLD 5 // Threshold to split or merge regions

typedef struct {
    int page_id;       // Page identifier
    bool accessed;     // Accessed flag (simulated)
} Page;

typedef struct {
    int region_id;         // Region identifier
    int start;             // Start page index
    int end;               // End page index (exclusive)
    int access_count;      // Number of pages accessed in the region
    int size;              // Number of pages in the region
} Region;

// Simulate memory pages
Page memory[NUM_PAGES];

// Dynamic region list
Region* regions;
int num_regions;

// Simulate page access
void simulate_access() {
    for (int i = 0; i < NUM_PAGES; i++) {
        // Randomly mark pages as accessed
        memory[i].accessed = rand() % 2;
    }
}

// Monitor a single region and update its access count
void monitor_region(Region* region) {
    int count = 0;
    for (int i = region->start; i < region->end; i++) {
        if (memory[i].accessed) {
            count++;
        }
    }
    region->access_count = count;
}

// Monitor all regions
void monitor_memory() {
    for (int i = 0; i < num_regions; i++) {
        monitor_region(&regions[i]);
    }
}

// Adjust regions dynamically based on access patterns
void adjust_regions() {
    for (int i = 0; i < num_regions; i++) {
        Region* region = &regions[i];
        
        // If the region is highly accessed, split it
        if (region->access_count > ACCESS_THRESHOLD && region->size > 1) {
            int mid = (region->start + region->end) / 2;

            // Create a new region by splitting the current one
            Region new_region = {
                .region_id = num_regions,
                .start = mid,
                .end = region->end,
                .access_count = 0,
                .size = region->end - mid
            };

            // Update the current region
            region->end = mid;
            region->size = mid - region->start;

            // Add the new region to the list
            regions = realloc(regions, sizeof(Region) * (num_regions + 1));
            regions[num_regions] = new_region;
            num_regions++;

            printf("Region %d split into Region %d and Region %d\n", 
                   region->region_id, region->region_id, new_region.region_id);
        }

        // If the region is underutilized, merge with the next region
        if (i < num_regions - 1 && region->access_count <= ACCESS_THRESHOLD) {
            Region* next_region = &regions[i + 1];

            // Merge the two regions
            region->end = next_region->end;
            region->size += next_region->size;

            // Shift remaining regions to fill the gap
            for (int j = i + 1; j < num_regions - 1; j++) {
                regions[j] = regions[j + 1];
            }

            num_regions--;
            regions = realloc(regions, sizeof(Region) * num_regions);

            printf("Region %d and Region %d merged into Region %d\n",
                   region->region_id, next_region->region_id, region->region_id);
        }
    }
}

// Display monitoring results
void display_results() {
    printf("Region Monitoring Results:\n");
    for (int i = 0; i < num_regions; i++) {
        printf("Region %d: Start=%d, End=%d, Size=%d, Accessed Pages=%d/%d\n", 
               regions[i].region_id, 
               regions[i].start, 
               regions[i].end, 
               regions[i].size, 
               regions[i].access_count, 
               regions[i].size);
    }
    printf("\n");
}

int main() {
    srand(time(NULL)); // Seed the random number generator

    // Initialize memory pages
    for (int i = 0; i < NUM_PAGES; i++) {
        memory[i].page_id = i;
        memory[i].accessed = false;
    }

    // Initialize regions
    num_regions = NUM_PAGES / INITIAL_REGION_SIZE;
    regions = malloc(sizeof(Region) * num_regions);
    for (int i = 0; i < num_regions; i++) {
        regions[i].region_id = i;
        regions[i].start = i * INITIAL_REGION_SIZE;
        regions[i].end = regions[i].start + INITIAL_REGION_SIZE;
        regions[i].access_count = 0;
        regions[i].size = INITIAL_REGION_SIZE;
    }

    // Monitoring loop
    for (int cycle = 0; cycle < 10; cycle++) {  // Monitor for 10 cycles
        printf("Monitoring Cycle %d:\n", cycle + 1);

        simulate_access(); // Simulate random page access
        monitor_memory();  // Monitor regions
        adjust_regions();  // Adjust regions dynamically
        display_results(); // Display the results

        sleep(INTERVAL);   // Wait for the next sampling interval
    }

    // Free allocated memory
    free(regions);

    return 0;
}