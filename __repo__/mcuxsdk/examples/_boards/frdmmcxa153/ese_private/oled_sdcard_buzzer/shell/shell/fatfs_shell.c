/*! ***************************************************************************
 *
 * \brief     Shell module for FatFS
 * \file      fatfs_shell.c
 * \author    Hugo Arends
 *            Mostly generated using GitHub Copilot
 * \date      February 2026
 *
 * \copyright 2026 HAN University of Applied Sciences. All Rights Reserved.
 *            \n\n
 *            Permission is hereby granted, free of charge, to any person
 *            obtaining a copy of this software and associated documentation
 *            files (the "Software"), to deal in the Software without
 *            restriction, including without limitation the rights to use,
 *            copy, modify, merge, publish, distribute, sublicense, and/or sell
 *            copies of the Software, and to permit persons to whom the
 *            Software is furnished to do so, subject to the following
 *            conditions:
 *            \n\n
 *            The above copyright notice and this permission notice shall be
 *            included in all copies or substantial portions of the Software.
 *            \n\n
 *            THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *            EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 *            OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *            NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 *            HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *            WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *            FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 *            OTHER DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************/
#include <stdio.h>
#include <string.h>
#include "ff.h"
#include "shell.h"
#include "fatfs_shell.h"

#define CMD_BUFFER_SIZE 128

static FATFS fs;
static shell_command_t fatfs_commands[16];
static size_t fatfs_cmd_count = 0;
static shell_context_t *g_shell_ctx = NULL;

void cmd_ls(const char *args) {
    DIR dir;
    FILINFO fno;
    FRESULT res;

    res = f_opendir(&dir, g_shell_ctx->cwd);
    if (res == FR_OK) {
        while (1) {
            res = f_readdir(&dir, &fno);
            if (res != FR_OK || fno.fname[0] == 0) break;
            printf("%s%s\t%lu bytes\n",
                   fno.fname,
                   (fno.fattrib & AM_DIR) ? "/" : "",
                   (unsigned long)(fno.fsize));
        }
        f_closedir(&dir);
    } else {
        printf("Error opening directory: %d\n", res);
    }
}

void cmd_cd(const char *args) {
    DIR dir;
    FRESULT res;
    char path[256];

    if (!args || strlen(args) == 0) {
        printf("Usage: cd <directory>\n");
        return;
    }

    // Normalize the path based on current directory
    shell_normalize_path(g_shell_ctx->cwd, args, path, sizeof(path));

    // Try to open directory to verify it exists
    res = f_opendir(&dir, path);
    if (res == FR_OK) {
        f_closedir(&dir);
        // Update the current working directory
        strncpy(g_shell_ctx->cwd, path, g_shell_ctx->cwd_size - 1);
        g_shell_ctx->cwd[g_shell_ctx->cwd_size - 1] = '\0';
    } else {
        printf("Directory not found: %s (error: %d)\n", path, res);
    }
}

void cmd_cat(const char *args) {
    FIL file;
    FRESULT res;
    char buffer[128];
    char path[256];
    UINT br;

    if (!args) {
        printf("Usage: cat <filename>\n");
        return;
    }

    // Normalize the path
    shell_normalize_path(g_shell_ctx->cwd, args, path, sizeof(path));

    res = f_open(&file, path, FA_READ);
    if (res == FR_OK) {
        while (f_read(&file, buffer, sizeof(buffer) - 1, &br) == FR_OK && br > 0) {
            buffer[br] = '\0';
            printf("%s", buffer);
        }
        printf("\n");
        f_close(&file);
    } else {
        printf("Error opening file: %d\n", res);
    }
}

void cmd_mkdir(const char *args) {
    FRESULT res;
    char path[256];

    if (!args) {
        printf("Usage: mkdir <directory>\n");
        return;
    }

    // Normalize the path
    shell_normalize_path(g_shell_ctx->cwd, args, path, sizeof(path));

    res = f_mkdir(path);
    if (res == FR_OK) {
        printf("Directory created: %s\n", path);
    } else {
        printf("Error creating directory: %d\n", res);
    }
}

void cmd_rm(const char *args) {
    FRESULT res;
    char path[256];

    if (!args) {
        printf("Usage: rm <file>\n");
        return;
    }

    // Normalize the path
    shell_normalize_path(g_shell_ctx->cwd, args, path, sizeof(path));

    res = f_unlink(path);
    if (res == FR_OK) {
        printf("Deleted: %s\n", path);
    } else {
        printf("Error deleting file: %d\n", res);
    }
}

void cmd_touch(const char *args) {
    FIL file;
    FRESULT res;
    char path[256];

    if (!args) {
        printf("Usage: touch <filename>\n");
        return;
    }

    // Normalize the path
    shell_normalize_path(g_shell_ctx->cwd, args, path, sizeof(path));

    res = f_open(&file, path, FA_CREATE_NEW | FA_WRITE);
    if (res == FR_OK) {
        f_close(&file);
        printf("File created: %s\n", path);
    } else if (res == FR_EXIST) {
        printf("File already exists: %s\n", path);
    } else {
        printf("Error creating file: %d\n", res);
    }
}

