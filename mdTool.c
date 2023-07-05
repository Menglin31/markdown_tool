#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <sys/stat.h>

// Function to create a directory
int createDirectory(const char *path) {
    int status = mkdir(path, 0700);
    if (status == 0 || errno == EEXIST) {
        return 0; // Directory created successfully or already exists
    } else if (status == -1) {
        // Check if the error is due to the parent directory not existing
        if (errno == ENOENT) {
            char *parentDir = strdup(path);
            char *lastSlash = strrchr(parentDir, '/');
            if (lastSlash != NULL) {
                *lastSlash = '\0';
                int parentStatus = createDirectory(parentDir);
                free(parentDir);
                if (parentStatus == 0) {
                    // Retry creating the directory
                    status = mkdir(path, 0700);
                    if (status == 0) {
                        return 0; // Directory created successfully
                    }
                }
            }
        }
        printf("Error creating directory: %s\n", path);
    }
    return 1; // Error creating directory
}

// Function to download the figure file
size_t write_callback(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    return fwrite(ptr, size, nmemb, stream);
}

int downloadFigure(const char *figureLink, const char *figurePath) {
    CURL *curl;
    FILE *file;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        file = fopen(figurePath, "wb");
        if (!file) {
            curl_easy_cleanup(curl);
            return 1; // Error opening the file
        }

        curl_easy_setopt(curl, CURLOPT_URL, figureLink);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fclose(file);
            remove(figurePath);
            curl_easy_cleanup(curl);
            return 1; // Error downloading the file
        }

        fclose(file);
        curl_easy_cleanup(curl);
    } else {
        return 1; // Error initializing curl
    }

    return 0; // File downloaded successfully
}

// Function to extract figure links and download the figures
void extractAndDownloadFigures(const char *filename, const char *figureDir) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Error opening the file.\n");
        return;
    }

    createDirectory(figureDir); // Create the figure directory if it doesn't exist

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
            char *figureLink = (char *)malloc(figureLinkEnd - figureLinkStart + 1);
            strncpy(figureLink, figureLinkStart, figureLinkEnd - figureLinkStart);
            figureLink[figureLinkEnd - figureLinkStart] = '\0';

            // Generate a new file path for the figure
            char figurePath[256];
            sprintf(figurePath, "%s/figure%d.png", figureDir, figureCount);

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

// Function to replace figure links with figure paths in the Markdown file
void replaceFigureLinks(const char *filename, const char *figureDir, const char *outputFilename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Error opening the file.\n");
        return;
    }

    FILE *outputFile = fopen(outputFilename, "w");
    if (!outputFile) {
        fclose(file);
        printf("Error creating the output file.\n");
        return;
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    int figureCount = 0; // Start figure numbering from 0

    while ((read = getline(&line, &len, file)) != -1) {
        // Check if the line contains a figure link
        if (strstr(line, "![") != NULL && strstr(line, "](") != NULL && strstr(line, "https://") != NULL) {
            char *figureStart = strstr(line, "![");
            char *figureEnd = strstr(line, "](");
            char *figureLinkStart = figureEnd + 2;
            char *figureLinkEnd = strstr(figureLinkStart, ")");

            // Generate a new file path for the figure
            char figurePath[256];
            sprintf(figurePath, "/%s/figure%d.png", figureDir, figureCount);

            // Replace the figure link with the modified format
            size_t figureLinkLen = figureLinkEnd - figureLinkStart;
            size_t modifiedLineLen = figureStart - line + 3 + strlen(figurePath) + figureLinkLen;
            char *modifiedLine = (char *)malloc(modifiedLineLen + 1);
            snprintf(modifiedLine, modifiedLineLen + 1, "%.*s![](%s)%.*s",
                     (int)(figureStart - line), line, figurePath, (int)figureLinkLen, figureLinkStart);

            fwrite(modifiedLine, modifiedLineLen, 1, outputFile);
            printf("Original Figure Link: %.*s\n", (int)(figureLinkEnd - figureStart + 2), figureStart);
            printf("Modified Figure Path: ![](%s)\n", figurePath);

            figureCount++;

            free(modifiedLine);
        } else {
            fwrite(line, read, 1, outputFile);
        }
    }

    free(line);
    fclose(file);
    fclose(outputFile);
}





int main() {
    const char *inputFilename = "input.md";
    const char *figureDir = "output/figures";
    const char *outputFilename = "output/modified.md";

    // Step 1: Extract figure links and download the figures
    extractAndDownloadFigures(inputFilename, figureDir);

    // Step 2: Replace figure links with figure paths in the Markdown file
    replaceFigureLinks(inputFilename, figureDir, outputFilename);

    return 0;
}
