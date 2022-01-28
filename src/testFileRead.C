#include <stdio.h>
#include <stdlib.h>

int testFileRead()
{
  FILE* testFile;
  testFile = fopen("test.txt", "r");
  

  fclose(testFile);
  
  return 0;
}

int main()
{
  testFileRead();
  return 0;
}
