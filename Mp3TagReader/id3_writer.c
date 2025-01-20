/**
 * @file id3_writer.c
 * @brief Implementation of functions for writing and editing ID3 tags in MP3 files.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "id3_writer.h"
#include "id3_reader.h"
#include "id3_utils.h"
#include "error_handling.h"

/**
 * @brief Writes the ID3 tags to an MP3 file.
 * 
 * @param filename The name of the MP3 file.
 * @param data Pointer to the TagData structure containing the ID3 tags.
 * @return 0 on success, non-zero on failure.
 */

#define GET_SIZE(size) (((size[0] & 0xFF) << 24) | \
                        ((size[1] & 0xFF) << 16) | \
                        ((size[2] & 0xFF) << 8)  | \
                         (size[3] & 0xFF))

#define FIX_SIZE(size, new_len) do { \
    size[0] = (new_len >> 24) & 0xFF; \
    size[1] = (new_len >> 16) & 0xFF; \
    size[2] = (new_len >> 8) & 0xFF; \
    size[3] = new_len & 0xFF; \
} while (0)

int write_id3_tags(const char *filename, const TagData *data) {
    if (!filename || !data) {
        fprintf(stderr, "Invalid arguments passed to write_id3_tags.\n");
        return 1;
    }

    FILE *file_src = fopen(filename, "rb");
    if (!file_src) {
        fprintf(stderr, "Error: Could not open file %s for reading.\n", filename);
        return 1;
    }

    FILE *file_dest = fopen("temp.mp3", "wb");
    if (!file_dest) {
        display_error("Could not create a temporary file for writing.\n");
        fclose(file_src);
        return 1;
    }

    char id3_header[10] = {0};
    if (fread(id3_header, 1, 10, file_src) != 10) {
        display_error("Failed to read the ID3 header. Assuming no existing ID3 tags.\n");

        // Write a new ID3 header
        id3_header[0] = 'I'; id3_header[1] = 'D'; id3_header[2] = '3';
        id3_header[3] = 3;  // Version: ID3v2.3.0
        id3_header[4] = 0;  // Flags
    }
    fwrite(id3_header, 1, 10, file_dest);

    // Write new tags
    const struct {
        const char *id;
        const char *value;
    } frames[] = {
        {"TIT2", data->title},
        {"TPE1", data->artist},
        {"TALB", data->album},
        {"TYER", data->year},
        {"TCON", data->genre},
        {"COMM", data->comment},
    };

    for (int i = 0; i < sizeof(frames) / sizeof(frames[0]); i++) {
        if (frames[i].value) {
            unsigned int frame_size = strlen(frames[i].value) + 1; // +1 for encoding byte
            unsigned char frame_header[10] = {0};
            memcpy(frame_header, frames[i].id, 4);
            frame_header[4] = (frame_size >> 24) & 0xFF;
            frame_header[5] = (frame_size >> 16) & 0xFF;
            frame_header[6] = (frame_size >> 8) & 0xFF;
            frame_header[7] = frame_size & 0xFF;

            fwrite(frame_header, 1, 10, file_dest);
            fputc(0x00, file_dest); // Encoding byte
            fwrite(frames[i].value, 1, frame_size - 1, file_dest);
        }
    }

    // Copy the rest of the file
    char buffer[1024];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file_src)) > 0) {
        fwrite(buffer, 1, bytes_read, file_dest);
    }

    fclose(file_src);
    fclose(file_dest);

    if (rename("temp.mp3", filename) != 0) {
        display_error("Failed to replace the original file with the updated file.\n");
        return 1;
    }

    printf("ID3 tags written successfully to %s.\n", filename);
    return 0;
}

/** TODO: Add documentation as sample given above */
int edit_tag(const char *filename, const char *tag, const char *value) 
{

    TagData *data = read_id3_tags(filename);
    if(!data)
    {
        return 1;
    }

    if (!filename || !tag || !value) 
    {
        display_error("Invalid arguments passed to edit_tag.\n");
        return 1;
    }    

    FILE *file_src = fopen(filename, "rb");
    if (!file_src) 
    {
        display_error("Failed to open source file.\n");
        return 1;
    }

    FILE *file_temp = fopen("temp.mp3", "wb");
    if (!file_temp) 
    {
        display_error("Failed to create temporary file.\n");
        fclose(file_src);
        return 1;
    }
    
    char buffer[10];
    fread(buffer, 1, 10, file_src); // Copy header
    fwrite(buffer, 1, 10, file_temp);

    char frame_id[5];
    unsigned char frame_size[4];
    unsigned char frame_flags[2];
    char *frame_data;
    int modified = 0;
    unsigned int frame_data_size;

    while (fread(frame_id, 1, 4, file_src) == 4) 
    {
        frame_id[4] = '\0';

        fread(frame_size, 1, 4, file_src);
        frame_data_size = GET_SIZE(frame_size);
        fread(frame_flags, 1, 2, file_src);

        frame_data = malloc(frame_data_size);
        if (!frame_data) 
        {
            display_error("Memory allocation failed.\n");
            fclose(file_src);
            fclose(file_temp);
            return 1;
        }

        fread(frame_data, 1, frame_data_size, file_src);

        if (!modified && strcmp(frame_id, tag) == 0) 
        {
            unsigned int new_size = strlen(value) + 1; // +1 for encoding byte
            FIX_SIZE(frame_size, new_size);

            fwrite(frame_id, 1, 4, file_temp);         // Write frame ID
            fwrite(frame_size, 1, 4, file_temp);       // Write new size
            fwrite(frame_flags, 1, 2, file_temp);      // Write flags
            fputc(0x00, file_temp);                   // Encoding byte
            fwrite(value, 1, new_size - 1, file_temp);// New frame data
            modified = 1;
        } 
        else 
        {
            fwrite(frame_id, 1, 4, file_temp);        // Write unmodified frame
            fwrite(frame_size, 1, 4, file_temp);
            fwrite(frame_flags, 1, 2, file_temp);
            fwrite(frame_data, 1, frame_data_size, file_temp);
        }

        free(frame_data);
    }

    fclose(file_src);
    fclose(file_temp);

    if (rename("temp.mp3", filename) != 0) 
    {
        display_error("Failed to replace the original file.\n");
        return 1;
    }

    if (!modified) 
    {
        fprintf(stderr, "Tag %s not found.\n", tag);
    }

    return 0;
}
