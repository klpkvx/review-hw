# include "window.h"

#include <QPainter>
#include <stdio.h>

void Window::draw_residual (QPainter *painter)
{
  QPen pen (Qt::darkRed, 0, Qt::SolidLine);
  QBrush brush (Qt::darkRed, Qt::SolidPattern);
  double hx = (b - a) / mx, hy = (d - c) / my;
  double hnx = (b - a) / nx, hny = (d - c) / ny;
  double r, g, b;
  double f_l1, f_l2;
  int i, j;
  painter->setPen (pen);
  for (i = 0; i < mx; i ++)
    {
      for (j = 0; j < my; j ++)
        {
          f_l1 = f_ (a + i * hx + hx / 3, c + j * hy + 2 * hy / 3) - P_f (x_coef, a + i * hx + hx / 3, c + j * hy + 2 * hy / 3, nx, ny, a, c, hnx, hny);
          f_to_rgb (f_l1, r, g, b);
          brush.setColor (QColor (r, g, b));
          pen.setColor (QColor (r, g, b));
          painter->setPen (pen);
          painter->setBrush (brush);
          QPointF points[3] = {QPointF (l2g (a + i * hx, c + j * hy)),
                              QPointF (l2g (a + i * hx, c + (j + 1) * hy)),
                              QPointF (l2g (a + (i + 1) * hx, c + (j + 1) * hy))};
          painter->drawPolygon (points, 3);

          f_l2 = f_ (a + i * hx + 2 * hx / 3, c + j * hy + hy / 3) - P_f (x_coef, a + i * hx + 2 * hx / 3, c + j * hy + hy / 3, nx, ny, a, c, hnx, hny);
          f_to_rgb (f_l2, r, g, b);
          brush.setColor (QColor (r, g, b));
          pen.setColor (QColor (r, g, b));
          painter->setPen (pen);
          painter->setBrush (brush);
          points[1] = QPointF (l2g (a + (i + 1) * hx, c + j * hy));
          painter->drawPolygon (points, 3);
        }
    }
}


void Window::set_args (Args *&args)
{
  double hx = (b - a) / nx;
  double hy = (d - c) / ny;
  for (int k = 0; k < p; k ++)
    {
      args[k].RES = RES;
      args[k].glob = glob;
      args[k].k = k;
      args[k].p = p;
      args[k].eps = eps;
      args[k].maxit = maxit;
      args[k].hx = hx;
      args[k].hy = hy;
      args[k].N = N;
      args[k].len_msr = glob->len_msr;
    }
}


int Window::parse_command_line (int argc, char * argv[])
{
  if (!(argc == 13
        && (sscanf (argv[1], "%lf", &a) == 1) && (sscanf (argv[2], "%lf", &b) == 1) && (a < b)
        && (sscanf (argv[3], "%lf", &c) == 1) && (sscanf (argv[4], "%lf", &d) == 1) && (c < d)
        && (sscanf (argv[5], "%d", &nx) == 1) && (nx > 0) && (sscanf (argv[6], "%d", &ny) == 1) && (ny > 0)
        && (sscanf (argv[7], "%d", &mx) == 1) && (mx > 0) && (sscanf (argv[8], "%d", &my) == 1) && (my > 0)
        && (sscanf (argv[9], "%d", &func_num) == 1) && (func_num >= 0 && func_num <= 7)
        && (sscanf (argv[10], "%lf", &eps) == 1) && (sscanf (argv[11], "%d", &maxit) == 1) && (maxit >= 0)
        && (sscanf (argv[12], "%d", &p) == 1) && (p > 0)))
    {
      return -1; // Стоит делать печать ошибки внутри функции, а не извне
    }
  return 0;
}

void Window::allocate_memory ()
{
  RES = new global_results ();
  glob = new global_args ();
  delete [] x_coef;
  N = (nx + 1) * (ny + 1);
  x_coef = new double[N];
  timer = new QTimer (this);
  glob->set_nx_ny (nx, ny);
  glob->set_abcd (a, b, c, d);
  glob->set_func_info (func_num, f_);
  glob->init_memory ();
  init_memory ();
}

void Window::init_memory ()
{
  for (int i = 0; i < N; i ++)
    x_coef[i] = 0.;
  f_ = f0_;
}

void Window::free_memory ()
{
  if (x_coef) delete [] x_coef; // Нужно использовать unique_ptr
  if (RES) delete RES;
  if (glob) delete glob;
  if (timer) delete timer;
}

QPointF Window::l2g (double x_loc, double y_loc)
{
  double x_glob = (x_loc - a) / (b - a) * width ();
  double y_glob = (d - y_loc) / (d - c) * height ();
  return QPointF (x_glob, y_glob);
}

void Window::start_timer ()
{
	connect (timer, SIGNAL (timeout ()), this, SLOT (vrema ())); // vrema переименовать на check_timers ()
  timer->start (100); // 100 вынести в отдельную константу с суффиксом единиц измерения
}

void Window::copy_to_gui ()
{
  nx = glob->nx;
  ny = glob->ny;
  N = glob->N;
  a = glob->a;
  b = glob->b;
  c = glob->c;
  d = glob->d;
  f_ = glob->f_ptr;
  delete [] x_coef;
  x_coef = new double[N];
  for (int i = 0; i < N; i ++)
    x_coef[i] = glob->x[i];
}

