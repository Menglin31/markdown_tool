#include <curl/curl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define DIR_LEN 100
#define FILENAME_LEN 100

void removeExtension(const char *filename, char *dirname) {
    char *lastDot = strrchr(filename, '.');
    if (lastDot != NULL) {
        size_t length = lastDot - filename;
        strncpy(dirname, filename, length);
        dirname[length] = '\0';
    } else {
        strcpy(dirname, filename);
    }
}

int createDirectory(char *path) {
    if (mkdir(path, 0777) == -1) {
        printf("Failed to create directory: %s\n", path);
        return 1;
    } else {
        printf("Created directory: %s\n", path);
    }
    return 0;
}

size_t write_callback(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    return fwrite(ptr, size, nmemb, stream);
}

int downloadFigure(char *figureLink, char *figurePath) {
    CURL *curl = curl_easy_init();
    if (!curl) {
        return 1;  // Error initializing curl
    }

    FILE *file = fopen(figurePath, "wb");
    if (!file) {
        curl_easy_cleanup(curl);
        return 1;  // Error opening the file
    }

    curl_easy_setopt(curl, CURLOPT_URL, figureLink);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);

    CURLcode res = curl_easy_perform(curl);
    fclose(file);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        remove(figurePath);
        return 1;  // Error downloading the file
    }

    return 0;  // File downloaded successfully
}

// Function to extract figure links and download the figures
void extractAndDownloadFigures(char *filename, char *figureDir) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Error opening the file.\n");
        return;
    }

    // Create the figure directory if it doesn't exist
    createDirectory(figureDir);

    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    int figureCount = 0;

    while ((read = getline(&line, &len, file)) != -1) {
        // Check if the line contains a figure link
        if (strstr(line, "![") != NULL && strstr(line, "](") != NULL) {
            char *figureEnd = strstr(line, "](");
            char *figureLinkStart = figureEnd + 2;
            char *figureLinkEnd = strstr(figureLinkStart, ")");

            // Extract the figure link
            size_t figureLinkLen = figureLinkEnd - figureLinkStart;
            char *figureLink = malloc(figureLinkLen + 1);
            strncpy(figureLink, figureLinkStart, figureLinkLen);
            figureLink[figureLinkLen] = '\0';

            // Generate a new file path for the figure
            char figurePath[64];
            snprintf(figurePath, sizeof(figurePath), "%s/figure%d.png",
                     figureDir, figureCount);

            printf("Extracted Figure Link: %s\n", figureLink);
            printf("Stored Figure Path: %s\n", figurePath);

            // Download the figure file
            int downloadResult = downloadFigure(figureLink, figurePath);
            if (downloadResult != 0) {
                printf("Error downloading the figure file.\n");
            }

            figureCount++;

            free(figureLink);
        }
    }

    free(line);
    fclose(file);
}
void gentmpfile(const char *inputFilename, const char *outputFilename) {
    FILE *inputFile = fopen(inputFilename, "r");
    if (!inputFile) {
        printf("Error opening the input file.\n");
        return;
    }

    FILE *outputFile = fopen(outputFilename, "w");
    if (!outputFile) {
        fclose(inputFile);
        printf("Error creating the output file.\n");
        return;
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&line, &len, inputFile)) != -1) {
        fputs(line, outputFile);  // Write the original line to the output file

        // Check if the line contains a figure link
        if (strstr(line, "![") != NULL && strstr(line, "](") != NULL) {
            // Add a new line after the figure link
            fputs("\n", outputFile);
        }
    }

    free(line);
    fclose(inputFile);
    fclose(outputFile);
}

void replaceFigureLinks(char *inputFileName, char *figureDir, char *outputFileName) {
    FILE *inputFile = fopen(inputFileName, "r");
    if (!inputFile) {
        printf("Error opening the input file.\n");
        return;
    }

    FILE *outputFile = fopen(outputFileName, "w");
    if (!outputFile) {
        fclose(inputFile);
        printf("Error creating the output file.\n");
        return;
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    int figureCount = 0;

    while ((read = getline(&line, &len, inputFile)) != -1) {
        char *figureStart = strstr(line, "![");
        if (figureStart != NULL && strstr(figureStart, "](") != NULL) {
            char *figureLinkStart = strstr(figureStart, "](") + 2;
            char *figureLinkEnd = strstr(figureLinkStart, ")");

            // Extract the figure link
            size_t figureLinkLen = figureLinkEnd - figureLinkStart;
            char *figureLink = malloc(figureLinkLen + 1);
            strncpy(figureLink, figureLinkStart, figureLinkLen);
            figureLink[figureLinkLen] = '\0';

            // Generate a new figure path
            char figurePath[64];
            snprintf(figurePath, sizeof(figurePath), "%s/figure%d.png",
                     figureDir, figureCount);

            // Replace the figure link in the line
            char *replacement = malloc(64 + 1 + figureLinkLen + 1);
            snprintf(replacement, 64 + 1 + figureLinkLen + 1, "![](%s)", figurePath);
            char *figureEnd = figureLinkEnd + 1;
            memmove(figureStart, replacement, strlen(replacement));
            memmove(figureStart + strlen(replacement), figureEnd, strlen(figureEnd) + 1);

            // Write the modified line to the output file
            fputs(line, outputFile);

            // Download the figure file
            int downloadResult = downloadFigure(figureLink, figurePath);
            if (downloadResult != 0) {
                printf("Error downloading the figure file.\n");
            }

            figureCount++;

            free(figureLink);
            free(replacement);
        } else {
            // Write the original line to the output file
            fputs(line, outputFile);
        }
    }

    free(line);
    fclose(inputFile);
    fclose(outputFile);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <inputfile.md>\n", argv[0]);
        return 1;
    }

    // xxx.md
    char *inputFileName = argv[1];
    // char *tmpfilename = "tmp.md";
    char dirname[FILENAME_LEN];

    // xxx/xxx_fig
    char figureDir[DIR_LEN];

    // xxx/xxx.md
    char outputFileName[FILENAME_LEN];

    // xxx.md to xxx
    removeExtension(inputFileName, dirname);

    snprintf(figureDir, sizeof(dirname) * 2 + 5, "%s/%s_fig", dirname, dirname);
    snprintf(outputFileName, sizeof(dirname) + sizeof(inputFileName) + 1,
             "%s/%s", dirname, inputFileName);

    createDirectory(dirname);
    createDirectory(figureDir);
    extractAndDownloadFigures(inputFileName, figureDir);
    replaceFigureLinks(inputFileName, figureDir, outputFileName);
    return 0;
}
