/**
 * @file id3_reader.c
 * @brief Implementation of functions for reading ID3 tags from MP3 files.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "id3_reader.h"
#include "error_handling.h"

/**
TODO: Add documention as sample given
 */

#define GET_SIZE(size) (((size[0] & 0xFF) << 24) | \
                        ((size[1] & 0xFF) << 16) | \
                        ((size[2] & 0xFF) << 8)  | \
                         (size[3] & 0xFF))

TagData* read_id3_tags(const char *filename) {
    // Implementation for reading ID3 tags

    FILE *file = fopen(filename, "rb");
    if (!file) {
        printf("Faild to open file %s\n",filename);
        return NULL;
    }

    char id3[4] = {0};
    unsigned char vers[2];
    fread(id3, 1, 3, file);

    if (strcmp(id3, "ID3") != 0) {
        display_error("File is not an ID3v2 MP3 file.\n");
        fclose(file);
        return NULL;
    }

    fread(vers, 1, 2, file);
    if (vers[0] != 3) {
        printf("Unsupported ID3 version: %d.%d\n",vers[0],vers[1]);
        fclose(file);
        return NULL;
    }

    int len = strlen(filename);
    if(len < 4 || strcmp(&filename[len - 4], ".mp3") != 0)
    {
        display_error("File extension is not .mp3\n");
        fclose(file);
        return NULL;
    }

    fseek(file, 10, SEEK_SET);

    TagData *data = calloc(1, sizeof(TagData));
    if (!data) {
        display_error("Memory allocation failed.\n");
        fclose(file);
        return NULL;
    }

    char frame_id[5];
    unsigned char frame_size[4];
    unsigned char frame_flags[2];
    unsigned int frame_data_size;
    char *frame_data;

    while (fread(frame_id, 1, 4, file) == 4) {
        frame_id[4] = '\0';
        fread(frame_size, 1, 4, file);
        frame_data_size = GET_SIZE(frame_size);
        fread(frame_flags, 1, 2, file);

        frame_data = (char *)malloc(frame_data_size + 1);
        if (!frame_data) {
            printf("Error: Memory allocation failed for frame data.\n");
            free_tag_data(data);
            fclose(file);
            return NULL;
        }

        fread(frame_data, 1, frame_data_size, file);
        frame_data[frame_data_size] = '\0';

        if (strcmp(frame_id, "TIT2") == 0) {
            data->title = strdup(frame_data + 1);
        } else if (strcmp(frame_id, "TPE1") == 0) {
            data->artist = strdup(frame_data + 1);
        } else if (strcmp(frame_id, "TALB") == 0) {
            data->album = strdup(frame_data + 1);
        } else if (strcmp(frame_id, "TYER") == 0) {
            data->year = strdup(frame_data + 1);
        } else if (strcmp(frame_id, "TCON") == 0) {
            data->genre = strdup(frame_data + 1);
        }

        free(frame_data);
    }

    fclose(file);
    return data;

}

/**
TODO: Add documention as sample given
 */
void display_metadata(const TagData *data) {
    // Implementation for displaying metadata

    if (!data) return;

    printf("-------------SELECTED VIEW DETAILS------------------\n\n");
    printf("----------------------------------------------------\n");
    printf("          MP3 TAG READER FOR ID3v2 TAGS             \n");
    printf("----------------------------------------------------\n");
    printf("Title: %s\n", data->title ? data->title : "Unknown");
    printf("Artist: %s\n", data->artist ? data->artist : "Unknown");
    printf("Album: %s\n", data->album ? data->album : "Unknown");
    printf("Year: %s\n", data->year ? data->year : "Unknown");
    printf("Genre: %s\n", data->genre ? data->genre : "Unknown");
    printf("----------------------------------------------------\n");
    printf("----------DETAILS DISPLAYED SUCCESSFULLY------------\n");
}

/**
TODO: Add documention as sample given
 */
void view_tags(const char *filename) {
    TagData *data = read_id3_tags(filename);
    if (!data) {
        printf("Failed to read ID3 tags.\n");
        return;
    }
    display_metadata(data);
    free_tag_data(data);

    //S1. Check for -v or not if no print error message
    //S2. open original.mp3 file in read mode
    //S3. Check ID3 file or not
    //S4. check version(03 00)
    //S5. Check for extension (.mp3)
    //S6. Read the tag TIT2
    //S7. Read the size
    //S8. Read the Title
    //s9. check for TIT2
    //s10. copy the read title into one variable
    //s11. print the variable
    //s12. repeate the process for other tags


}
