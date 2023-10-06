#include <iostream>
#include <dirent.h>
#include <vector>
#include <string>
#include <gtk/gtk.h>
#include <bits/stdc++.h>
#include <opencv2/opencv.hpp>
#include <fstream>
#include <X11/Xlib.h>

GtkWidget *window;
GtkWidget *grid;
std::string SHOW_PATH;
std::string MOVIE_PATH;
std::string POSTER_PATH;
int COLUMN_MAX;

class Movie {
  public:
    std::string path;
    std::string title;
    GtkWidget* window;

    Movie(std::string path_, std::string title_, GtkWidget* window_) {
      path = path_;
      title = title_;
      window = window_;
    }

    void play() {
      pid_t childPid = fork();

      if (childPid == 0) {
        std::string command = "vlc " + this->path + this->title;
        int result = system(command.c_str());
        if (result == 0) {
        } else {
            std::cerr << "Command execution failed." << '\n';
        }
      }
    }
};


class Show {
  public:
    std::string path;
    std::string title;
    GtkWidget* window;

    Show(std::string path_, std::string title_, GtkWidget* window_) {
      path = path_;
      title = title_;
      window = window_;
    }

    void play() {
      pid_t childPid = fork();

      if (childPid == 0) {
        std::string command = "vlc " + this->path + this->title;
        int result = system(command.c_str());
        if (result == 0) {
        } else {
            std::cerr << "Command execution failed." << '\n';
        }
      }
    }
};


std::vector<std::string> getFileNames(std::string path);
static void buttonClicked(GtkWidget* widget, gpointer data);
static void buttonShowClicked(GtkWidget*, gpointer);
static void buttonMovieClicked(GtkWidget*, gpointer);
static void buttonShowButtonClicked(GtkWidget*, gpointer);
void scaleImage(std::string stringpath);
void createMovieButtons(std::vector<std::string>);
void createEpisodeButtons(std::vector<std::string>, std::string);
void createShowButtons();


int main(int argc, char* argv[]) {
  std::ifstream f;
  f.open("/home/louisj/.config/mmconfig.txt");
  std::string config;
  std::ostringstream ss;
  ss << f.rdbuf(); 
  f.close();
  config = ss.str();

  // Define constants
  int index1 = config.find("moviepath=") + 10;
  int index2 = config.find("posterpath=") - 1;
  int index3 = index2 + 12;
  int index4 = config.find("showpath=") - 1;
  int index5 = index4 + 10;
  int index6 = config.find("$END") - 1;;;
  MOVIE_PATH = std::string(config.substr(index1, index2 - index1));
  POSTER_PATH = config.substr(index3, index4 - index3);
  SHOW_PATH = config.substr(index5, index6 - index5);
  
  Display *display = XOpenDisplay(nullptr);
  Screen *screen = XDefaultScreenOfDisplay(display);
  int screenWidth = WidthOfScreen(screen);
  XCloseDisplay(display);

  COLUMN_MAX = (screenWidth / 123.0) - 2;
  std::string chosen_movie, command;
  std::vector<std::string> files = getFileNames(MOVIE_PATH);

  gtk_init(&argc, &argv);

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), "Movie Menu");
  g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

  GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
  gtk_container_set_border_width(GTK_CONTAINER(scrolled_window), 10);
  gtk_container_add(GTK_CONTAINER(window), scrolled_window);

  GtkWidget *viewport = gtk_viewport_new(NULL, NULL);
  gtk_container_add(GTK_CONTAINER(scrolled_window), viewport);

  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  gtk_container_add(GTK_CONTAINER(viewport), box);

  grid = gtk_grid_new();
  gtk_box_pack_start(GTK_BOX(box), grid, TRUE, TRUE, 0);

  GtkWidget *titlelabel = gtk_label_new("Movies\n");
  gtk_container_add(GTK_CONTAINER(grid), titlelabel);

  createMovieButtons(files); 

  GtkWidget *button = gtk_button_new_with_label("Shows");
  gtk_grid_attach(GTK_GRID(grid), button, 1, 0, 1, 1);
  g_signal_connect(button, "clicked", G_CALLBACK(buttonShowClicked), NULL);

  gtk_widget_show_all(window);
  gtk_main();

  return 0;
}