void Window::fill_func_info ()
{
  switch (func_num)
    {
      case 0:
        func_text = "f = 1";
        glob->f_ptr = f0_;
        break;
      case 1:
        func_text = "f = x";
        glob->f_ptr = f1_;
        break;
      case 2:
        func_text = "f = y";
        glob->f_ptr = f2_;
        break;
      case 3:
        func_text = "f = x + y";
        glob->f_ptr = f3_;
        break;
      case 4:
        func_text = "f = sqrt (x^2 + y^2)";
        glob->f_ptr = f4_;
        break;
      case 5:
        func_text = "f = x^2 + y^2";
        glob->f_ptr = f5_;
        break;
      case 6:
        func_text = "f = exp (x^2 - y^2)";
        glob->f_ptr = f6_;
        break;
      case 7:
        func_text = "f = 1 / (25 (x^2 + y^2) + 1)";
        glob->f_ptr = f7_;
        break;
    }
}

void Window::draw_approximation (QPainter *painter)
{
  QPen pen (Qt::darkRed, 0, Qt::SolidLine);
  QBrush brush (Qt::darkRed, Qt::SolidPattern);
  double hx = (b - a) / mx, hy = (d - c) / my;
  double hnx = (b - a) / nx, hny = (d - c) / ny;
  double r, g, b;
  double f_l1, f_l2;
  int i, j;
  painter->setPen (pen);
  for (i = 0; i < mx; i ++)
    {
      for (j = 0; j < my; j ++)
        {
          f_l1 = P_f (x_coef, a + i * hx + hx / 3, c + j * hy + 2 * hy / 3, nx, ny, a, c, hnx, hny); // выделить a + ... c + ... в отдельные переменные. Не очень понятно что такое hnx, hny.
          f_l1 += (i == mx / 2 && j == my / 2) ? (perturbation * 0.1 * F_MAX_MODULO) : 0; // стоит разбить на переменные, куча условий, код не читается
          f_to_rgb (f_l1, r, g, b);
          brush.setColor (QColor (r, g, b));
          pen.setColor (QColor (r, g, b));
          painter->setPen (pen);
          painter->setBrush (brush);
          QPointF points[3] = {QPointF (l2g (a + i * hx, c + j * hy)),
                              QPointF (l2g (a + i * hx, c + (j + 1) * hy)),
                              QPointF (l2g (a + (i + 1) * hx, c + (j + 1) * hy))};
          painter->drawPolygon (points, 3);

          f_l2 = P_f (x_coef, a + i * hx + 2 * hx / 3, c + j * hy + hy / 3, nx, ny, a, c, hnx, hny);
          f_l2 += (i == mx / 2 && j == my / 2) ? (perturbation * 0.1 * F_MAX_MODULO) : 0; // код дублируется. Это невозможно поддерживать будет.
          f_to_rgb (f_l2, r, g, b);
          brush.setColor (QColor (r, g, b));
          pen.setColor (QColor (r, g, b));
          painter->setPen (pen);
          painter->setBrush (brush);
          points[1] = QPointF (l2g (a + (i + 1) * hx, c + j * hy));
          painter->drawPolygon (points, 3);
        }
    }
}
int do_no (Window *ptr) // Window * без ptr
{
  (void) ptr; // удалить void(ptr)
  return 0;
}

int do_del (Window *ptr)
{
  return ptr->exit_func();
}

void Window::closeEvent (QCloseEvent *)
{
  funex (this); // непонятно что такое funex: function exception?
}

int Window::exit_func ()
{
  prst = 0;
	pthread_mutex_lock (&glob->m);
	int calc_st = glob->calc_st;
	pthread_mutex_unlock (&glob->m);
  if (calc_st == calc_status::CALC_IN_PROCESS)
    {
      QMessageBox::warning (0, "Attention",
                            "Please wait for the end of calculation");
      return 1;
    }
  pthread_mutex_lock (&glob->m);
  glob->request = requests::EXIT;
  pthread_cond_broadcast (&glob->cond);
  pthread_mutex_unlock (&glob->m);
  return 0;
}

void Window::vrema () // переименовать на check_timers ()
{
  int request, calc_st, alg_st; // стоит удалить и на месте инициализировать
  pthread_mutex_lock (&glob->m);
  request = glob->request; // int request =
  calc_st = glob->calc_st; // int calc_st =
  alg_st = glob->alg_st; // int alg_st =
  pthread_mutex_unlock (&glob->m);
  if (request == requests::EXIT) // сделать switch (request)
    {
      window ()->close ();
    }
  else if (request == requests::WAIT)
    {
      if (calc_st == calc_status::CALC_FINISHED) // вынести внутреннюю часть if в отдельную функцию. Если в классе появится еще timer, его обработка превратит эту функцию в нечитаемый код
        {
          prst = 1;
          pthread_mutex_lock (&glob->m);
          if (alg_st)
            {
              printf ("ERROR: algorithm\n"); // Ошибка для пользователя совсем непонятная будет
              glob->request = requests::EXIT;
              window ()->close ();
            }
          else
            {
              copy_to_gui ();
              glob->calc_st = calc_status::SLEEP;
            }
          pthread_mutex_unlock (&glob->m);
          status_bar = ""; // status_bar.clear (), status_bar заменить на std::string
          rdy = 1;

          funex = do_del;
          update ();
        }
      else
        prst = 1; // непонятное название переменной и смысл значения
    }
}

void Window::update_all ()
{
	pthread_mutex_lock (&glob->m); // Под windows этот код не соберется
  if (glob->init_memory ())
    {
      printf ("ERROR: MEMORY\n"); // Непонятная ошибка для пользователя. Будет утечка памяти, нужно использовать unique_ptr для расчетных данных. Qt шные объекты с parent можно использовать голыми указателями
      window ()->close ();
    }
  glob->request = requests::CALCULATE;
  pthread_cond_broadcast (&glob->cond);
	pthread_mutex_unlock (&glob->m);
}

