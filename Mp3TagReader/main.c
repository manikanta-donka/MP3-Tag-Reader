/**
 * @file main.c
 * @brief Main entry point for the MP3 Tag Reader application.
 */

#include <stdio.h>
#include <string.h>
#include "main.h"
#include "id3_reader.h"
#include "id3_writer.h"
#include "error_handling.h"

/**
 * @brief Displays the help message for the MP3 Tag Reader application.
 */
void display_help() {
    printf("Usage: mp3tagreader [options] filename\n");
    printf("Options:\n");
    printf("  -h               Display help\n");
    printf("  -v               View tags\n");
    printf("  -e -t/-T/-a/-A/-y/-c/-g <value>  Edit tags\n");
}

/**
 * @brief Main function to handle command-line arguments and execute appropriate actions.
 * 
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return 0 on success, non-zero on failure.
 */
int main(int argc, char *argv[]) 
{
    if (argc < 2) 
    {
        display_help();
        return 1;
    }

    if (strcmp(argv[1], "-h") == 0) 
    {
        display_help();
        return 0;
    } 
    else if (strcmp(argv[1], "-v") == 0 && argc == 3) 
    {
        view_tags(argv[2]);
    } 
    else if (strcmp(argv[1], "-e") == 0 && argc == 5) 
    {
        const char *tag = NULL;
        
        if (strcmp(argv[2], "-t") == 0) tag = "TIT2"; // Title
        else if (strcmp(argv[2], "-a") == 0) tag = "TPE1"; // Artist
        else if (strcmp(argv[2], "-A") == 0) tag = "TALB"; // Album
        else if (strcmp(argv[2], "-y") == 0) tag = "TYER"; // Year
        else if (strcmp(argv[2], "-g") == 0) tag = "TCON"; // Genre
        else if (strcmp(argv[2], "-c") == 0) tag = "COMM"; // Comment
        else 
        {
            fprintf(stderr, "Invalid tag option: %s\n", argv[2]);
            return 1;
        }

        const char *value = argv[3];
        const char *filename = argv[4];

        if (edit_tag(filename, tag, value) != 0) 
        {
            display_error("Failed to edit tag.");
            return 1;
        }
        printf("Tag edited successfully.\n");
    } 
    else 
    {
        display_help();
        return 1;
    }

    return 0;
}


//1.create temp.mp3 file
//2.copy 10 bytes of header file from origigal file to temp file
//3.read 4 bytes of tag1 and store in temp file
//4.read 4 bytes for size from tag1
//5.read 3 bytes of flag from tag1 and store in temp file
//6.read the contents for (size - 1)bytes and store the new content (size + 1)bytes in temp file
//7.open original file only write mode and copy all contents in temp file to original file