std::vector<std::string> getFileNames(std::string path) {
  std::vector<std::string> files;
  DIR* dir = opendir(path.c_str());

  if (dir) {
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
      if (entry->d_type == DT_REG) {
        files.push_back(entry->d_name);
      }
    }
    closedir(dir);
  } else {
    std::cerr << "Error opening directory " << path << '\n';
  }

  return files;
}


static void buttonClicked(GtkWidget* widget, gpointer data) {
  Movie *movie = static_cast<Movie*>(data);
  gtk_window_close(GTK_WINDOW(window));
  movie->play();
}


static void buttonShowButtonClicked(GtkWidget* widget, gpointer data) {
  Show *show = static_cast<Show*>(data);
  GList* children = gtk_container_get_children(GTK_CONTAINER(grid));
    for (GList* iter = children; iter != NULL; iter = g_list_next(iter)) {
        GtkWidget* child = GTK_WIDGET(iter->data);
        gtk_widget_destroy(child);
    }
  g_list_free(children); 
  
  GtkWidget *titlelabel = gtk_label_new(show->title.c_str());
  gtk_container_add(GTK_CONTAINER(grid), titlelabel);

  GtkWidget *button = gtk_button_new_with_label("Shows");
  gtk_grid_attach(GTK_GRID(grid), button, 1, 0, 1, 1);
  g_signal_connect(button, "clicked", G_CALLBACK(buttonShowClicked), NULL);

  std::vector<std::string> files = getFileNames(show->path + show->title);
  createEpisodeButtons(files, show->title + '/');
  gtk_widget_show_all(window);
  gtk_widget_show_all(grid);
}

static void buttonShowClicked(GtkWidget* widget, gpointer data) {
  GList* children = gtk_container_get_children(GTK_CONTAINER(grid));
    for (GList* iter = children; iter != NULL; iter = g_list_next(iter)) {
        GtkWidget* child = GTK_WIDGET(iter->data);
        gtk_widget_destroy(child);
    }
  g_list_free(children); 

  std::vector<std::string> files = getFileNames(SHOW_PATH);
  createShowButtons(); 

  gtk_widget_show_all(grid);
}


static void buttonMovieClicked(GtkWidget*, gpointer) {
  GList* children = gtk_container_get_children(GTK_CONTAINER(grid));
    for (GList* iter = children; iter != NULL; iter = g_list_next(iter)) {
        GtkWidget* child = GTK_WIDGET(iter->data);
        gtk_widget_destroy(child);
    }
  g_list_free(children); 

  GtkWidget *titlelabel = gtk_label_new("Movies\n");
  gtk_container_add(GTK_CONTAINER(grid), titlelabel);

  GtkWidget *button = gtk_button_new_with_label("Shows");
  gtk_grid_attach(GTK_GRID(grid), button, 1, 0, 1, 1);
  g_signal_connect(button, "clicked", G_CALLBACK(buttonShowClicked), NULL);

  std::vector<std::string> files = getFileNames(MOVIE_PATH);
  createMovieButtons(files);

  gtk_widget_show_all(grid);
}


void scaleImage(std::string stringpath) {
  cv::Mat inputImage = cv::imread(stringpath);
  if (inputImage.empty()) {
    return;
  }
  else if (inputImage.cols == 123) {
    return;
  } else {
    int targetWidth = 123;
    double scaleFactor = static_cast<double>(targetWidth) / inputImage.cols;
    cv::Mat scaledImage;
    cv::resize(inputImage, scaledImage, cv::Size(targetWidth, static_cast<int>(inputImage.rows * scaleFactor)));
    cv::imwrite(stringpath, scaledImage);
  }
}