void Window::change_function ()
{
  int calc_st;
	pthread_mutex_lock (&glob->m);
	calc_st = glob->calc_st;
	pthread_mutex_unlock (&glob->m);
  if (calc_st == calc_status::CALC_IN_PROCESS)
    {
      QMessageBox::warning (0, "Attention",
                            "Please wait for the end of calculation");
      return;
    }
  status_bar = "Calculating...";
  rdy = 0;

  funex = do_no;
  update ();
  func_num = (func_num < 7) ? func_num + 1 : 0;
	pthread_mutex_lock (&glob->m);
  fill_func_info ();
	pthread_mutex_unlock (&glob->m);
  update_all ();
}

void Window::change_graphs ()
{
  graph_num = (graph_num < 2) ? graph_num + 1 : 0;
  switch (graph_num)
    {
      case 0:
        graph_text = "f";
        break;
      case 1:
        graph_text = "approx"; // approximation
        break;
      case 2:
        graph_text = "res"; // result. сокращения для пользователя будут непонятными
        break;
    }
  update ();
}

void Window::increase_scale ()
{
  scale--;
  a *= 2;
  b *= 2;
  c *= 2;
  d *= 2;
  if (b - a > VIS_NEPS)
    {
      a *= 0.5;
      b *= 0.5;
      QMessageBox::warning (0, "Warning", "The maximum scale X has been reached!");
    }
  if (d - c > VIS_NEPS)
    {
      c *= 0.5;
      d *= 0.5;
      QMessageBox::warning (0, "Warning", "The maximum scale Y has been reached!");
    }
  update ();
}

void Window::decrease_scale ()
{
  scale ++;
  a *= 0.5;
  b *= 0.5;
  c *= 0.5;
  d *= 0.5;
  if (b - a < MY_EP)
    {
      a *= 2;
      b *= 2;
      QMessageBox::warning (0, "Warning", "The minimum X scale has been reached!");
    }
  if (d - c < MY_EP)
    {
      c *= 2;
      d *= 2;
      QMessageBox::warning (0, "Warning", "The minimum Y scale has been reached!");
    }
  update ();
}

void Window::paintEvent (QPaintEvent * /* event */)
{
  char text_buf[128] = {};
  char text_buf1[128] = {};
  QPainter painter (this);
  QPen pen_black (Qt::black, 0, Qt::SolidLine);
  switch (graph_num)
    {
      case 0:
        {
          set_minmax_f ();
          if (fabs (F_MIN - F_MAX) < MY_EP)
            {
              F_MIN -= MY_EP;
              F_MAX += MY_EP;
            }
          draw_func (&painter);
          sprintf (text_buf1, "Function");
          painter.setPen ("black");
          break;
        }
      case 1:
        {
          set_minmax_approximation ();
          if (fabs (F_MIN - F_MAX) < MY_EP)
            {
              F_MIN -= MY_EP;
              F_MAX += MY_EP;
            }
          draw_approximation (&painter);
          sprintf (text_buf1, "Approximation");
          painter.setPen ("black");
          break;
        }
      case 2:
        {
          set_minmax_residual ();
          if (fabs (F_MIN - F_MAX) < EPS)
            {
              F_MIN -= EPS;
              F_MAX += EPS;
            }
          draw_residual (&painter);
          sprintf (text_buf1, "Residual");
          painter.setPen ("black");
          break;
        }
    }
  painter.setPen (pen_black);
  painter.drawText (5, 20, text_buf1);
  sprintf (text_buf, "(k = %d)   %s", func_num, func_text);
  painter.drawText (5, 20 + 20 * 1, text_buf);
  sprintf (text_buf, "a = %.4f   b = %.4f", a, b);
  painter.drawText (5, 20 + 20 * 2, text_buf);
  sprintf (text_buf, "c = %.4f   d = %.4f", c, d);
  painter.drawText (5, 20 + 20 * 3, text_buf);
  sprintf (text_buf, "nx = %d; ny = %d", nx, ny);
  painter.drawText (5, 20 + 20 * 4, text_buf);
  sprintf (text_buf, "mx = %d; my = %d", mx, my);
  painter.drawText (5, 20 + 20 * 5, text_buf);
  sprintf (text_buf, "perturbation = %d", perturbation);
  painter.drawText (5, 20 + 20 * 6, text_buf);
  sprintf (text_buf, "scale = %d", scale);
  painter.drawText (5, 20 + 20 * 7, text_buf);
  sprintf (text_buf, "max{|F(x)|} = %10.5e", F_MAX_MODULO);
  painter.drawText (5, 20 + 20 * 8, text_buf);
  printf ("%s\n", text_buf);

  painter.drawText (5, height () - 10, status_bar);
}


void Window::increase_n ()
{
  int calc_st = 0;
	pthread_mutex_lock (&glob->m);
	calc_st = glob->calc_st;
	pthread_mutex_unlock (&glob->m);
  if (calc_st == calc_status::CALC_IN_PROCESS)
    {
      QMessageBox::warning (0, "Attention",
                            "Please wait for the end of calculation");
      return;
    }
  status_bar = "Calculating...";
  rdy = 0;

  funex = do_no;
  update ();
	pthread_mutex_lock (&glob->m);
  glob->nx *= 2;
  glob->ny *= 2;
  glob->N = (glob->nx + 1) * (glob->ny + 1);
	pthread_mutex_unlock (&glob->m);
  update_all ();
}

