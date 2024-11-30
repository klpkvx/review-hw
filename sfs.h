#ifndef SFS_H
#define SFS_H

double f0_ (double, double);

double f1_ (double x, double);

double f2_ (double, double y);

double f3_ (double x, double y);

double f4_ (double x, double y);

double f5_ (double x, double y);

double f6_ (double x, double y);

double f7_ (double x, double y);

void ij2l (int nx, int, int i, int j, int &l);

void l2ij (int nx, int, int &i, int &j, int l);

int get_len_msr (int nx, int ny);

int get_len_msr_off_diag (int nx, int ny);

int get_off_diag (int nx, int ny, double hx, double hy, int i, int j, int *I = nullptr, double *A = nullptr);

int IA_ij (int nx, int ny, double hx, double hy, int i, int j, int is, int js, int s, int *I, double *A);

int get_diag (int nx, int ny, double hx, double hy, int i, int j, int *I, double *A);

double P_f (double *x, double x0, double y0, int nx, int ny, double a, double c, double hx, double hy);

#endif
