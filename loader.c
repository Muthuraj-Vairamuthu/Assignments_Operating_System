#define _GNU_SOURCE
#include "loader.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>



Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;

typedef int (*fnc)();
int fd; 
int faults = 0;
int page_allocations = 0;
int fragmentation = 0;
int *sizes = NULL;
int *sizes_allocated = NULL;
unsigned short no_of_program_headers;


void loader_cleanup() {

  //Freeing the ehdr table
  free(ehdr);
  ehdr = NULL;
  
  //Munmapping the segments
  for (int i = 0 ; i < no_of_program_headers ; i++){
    if (sizes_allocated[i] != 0 && ((void *)phdr[i].p_vaddr,sizes_allocated[i]) == -1){
      printf("Munmap failed\n");
    }
  }

  free(sizes);
  sizes = NULL;
  
  free(sizes_allocated);
  sizes_allocated = NULL;
  
  close(fd);
  exit(0);
}

static void segment_handler(int signum, siginfo_t* info) {
    
    //p is the faulting address
    void *p = info->si_addr;
    lseek(fd, ehdr->e_phoff, SEEK_SET);
    read(fd, phdr, sizeof(Elf32_Phdr) * no_of_program_headers);
    faults++;
    
    // Find the segment for the entry point
    for (int i = 0; i < ehdr->e_phnum; i++) {
      
        unsigned int p_start_address = phdr[i].p_vaddr;
        unsigned int p_ending_address = phdr[i].p_vaddr + phdr[i].p_memsz;
        // Check if the faulting address is within the range of this segment
        if (p >= (void *)p_start_address && p < (void *)p_ending_address) {
              // printf("Address start is %p\n", p_start_address);
              // printf("Address point is %p\n", p_ending_address);
              // printf("Segment size %d\n", sizes[i]);
              
              if(sizes[i] <= 4096){
                
                //If the size left is less than 4096
                sizes_allocated[i] = sizes_allocated[i] + 4096;
                sizes[i] -= 4096;
                fragmentation +=  4096 - phdr[i].p_memsz;
                void *mem = mmap(p, 4096, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
                if (mem == MAP_FAILED){
                  printf("Could not map memory\n");
                }
                lseek(fd, phdr[i].p_offset, SEEK_SET);
                read(fd, mem, phdr[i].p_memsz);
                page_allocations++;

              }else{
                //If the segment size is greater than 4096
                void *mem = mmap(p, 4096, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
                sizes_allocated[i] = sizes_allocated[i] + 4096;
                sizes[i] =  sizes[i] - 4096;
                if (mem == MAP_FAILED){
                  printf("Map could not be allocated\n");
                }
                lseek(fd, phdr[i].p_offset, SEEK_SET);
                read(fd, mem, phdr[i].p_memsz);
                page_allocations++;

              }

              return;
        }
      
    }
    // Copy the content of the segment from the ELF file into the newly mapped memory
    // Adjust the program counter to point to the entry point of the newly mapped segment
    
    
  // }
}



void load_and_run_elf(char** exe) {

  //Opening the EXE file
  fd = open(exe[0], O_RDONLY);
  if (fd == -1){
    printf("Could not read file!\n");
    exit(1);
  }

  //Initializing the signal handler struct with handler
  struct sigaction handler;
  memset(&handler,0,sizeof(struct sigaction));
  handler.sa_flags = SA_SIGINFO;
  handler.sa_sigaction = segment_handler;

  if (sigaction(SIGSEGV, &handler, NULL) == -1){
    printf("Error in sigaction!\n");
  }
 

  // Allocate memory for the ELF header
  ehdr = (Elf32_Ehdr *)malloc(sizeof(Elf32_Ehdr));

  read(fd, ehdr, sizeof(Elf32_Ehdr));
  
  if (ehdr == NULL){
    printf("Could not read PHDR\n");
    return;
  }

  if (ehdr->e_type != ET_EXEC){
    printf("Not a executable file\n");
    return;
  }
  no_of_program_headers = ehdr->e_phnum;

  sizes = (int *)malloc(no_of_program_headers * sizeof(int));
  sizes_allocated = (int *)malloc(no_of_program_headers * sizeof(int));

  lseek(fd, ehdr->e_phoff, SEEK_SET); 

  phdr = (Elf32_Phdr *)malloc(sizeof(Elf32_Phdr) * no_of_program_headers);
  // Elf32_Phdr *virtual_mem = NULL;

  int i;
  void *virtual_mem;

  read(fd, phdr, sizeof(Elf32_Phdr) * no_of_program_headers);


  for (i = 0; i < no_of_program_headers; i++) {
    Elf32_Phdr elf_phdr = phdr[i];
    if (elf_phdr.p_type == PT_LOAD) {
      sizes[i] = phdr[i].p_memsz;      
      sizes_allocated[i] = 0;
    }
  }


  fnc _start = (fnc)(int)ehdr->e_entry;
  int result = _start();

  printf("Result: %d\n",result);
  
  for (int i = 0 ; i < no_of_program_headers ; i++){
    if (sizes_allocated[i] != 0 && sizes[i] > 0){
      fragmentation +=  sizes_allocated[i] + 4096 - phdr[i].p_memsz;
    }
  }


  printf("Page faults: %d\n",faults);
  printf("Page allocations: %d\n",page_allocations);
  printf("Internal Fragmentation: %d B\n",fragmentation);
  
  float kb = fragmentation / 1024.0;
  printf("Internal Fragmentation: %f KB\n",kb);
  loader_cleanup();



  

}


int main(int argc, char** argv) 
{


  if(argc != 2) {

    printf("Usage: %s <ELF Executable> \n",argv[0]);
    exit(1);

  }
  // signal(SIGSEGV,my_handler);
  // 1. carry out necessary checks on the input ELF file
  // 2. passing it to the loader for carrying out the loading/execution
  load_and_run_elf(&argv[1]);
  // 3. invoke the cleanup routine inside the loader  
  loader_cleanup();
  return 0;
}

//siginfo_t info taken from https://pubs.opengroup.org/onlinepubs/007904875/functions/sigaction.html
