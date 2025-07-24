#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdbool.h>
#include <math.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define HASH_SIZE 64
#define SCALED_SIZE 8
#define IMAGE_PATH_MAX 1024
#define MAX_FILES 1000

typedef struct {
    char path[IMAGE_PATH_MAX];
    unsigned long long hash;
} ImageHash;


int hash_distance(unsigned long long h1, unsigned long long h2) {
    unsigned long long diff = h1 ^ h2;
    int distance = 0;
    
    while (diff) {
        distance += diff & 1;
        diff >>= 1;
    }
    
    return distance;
}


unsigned long long calculate_phash(const char* filename) {
    int width, height, channels;
    unsigned long long hash = 0;
    
    // Load the image using stb_image
    unsigned char* img = stbi_load(filename, &width, &height, &channels, 1); // Convert to grayscale
    if (!img) {
        printf("Error loading image %s\n", filename);
        return 0;
    }
    
    // Scale down to SCALED_SIZE x SCALED_SIZE
    double scaled[SCALED_SIZE][SCALED_SIZE];
    double pixel_sum = 0;
    
    for (int i = 0; i < SCALED_SIZE; i++) {
        for (int j = 0; j < SCALED_SIZE; j++) {
            scaled[i][j] = 0;
            int samples = 0;
            
            // Average the pixels in the corresponding block
            for (int x = i * width / SCALED_SIZE; x < (i + 1) * width / SCALED_SIZE; x++) {
                for (int y = j * height / SCALED_SIZE; y < (j + 1) * height / SCALED_SIZE; y++) {
                    if (x < width && y < height) {
                        scaled[i][j] += img[y * width + x];
                        samples++;
                    }
                }
            }
            
            if (samples > 0) {
                scaled[i][j] /= samples;
                pixel_sum += scaled[i][j];
            }
        }
    }
    
    // Calculate average pixel value
    double avg = pixel_sum / (SCALED_SIZE * SCALED_SIZE);
    
    // Create a 64-bit hash based on whether each pixel is above or below the average
    for (int i = 0; i < SCALED_SIZE; i++) {
        for (int j = 0; j < SCALED_SIZE; j++) {
            if (scaled[i][j] > avg) {
                hash |= 1ULL << (i * SCALED_SIZE + j);
            }
        }
    }
    
    stbi_image_free(img);
    return hash;
}

int is_image_file(const char* filename) {
    const char* extensions[] = {".jpg", ".jpeg", ".png", ".bmp", ".gif", ".tiff", NULL};
    const char* ext = strrchr(filename, '.');
    
    if (!ext) return 0;
    
    for (int i = 0; extensions[i] != NULL; i++) {
        if (strcasecmp(ext, extensions[i]) == 0) {
            return 1;
        }
    }
    
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <directory>\n", argv[0]);
        return 1;
    }
    
    DIR* dir;
    struct dirent* entry;
    ImageHash hashes[MAX_FILES];
    int file_count = 0;
    
    dir = opendir(argv[1]);
    if (!dir) {
        perror("Error opening directory");
        return 1;
    }
    
    printf("Scanning directory: %s\n", argv[1]);
    
    // Process all image files in the directory
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG && is_image_file(entry->d_name)) {
            char full_path[IMAGE_PATH_MAX];
            snprintf(full_path, IMAGE_PATH_MAX, "%s/%s", argv[1], entry->d_name);
            
            printf("Processing: %s\n", entry->d_name);
            
            // Calculate perceptual hash
            unsigned long long hash = calculate_phash(full_path);
            
            if (hash != 0) {
                strcpy(hashes[file_count].path, full_path);
                hashes[file_count].hash = hash;
                file_count++;
                
                if (file_count >= MAX_FILES) {
                    printf("Warning: Maximum file limit reached (%d)\n", MAX_FILES);
                    break;
                }
            }
        }
    }
    
    closedir(dir);
    
    printf("\nFound %d images\n", file_count);
    
    // Compare hashes to find similar images
    printf("\nPossible duplicates (threshold distance <= 8):\n");
    for (int i = 0; i < file_count; i++) {
        for (int j = i + 1; j < file_count; j++) {
            int distance = hash_distance(hashes[i].hash, hashes[j].hash);
            
            // Images with small hash distance are likely duplicates
            if (distance <= 8) {
                printf("Similar images:\n");
                printf("  %s\n  %s\n", hashes[i].path, hashes[j].path);
                printf("  Distance: %d\n\n", distance);
            }
        }
    }
    
    return 0;
}