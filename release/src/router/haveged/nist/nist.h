/**
 * Supply function prototypes for the test suite.
 */
int PackTestF (int *ARRAY, int ArraySize, char *C);
int PackTestL (int *ARRAY, int ArraySize, char *C);

int ApproximateEntropy (int mmin, int mmax, int n, int *ARRAY);
int BlockFrequency (int ArraySize, int m, int *ARRAY);
int CumulativeSums (int n, int *ARRAY);
int DiscreteFourierTransform (int N, int *ARRAY);
int Frequency (int n, int *ARRAY);
int LempelZivCompression (int n, int *ARRAY, int *DATA, int *pt, int *PT);
int LinearComplexity (int M, int N, int *ARRAY, int PT);
int LongestRunOfOnes (int n, int *ARRAY);
int NonOverlappingTemplateMatchings (int m, int n, int *ARRAY);
int OverlappingTemplateMatchings (int m, int n, int *ARRAY);
int RandomExcursions (int n, int *ARRAY);
int RandomExcursionsVariant (int n, int *ARRAY);
int Rank (int n, int *ARRAY);
int Runs (int n, int *ARRAY);
int Serial (int m, int n, int *ARRAY, int PT);
int UNIVERSAL (int n, int *ARRAY);
int Universal (int n, int *ARRAY);

double psi2 (int m, int n, int *ARRAY, int PT);
/**
 * From dfft.c
 */
void __ogg_fdrffti(int n, double *wsave, int *ifac);
void __ogg_fdrfftf(int n,double *r,double *wsave,int *ifac);

char *GetBaseDir(void);

