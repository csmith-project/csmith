#include <assert.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>

static inline int INDEX(int x, int y, int z, int s) {
  int i = (x * (z+1)) + y;
  // assert (i>=0);
  // assert (i<s);
  return i;
}

int lev (const char *word1, int len1, const char *word2, int len2)
{
  size_t s = (len1+1)*(len2+1);
  int *matrix = (int *)malloc (s*sizeof(int));
  assert (matrix);
  int i;
  for (i = 0; i <= len1; i++) {
    matrix[INDEX(i,0,len2,s)] = i;
  }
  for (i = 0; i <= len2; i++) {
    matrix[INDEX(0,i,len2,s)] = i;
  }
  for (i = 1; i <= len1; i++) {
    char c1 = word1[i-1];
    int j;
    for (j = 1; j <= len2; j++) {
      char c2 = word2[j-1];
      if (c1 == c2) {
	matrix[INDEX(i,j,len2,s)] = matrix[INDEX(i-1,j-1,len2,s)];
      } else {
	int delete = matrix[INDEX(i-1,j,len2,s)] + 1;
	int insert = matrix[INDEX(i,j-1,len2,s)] + 1;
	int substitute = matrix[INDEX(i-1,j-1,len2,s)] + 1;
	int minimum = delete;
	if (insert < minimum) {
	  minimum = insert;
	}
	if (substitute < minimum) {
	  minimum = substitute;
	}
	matrix[INDEX(i,j,len2,s)] = minimum;
      }
    }
  }
  int res = matrix[INDEX(len1,len2,len2,s)];
  free (matrix);
  return res;
}

#if 0

struct fil_t {
  char *b;
  off_t size;
};

struct fil_t readfile (char *fn)
{
  struct stat buf;
  int res = stat (fn, &buf);  
  assert (res==0);
  off_t size = buf.st_size;
  char *b = (char *)malloc(size+1);
  assert (b);
  FILE *f = fopen (fn, "r");
  assert (f);
  res = fread (b, 1, size, f);
  // printf ("res=%d\n", res);
  assert (res==size);
  fclose (f);
  struct fil_t fil = { b, size };
  return fil;
}

int main (int argc, char *argv[])
{
  assert (argc==3);
  struct fil_t f1, f2;
  f1 = readfile (argv[1]);
  f2 = readfile (argv[2]);
  printf ("%d\n", lev (f1.b, f1.size, f2.b, f2.size));
  return 0;
}

#endif
