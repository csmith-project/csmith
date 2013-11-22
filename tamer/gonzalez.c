#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>

#define MAXVECS 3000

static const char* delim = " \n";
static double **vecs;
static int veclen;
static int numvecs = -1;
static int *in_subset;
static int *in_all_subset;
static int subset_size;
static int all_subset_size;

static void reset_subset (void)
{
  int i;
  for (i=0; i<numvecs; i++) {
    in_subset[i] = 0;
    in_all_subset[i] = 0;
  }
  subset_size = 0;
  all_subset_size = 0;
}

static void load_vectors (char *fn)
{
  FILE *inf = fopen (fn, "r");
  if (!inf) {
    printf ("oops, could not open '%s'\n", fn);
    exit (-1);
  }

  int res;
  while (1) {
    char *line = NULL;
    size_t n;
    res = getline (&line, &n, inf);
    if (res == -1) {
      free (line);
      break;
    }
    if (numvecs == -1) {
      char *s = strtok (line, delim);
      while (1) {
	if (s) {
	  veclen++;
	} else {
	  break;
	}
	s = strtok (NULL, delim);
      }
      printf ("vector length = %d\n", veclen);
      vecs = (double **) malloc (MAXVECS * sizeof(double *));
      assert (vecs);
      int i;
      for (i=0; i<MAXVECS; i++) {
	vecs[i] = NULL;
      }
    } else {
      char *s = strtok (line, delim);
      int vecpos = 0;
      vecs[numvecs] = (double *) malloc (veclen * sizeof (double));
      assert (vecs[numvecs]);
      while (1) {
	if (s) {
	  char *newptr;
	  double d = strtod (s, &newptr);
	  assert (newptr > s);
	  assert (vecpos < veclen);
	  assert (vecs);
	  vecs[numvecs][vecpos] = d;
	  vecpos++;
	} else {
	  break;
	}
	s = strtok (NULL, delim);
      }
      assert (vecpos == veclen);
    }
    numvecs++;
    assert (numvecs < MAXVECS);
    free (line);
  } 
  printf ("numvecs = %d\n", numvecs);

  in_subset = (int *) malloc (numvecs * sizeof (int));
  assert (in_subset);
  in_all_subset = (int *) malloc (numvecs * sizeof (int));
  assert (in_all_subset);
  reset_subset();

  fclose (inf);
}

static void load_subsets (char *fn, char *fn2)
{
  reset_subset();
  
  FILE *inf = fopen (fn, "r");
  if (!inf) {
    printf ("oops, could not open '%s'\n", fn);
    exit (-1);
  }
  int res;
  while (1) {
    char *line = NULL;
    size_t n;
    res = getline (&line, &n, inf);
    if (res == -1) {
      free (line);
      break;
    }
    int which;
    res = sscanf (line, "%d ", &which);
    assert (res==1);
    in_subset[which] = 1;
    subset_size++;
    free (line);
  }
  fclose (inf);

  inf = fopen (fn2, "r");
  if (!inf) {
    printf ("oops, could not open '%s'\n", fn);
    exit (-1);
  }
  while (1) {
    char *line = NULL;
    size_t n;
    res = getline (&line, &n, inf);
    if (res == -1) {
      free (line);
      break;
    }
    int which;
    res = sscanf (line, "%d ", &which);
    assert (res==1);
    in_all_subset[which] = 1;
    all_subset_size++;
    free (line);
  }
  fclose (inf);
}

// euclidean 
static double distance (double *a, double *b)
{
  double d = 0;
  int i;
  for (i=0; i<veclen; i++) {
    double x = a[i] - b[i];
    d += x*x;
  }
  return sqrt (d);
}

struct dist_t {
  int a,b;
  double d;
};

int dcomp (const void *a, const void *b)
{
  const struct dist_t *ap = (const struct dist_t *)a;
  const struct dist_t *bp = (const struct dist_t *)b;
  return ap->d < bp->d;
}

static double **distances;
struct dist_t *dlist;

static void compute_distances (void)
{
  distances = (double **) malloc (numvecs * sizeof (double *));
  assert (distances);

  dlist = (struct dist_t *) malloc (numvecs * numvecs * sizeof (struct dist_t));
  assert (dlist);

  int i;
  for (i=0; i<numvecs; i++) {
    distances[i] = (double *) malloc (numvecs * sizeof (double));
    assert (distances[i]);
  }

  int dpos = 0;

  for (i=0; i<numvecs; i++) {
    int j;
    for (j=0; j<numvecs; j++) {
      if (j<i) {
	double d = distance (vecs[i],vecs[j]);
	
	dlist[dpos].a = i;
	dlist[dpos].b = j;
	dlist[dpos].d = d;
	dpos++;

	dlist[dpos].a = j;
	dlist[dpos].b = i;
	dlist[dpos].d = d;
	dpos++;

	distances[i][j] = d;
	distances[j][i] = d;
      } else if (j>i) {
      } else {
	dlist[dpos].a = j;
	dlist[dpos].b = i;
	dlist[dpos].d = 0.0f;
	dpos++;
	distances[i][i] = 0.0f;
      }
    }
  }

  qsort (dlist, numvecs*numvecs, sizeof (struct dist_t), dcomp);
}

static double mind (double a, double b)
{
  if (a<b) return a;
  return b;
}

/*
 * if baseline is true then we just leave out the subset totally
 */