void cmd_write(const char *args) {
    FIL file;
    char *filename;
    char *content;
    char args_copy[CMD_BUFFER_SIZE];
    char path[256];
    FRESULT res;
    UINT bw;

    if (!args) {
        printf("Usage: write <filename> <content>\n");
        return;
    }

    strncpy(args_copy, args, sizeof(args_copy) - 1);
    args_copy[sizeof(args_copy) - 1] = '\0';

    filename = strtok(args_copy, " ");
    content = strtok(NULL, "");

    if (!filename || !content) {
        printf("Usage: write <filename> <content>\n");
        return;
    }

    // Normalize the path
    shell_normalize_path(g_shell_ctx->cwd, filename, path, sizeof(path));

    res = f_open(&file, path, FA_WRITE | FA_CREATE_ALWAYS);
    if (res == FR_OK) {
        res = f_write(&file, content, strlen(content), &bw);
        f_close(&file);
        if (res == FR_OK) {
            printf("Written %u bytes to %s\n", bw, path);
        } else {
            printf("Error writing file: %d\n", res);
        }
    } else {
        printf("Error opening file: %d\n", res);
    }
}

void cmd_mv(const char *args) {
    char *src;
    char *dst;
    char args_copy[CMD_BUFFER_SIZE];
    char src_path[256];
    char dst_path[256];
    FRESULT res;

    if (!args) {
        printf("Usage: mv <source> <destination>\n");
        return;
    }

    strncpy(args_copy, args, sizeof(args_copy) - 1);
    args_copy[sizeof(args_copy) - 1] = '\0';

    src = strtok(args_copy, " ");
    dst = strtok(NULL, " ");

    if (!src || !dst) {
        printf("Usage: mv <source> <destination>\n");
        return;
    }

    // Normalize paths
    shell_normalize_path(g_shell_ctx->cwd, src, src_path, sizeof(src_path));
    shell_normalize_path(g_shell_ctx->cwd, dst, dst_path, sizeof(dst_path));

    res = f_rename(src_path, dst_path);
    if (res == FR_OK) {
        printf("Moved: %s -> %s\n", src_path, dst_path);
    } else {
        printf("Error moving file: %d\n", res);
    }
}

void fatfs_shell_init(shell_context_t *ctx) {
    FRESULT res = f_mount(&fs, "", 1);

    if (res != FR_OK) {
        printf("Failed to mount FatFS filesystem: %d\n", res);
        return;
    }

    // Store global reference to shell context
    g_shell_ctx = ctx;

    // Register FatFS commands
    fatfs_commands[fatfs_cmd_count].name = "ls";
    fatfs_commands[fatfs_cmd_count].description = "List directory contents";
    fatfs_commands[fatfs_cmd_count].callback = cmd_ls;
    ctx->commands[ctx->command_count++] = fatfs_commands[fatfs_cmd_count++];

    fatfs_commands[fatfs_cmd_count].name = "cd";
    fatfs_commands[fatfs_cmd_count].description = "Change directory";
    fatfs_commands[fatfs_cmd_count].callback = cmd_cd;
    ctx->commands[ctx->command_count++] = fatfs_commands[fatfs_cmd_count++];

    fatfs_commands[fatfs_cmd_count].name = "cat";
    fatfs_commands[fatfs_cmd_count].description = "Display file contents";
    fatfs_commands[fatfs_cmd_count].callback = cmd_cat;
    ctx->commands[ctx->command_count++] = fatfs_commands[fatfs_cmd_count++];

    fatfs_commands[fatfs_cmd_count].name = "mkdir";
    fatfs_commands[fatfs_cmd_count].description = "Create directory";
    fatfs_commands[fatfs_cmd_count].callback = cmd_mkdir;
    ctx->commands[ctx->command_count++] = fatfs_commands[fatfs_cmd_count++];

    fatfs_commands[fatfs_cmd_count].name = "rm";
    fatfs_commands[fatfs_cmd_count].description = "Remove file/directory";
    fatfs_commands[fatfs_cmd_count].callback = cmd_rm;
    ctx->commands[ctx->command_count++] = fatfs_commands[fatfs_cmd_count++];

    fatfs_commands[fatfs_cmd_count].name = "touch";
    fatfs_commands[fatfs_cmd_count].description = "Create empty file";
    fatfs_commands[fatfs_cmd_count].callback = cmd_touch;
    ctx->commands[ctx->command_count++] = fatfs_commands[fatfs_cmd_count++];

    fatfs_commands[fatfs_cmd_count].name = "write";
    fatfs_commands[fatfs_cmd_count].description = "Write text to file";
    fatfs_commands[fatfs_cmd_count].callback = cmd_write;
    ctx->commands[ctx->command_count++] = fatfs_commands[fatfs_cmd_count++];

    fatfs_commands[fatfs_cmd_count].name = "mv";
    fatfs_commands[fatfs_cmd_count].description = "Move/rename file";
    fatfs_commands[fatfs_cmd_count].callback = cmd_mv;
    ctx->commands[ctx->command_count++] = fatfs_commands[fatfs_cmd_count++];

    printf("FatFS shell initialized\n");
}