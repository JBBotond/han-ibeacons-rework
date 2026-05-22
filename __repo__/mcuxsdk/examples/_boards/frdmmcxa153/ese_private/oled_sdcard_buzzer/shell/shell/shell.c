/*! ***************************************************************************
 *
 * \brief     Shell framework for embedded applications
 * \file      shell.c
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
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "shell.h"
#include "ff.h"

#define CMD_BUFFER_SIZE (128)
#define BACKSPACE       (0x08)
#define DELETE          (0x7F)
#define ENTER           (0x0D)
#define NEWLINE         (0x0A)
#define TAB             (0x09)

/*!
 * \brief Normalize path with breadcrumb support (../, ./, etc.)
 *
 * \param[in]  base       Base path
 * \param[in]  input      Input path (relative or absolute)
 * \param[out] output     Normalized output path
 * \param[in]  output_size Size of output buffer
 */
void shell_normalize_path(const char *base, const char *input, char *output, size_t output_size) {
    char temp[256];
    char *segments[32];
    int seg_count = 0;
    char *token;
    char work[256];

    // Input validation
    if (!base || !input || !output || output_size == 0) {
        return;
    }

    // Start with base path if input is relative
    if (input[0] == '/') {
        strncpy(temp, input, sizeof(temp) - 1);
    } else {
        snprintf(temp, sizeof(temp), "%s/%s", base, input);
    }
    temp[sizeof(temp) - 1] = '\0';

    // Tokenize path
    strncpy(work, temp, sizeof(work) - 1);
    work[sizeof(work) - 1] = '\0';
    token = strtok(work, "/");

    while (token != NULL && seg_count < 32) {
        if (strcmp(token, ".") == 0) {
            // Current directory, skip
        } else if (strcmp(token, "..") == 0) {
            // Parent directory, pop last segment
            if (seg_count > 0) {
                seg_count--;
            }
        } else if (strlen(token) > 0) {
            // Regular directory name
            segments[seg_count++] = token;
        }
        token = strtok(NULL, "/");
    }

    // Build normalized path
    output[0] = '\0';
    if (seg_count == 0) {
        strncpy(output, "/", output_size - 1);
    } else {
        for (int i = 0; i < seg_count; i++) {
            strncat(output, "/", output_size - strlen(output) - 1);
            strncat(output, segments[i], output_size - strlen(output) - 1);
        }
    }
    output[output_size - 1] = '\0';
}

/*!
 * \brief Find matching commands for autocomplete
 *
 * \param[in]  ctx         Shell context
 * \param[in]  prefix      Prefix to match
 * \param[out] matches     Array of matching command pointers
 * \param[in]  max_matches Maximum number of matches
 *
 * \return Number of matches found
 */
static int find_matching_commands(shell_context_t *ctx, const char *prefix, shell_command_t **matches, int max_matches) {
    int match_count = 0;
    size_t prefix_len;

    if (!ctx || !prefix || !matches || max_matches <= 0) {
        return 0;
    }

    prefix_len = strlen(prefix);

    for (size_t i = 0; i < ctx->command_count && match_count < max_matches; i++) {
        if (strncmp(ctx->commands[i].name, prefix, prefix_len) == 0) {
            matches[match_count++] = &ctx->commands[i];
        }
    }

    return match_count;
}

/*!
 * \brief Find matching files and directories
 *
 * \param[in]  cwd        Current working directory
 * \param[in]  prefix     Full path prefix to match
 * \param[out] matches    Array of matching filename pointers
 * \param[in]  max_matches Maximum number of matches
 *
 * \return Number of matches found
 */
