#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
 
int main(int argc, char** argv) { 
 
  int fp;
  long int file_size;
 
  if (argc < 2) {
    fprintf(stderr, "Correct usage: %s <filename>\n", argv[0]); 
    return 1; 
  }
 
  if ((fp = open(argv[1], O_RDONLY)) == -1) {
    fprintf(stderr, "Error opening the file \n"); 
    return 1;
  }
 
  file_size = filelength(fp); 
  printf("The file size in is %ld bytes.\n", file_size); 
  close(fp); 
 
  return 0; 
}
