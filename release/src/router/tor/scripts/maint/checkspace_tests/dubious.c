
// The { coming up should be on its own line.
int
foo(void) {
  // There should be a space before (1)
  if(1) x += 1;

  // The following empty line is unnecessary.

}


// There should be a newline between void and bar.
void bar(void)
{
  // too wide:
  testing("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
}

long
bad_spacing()
{
  // here comes a tab
	return 2;
  // here comes a label without space:
foo:
  ;
}

// Here comes a CR:

// Trailing space:         

int
non_k_and_r(void)
{
  // non-k&r
  if (foo)
    {
      // double-semi
      return 1;;
    }
  else
    {
      return 2;
    }
}

// #else #if causes a warning.
#if 1
#else
#if 2
#else
#endif
#endif

// always space before a brace.
foo{
}

void
unexpected_space(void)
{
  // This space gives a warning.
  foobar (77);
}

void
bad_function_calls(long)
{
  // These are forbidden:
  assert(1);
  memcmp("a","b",1);
  strcat(foo,x);
  strcpy(foo,y);
  sprintf(foo,"x");
  malloc(7);
  free(p);
  realloc(p);
  strdup(s);
  strndup(s,10);
  calloc(a,b);
}
