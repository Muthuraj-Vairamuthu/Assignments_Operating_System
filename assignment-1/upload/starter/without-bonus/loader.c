#include "loader.h"

Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;
int fd; 

/*
 * release memory and other cleanups
 */

//Implemented by Muthuraj
void loader_cleanup() {
  free(ehdr);
  free(phdr);
  close(fd); // Use close for file descriptor
}

/*
 * Load and run the ELF executable file
 */

void load_and_run_elf(char** exe) {
  //Implemented by Noel
  // 1. Load entire binary content into the memory from the ELF file.

  fd = open(exe[0], O_RDONLY);
  if (fd == -1){
    printf("Could not read file!\n");
    return;
  }

  // Allocate memory for the ELF header
  ehdr = (Elf32_Ehdr *)malloc(sizeof(Elf32_Ehdr));

  if (ehdr == NULL){
    printf("Could not read PHDR\n");
    return;
  }

  // Read the ELF header using read
  read(fd, ehdr, sizeof(Elf32_Ehdr));

  unsigned short no_of_program_headers = ehdr->e_phnum;
  lseek(fd, ehdr->e_phoff, SEEK_SET); 

  // Allocate memory for the program headers
  phdr = (Elf32_Phdr *)malloc(sizeof(Elf32_Phdr) * no_of_program_headers);

  // Read the program headeThe looprs using read
  read(fd, phdr, sizeof(Elf32_Phdr) * no_of_program_headers);

  if (phdr == NULL){
    printf("Could not read PHDR\n");
    return;
  }
  //Steps 2,3 implemented by Muthuraj:

  //2. Iterate through the PHDR table and find the section of PT_LOAD 
  //type that contains the address of the entrypoint method in fib.c

  typedef int (*fnc)(); //Typecasting
  Elf32_Phdr *virtual_mem = NULL;
  int i = 0;

  for (i = 0; i < no_of_program_headers; i++) {
    Elf32_Phdr elf_phdr = phdr[i];
    if (elf_phdr.p_type == PT_LOAD) {
      unsigned int p_start_address = elf_phdr.p_vaddr;
      unsigned int p_ending_address = elf_phdr.p_vaddr + elf_phdr.p_memsz;
      if (ehdr->e_entry < p_ending_address && p_start_address <= ehdr->e_entry ){
        // 3. Allocate memory of the size "p_memsz" using mmap function 
        //    and then copy the segment conten
        virtual_mem = mmap(NULL, elf_phdr.p_memsz, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
        lseek(fd, elf_phdr.p_offset, SEEK_SET);
        read(fd, virtual_mem, elf_phdr.p_memsz);
        break;
      }
    }
  }
  //Steps 4,5,6 implemented by Noel:

  // 4. Navigate to the entrypoint address into the segment loaded in the memory in the above step
  int entry_point = (int)virtual_mem + (ehdr->e_entry - phdr[i].p_vaddr);
  

  // 5. Typecast the address to that of a function pointer matching "_start" method in fib.c.
  fnc _start = (fnc)(int)entry_point;

  // 6. Call the "_start" method and print the value returned from the "_start"
  int result = _start();
  printf("User _start return value = %d\n", result);
}

int main(int argc, char** argv) 
{
  if(argc != 2) {
    printf("Usage: %s <ELF Executable> \n",argv[0]);
    exit(1);
  }
  // 1. carry out necessary checks on the input ELF file
  // 2. passing it to the loader for carrying out the loading/execution
  load_and_run_elf(&argv[1]);
  // 3. invoke the cleanup routine inside the loader  
  loader_cleanup();
  return 0;
}