void Window::decrease_n ()
{
  int calc_st = 0;
	pthread_mutex_lock (&glob->m);
	calc_st = glob->calc_st;
	pthread_mutex_unlock (&glob->m);
  if (calc_st == calc_status::CALC_IN_PROCESS)
    {
      QMessageBox::warning (0, "Attention",
                            "Please wait for the end of calculation");
      return;
    }
  status_bar = "Calculating...";
  rdy = 0;

  funex = do_no;
  update ();
	pthread_mutex_lock (&glob->m);
  glob->nx /= 2;
  glob->ny /= 2;
  if (glob->nx < 5)
    {
      glob->nx = 5;
      QMessageBox::warning (0, "Warning", "The minimum number of approximation points (nx) has been reached!");
    }
  if (glob->ny < 5)
    {
      glob->ny = 5;
      QMessageBox::warning (0, "Warning", "The minimum number of approximation points (ny) has been reached!");
    }
  glob->N = (glob->nx + 1) * (glob->ny + 1);
	pthread_mutex_unlock (&glob->m);
  update_all ();
}

void Window::increase_m ()
{
  mx *= 2;
  my *= 2;
  update ();
}

void Window::decrease_m ()
{
  mx /= 2;
  my /= 2;
  if (mx < 3)
    {
      mx = 3;
      QMessageBox::warning (0, "Warning", "The minimum number of visualization points (mx) has been reached!");
    }
  if (my < 3)
    {
      my = 3;
      QMessageBox::warning (0, "Warning", "The minimum number of visualization points (my) has been reached!");
    }
  update ();
}

void Window::increase_p ()
{
  perturbation++;
  update ();
}

void Window::decrease_p ()
{
  perturbation--;
  update ();
}

QSize Window::minimumSizeHint () const
{
  return QSize (500, 500);
}

QSize Window::sizeHint () const
{
  return QSize (1000, 1000);
}

void Window::set_minmax_f ()
{
  int i = 0, j = 0;
  F_MIN = F_MAX = f_ (a, c);
  double f0 = 0.;
  double hx = (b - a) / mx, hy = (d - c) / my;
  for (i = 0; i < mx; i ++)
    {
      for (j = 0; j < my; j ++)
        {
          f0 = f_ (a + i * hx + hx / 3., c + j * hy + 2. * hy / 3.); // Стоит попробовать объединить код с IF, пока он дублируется.
          if (f0 < F_MIN)
            F_MIN = f0;
          if (f0 > F_MAX)
            F_MAX = f0;
          f0 = f_ (a + i * hx + 2. * hx / 3., c + j * hy + hy / 3.);
          if (f0 < F_MIN)
            F_MIN = f0;
          if (f0 > F_MAX)
            F_MAX = f0;
        }
    }
  F_MAX_MODULO = (fabs (F_MIN) < fabs (F_MAX)) ? fabs (F_MAX) : fabs (F_MIN);
  if (perturbation)
    F_MAX_MODULO += perturbation * 0.1 * F_MAX_MODULO;
}

void Window::set_minmax_approximation ()
{
  int i = 0, j = 0;
  double f0 = 0.;
  double hx = (b - a) / mx, hy = (d - c) / my;
  double hnx = (b - a) / nx, hny = (d - c) / ny;
  F_MIN = F_MAX = P_f (x_coef, a, c, nx, ny, a, c, hnx, hny);
  for (i = 0; i < mx; i ++)
    {
      for (j = 0; j < my; j ++)
        {
          f0 = P_f (x_coef, a + i * hx + hx / 3., c + j * hy + 2. * hy / 3., nx, ny, a, c, hnx, hny); // стоит выделить в отдельные переменные параметры функции и убрать дублирование кода
          if (f0 < F_MIN)
            F_MIN = f0;
          if (f0 > F_MAX)
            F_MAX = f0;
          f0 = P_f (x_coef, a + i * hx + 2. * hx / 3., c + j * hy + hy / 3., nx, ny, a, c, hnx, hny);
          if (f0 < F_MIN)
            F_MIN = f0;
          if (f0 > F_MAX)
            F_MAX = f0;
        }
    }
  F_MAX_MODULO = (fabs (F_MIN) < fabs (F_MAX)) ? fabs (F_MAX) : fabs (F_MIN);
  if (perturbation)
    F_MAX_MODULO += perturbation * 0.1 * F_MAX_MODULO;
}





void Window::f_to_rgb (double f_value, double &c1, double &c2, double &c3)
{
  double pt = 0.;
  c1 = c2 = c3 = 128;
  if (F_MAX - F_MIN < 1e-13) // константы нужно выделить в namespace
  {
    c1 = c2 = c3 = 128;
    return;
  }

  pt =  (f_value - F_MIN) / (F_MAX - F_MIN);

  if (pt < 1. / 7.)
    {
      c1 = (int)(pt * 128 * 7) % 256;
      c2 = 0;
      c3 = (int)(pt * 128 * 7) % 256;
      return;

    }
  if (pt < 2. / 7.)
    {

      c1 = 0;
      c3 = (int)(pt * 255 * 7 / 2) % 256;
      c2 = 0;
      return;

    }
  if (pt < 3. / 7.)
    {
      c1 = 0;
      c2 = (int)(pt * 191 * 7 / 3) % 256;
      c3 = (int)(pt * 255 * 7 / 3) % 256;
      return;
    }
  if (pt < 4. / 7.)
    {
      c1 = 0;
      c2 = (int)(pt * 255 * 7 / 4) % 256;
      c3 = 0;
      return;
    }
  if (pt < 5. / 7.)
    {
      c1 = (int)(pt * 255 * 7 / 5) % 256;
      c2 = (int)(pt * 255 * 7 / 5) % 256;
      c3 = 0;
      return;


    }
  if (pt < 6. / 7.)
    {
      c1 = (int)(pt * (255 * 7. / 6)) % 256;
      c2 = (int)(pt * (140 * 7. / 6)) % 256;
      c3 = 0;
      return;
    }
  c1 = (int)(pt * 255) % 256;
  c2 = 0;
  c3 = 0;
  return;
}