static void cluster (char *outfn, int baseline)
{
  // ranking[n] is the index of the nth testcase we want a user to
  // look at, or else -1
  int *ranking = (int *) malloc (numvecs * sizeof (int));
  assert (ranking);

  // ranked[0] is true iff point 0 has been placed in the list
  int *ranked = (int *) malloc (numvecs * sizeof (int));
  assert (ranked);

  // number of entries in ranking[] that are filled in now
  int cur_ranking = 0;

  int x;
  for (x=0; x<numvecs; x++) {
    ranking[x] = -1;
    ranked[x] = 0;
  }

  printf ("output file: '%s'\n", outfn);
  FILE *outf = fopen (outfn, "w+");
  assert (outf);

  if (baseline == 0) {
    // give a place to each member (if any) of the already-found subset
    for (x=0; x<numvecs; x++) {
      if (in_subset[x]) {
	ranked[x] = 1;
	ranking[cur_ranking] = x;
	cur_ranking++;
      }
    }
  }

  // need to start with at least one ranked point, so start
  // with the one farthest from everyone else
  if (cur_ranking==0) {

    int dpos;
    int i = -1;
    for (dpos=0; dpos<numvecs*numvecs; dpos++) {
      i = dlist[dpos].a;
      int j = dlist[dpos].b;
      if (baseline==2) {
	if (!in_all_subset[i] &&
	    !in_all_subset[j]) break;
      }
      if (!in_subset[i] &&
	  !in_subset[j]) break;
    }
    assert (!in_subset[i]);
    ranking[cur_ranking] = i;
    ranked[i] = 1;
    cur_ranking++;
    fprintf (outf, "%d\n", i);
  }

  while (1) {

    // find the point that maximizes the minimum distance from
    // any ranked point
    int y;
    double max_dist = -HUGE_VALF;
    int i = -1;
    for (y=0; y<numvecs; y++) {
      if (ranked[y]) continue;
      if ((baseline==1) && in_subset[y]) continue;
      if ((baseline==2) && in_all_subset[y]) continue;
      int z;
      double min_dist = HUGE_VALF;
      for (z=0; z<cur_ranking; z++) {
        int new_z = ranking[z];
	if ((baseline==1) && in_subset[new_z]) continue;
	if ((baseline==2) && in_all_subset[new_z]) continue;
	min_dist = mind (min_dist, distances[y][new_z]);
      }
      
      // alternate, simpler implementation
      int zz;
      double min_dist2 = HUGE_VALF;
      for (zz=0; zz<numvecs; zz++) {
	if ((baseline==1) && in_subset[zz]) continue;
	if ((baseline==2) && in_all_subset[zz]) continue;
	if (ranked[zz])
	  min_dist2 = mind (min_dist2, distances[y][zz]);
      }
      assert ((min_dist == min_dist2) && "not the same min_dist!");

      if (min_dist > max_dist) {
	max_dist = min_dist;
	i = y;
      }
    }

    if (i == -1) {
      printf ("cur_ranking = %d, subset_size = %d, all_subset_size = %d, numvecs = %d\n", 
	      cur_ranking, subset_size, all_subset_size, numvecs);
      switch (baseline) {
      case 0:
	assert (cur_ranking == numvecs);
	break;
      case 1:
	assert ((subset_size + cur_ranking) == numvecs);
	break;
      case 2:
	// assert ((all_subset_size + cur_ranking) == numvecs);
	break;
      default:
	assert (0);
      }
      break;
    }

    ranking[cur_ranking] = i;
    ranked[i] = 1;
    cur_ranking++;
    fprintf (outf, "%d\n", i);
  }

  fclose (outf);
  free (ranking);
  free (ranked);
}

static void usage (void)
{
  printf ("usage: gonzalez fileroot [subsetfiles]\n");
  exit (-1);
}

int main (int argc, char *argv[])
{
  if (argc < 2) usage();
  char *root = argv[1];
  char vecfn[255];
  sprintf (vecfn, "vectors/%s.ds", root);
  printf ("vector file '%s'\n", vecfn);

  load_vectors (vecfn);
  compute_distances ();

  char outfn[255];
  sprintf (outfn, "results-gonzalez/%s.rank", root);
  cluster (outfn, 0);

  int index = 2;
  while (index < argc) {
    char subsetfn[255];
    sprintf (subsetfn, "vectors/%s", argv[index]);
    printf ("subset file '%s'\n", subsetfn);
    char subsetfn2[255];
    sprintf (subsetfn2, "vectors/%s.all", argv[index]);
    printf ("subset file '%s'\n", subsetfn2);
    load_subsets (subsetfn, subsetfn2);

    char outfn2[255];
    sprintf (outfn2, "results-gonzalez/%s.%s.rank", root, argv[index]);
    cluster (outfn2, 0);

    sprintf (outfn2, "results-gonzalez/%s.%s.baseline.rank", root, argv[index]);
    cluster (outfn2, 1);

    sprintf (outfn2, "results-gonzalez/%s.%s.best.rank", root, argv[index]);
    cluster (outfn2, 2);

    index++;
  }

  return 0;
}

#if 0
// cosine distance doesn't give good results here!

static double dot (double *v1, double *v2)
{
  double d = 0;
  int i;
  for (i=0; i<veclen; i++) {
    d += v1[i]*v2[i];
  }
  return d;
}

static const double EPSILON = 1e-10f;

static double distance (double *a, double *b)
{
  double dotaa = dot (a,a);
  double dotbb = dot (b,b);
  if ((fabs(dotaa)<EPSILON) && (fabs(dotbb)<EPSILON)) return 0.0;
  if ((fabs(dotaa)<EPSILON) || (fabs(dotbb)<EPSILON)) return 1.0;
  double dotab = dot (a,b);
  return dotab / (sqrt(dotaa)*sqrt(dotbb));
}
#endif

