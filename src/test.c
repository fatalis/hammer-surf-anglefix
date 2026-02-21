#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <keyvalues.h>

// mostly chatgpt code

char *anglefix_generate_output(const char *path, size_t *out_length);

static void test_fail(const char *msg) {
    fprintf(stderr, "FAIL: %s\n", msg);
}

static void test_pass(const char *msg) {
    printf("PASS: %s\n", msg);
}

int main(int argc, const char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <input_vmf> <expected_output_vmf>\n", argv[0]);
        return 1;
    }

    const char *input_vmf = argv[1];
    const char *expected_vmf = argv[2];

    // Check if input file exists (sanity)
    FILE *f = fopen(input_vmf, "rb");
    if (!f) {
        fprintf(stderr, "Input file does not exist: %s\n", input_vmf);
        test_fail("Input file not found");
        return 1;
    }
    fclose(f);

    // Run anglefix logic in-process and get the output as a string
    char *output = anglefix_generate_output(input_vmf, NULL);

    // Read expected output file into memory
    FILE *expected = fopen(expected_vmf, "rb");
    if (!expected) {
        fprintf(stderr, "Expected file does not exist: %s\n", expected_vmf);
        KV_free(output);
        test_fail("Expected file not found");
        return 1;
    }

    if (fseek(expected, 0, SEEK_END) != 0) {
        fclose(expected);
        KV_free(output);
        test_fail("Failed to seek in expected file");
        return 1;
    }

    long expected_size_long = ftell(expected);
    if (expected_size_long < 0) {
        fclose(expected);
        KV_free(output);
        test_fail("Failed to tell size of expected file");
        return 1;
    }

    size_t expected_size = (size_t)expected_size_long;
    if (fseek(expected, 0, SEEK_SET) != 0) {
        fclose(expected);
        KV_free(output);
        test_fail("Failed to rewind expected file");
        return 1;
    }

    /* +1 for NUL so we can use string functions. */
    char *expected_buf = (char *)malloc(expected_size + 1);
    if (!expected_buf) {
        fclose(expected);
        KV_free(output);
        test_fail("Out of memory reading expected file");
        return 1;
    }

    size_t read_size = fread(expected_buf, 1, expected_size, expected);
    fclose(expected);
    if (read_size != expected_size) {
        free(expected_buf);
        KV_free(output);
        test_fail("Failed to read entire expected file");
        return 1;
    }
    expected_buf[expected_size] = '\0';

    printf("Comparing in-memory output with %s\n", expected_vmf);

    size_t output_len = strlen(output);

    // Compare lengths and contents using string functions
    if (output_len == expected_size && strcmp(output, expected_buf) == 0) {
        test_pass("Output matches expected");
        free(expected_buf);
        KV_free(output);
        return 0;
    } else {
        test_fail("Output does not match expected");

        /* Write fail output to a file in the current directory so the user can diff manually. */
        const char *suffix = ".fail";
        const char *slash1 = strrchr(expected_vmf, '/');
        const char *slash2 = strrchr(expected_vmf, '\\');
        const char *slash = (slash1 > slash2) ? slash1 : slash2;
        const char *filename = (slash) ? slash + 1 : expected_vmf;
        size_t filename_len = strlen(filename);
        size_t suffix_len = strlen(suffix);
        char *fail_path = (char *)malloc(filename_len + suffix_len + 1);

        if (!fail_path) {
            fprintf(stderr, "Out of memory while preparing fail output path\n");
        } else {
            memcpy(fail_path, filename, filename_len);
            memcpy(fail_path + filename_len, suffix, suffix_len + 1);

            FILE *out_file = fopen(fail_path, "wb");
            if (!out_file) {
                fprintf(stderr, "Failed to write fail output file: %s\n", fail_path);
            } else {
                /* Use string I/O since 'output' is NUL-terminated text. */
                if (fputs(output, out_file) == EOF) {
                    fprintf(stderr, "Failed to write fail output contents to: %s\n", fail_path);
                }
                fclose(out_file);

                printf("Wrote fail output to: %s\n", fail_path);
                printf("Example diff command (Linux):\n");
                printf("  diff -u '%s' '%s'\n", expected_vmf, fail_path);
            }

            free(fail_path);
        }

        free(expected_buf);
        KV_free(output);
        return 1;
    }
}