void Window::set_minmax_residual ()
{
  int i = 0, j = 0;
  double f0 = 0.;
  double hx = (b - a) / mx, hy = (d - c) / my;
  double hnx = (b - a) / nx, hny = (d - c) / ny;
  F_MIN = F_MAX = f_ (a, c) - P_f (x_coef, a, c, nx, ny, a, c, hnx, hny);
  for (i = 0; i < mx; i ++)
    {
      for (j = 0; j < my; j ++) // стоит отдельно вычислить значения для улучшения читаемости кода
        {
          f0 = f_ (a + i * hx + hx / 3, c + j * hy + 2 * hy / 3) - P_f (x_coef, a + i * hx + hx / 3, c + j * hy + 2 * hy / 3, nx, ny, a, c, hnx, hny);
          if (f0 < F_MIN)
            F_MIN = f0;
          if (f0 > F_MAX)
            F_MAX = f0;
          f0 = f_ (a + i * hx + 2 * hx / 3, c + j * hy + hy / 3) - P_f (x_coef, a + i * hx + 2 * hx / 3, c + j * hy + hy / 3, nx, ny, a, c, hnx, hny);
          if (f0 < F_MIN)
            F_MIN = f0;
          if (f0 > F_MAX)
            F_MAX = f0;
        }
    }
  F_MAX_MODULO = (fabs (F_MIN) < fabs (F_MAX)) ? fabs (F_MAX) : fabs (F_MIN);
}



void Window::draw_func (QPainter *painter)
{
  QPen pen (Qt::darkRed, 0, Qt::SolidLine);
  QBrush brush (Qt::darkRed, Qt::SolidPattern);
  double hx = (b - a) / mx, hy = (d - c) / my;
  double r, g, b;
  double f_l1, f_l2;
  int i, j;
  painter->setPen (pen);
  for (i = 0; i < mx; i ++)
    {
      for (j = 0; j < my; j ++)
        {
          f_l1 = f_ (a + i * hx + hx / 3, c + j * hy + 2 * hy / 3);
          f_l1 += (i == mx / 2 && j == my / 2) ? (perturbation * 0.1 * F_MAX_MODULO) : 0;
          f_to_rgb (f_l1, r, g, b);
          brush.setColor (QColor (r, g, b));
          pen.setColor (QColor (r, g, b));
          painter->setPen (pen);
          painter->setBrush (brush);
          QPointF points[3] = {QPointF (l2g (a + i * hx, c + j * hy)),
                              QPointF (l2g (a + i * hx, c + (j + 1) * hy)),
                              QPointF (l2g (a + (i + 1) * hx, c + (j + 1) * hy))};
          painter->drawPolygon (points, 3);

          f_l2 = f_ (a + i * hx + 2 * hx / 3, c + j * hy + hy / 3);
          f_l2 += (i == mx / 2 && j == my / 2) ? (perturbation * 0.1 * F_MAX_MODULO) : 0;
          f_to_rgb (f_l2, r, g, b);
          brush.setColor (QColor (r, g, b));
          pen.setColor (QColor (r, g, b));
          painter->setPen (pen);
          painter->setBrush (brush);
          points[1] = QPointF (l2g (a + (i + 1) * hx, c + j * hy));
          painter->drawPolygon (points, 3);
        }
    }
}

// #include "thread.h"
// #include "thread.h" нужно удалить неиспользуемые headers

static double *results = nullptr;

class global_results;

class global_args;

class Args;

int init_reduce_sum (int p)
{
  results = new double[p];
  if (!results)
    return -1;
  return 0;
}

void free_reduce_sum ()
{
  if (results)
    delete [] results;
}

double reduce_sum_det (int p, int k, double s)
{
  double sum = 0.;
  int l = 0;
  results[k] = s;
  reduce_sum (p);
  for (l = 0; l < p; l ++)
    sum += results[l];
  reduce_sum (p);
  return sum;
}


// y = Ax
void mult_msr_matrix_vector (double * A, int * I, int n, double * x, double * y, int p, int k)
{
  int i = 0, j = 0, l = 0, J = 0, l1 = 0, l2 = 0;
  double s = 0.;
  l1 = n * k / p;
  l2 = n * (k + 1) / p;
  for (i = l1; i < l2; i ++)
    {
      s = A[i] * x[i];
      l = I[i + 1] - I[i];
      J = I[i];
      for (j = 0; j < l; j ++)
        {
          s += A[J + j] * x[I[J + j]];
        }
      y[i] = s;
    }
  reduce_sum (p);
}

// x -= t * y
void mult_sub_vector (int n, double * x, double * y, double t, int p, int k)
{
  int l;
  int l1 = n * k / p;
  int l2 = n * (k + 1) / p;
  for (l = l1; l < l2; l ++)
    x[l] -= t * y[l];

  reduce_sum (p);
}

double scalar_product (int n, double * x, double * y, int p, int k)
{
  int l = 0;
  int l1 = n * k / p;
  int l2 = n * (k + 1) / p;
  double s = 0.;
  for (l = l1; l < l2; l ++)
    s += x[l] * y[l];
  s = reduce_sum_det (p, k, s);
  return s;
}

