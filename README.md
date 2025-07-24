# Image_Deduplication_Algorithm
Uses perceptual hashing to identify similar images, even if they've been resized, slightly modified, or saved in different formats (Uses std_image.h header library). 
Here's how it functions:

### 1. Perceptual Hashing - Unlike cryptographic hashes (MD5, SHA), perceptual hashes identify visually similar content:
- Reduce each image to 8x8 grayscale
- Calculate the average pixel value
- Create a 64-bit hash where each bit represents if a pixel is above/below average

### 2. Image Comparison - Images are compared using Hamming distance (number of different bits) between their hashes

# Features:
- Processes common image formats (JPG, PNG, BMP, GIF, TIFF)
- Configurable similarity threshold
- Reports duplicate sets with similarity scores
