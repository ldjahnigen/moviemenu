#include <iostream>
#include <dirent.h>
#include <vector>
#include <string>
#include <gtk/gtk.h>
#include <bits/stdc++.h>
#include <opencv2/opencv.hpp>
#include <fstream>


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
    std::string title;
    std::string pathToEps;
    std::string poster;
    GtkWidget* Window;
  
    Show(std::string title_, std::string pathToEps_, std::string poster_, GtkWidget* Window_) {
      title = title_;
      pathToEps = pathToEps_;
      poster = poster_;
      Window = Window_;
    }

    void play(std::string episode) {
      pid_t childPid = fork();

      if (childPid == 0) {
        std::string command = "vlc " + this->pathToEps + episode;
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
static void buttonPageClicked(GtkWidget*);
void scaleImage(std::string stringpath);
void createMovieButtons(GtkWidget*, 
                   GtkWidget*, 
                   std::vector<std::string>, 
                   std::string*, 
                   const std::string, 
                  const int);
void createShowButtons(GtkWidget*, 
                   GtkWidget*, 
                   std::vector<std::string>, 
                   std::string*,
                   const std::string,
                   const int);
void deleteButtons(GtkWidget*);
bool isMovie(std::string);


int main(int argc, char* argv[]) {
  std::ifstream f;
  f.open("/home/louisj/.config/mmconfig.txt");
  std::string config;
  std::ostringstream ss;
  ss << f.rdbuf(); 
  f.close();
  config = ss.str();

  int index1 = config.find("moviepath=") + 10;
  int index2 = config.find("posterpath=") - 1;
  int index3 = index2 + 12;
  int index4 = config.find("$END") - 1;
  std::string *MOVIE_PATH = new std::string(config.substr(index1, index2 - index1));
  const std::string POSTER_PATH = config.substr(index3, index4 - index3);
  const int COLUMN_MAX = 8;

  std::string chosen_movie, command;
  std::vector<std::string> files = getFileNames(*MOVIE_PATH);

  gtk_init(&argc, &argv);

  GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), "Movie Menu");
  g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

  GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
  gtk_container_set_border_width(GTK_CONTAINER(scrolled_window), 10);
  gtk_container_add(GTK_CONTAINER(window), scrolled_window);

  GtkWidget *viewport = gtk_viewport_new(NULL, NULL);
  gtk_container_add(GTK_CONTAINER(scrolled_window), viewport);

  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  gtk_container_add(GTK_CONTAINER(viewport), box);

  GtkWidget *grid = gtk_grid_new();
  gtk_box_pack_start(GTK_BOX(box), grid, TRUE, TRUE, 0);

  GtkWidget *titlelabel = gtk_label_new("Movies\n");
  gtk_container_add(GTK_CONTAINER(grid), titlelabel);

  createMovieButtons(grid, window, files, MOVIE_PATH, POSTER_PATH, COLUMN_MAX); 

  GtkWidget *button = gtk_button_new_with_label("Shows");
  gtk_grid_attach(GTK_GRID(grid), button, 1, 0, 1, 1);
  g_signal_connect(button, "clicked", G_CALLBACK(buttonPageClicked), NULL);

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
  GtkWidget *window = movie->window;
  std::string movie_title = movie->title;
  std::string MOVIE_PATH = movie->path;

  gtk_window_close(GTK_WINDOW(window));
  movie->play();
}


static void buttonPageClicked(GtkWidget* grid) {
  deleteButtons(grid);
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


void createMovieButtons(GtkWidget* grid, 
                   GtkWidget* window, 
                   std::vector<std::string> files, 
                   std::string *MOVIE_PATH,
                   const std::string POSTER_PATH,
                   const int COLUMN_MAX
                  ) {
  int row = 1;
  int col = 0;
  std::string stringpath, title, pathToEps;
  for (std::string s : files) {
    Movie* movie = new Movie(*MOVIE_PATH, s, window);

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


void createShowButtons(GtkWidget* grid, 
                   GtkWidget* window, 
                   std::vector<std::string> files, 
                   std::string *MOVIE_PATH,
                   const std::string POSTER_PATH,
                   const int COLUMN_MAX) {
  int row, col = 0;
  std::string stringpath, title, pathToEps;
  for (std::string s : files) {
    Movie* movie = new Movie(*MOVIE_PATH, s, window);

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


void deleteButtons(GtkWidget* grid) {
  GList* children = gtk_container_get_children(GTK_CONTAINER(grid));
    for (GList* iter = children; iter != NULL; iter = g_list_next(iter)) {
        GtkWidget* child = GTK_WIDGET(iter->data);
        gtk_widget_destroy(child);
    }
    g_list_free(children); 
}


bool isMovie(std::string s) {
  if (s[0] == 'M') {
    return true;    
  }
  return false;
}