// r = Mv
void apply_preconditioner_msr_matrix (int n, double * A, int * I, double * v, double * y, double * r, int p, int k)
{
  int l, len, j, i_curr;
  int l1 = (n * k) / p;
  int l2 = (n * (k + 1)) / p;
  double s;
  for (l = l1; l < l2; l ++)
    {
      s = 0;
      len = I[l + 1] - I[l];
      i_curr = I[l];
      for (j = 0; j < len; j ++)
        {
          if (I[i_curr + j] >= l1)
            {
              if (I[i_curr + j] < l)
                s += A[i_curr + j] * y[I[i_curr + j]];
              else
                break;
            }
        }
      s *= W;
      y[l] = (r[l] - s) / A[l];
    }

  for (l = l1; l < l2; l++)
    y[l] *= A[l];

  for (l = l2 - 1; l >= l1; l --)
    {
      s = 0;
      len = I[l + 1] - I[l];
      i_curr = I[l];
      for (j = 0; j < len; j ++)
        {
          if (I[i_curr + j] > l)
            {
              if (I[i_curr + j] < l2)
                s += A[i_curr + j] * v[I[i_curr + j]];
              else
                break;
            }
        }
      s *= W;
      v[l] = (y[l] - s) / A[l];
    }

  for (l = l1; l < l2; l ++)
    v[l] *= (W * (2 - W));

  reduce_sum (p);
}

int fill_IA (int nx, int ny, double hx, double hy, int * I, double * A, int p, int k)
{
  int i, j, l, l1, l2, s, r, t;
  int n = (nx + 1) * (ny + 1);
  double err = 0.;
  int len = 0;
  l1 = n * k / p;
  l2 = n * (k + 1) / p;
  for (l = l1; l < l2; l ++)
    {
      l2ij (nx, ny, i, j, l);
      if (get_diag (nx, ny, hx, hy, i, j, I, A) != 0)
        {
          err = -1;
          break;
        }
      r = I[l];
      s = I[l + 1] - I[l];

      t = get_off_diag (nx, ny, hx, hy, i, j, I + r, A + r);
      if (s != t)
        {
          err = -2;
          break;
        }
      len += s;
    }
  reduce_sum (p, &err, 1);
  if (err < 0) // стоит в stderr печатать что происходит
    return -1;
  reduce_sum (p, &len, 1);
  if (I[n] != n + 1 + len)
    return -2;
  return 0;
}

int fill_I (int nx, int ny, int * I)
{
  int i, j, l, r;
  int n = (nx + 1) * (ny + 1);
  double hx = 0, hy = 0;
  r = n + 1;
  for (l = 0; l < n; l ++)
    {
      l2ij (nx, ny, i, j, l);
      int s = get_off_diag (nx, ny, hx, hy, i, j, 0, 0);
      I[l] = r;
      r += s;
    }
  I[l] = r;
  return r;
}

double F_ij (int nx, int ny, double hx, double hy, double a, double c, double (*f) (double, double), int l)
{
  int i = 0, j = 0;
  l2ij (nx, ny, i, j, l);
  return (
      ((i < nx && j > 0) ? (2 * FUNC (f, a, c, i, j, hx, hy)
                          + FUNC (f, a, c, i + 1, j, hx, hy)
                          + FUNC (f, a, c, i, j - 1, hx, hy)) : 0)
    + ((i > 0 && j > 0) ? (4 * FUNC (f, a, c, i, j, hx, hy)
                         + 2 * FUNC (f, a, c, i - 1, j - 1, hx, hy)
                         + FUNC (f, a, c, i - 1, j, hx, hy)
                         + FUNC (f, a, c, i, j - 1, hx, hy)) : 0)
    + ((i > 0 && j < ny) ? (2 * FUNC (f, a, c, i, j, hx, hy)
                          + FUNC (f, a, c, i - 1, j, hx, hy)
                          + FUNC (f, a, c, i, j + 1, hx, hy)) : 0)
    + ((i < nx && j < ny) ? (4 * FUNC (f, a, c, i, j, hx, hy)
                           + 2 * FUNC (f, a, c, i + 1, j + 1, hx, hy)
                           + FUNC (f, a, c, i, j + 1, hx, hy)
                           + FUNC (f, a, c, i + 1, j, hx, hy)) : 0)
         ) * hx * hy / 24;
}

void fill_BBB (int n, int nx, int ny, double hx, double hy, double *b, double x0, double y0, int p, int k, double (*f)(double, double))
{
  int l = 0;
  int l1 = n * k / p;
  int l2 = n * (k + 1) / p;

  for (l = l1; l < l2; l++)
    {
      b[l] = F_ij(nx, ny, hx, hy, x0, y0, f, l);
    }

  reduce_sum (p);
}

int min_error_msr_matrix (double * A, int * I, double * BBB, double * x, double * r, double * u, double * v,
             double eps, int maxit, int N, int p, int k)
{
  double prec = 0., b_norm2 = 0., tau = 0., c1 = 0., c2 = 0.;
  int it = 0;
  b_norm2 = scalar_product (N, BBB, BBB, p, k);
  prec = b_norm2 * eps * eps;
  mult_msr_matrix_vector (A, I, N, x, r, p, k);
  mult_sub_vector (N, r, BBB, 1., p, k);
  for (it = 0; it < maxit; it ++)
    {
      apply_preconditioner_msr_matrix (N, A, I, v, u, r, p, k); // r = Mv
      mult_msr_matrix_vector (A, I, N, v, u, p, k); // u = Av
      c1 = scalar_product (N, v, r, p, k);
      c2 = scalar_product (N, u, v, p, k);
      if (fabs (c1) < prec || fabs (c2) < prec)
        break;
      tau = c1 / c2;
      mult_sub_vector (N, x, v, tau, p, k);
      mult_sub_vector (N, r, u, tau, p, k);
    }
  reduce_sum (p);
  return (it < maxit) ? it : -1;
}

