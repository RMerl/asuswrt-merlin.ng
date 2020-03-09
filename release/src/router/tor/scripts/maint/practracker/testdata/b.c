
MOCK_IMPL(int,
foo,(void))
{
  // blah1
  return 0;
}

MOCK_IMPL(int,
bar,( long z))
{
  // blah2

  return (int)(z+2);
}
