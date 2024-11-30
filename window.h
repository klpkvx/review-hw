# ifndef WINDOW_H
# define WINDOW_H

#include <stdio.h>
#include <cstdlib>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <math.h>
#include <cstring>
# include <QtWidgets/QtWidgets>
# include "sfs.h"



# define W 0.7

int init_reduce_sum (int p);
void free_reduce_sum ();




class global_results
{
public:
  int iter = 0;
  double t_total_alg = 0;
  double t_total_resid = 0;
  double r1 = -1;
  double r2 = -1;
  double r3 = -1;
  double r4 = -1;

public:
  global_results () = default;
  ~global_results () = default;
};

class global_args
{
public:
  int nx;
  int ny;
  int N;
  double a;
  double b;
  double c;
  double d;
  int len_msr;
  int func_num;
  double (*f_ptr) (double, double);
  double * A = nullptr;
  int * I = nullptr;
  double * B = nullptr;
  double * x = nullptr;
  double * r = nullptr;
  double * u = nullptr;
  double * v = nullptr;
  pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
  pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
  int request = 0;
  int calc_st = 0;
  int alg_st = 0;
  int ppppp = 0;

public:
  global_args () = default;
  int init_memory ()
    {
      free_memory ();
      N = (nx + 1) * (ny + 1);
      len_msr =  N + 1 + get_len_msr (nx, ny);
      A = new double[len_msr];
      I = new int[len_msr];
      B = new double[N];
      x = new double[N];
      r = new double[N];
      u = new double[N];
      v = new double[N];
      if (!A || !I || !B || !x || !r || !u || !v)
        {
          printf ("ERROR: not enough memory\n");
          free_memory ();
          free_reduce_sum ();
          return -1;
        }
      memset (A, 0, len_msr * sizeof (double));
      memset (I, 0, len_msr * sizeof (int));
      return 0;
    }

  void set_nx_ny (int nx_arg, int ny_arg)
    {
      nx = nx_arg;
      ny = ny_arg;
    }
  void set_abcd (double a_arg, double b_arg, double c_arg, double d_arg)
    {
      a = a_arg;
      b = b_arg;
      c = c_arg;
      d = d_arg;
    }
  void set_func_info (int func_num_arg, double (*f) (double, double))
    {
      func_num = func_num_arg;
      f_ptr = f;
    }
  ~global_args ()
    {
      free_memory ();
    }

private:
  void free_memory ()
    {
      if (A) delete [] A;
      if (I) delete [] I;
      if (B) delete [] B;
      if (x) delete [] x;
      if (r) delete [] r;
      if (u) delete [] u;
      if (v) delete [] v;
    }
};

enum requests
{
  WAIT = 0,
  CALCULATE = 1,
  EXIT = 2,
};

enum calc_status
{
  CALC_IN_PROCESS = 0,
  CALC_FINISHED = 1,
  SLEEP = 2,
};

class Args
{
public:
  global_results * RES = nullptr;
  global_args * glob = nullptr;
  char * argv0;
  int k = 0;
  int p = 0;

  double eps;
  int maxit;

  double hx;
  double hy;
  int N;
  int len_msr;

  double t_cpu = 0;
  double t_total = 0;

  int request = 0;
  
  pthread_t tid = -1;
};

double reduce_sum_det (int p, int k, double s);

// y = Ax
void mult_msr_matrix_vector (double * A, int * I, int n, double * x, double * y, int p, int k);

// x -= t * y
void mult_sub_vector (int n, double * x, double * y, double t, int p, int k);

double scalar_product (int n, double * x, double * y, int p, int k);

// r = Mv
void apply_preconditioner_msr_matrix (int n, double * A, int * I, double * v, double * y, double * r, int p, int k);

int fill_IA (int nx, int ny, double hx, double hy, int * I, double * A, int p, int k);

int fill_I (int nx, int ny, int * I);

# define FUNC(F, A, C, I, J, HX, HY) (F ((A) + (I) * (HX), (C) + (J) * (HY)))

double F_ij (int nx, int ny, double hx, double hy, double a, double c, double (*f) (double, double), int l);
void fill_B (int n, int nx, int ny, double hx, double hy, double *b, double x0, double y0, int p, int k, double (*f)(double, double));

# define MAX_STEPS 10

int min_error_msr_matrix (double * A, int * I, double * B, double * x, double * r, double * u, double * v,
             double eps, int maxit, int N, int p, int k);

int algorithm_ (double * A, int * I, double * B, double * x, double * r, double * u, double * v,
                double eps, int max_iter, int N, int p, int k);

void residual_1 (int n, int nx, int ny, double hx, double hy, double a, double c, int p, int k, double * x, double (* f_) (double, double));

void residual_2 (int n, int nx, int ny, double hx, double hy, double a, double c, int p, int k, double * x, double (* f_) (double, double));