static int find_matching_files(const char *cwd, const char *prefix, char **matches, int max_matches) {
    DIR dir;
    FILINFO fno;
    FRESULT res;
    int match_count = 0;
    char search_dir[256];
    char search_prefix[256];
    char full_path[256];
    char *last_slash;
    size_t prefix_len;

    if (!cwd || !prefix || !matches || max_matches <= 0) {
        return 0;
    }

    // Initialize arrays to NULL
    memset(matches, 0, sizeof(char *) * max_matches);
    memset(search_dir, 0, sizeof(search_dir));
    memset(search_prefix, 0, sizeof(search_prefix));
    memset(full_path, 0, sizeof(full_path));

    // Make a copy of prefix to work with
    strncpy(search_prefix, prefix, sizeof(search_prefix) - 1);
    search_prefix[sizeof(search_prefix) - 1] = '\0';

    // Find the last slash to separate directory from filename
    last_slash = strrchr(search_prefix, '/');

    if (last_slash != NULL) {
        // We have a path with directory (absolute or relative)
        char dir_part[256];
        char file_part[256];

        // Copy directory part
        int dir_len = last_slash - search_prefix;
        strncpy(dir_part, search_prefix, dir_len);
        dir_part[dir_len] = '\0';

        // Copy file part
        strncpy(file_part, last_slash + 1, sizeof(file_part) - 1);
        file_part[sizeof(file_part) - 1] = '\0';

        // Normalize the directory path relative to current directory
        if (dir_part[0] == '/') {
            // Absolute path
            strncpy(search_dir, dir_part, sizeof(search_dir) - 1);
        } else {
            // Relative path - build full path
            shell_normalize_path(cwd, dir_part, search_dir, sizeof(search_dir));
        }
        search_dir[sizeof(search_dir) - 1] = '\0';

        strncpy(search_prefix, file_part, sizeof(search_prefix) - 1);
        search_prefix[sizeof(search_prefix) - 1] = '\0';
    } else {
        // No directory specified, search in current directory
        strncpy(search_dir, cwd, sizeof(search_dir) - 1);
        search_dir[sizeof(search_dir) - 1] = '\0';
        // search_prefix already contains the filename
    }

    prefix_len = strlen(search_prefix);

    res = f_opendir(&dir, search_dir);
    if (res == FR_OK) {
        while (f_readdir(&dir, &fno) == FR_OK && fno.fname[0] != 0) {
            if (strncmp(fno.fname, search_prefix, prefix_len) == 0 && match_count < max_matches) {
                size_t fname_len = strlen(fno.fname);
                matches[match_count] = (char *)malloc(fname_len + 2);
                if (matches[match_count]) {
                    strcpy(matches[match_count], fno.fname);
                    if (fno.fattrib & AM_DIR) {
                        strcat(matches[match_count], "/");
                    }
                    match_count++;
                }
            }
        }
        f_closedir(&dir);
    }

    return match_count;
}

/*!
 * \brief Handle TAB completion for commands and files
 *
 * \param[in]     ctx    Shell context
 * \param[in,out] buffer Input buffer
 * \param[in,out] pos    Current position in buffer
 */
