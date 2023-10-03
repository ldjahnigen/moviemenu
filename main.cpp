#include <iostream>
#include <dirent.h>
#include <vector>
#include <string>
#include <gtk/gtk.h>
#include <bits/stdc++.h>
#include <opencv2/opencv.hpp>


const std::string MOVIE_PATH = "/home/louisj/Downloads/movies/";
const std::string POSTER_PATH = "/home/louisj/Downloads/movies/posters/";

struct CallbackData {
    GtkWidget *window;
    std::string *movie_title;
};

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
  struct CallbackData *callbackData = static_cast<struct CallbackData*>(data);
  GtkWidget *window = callbackData->window;
  std::string movie_title = *callbackData->movie_title;
  delete callbackData;

  gtk_window_close(GTK_WINDOW(window));
  pid_t childPid = fork();

  if (childPid == 0) {
    std::string command = "vlc " + MOVIE_PATH + movie_title;
    int result = system(command.c_str());
    if (result == 0) {
    } else {
        std::cerr << "Command execution failed." << '\n';
    }
  }
}


int main(int argc, char* argv[]) {
  std::string chosen_movie, command;
  std::vector<std::string> files = getFileNames(MOVIE_PATH);

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

  // Create buttons
  int row, col = 0;
  std::string stringpath;
  for (std::string s : files) {
    std::string* title = new std::string;
    *title = s;

    struct CallbackData *callbackData = new struct CallbackData;
    callbackData->window = window;
    callbackData->movie_title = title;

    std::stringstream ss(s);
    getline(ss, s, '.');

    stringpath = POSTER_PATH + s + ".jpg";

    // Scale all images to a uniform size
    cv::Mat inputImage = cv::imread(stringpath);
    if (inputImage.empty()) {
    }
    else if (inputImage.cols == 123) {
    } else {
      int targetWidth = 123;
      double scaleFactor = static_cast<double>(targetWidth) / inputImage.cols;
      cv::Mat scaledImage;
      cv::resize(inputImage, scaledImage, cv::Size(targetWidth, static_cast<int>(inputImage.rows * scaleFactor)));
      cv::imwrite(stringpath, scaledImage);
    }

    const char* path = stringpath.c_str();
    GError *error = NULL;
    GdkPixbuf *image = gdk_pixbuf_new_from_file(path, &error);

    if (error != NULL) {
      std::cout << "No image found for " << s << '\n';
      const char *label = s.c_str();
      GtkWidget *button = gtk_button_new_with_label(label);

      g_signal_connect(button, "clicked", G_CALLBACK(buttonClicked), callbackData);
      gtk_grid_attach(GTK_GRID(grid), button, col, row, 1, 1);
    } else {
      GtkWidget *button = gtk_button_new();
      GtkWidget *imageWidget = gtk_image_new_from_pixbuf(image);
      gtk_button_set_image(GTK_BUTTON(button), imageWidget);

      g_signal_connect(button, "clicked", G_CALLBACK(buttonClicked), callbackData);
      gtk_grid_attach(GTK_GRID(grid), button, col, row, 1, 1);
    }  
    
    if (col == 8) {
        col = 0;
        row++;
    } else {
      col++;
    }
  }
  
  gtk_widget_show_all(window);
  gtk_main();

  return 0;
}