int algorithm_ (double * A, int * I, double * BBB, double * x, double * r, double * u, double * v,
                double eps, int max_iter, int N, int p, int k)
{
  int step, its = 0;
  for (step = 0; step < MAX_STEPS; step ++)
    {
      int res = min_error_msr_matrix (A, I, BBB, x, r, u, v, eps, max_iter, N, p, k);
      if (res >= 0)
        {
          its += res;
          break;
        }
      its += max_iter;
    }
  if (step >= MAX_STEPS)
    return -1;
  return its;
}

void residual_1 (int n, int nx, int ny, double hx, double hy, double a, double c, int p, int k, double * x, double (* f_) (double, double)) //  код дублируется в последующих функциях. стоит его попытаться унифицировать.
{
  int l = 0, i = 0, j = 0;
  int l1 = n * k / p;
  int l2 = n * (k + 1) / p;
  double res = 0;
  double f_l1, f_l2;
  double pf_l1, pf_l2;
  for (l = l1; l < l2; l++)
    {
      l2ij (nx, ny, i, j, l);
      if (i < nx && j < ny)
        {
          f_l1 = f_ (a + i * hx + hx / 3, c + j * hy + 2 * hy / 3);
          pf_l1 = P_f (x, a + i * hx + hx / 3, c + j * hy + 2 * hy / 3, nx, ny, a, c, hx, hy);
          if (fabs (f_l1 - pf_l1) > res)
            res = fabs (f_l1 - pf_l1);

          f_l2 = f_ (a + i * hx + 2 * hx / 3, c + j * hy + hy / 3);
          pf_l2 = P_f (x, a + i * hx + 2 * hx / 3, c + j * hy + hy / 3, nx, ny, a, c, hx, hy);
          if (fabs (f_l2 - pf_l2) > res)
            res = fabs (f_l2 - pf_l2);
        }
    }
  results[k] = res;
  reduce_sum (p);
}

void residual_2 (int n, int nx, int ny, double hx, double hy, double a, double c, int p, int k, double * x, double (* f_) (double, double))
{
  int l = 0, i = 0, j = 0;
  int l1 = n * k / p;
  int l2 = n * (k + 1) / p;
  double res = 0;
  double f_l1, f_l2;
  double pf_l1, pf_l2;
  for (l = l1; l < l2; l++)
    {
      l2ij (nx, ny, i, j, l);
      if (i < nx && j < ny)
        {
          f_l1 = f_ (a + i * hx + hx / 3, c + j * hy + 2 * hy / 3);
          pf_l1 = P_f (x, a + i * hx + hx / 3, c + j * hy + 2 * hy / 3, nx, ny, a, c, hx, hy);
          res += fabs (f_l1 - pf_l1);

          f_l2 = f_ (a + i * hx + 2 * hx / 3, c + j * hy + hy / 3);
          pf_l2 = P_f (x, a + i * hx + 2 * hx / 3, c + j * hy + hy / 3, nx, ny, a, c, hx, hy);
          res += fabs (f_l2 - pf_l2);
        }
    }
  results[k] = res * hx * hy * 0.5;
  reduce_sum (p);
}

void residual_3 (int n, int nx, int ny, double hx, double hy, double a, double c, int p, int k, double * x, double (* f_) (double, double))
{
  int l = 0, i = 0, j = 0;
  int l1 = n * k / p;
  int l2 = n * (k + 1) / p;
  double res = 0;
  double f_l;
  double pf_l;
  for (l = l1; l < l2; l++)
    {
      l2ij (nx, ny, i, j, l);
      if (i < nx && j < ny)
        {
          f_l = f_ (a + i * hx, c + j * hy);
          pf_l = x[l];
          if (fabs (f_l - pf_l) > res)
            res = fabs (f_l - pf_l);
        }
    }
  results[k] = res;
  reduce_sum (p);
}

void residual_4 (int n, int nx, int ny, double hx, double hy, double a, double c, int p, int k, double * x, double (* f_) (double, double))
{
  int l = 0, i = 0, j = 0;
  int l1 = n * k / p;
  int l2 = n * (k + 1) / p;
  double res = 0;
  double f_l;
  double pf_l;
  for (l = l1; l < l2; l++)
    {
      l2ij (nx, ny, i, j, l);
      if (i < nx && j < ny)
        {
          f_l = f_ (a + i * hx, c + j * hy);
          pf_l = x[l];
          res += fabs (f_l - pf_l);
        }
    }
  results[k] = res * hx * hy;
  reduce_sum (p);
}

double get_full_time ()
{
  struct timeval buf;
  gettimeofday (&buf, 0);
  return buf.tv_sec + buf.tv_usec / 1.e6;
}

double get_cpu_time ()
{
  struct rusage buf;
  getrusage (RUSAGE_THREAD, &buf);
  return buf.ru_utime.tv_sec + buf.ru_utime.tv_usec / 1.e6;
}

void *thread_loop (void * ptr)
{
  Args * args = (Args *) ptr;
  global_args * glob = args->glob;
  reduce_sum (args->p);
  while (1)
    {
      pthread_mutex_lock (&glob->m);
      for (; glob->request == requests::WAIT; )
        pthread_cond_wait (&glob->cond, &glob->m);
      pthread_mutex_unlock (&glob->m);
      switch (glob->request)
        {
          case requests::CALCULATE:
            thread_func (args);
            break;
          case requests::EXIT:
            reduce_sum (args->p);
            return 0;
          default:
            return 0;
        }
    }
  return 0;
}

