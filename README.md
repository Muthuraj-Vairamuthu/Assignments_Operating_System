*Documentation of the code:*


*Contribution By members:*

Both the Members have equal contribution towards the functioning of the code. Every line of code was discussed equally by both the members before typing it into the c file, we have also attached our resources for certain snippets along with the code.
Team Members: Muthuraj & Noel



*Working of the code:*
1. The core of the code is composed in the segment_handler function:
The segment_handler function starts when a page fault occurs.
2. It gets the memory location/address corresponding to the page fault.
3. Then iterates through the program headers to find the correct segment which is causing the problem of page fault.
4. Once it identifies that it maps memory using the mmap function to map a new memory page at the faulting address,and then copies the content from to the new memory.
5. After all this we use various metrics for the calculation such as follows:

sum.c
- Result: 2048
- Page faults: 3
- Page allocations: 3
- Internal Fragmentation: 8078 B
- Internal Fragmentation: 7.888672 KB

fib.c
- Result: 102334155
- Page faults: 1
- Page allocations: 1
- Internal Fragmentation: 3982 B
- Internal Fragmentation: 3.888672 KB