void residual_3 (int n, int nx, int ny, double hx, double hy, double a, double c, int p, int k, double * x, double (* f_) (double, double));

void residual_4 (int n, int nx, int ny, double hx, double hy, double a, double c, int p, int k, double * x, double (* f_) (double, double));

double get_full_time ();

double get_cpu_time ();

void * thread_loop (void * ptr);

void * thread_func (void * ptr);


template <class T = int> 
void reduce_sum (int p, T * a = nullptr, int n = 0)
{
  static pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
  static pthread_cond_t c_in = PTHREAD_COND_INITIALIZER;
  static pthread_cond_t c_out = PTHREAD_COND_INITIALIZER;
  static int t_in = 0;
  static int t_out = 0;
  static T * r = nullptr;
  int i;
  if (p <= 1)
    return;
  pthread_mutex_lock (&m);
  if (r == nullptr)
    r = a;
  else
    {
      for (i = 0; i < n; i ++)
        r[i] += a[i];
    }
  t_in ++;
  if (t_in >= p)
    {
      t_out = 0;
      pthread_cond_broadcast (&c_in);
    }
  else
    {
      while (t_in < p)
        pthread_cond_wait (&c_in, &m);
    }
  if (r != a)
    {
      for (i = 0; i < n; i ++)
        a[i] = r[i];
    }
  t_out ++;
  if (t_out >= p)
    {
      t_in = 0;
      r = nullptr;
      pthread_cond_broadcast (&c_out);
    }
  else
    {
      while (t_out < p)
        pthread_cond_wait (&c_out, &m);
    }
  pthread_mutex_unlock (&m);
}




class Args;

# define AA (-1)
# define BB 1
# define C (-2)
# define D 2
# define N0 10
# define M0 3
# define F0 0
# define EPS 1e-15
# define MY_EP 1e-6
# define VIS_NEPS 1e6

#ifndef DONO
#define DONO 
class Window;
int do_no (Window *ptr);
int do_del (Window *ptr);
#endif
class Window : public QWidget
{
  Q_OBJECT

private:
  double a = AA;
  double b = BB;
  double c = C;
  double d = D;
  int nx = N0;
  int ny = N0;
  int mx = 10;
  int my = 10;
  int N = N0;
  int n_width = N0;
  int n_height = N0;
  int scale = 0;
  int perturbation = 0;
  int func_num = F0;
  const char * func_text = "f(x) = 1";
  int graph_num = 0;
  const char * graph_text = "Function";
  const char * status_bar = "Calculating...";
  int rdy = 0;
  int only_status = 0;
  double * x_coef = nullptr;
  double eps;
  int maxit;
  int p = 2;
  double F_MIN = 0;
  double F_MAX = 0;
  int prst = 1;
  double F_MAX_MODULO = 0;
  double (* f_) (double, double);
  
  int (*funex) (Window*) = do_no;
  bool if_n_changed = true;
  global_results * RES = nullptr;
  global_args * glob = nullptr;
  QTimer * timer = nullptr;

public:
  Args* arrr = nullptr;
  int ex2f ()
  {

    return do_del (this);
    // ffff(this);
  }
  Window (QWidget * parent) : QWidget (parent)
    {
      a = AA;
      b = BB;
      c = C;
      d = D;
      nx = N0;
      ny = N0;
      mx = M0;
      my = M0;
      func_num = F0;
    }
  ~Window ()
    {
      if (glob->request != requests::WAIT)
      {pthread_mutex_lock (&glob->m);
            glob->ppppp = 1;
            pthread_cond_broadcast (&glob->cond);
            pthread_mutex_unlock (&glob->m);
      
            for(int k = 1; k < p; k++)
              pthread_join(arrr[k].tid, 0);}

      free_memory ();
    }
private:
  void free_memory ();

public:

  int parse_command_line (int argc, char * argv[]);

  void allocate_memory ();
  void init_memory ();


  void fill_func_info ();
  void start_timer ();

  QPointF l2g (double x_loc, double y_loc);

  void f_to_rgb (double f_val, double &r, double &g, double &b);
  void draw_func (QPainter * painter);
  void draw_approximation (QPainter * painter);
  void draw_residual (QPainter * painter);

  void set_minmax_f ();
  void set_minmax_approximation ();
  void set_minmax_residual ();

  QSize minimumSizeHint () const;
  QSize sizeHint () const;

  int get_p () { return p; }
  void set_args (Args *&args);
  void set_args_memory (Args *&args);

  void update_all ();
  void copy_to_gui ();

  void view_status (QPainter * painter);




protected:
  void paintEvent (QPaintEvent * event);
  void closeEvent (QCloseEvent *event);

public slots:
  void change_function ();
  void change_graphs ();
  void increase_scale ();
  void decrease_scale ();
  void increase_n ();
  void decrease_n ();
  void increase_m ();
  void decrease_m ();
  void increase_p ();
  void decrease_p ();
  void vrema ();
  int exit_func ();
};


#endif
