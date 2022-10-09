#include <stdio.h>
#include <stdlib.h>
#include<stdbool.h>  
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <errno.h>
#include <semaphore.h> 
#include <time.h>

int main(int argc,char *argv[])  {
  const int SIZE = 4096;		// file size
  char name[100];	// file name
	char *a = argv[2];
  // Tiempo activo
	int seconds = atoi(a);
  // Nombre para la memoria compartida
  sprintf(name, "/%s_MEM",argv[1]);
  
  char nameEmpty[100];
  char nameFull[100];
  // Nombre para los semaforos
  sprintf(nameEmpty, "/%s_EMPTY",argv[1]);
  sprintf(nameFull, "/%s_FULL",argv[1]);

  int shm_fd;		  // file descriptor, used with shm_open()
  int nc;
  char *shm_base;	// base address, used with mmap()

	int n;
	bool done = false;
  sem_t * empty_sem, *full_sem;
	printf("%ld \n", clock());
  /* create the shared memory segment as if it was a file */
  shm_fd = shm_open(name, O_RDWR, 0666);
  if (shm_fd == -1) {
    printf("prod: Shared memory failed: %s\n", strerror(errno));
    exit(1);
  }

  /* configure the size of the shared memory segment */
  ftruncate(shm_fd, SIZE);

  /* map the shared memory segment to the address space of the process */
  shm_base = mmap(0, SIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0);
  if (shm_base == MAP_FAILED) {
    printf("prod: Map failed: %s\n", strerror(errno));
    close(shm_fd);
    shm_unlink(name);   // cleanup: close and shm_unlink shared memory
    exit(1);
  }
	/**
	 * Create two named semaphores
	 */
	// first remove the semaphore if it already exists
	if (sem_unlink(nameEmpty) == -1)
			printf("Error removing %s: %s\n", nameEmpty, strerror(errno));
	if (sem_unlink(nameFull) == -1)
			printf("Error removing %s: %s\n", nameEmpty, strerror(errno));

	// create and initialize the semaphore
	if ( (empty_sem = sem_open(nameEmpty, O_CREAT, 0666, 1)) == SEM_FAILED)
			printf("Error creating %s: %s\n", nameEmpty, strerror(errno));
	if ( (full_sem = sem_open(nameFull, O_CREAT, 0666, 0)) == SEM_FAILED)
			printf("Error creating %s: %s\n", nameFull, strerror(errno));

	n=1;
	while (!done) {
		// posicionarse en primera posición de la zona compartida.
		// espera por memoria compartida haya sido leída;
		clock_t difference = clock() - before;
		if (difference> (seconds*1000))
		{
			done = true;
			break;
		}
		
		if (sem_wait(empty_sem)!=0) 
				printf("Error waiting %s\n",strerror(errno));
		else {
			int i;
			for (i=0; i< n; i++){
				// escriba a fifo -1;
				// escriba a fifo +1;
				nc = read(STDIN_FILENO, shm_base, SIZE-1);
				shm_base[nc] ='\0';  /* to make it NULL termited string */
				strncpy(shm_base, "-1", SIZE);
			}
			// Avisa que la memoria compartida tiene datos nuevos;
			if (sem_post(full_sem)!=0)
						printf("Error posting %s\n",strerror(errno));
			n++;
		}
	}

	sem_close(empty_sem);
	sem_close(full_sem);
  /* remove the mapped memory segment from the address space of the process */
  if (munmap(shm_base, SIZE) == -1) {
    printf("prod: Unmap failed: %s\n", strerror(errno));
    exit(1);
  }

  /* close the shared memory segment as if it was a file */
  if (close(shm_fd) == -1) {
    printf("prod: Close failed: %s\n", strerror(errno));
    exit(1);
  }
  return 0;
}
