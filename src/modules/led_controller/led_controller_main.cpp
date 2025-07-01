#include <nuttx/config.h>
#include <sched.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <debug.h>
#include <sys/ioctl.h>
#include "arch/board/board.h"

#include <syslog.h>
#include <unistd.h>

bool g_led_controller_started = false;

static int led_controller(int argc, char *argv[])
{
    g_led_controller_started = true;

    const char *fd_path = "/dev/leds";
    int fd = open(fd_path, O_RDONLY);
    if (fd < 0) {
        int errcode = errno;
        syslog(LOG_ERR, "Led controller: Failed to open %s:%d.\n", fd_path, errcode);
        close(fd);
        return EXIT_FAILURE;
    }

    while (true) {
        static bool status = false;

        ioctl(fd, MSTATUS_LED, status ? 1 : 0);
        status = !status;

        usleep(500 * 1000);
    }

    return EXIT_SUCCESS;
}

extern "C" int main(int argc, FAR char *argv[])
{
    if (g_led_controller_started) {
        printf("led_controller_main: Is already running\n");
        return EXIT_SUCCESS;
    }

    int ret = task_create(CONFIG_MODULES_LED_CONTROLLER_PROGNAME,
                          CONFIG_MODULES_LED_CONTROLLER_PRIORITY,
                          CONFIG_MODULES_LED_CONTROLLER_STACKSIZE,
                          led_controller,
                          NULL);

    if (ret < 0) {
        int errcode = errno;
        printf("led_controller_main: Failed to create led controller: %d.\n", errcode);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
