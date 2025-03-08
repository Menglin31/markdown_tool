# markdown_tool

## Project Introduction
This project is a Markdown tool written in C, aimed at providing simple Markdown text processing functions. The main features include parsing Markdown syntax and generating corresponding output formats.
When downloading Markdown source files from the internet, images are usually not downloaded together. This tool can download images embedded in the article and update the image relative paths in the article.

## Installation Steps
1. Ensure that CMake and GCC compiler are installed.
2. Download or clone this project locally.
3. Create a build directory in the project root directory:
   ```bash
   mkdir build
   cd build
   ```
4. Configure the project using CMake:
   ```bash
   cmake ..
   ```
5. Compile the project:
   ```bash
   make
   ```

## Usage Instructions
After compilation, you can use the generated executable `mdTool` to process Markdown files. The usage is as follows:
```bash
./mdTool <input_file.md> <output_file>
```
This will convert `input_file.md` to the specified `output_file` format.