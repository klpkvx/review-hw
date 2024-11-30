#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QAction>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QMessageBox>

#include "window.h"
#include "sfs.h"

int main (int argc, char * argv[])
{
  QApplication app(argc, argv);
  QMainWindow *window = new QMainWindow;
  Window *graph_area = new Window (window);

  if (graph_area->parse_command_line (argc, argv))
    {
      char a[200];
      sprintf(a, "Usage: %s a b c d nx ny mx my k eps maxit p", argv[0]);
      QMessageBox msgBox;
      msgBox.setText(a);
      msgBox.setInformativeText("ERROR!!!");
      msgBox.setStandardButtons(QMessageBox::Yes);
      msgBox.setDefaultButton(QMessageBox::Yes);
      int ret = msgBox.exec();
      if (ret == QMessageBox::Yes)
        {
          delete window;
          return -1;          
        }
    }
  QMenuBar *tool_bar = new QMenuBar (window);
  QAction *action;
  int p = graph_area->get_p ();
  Args *args = new Args[p];
  if (!args)
    {
      printf ("ERROR: MEMORY\n");
      delete graph_area;
      delete tool_bar;
      delete window;
      return 2;
    }
  for (int i = 0; i < p; i ++)
    args[i].argv0 = argv[0];
  int res = init_reduce_sum (p);
  if (res != 0)
    {
      printf ("ERROR: MEMORY\n");
      delete graph_area;
      delete tool_bar;
      delete window;
      delete [] args;
      return 3;
    }
  graph_area->arrr = args;
  graph_area->allocate_memory ();
  graph_area->init_memory ();
  graph_area->start_timer ();
  graph_area->fill_func_info ();
  graph_area->set_args (args);
  action = tool_bar->addAction ("function", graph_area, SLOT (change_function ()));
  action->setShortcut (QString ("0"));
  action = tool_bar->addAction ("graphs", graph_area, SLOT (change_graphs ()));
  action->setShortcut (QString ("1"));
  action = tool_bar->addAction ("zoom-", graph_area, SLOT (increase_scale ()));
  action->setShortcut (QString ("2"));
  action = tool_bar->addAction ("zoom+", graph_area, SLOT (decrease_scale ()));
  action->setShortcut (QString ("3"));
  action = tool_bar->addAction ("n+", graph_area, SLOT (increase_n ()));
  action->setShortcut (QString ("4"));
  action = tool_bar->addAction ("n-", graph_area, SLOT (decrease_n ()));
  action->setShortcut (QString ("5"));
  action = tool_bar->addAction ("p+", graph_area, SLOT (increase_p ()));
  action->setShortcut (QString ("6"));
  action = tool_bar->addAction ("p-", graph_area, SLOT (decrease_p ()));
  action->setShortcut (QString ("7"));
  action = tool_bar->addAction ("m+", graph_area, SLOT (increase_m ()));
  action->setShortcut (QString ("8"));
  action = tool_bar->addAction ("m-", graph_area, SLOT (decrease_m ()));
  action->setShortcut (QString ("9"));
  action = tool_bar->addAction ("E&xit", graph_area, SLOT (exit_func()));
  action->setShortcut (QString ("Ctrl+X"));
  tool_bar->setMaximumHeight (30);
  window->setMenuBar (tool_bar);
  window->setCentralWidget (graph_area);
  window->setWindowTitle ("TEO");
  for (int k = 0; k < p; k ++)
    {
      if (pthread_create (&args[k].tid, 0, thread_loop, args + k))
        {
          printf ("ERROR: ERROR: ERROR: CREATE %d!!!\n", k);
          abort ();
        }
    }
  graph_area->update_all ();
  window->show ();
  app.exec ();
  delete action;
  delete graph_area;
  delete tool_bar;
  delete window;
  delete [] args;
  free_reduce_sum ();
  return 0;
}