void createMovieButtons(std::vector<std::string> files) {
  int row = 1;
  int col = 0;
  std::string stringpath, title, pathToEps;
  for (std::string s : files) {
    Movie* movie = new Movie(MOVIE_PATH, s, window);

    std::stringstream ss(s);
    getline(ss, s, '.');

    stringpath = POSTER_PATH + s + ".jpg";

    scaleImage(stringpath);

    const char* path = stringpath.c_str();
    GError *error = NULL;
    GdkPixbuf *image = gdk_pixbuf_new_from_file(path, &error);

    if (error != NULL) {
      std::cout << "No image found for " << s << '\n';
      const char *label = s.c_str();
      GtkWidget *button = gtk_button_new_with_label(label);

      g_signal_connect(button, "clicked", G_CALLBACK(buttonClicked), movie);
      gtk_grid_attach(GTK_GRID(grid), button, col, row, 1, 1);
    } else {
      GtkWidget *button = gtk_button_new();
      GtkWidget *imageWidget = gtk_image_new_from_pixbuf(image);
      gtk_button_set_image(GTK_BUTTON(button), imageWidget);

      g_signal_connect(button, "clicked", G_CALLBACK(buttonClicked), movie);
      gtk_grid_attach(GTK_GRID(grid), button, col, row, 1, 1);
    }  
    
    if (col == COLUMN_MAX) {
        col = 0;
        row++;
    } else {
      col++;
    }
  }
}


void createEpisodeButtons(std::vector<std::string> files, std::string series) {
  int row = 1;
  int col = 0;

  std::sort(files.begin(), files.end());

  for (std::string s : files) {
    Show* movie = new Show(SHOW_PATH + series, s, window);

    const char *label = s.c_str();
    GtkWidget *button = gtk_button_new_with_label(label);
    g_signal_connect(button, "clicked", G_CALLBACK(buttonClicked), movie);
    gtk_grid_attach(GTK_GRID(grid), button, col, row, 1, 1);    

    if (col == COLUMN_MAX) {
        col = 0;
        row++;
    } else {
      col++;
    }
  }
}

void createShowButtons() {
  std::vector<std::string> files;

  DIR* dir = opendir(SHOW_PATH.c_str());
  if (dir == nullptr) {
      std::cerr << "Error opening directory" << '\n';
      return;
  }

  struct dirent* entry;

  while ((entry = readdir(dir)) != nullptr) {
      if (entry->d_type == DT_DIR) {
          if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
              files.push_back(entry->d_name);
          }
      }
  }

  closedir(dir);

  GtkWidget *titlelabel = gtk_label_new("Shows\n");
  gtk_container_add(GTK_CONTAINER(grid), titlelabel);

  GtkWidget *button = gtk_button_new_with_label("Movies");
  gtk_grid_attach(GTK_GRID(grid), button, 1, 0, 1, 1);
  g_signal_connect(button, "clicked", G_CALLBACK(buttonMovieClicked), NULL);

  int row = 1;
  int col = 0;
  std::string stringpath;
  for (std::string s : files) {
    Show* show = new Show(SHOW_PATH, s, window);

    std::stringstream ss(s);
    getline(ss, s, '.');

    stringpath = POSTER_PATH + s + ".jpg";

    scaleImage(stringpath);

    const char* path = stringpath.c_str();
    GError *error = NULL;
    GdkPixbuf *image = gdk_pixbuf_new_from_file(path, &error);

    if (error != NULL) {
      std::cout << "No image found for " << s << '\n';
      const char *label = s.c_str();
      GtkWidget *button = gtk_button_new_with_label(label);

      g_signal_connect(button, "clicked", G_CALLBACK(buttonShowButtonClicked), show);
      gtk_grid_attach(GTK_GRID(grid), button, col, row, 1, 1);
    } else {
      GtkWidget *button = gtk_button_new();
      GtkWidget *imageWidget = gtk_image_new_from_pixbuf(image);
      gtk_button_set_image(GTK_BUTTON(button), imageWidget);

      g_signal_connect(button, "clicked", G_CALLBACK(buttonShowButtonClicked), show);
      gtk_grid_attach(GTK_GRID(grid), button, col, row, 1, 1);
      gtk_widget_queue_draw(window);
      gtk_widget_queue_draw(grid);
    }  
    
    if (col == COLUMN_MAX) {
        col = 0;
        row++;
    } else {
      col++;
    }
  }
}