static void handle_tab_completion(shell_context_t *ctx, char *buffer, int *pos) {
    shell_command_t *cmd_matches[32];
    char *file_matches[32];
    int cmd_match_count = 0;
    int file_match_count = 0;
    int i;

    if (!ctx || !buffer || !pos) {
        return;
    }

    // Clear the match arrays
    memset(cmd_matches, 0, sizeof(cmd_matches));
    memset(file_matches, 0, sizeof(file_matches));

    // Extract the current word being typed
    int word_start = *pos - 1;
    while (word_start >= 0 && !isspace((unsigned char)buffer[word_start])) {
        word_start--;
    }
    word_start++;

    char prefix[CMD_BUFFER_SIZE];
    int prefix_len = *pos - word_start;

    if (prefix_len >= CMD_BUFFER_SIZE) {
        prefix_len = CMD_BUFFER_SIZE - 1;
    }

    if (prefix_len < 0) {
        prefix_len = 0;
    }

    memset(prefix, 0, sizeof(prefix));
    strncpy(prefix, &buffer[word_start], prefix_len);
    prefix[prefix_len] = '\0';

    // Count spaces to determine if this is command (first word) or argument (subsequent words)
    int space_count = 0;
    for (i = 0; i < word_start; i++) {
        if (isspace((unsigned char)buffer[i])) {
            space_count++;
        }
    }

    if (space_count == 0) {
        // First word - complete commands only
        cmd_match_count = find_matching_commands(ctx, prefix, cmd_matches, 32);

        if (cmd_match_count == 0) {
            printf("\a");  // Bell sound
            fflush(stdout);
        } else if (cmd_match_count == 1) {
            // Single match - autocomplete
            const char *completion = cmd_matches[0]->name + prefix_len;
            printf("%s ", completion);
            fflush(stdout);
            strcpy(&buffer[*pos], completion);
            *pos += strlen(completion);
            buffer[*pos++] = ' ';
        } else {
            // Multiple matches - show them
            printf("\n");
            for (i = 0; i < cmd_match_count; i++) {
                printf("  %-15s - %s\n", cmd_matches[i]->name, cmd_matches[i]->description);
            }
            printf("%s> %s", ctx->cwd, buffer);
            fflush(stdout);
        }
    } else {
        // Subsequent words - complete files/directories
        file_match_count = find_matching_files(ctx->cwd, prefix, file_matches, 32);

        if (file_match_count == 0) {
            printf("\a");  // Bell sound
            fflush(stdout);
        } else if (file_match_count == 1) {
            // Single match - autocomplete
            size_t search_prefix_len = 0;
            char *last_slash = strrchr(prefix, '/');

            if (last_slash != NULL) {
                // We have a path, get only the filename part that was typed
                search_prefix_len = strlen(last_slash + 1);
            } else {
                // No path, search_prefix_len is the whole prefix
                search_prefix_len = strlen(prefix);
            }

            const char *completion = file_matches[0] + search_prefix_len;
            printf("%s", completion);
            fflush(stdout);

            if (strlen(completion) > 0) {
                strcpy(&buffer[*pos], completion);
                *pos += strlen(completion);
            }
        } else {
            // Multiple matches - show them
            printf("\n");
            for (i = 0; i < file_match_count; i++) {
                printf("  %s\n", file_matches[i]);
            }
            printf("%s> %s", ctx->cwd, buffer);
            fflush(stdout);
        }

        // Free allocated memory
        for (i = 0; i < file_match_count; i++) {
            if (file_matches[i] != NULL) {
                free(file_matches[i]);
                file_matches[i] = NULL;
            }
        }
    }
}

/*!
 * \brief Read line with character echo, backspace, and TAB completion
 *
 * \param[in]  ctx     Shell context
 * \param[out] buffer  Output buffer
 * \param[in]  max_len Maximum buffer length
 *
 * \return Length of line read
 */
static int read_line(shell_context_t *ctx, char *buffer, int max_len) {
    int pos = 0;
    char ch;

    if (!ctx || !buffer || max_len <= 0) {
        return 0;
    }

    memset(buffer, 0, max_len);

    while (1) {
        ch = getchar();

        // Handle TAB completion
        if (ch == TAB) {
            handle_tab_completion(ctx, buffer, &pos);
        }
        // Handle backspace or delete
        else if (ch == BACKSPACE || ch == DELETE) {
            if (pos > 0) {
                pos--;
                printf("\b \b");
                fflush(stdout);
            }
        }
        // Handle enter/return
        else if (ch == ENTER || ch == NEWLINE) {
            buffer[pos] = '\0';
            printf("\n");
            fflush(stdout);
            return pos;
        }
        // Handle printable characters
        else if (isprint((unsigned char)ch) && pos < max_len - 1) {
            buffer[pos++] = ch;
            putchar(ch);
            fflush(stdout);
        }
        // Ignore other control characters
    }
}

/*!
 * \brief Built-in help command
 *
 * \param[in] ctx  Shell context
 * \param[in] args Arguments (unused)
 */
static void cmd_help_builtin(shell_context_t *ctx, const char *args) {
    (void)args;  // Suppress unused parameter warning

    if (!ctx) {
        return;
    }

    printf("Available commands:\n");
    for (size_t i = 0; i < ctx->command_count; i++) {
        printf("  %-15s - %s\n", ctx->commands[i].name, ctx->commands[i].description);
    }
}

/*!
 * \brief Built-in pwd command
 *
 * \param[in] ctx  Shell context
 * \param[in] args Arguments (unused)
 */