void *thread_func (void * ptr)
{
  Args * args = (Args *) ptr;
  global_args * glob = args -> glob;
  pthread_mutex_lock (&glob->m);
  glob->calc_st = calc_status::CALC_IN_PROCESS;
  pthread_mutex_unlock (&glob->m);
  double * A = glob -> A;
  double * BBB = glob -> B; // стоит дать осмысленное переменной BBB
  int * I = glob -> I;
  double * x = glob -> x;
  double * r = glob -> r; // инициализацию парамтероа стоит вынести в отдельную функцию. Рассчетное ядро должно находиться в другом файле.
  double * u = glob -> u;
  double * v = glob -> v;
  double (*f_) (double, double) = glob -> f_ptr;
  int k = args -> k;
  if (k == 0)
    printf ("Calculating...\n");
  int p = args -> p;
  reduce_sum (p);
  int nx = glob -> nx;
  int ny = glob -> ny;
  int N = glob -> N;
  int LEN = glob -> len_msr;
  double a = glob -> a; // пробелы между стрелками стоит убрать
  double b = glob -> b;
  double c = glob -> c;
  double d = glob -> d;
  double eps = args -> eps;
  int max_iter = args -> maxit;

  double hx = (b - a) / nx;
  double hy = (d - c) / ny;

  int l1 = N * k / p;
  int l2 = N * (k + 1) / p;
  int res, err = 0;
  double t1 = 0, t2 = 0;
  double r1, r2, r3, r4;
  cpu_set_t cpu;
  CPU_ZERO (&cpu);
  int n_cpus = get_nprocs ();
  int cpu_id = n_cpus - 1 - (k % n_cpus);
  CPU_SET (cpu_id, &cpu);
  pthread_t tid = pthread_self ();
  pthread_setaffinity_np (tid, sizeof (cpu), &cpu);
  memset (BBB + l1, 0, (l2 - l1) * sizeof (double));
  memset (x + l1, 0, (l2 - l1) * sizeof (double));
  memset (u + l1, 0, (l2 - l1) * sizeof (double));
  memset (v + l1, 0, (l2 - l1) * sizeof (double));
  memset (r + l1, 0, (l2 - l1) * sizeof (double));

  reduce_sum (p);
  if (k == 0)
    {
      res = fill_I (nx, ny, I);
      if (res != LEN)
        err = 1;
    }
  reduce_sum (p, &err, 1);
  if (err != 0)
    {
      pthread_mutex_lock (&glob->m);
      glob->calc_st = calc_status::CALC_FINISHED;
      glob->alg_st = err;
      pthread_mutex_unlock (&glob->m);
      return 0;
    }

  err = fill_IA (nx, ny, hx, hy, I, A, p, k);
  reduce_sum (p, &err, 1);
  if (err != 0)
    {
      pthread_mutex_lock (&glob->m);
      glob->calc_st = calc_status::CALC_FINISHED;
      glob->alg_st = err;
      pthread_mutex_unlock (&glob->m);
      return 0;
    }

  fill_BBB (N, nx, ny, hx, hy, BBB, a, c, p, k, f_);
  reduce_sum (p);

  t1 = get_full_time ();
  res = algorithm_ (A, I, BBB, x, r, u, v, eps, max_iter, N, p, k);
  reduce_sum (p);
  t1 = get_full_time () - t1;

  if (res < 0)
    {
      pthread_mutex_lock (&glob->m);
      glob->calc_st = calc_status::CALC_FINISHED;
      glob->alg_st = res;
      pthread_mutex_unlock (&glob->m);
      return 0;
    }

  t2 = get_full_time ();

  r1 = r2 = r3 = r4 = 0;

  residual_1 (N, nx, ny, hx, hy, a, c, p, k, x, f_); // вычисление невязок вынести в отдельную функцию
  for (int i = 0; i < p; i ++)
    if (results[i] > r1)
      r1 = results[i];
  reduce_sum (p);

  residual_2 (N, nx, ny, hx, hy, a, c, p, k, x, f_);
  for (int i = 0; i < p; i ++)
    r2 += results[i];
  reduce_sum (p);

  residual_3 (N, nx, ny, hx, hy, a, c, p, k, x, f_);
  for (int i = 0; i < p; i ++)
    if (results[i] > r3)
      r3 = results[i];
  reduce_sum (p);

  residual_4 (N, nx, ny, hx, hy, a, c, p, k, x, f_);
  for (int i = 0; i < p; i ++)
    r4 += results[i];
  reduce_sum (p);

  t2 = get_full_time () - t2;

  if (k == 0)
    {
      printf ("%s : Task = %d R1 = %e R2 = %e R3 = %e R4 = %e T1 = %.2f T2 = %.2f\
              It = %d E = %e K = %d Nx = %d Ny = %d P = %d\n",
              args->argv0, 7, r1, r2, r3, r4, t1, t2, res, eps, glob->func_num, nx, ny, p);
    }
  reduce_sum (p);
  pthread_mutex_lock (&glob->m);
  glob->calc_st = calc_status::CALC_FINISHED;
  glob->alg_st = 0;
  if (!glob->ppppp)
    glob->request = requests::WAIT;
  else
    glob->request = requests::EXIT;
  pthread_mutex_unlock (&glob->m);
  return 0;
}

// посмотрел. код работает, но пушить такое нельзя