#include <stdio.h>
#include <fcntl.h>
#include <string.h>

int
openfile(char *name)
{
  int fd;
  struct flock locker;
  memset(&locker, 0, sizeof(locker));
  locker.l_start = 0;         // offset from whence       
  locker.l_len   = 0;         // whole file               
  locker.l_pid   = getpid();  // don't actually care      
  locker.l_type  = F_WRLCK;
  locker.l_whence= SEEK_SET;  // start of file.           
  
  fd = open(name, O_WRONLY|O_CREAT, 0600);
  
  if (fd >= 0) {
  
    if (flock(fd, LOCK_EX | LOCK_NB) == -1) {
      close(fd);
      return (-1);
    }
    printf("LOCK_EX ok\n");
    if (fcntl(fd, F_SETLK, &locker) == -1) {
      close(fd);
      return (-1);
    }
    printf("Was allowed lock\n");
  }
  return (fd);
}


int 
main(int argc, char **argv)
{
  int file1;

  file1 = openfile("writetest.bin");
  if (file1 >= 0) {
    char buf[1024];
    write(file1, buf, 1024);

    int file2;
    file2 = openfile("writetest.bin");
    if (file2 == -1)
      printf("file2 failed as expected\n");
    else {
      printf("file2 did not fail\n");
      close(file2);
    }
    close(file1);
  }
}
