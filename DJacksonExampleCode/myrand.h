int    seed;

int intrnd () /* 1<=seed<=m */
{
  double const a    = 16807;      /* ie 7**5 */
  double const m    = 2147483647; /* ie 2**31-1 */

  double temp = seed * a;
  seed = (int) (temp - m * floor ( temp / m ));
  return seed;
}


double rand_0to1()
{ return ( intrnd() / 2147483647.0 ); }


double random_number()
{
  double r;

  while ((r = rand_0to1() ) >= 1.0 ) ; /* ensure we don't return 1.0 */

  return ( r );

} /* end random_number()*/

#define Randint(UPPER) ((int)(random_number()*(UPPER)))