static void cmd_pwd_builtin(shell_context_t *ctx, const char *args) {
    (void)args;  // Suppress unused parameter warning

    if (!ctx) {
        return;
    }

    printf("%s\n", ctx->cwd);
}

/*!
 * \brief Built-in exit command
 *
 * \param[in] ctx  Shell context
 * \param[in] args Arguments (unused)
 */
static void cmd_exit_builtin(shell_context_t *ctx, const char *args) {
    (void)args;  // Suppress unused parameter warning
    (void)ctx;   // Suppress unused parameter warning

    printf("Exiting shell...\n");
}

/*!
 * \brief Initialize the shell context
 *
 * \param[in,out] ctx       Shell context to initialize
 * \param[in]     commands  Command array
 * \param[in]     cmd_count Number of commands
 * \param[in]     cwd_buffer Current working directory buffer
 * \param[in]     cwd_size  Size of cwd buffer
 */
void shell_init(shell_context_t *ctx, shell_command_t *commands, size_t cmd_count, char *cwd_buffer, size_t cwd_size) {
    if (!ctx || !commands || !cwd_buffer || cwd_size == 0) {
        return;
    }

    ctx->commands = commands;
    ctx->command_count = cmd_count;
    ctx->cwd = cwd_buffer;
    ctx->cwd_size = cwd_size;

    // Initialize working directory
    memset(ctx->cwd, 0, ctx->cwd_size);
    strncpy(ctx->cwd, "/", ctx->cwd_size - 1);
    ctx->cwd[ctx->cwd_size - 1] = '\0';
}

/*!
 * \brief Run the shell main loop
 *
 * \param[in] ctx Shell context
 */
void shell_run(shell_context_t *ctx) {
    char cmd_buffer[CMD_BUFFER_SIZE];
    char *token;
    char *args;
    int len;
    int found;

    if (!ctx) {
        return;
    }

    printf("\nEmbedded shell framework\n");
    printf("Type 'help' for available commands\n");
    printf("(Supports TAB for auto-completion of commands and files, backspace to edit)\n\n");

    while (1) {
        printf("%s> ", ctx->cwd);
        fflush(stdout);

        len = read_line(ctx, cmd_buffer, CMD_BUFFER_SIZE);

        if (len == 0) {
            continue;
        }

        // Make a copy for strtok since it modifies the string
        char cmd_copy[CMD_BUFFER_SIZE];
        strncpy(cmd_copy, cmd_buffer, CMD_BUFFER_SIZE - 1);
        cmd_copy[CMD_BUFFER_SIZE - 1] = '\0';

        token = strtok(cmd_copy, " ");
        if (token == NULL) {
            continue;
        }

        args = strtok(NULL, "");

        // Check for built-in commands
        if (strcmp(token, "help") == 0) {
            cmd_help_builtin(ctx, args);
            continue;
        } else if (strcmp(token, "pwd") == 0) {
            cmd_pwd_builtin(ctx, args);
            continue;
        } else if (strcmp(token, "exit") == 0) {
            cmd_exit_builtin(ctx, args);
            break;
        }

        // Search for registered command
        found = 0;
        for (size_t i = 0; i < ctx->command_count; i++) {
            if (strcmp(token, ctx->commands[i].name) == 0) {
                ctx->commands[i].callback(args);
                found = 1;
                break;
            }
        }

        if (!found) {
            printf("Unknown command: %s\n", token);
            printf("Type 'help' for available commands\n");
        }
    }
}

/*!
 * \brief Register a command with the shell
 *
 * \param[in,out] ctx      Shell context
 * \param[in]     name     Command name
 * \param[in]     desc     Command description
 * \param[in]     callback Command callback function
 */
void shell_register_command(shell_context_t *ctx, const char *name, const char *desc, void (*callback)(const char *)) {
    if (!ctx || !name || !desc || !callback) {
        return;
    }

    if (ctx->command_count < 32) {  // Limit to 32 commands
        ctx->commands[ctx->command_count].name = name;
        ctx->commands[ctx->command_count].description = desc;
        ctx->commands[ctx->command_count].callback = callback;
        ctx->command_count++;
    }
}