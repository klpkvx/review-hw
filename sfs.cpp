# include <math.h>
# include <stdio.h>
# include "sfs.h"
double f0_ (double, double)
{
  return 1.;
}

double f1_ (double x, double)
{
  return x;
}

double f2_ (double, double y)
{
  return y;
}

double f3_ (double x, double y)
{
  return x + y;
}

double f4_ (double x, double y)
{
  return sqrt (x * x + y * y);
}

double f5_ (double x, double y)
{
  return x * x + y * y;
}

double f6_ (double x, double y)
{
  return exp (x * x - y * y);
}

double f7_ (double x, double y)
{
  return 1. / (25. * (x * x + y * y) + 1.);
}

void ij2l (int nx, int, int i, int j, int &l)
{
  l = i + j * (nx + 1);
}

void l2ij (int nx, int, int &i, int &j, int l)
{
  j = l / (nx + 1);
  i = l - j * (nx + 1);
}

int get_len_msr (int nx, int ny)
{
  return (nx - 1) * (ny - 1) * 6 + 4 * (2 * (nx - 1) + 2 * (ny - 1)) + 2 * 3 + 2 * 2;
}

int get_len_msr_off_diag (int nx, int ny)
{
  double hx = 0, hy = 0;
  int i, j, res = 0;
  for (i = 0; i < nx; i ++)
    for (j = 0; j < ny; j ++)
      res += get_off_diag (nx, ny, hx, hy, i, j);
  return res;
}

# define F(IS, JS, S) (IA_ij (nx, ny, hx, hy, i, j, (IS), (JS), (S), I, A))

int get_off_diag (int nx, int ny, double hx, double hy, int i, int j, int *I, double *A)
{
  int s = 0;
	if (i > 0 && j > 0)
    F (i - 1, j - 1, s ++);
	if (j > 0)
    F (i, j - 1, s ++);
	if (i > 0)
    F (i - 1, j, s ++);
	if (i < nx)
    F (i + 1, j, s ++);
	if (j < ny) 
    F (i, j + 1, s ++);
	if (i < nx && j < ny)
    F (i + 1, j + 1, s ++);
  return s;
}

int IA_ij (int nx, int ny, double hx, double hy, int i, int j, int is, int js, int s, int *I, double *A)
{
	int l, ls;
	ij2l (nx, ny, i, j, l);
	ij2l (nx, ny, is, js, ls);
	if (I)
    I[s] = ls;
	if (A) 
    {
		  if (l == ls)
			  A[s] = (((i < nx && j > 0) ? 1 : 0) + ((i > 0 && j > 0) ? 2 : 0) + ((i > 0 && j < ny) ? 1 : 0) + ((i < nx && j < ny) ? 2 : 0)) * hx * hy / 12;
		  else
			  A[s] = (((is == i + 1 && js == j) ? ((j < ny ? 1 : 0) + (j > 0 ? 1 : 0)) : 0) + ((is == i && js == j - 1) ? ((i < nx ? 1 : 0) + (i > 0 ? 1 : 0)) : 0) + 
							  ((is == i - 1 && js == j - 1) ? 2 : 0) + ((is == i - 1 && js == j) ? ((j > 0 ? 1 : 0) + (j < ny ? 1 : 0)) : 0) + 
							  ((is == i && js == j + 1) ? ((i > 0 ? 1 : 0) + (i < nx ? 1 : 0)) : 0) + ((is == i + 1 && js == j + 1) ? 2 : 0)) * hx * hy / 24;
		  return 0;
	  }
	return 1;
}

int get_diag (int nx, int ny, double hx, double hy, int i, int j, int *, double *A)
{
  int l;
  ij2l (nx, ny, i, j, l);
  return IA_ij (nx, ny, hx, hy, i, j, i, j, l, nullptr, A);
}

double P_f (double *x, double x0, double y0, int nx, int ny, double a, double c, double hx, double hy)
{
	int i = (x0 - a) / hx;
  int j = (y0 - c) / hy;
  int l;
  ij2l (nx, ny, i, j, l);
	if (y0 - hy * (x0 - (a + i * hx)) / hx - (c + j * hy) > 0) 
    return -x[l] * (y0 - c - (j + 1) * hy) / hy + x[l + 1 + nx + 1] * (x0 - a - i * hx) / hx + x[l + nx + 1] * (1 - (x0 - a - i * hx) / hx + (y0 - c - (j + 1) * hy) / hy);
  else
    return -x[l] * (x0 - a - (i + 1) * hx) / hx + x[l + 1] * (1 + (x0 - a - (i + 1) * hx) / hx - (y0 - c - j * hy) / hy) + x[l + 1 + nx + 1] * (y0 - c - j * hy) / hy;
}
