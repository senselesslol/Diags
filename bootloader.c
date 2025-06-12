#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <mach-o/dyld.h>
#include <sys/stat.h>

#define MAGIC "PAYLOADTABLE"
#define MAGIC_LEN 12

typedef struct {
    long iBSS_offset;
    long iBSS_size;
    long iBEC_offset;
    long iBEC_size;
    long diag_offset;
    long diag_size;
} PayloadTable;

int extract_segment(FILE *f, long offset, long size, const char *out_path) {
    FILE *out = fopen(out_path, "wb");
    if (!out) {
        perror("fopen out");
        return 1;
    }

    fseek(f, offset, SEEK_SET);
    char *buf = malloc(size);
    if (!buf) {
        fprintf(stderr, "Memory allocation failed\n");
        fclose(out);
        return 1;
    }

    fread(buf, 1, size, f);
    fwrite(buf, 1, size, out);
    fclose(out);
    free(buf);
    return 0;
}

int main() {
    char exec_path[1024];
    uint32_t size = sizeof(exec_path);
    _NSGetExecutablePath(exec_path, &size);

    FILE *f = fopen(exec_path, "rb");
    if (!f) {
        perror("fopen self");
        return 1;
    }

    fseek(f, -MAGIC_LEN - sizeof(PayloadTable), SEEK_END);

    char magic[MAGIC_LEN + 1] = {0};
    fread(magic, 1, MAGIC_LEN, f);
    if (strncmp(magic, MAGIC, MAGIC_LEN) != 0) {
        fprintf(stderr, "Payload table not found\n");
        fclose(f);
        return 1;
    }

    PayloadTable pt;
    fread(&pt, 1, sizeof(PayloadTable), f);
    fclose(f);

    f = fopen(exec_path, "rb");

    char tmp_iBSS[] = "/tmp/iBSSXXXXXX";
    char tmp_iBEC[] = "/tmp/iBECXXXXXX";
    char tmp_diag[] = "/tmp/diagXXXXXX";
    int fd;

    fd = mkstemp(tmp_iBSS); close(fd);
    fd = mkstemp(tmp_iBEC); close(fd);
    fd = mkstemp(tmp_diag); close(fd);

    extract_segment(f, pt.iBSS_offset, pt.iBSS_size, tmp_iBSS);
    extract_segment(f, pt.iBEC_offset, pt.iBEC_size, tmp_iBEC);
    extract_segment(f, pt.diag_offset, pt.diag_size, tmp_diag);

    fclose(f);

    printf("Sending iBSS...\n");
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "irecovery -f %s", tmp_iBSS);
    system(cmd);
    sleep(12);

    printf("Sending iBEC...\n");
    snprintf(cmd, sizeof(cmd), "irecovery -f %s", tmp_iBEC);
    system(cmd);
    sleep(2);

    printf("Sending diag...\n");
    snprintf(cmd, sizeof(cmd), "irecovery -f %s", tmp_diag);
    system(cmd);
    sleep(2);

    printf("Booting device...\n");
    system("irecovery -c go");
    printf("Done!\n");

    return 0;
}
