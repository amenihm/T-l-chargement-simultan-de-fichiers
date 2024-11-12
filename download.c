#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <curl/curl.h>

// Structure pour stocker les informations de téléchargement
typedef struct {
    char *url;
    char *output_file;
} download_task_t;

// Fonction qui sera appelée pour effectuer le téléchargement
size_t write_callback(void *ptr, size_t size, size_t nmemb, void *userdata) {
    FILE *output_file = (FILE *)userdata;
    size_t total_size = size * nmemb;
    fwrite(ptr, size, nmemb, output_file);
    return total_size;
}

void *download_file(void *task) {
    download_task_t *download_task = (download_task_t *)task;
    
    CURL *curl;
    CURLcode res;
    FILE *output_file = fopen(download_task->output_file, "wb");

    if (!output_file) {
        fprintf(stderr, "Erreur d'ouverture du fichier de sortie %s\n", download_task->output_file);
        return NULL;
    }

    curl_global_init(CURL_GLOBAL_DEFAULT); // Initialiser libcurl
    curl = curl_easy_init();

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, download_task->url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, output_file);

        res = curl_easy_perform(curl); // Effectuer le téléchargement

        if (res != CURLE_OK) {
            fprintf(stderr, "Échec du téléchargement : %s\n", curl_easy_strerror(res));
        } else {
            printf("Téléchargement terminé pour %s\n", download_task->url);
        }

        curl_easy_cleanup(curl);  // Nettoyer après utilisation
    }

    fclose(output_file);
    curl_global_cleanup(); // Libérer les ressources de libcurl
    return NULL;
}

int main() {
    pthread_t threads[3];  // Déclaration de 3 threads
    download_task_t tasks[3];  // Structure pour les tâches

    // Initialisation des URLs et des fichiers de sortie
    tasks[0].url = "https://example.com/file1.zip";
    tasks[0].output_file = "file1.zip";

    tasks[1].url = "https://example.com/file2.zip";
    tasks[1].output_file = "file2.zip";

    tasks[2].url = "https://example.com/file3.zip";
    tasks[2].output_file = "file3.zip";

    // Créer des threads pour télécharger les fichiers simultanément
    for (int i = 0; i < 3; i++) {
        pthread_create(&threads[i], NULL, download_file, (void *)&tasks[i]);
    }

    // Attendre que tous les threads terminent
    for (int i = 0; i < 3; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Tous les téléchargements sont terminés.\n");

    return 0;
}

