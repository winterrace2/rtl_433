/*
 * rtl_433, turns your Realtek RTL2832 based DVB dongle into a 433.92MHz generic data receiver
 * Copyright (C) 2012 by Benjamin Larsson <benjamin@southpole.se>
 *
 * Based on rtl_sdr
 *
 * Copyright (C) 2012 by Steve Markgraf <steve@steve-m.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <stdbool.h>

#include "librtl_433.h"
#include "configure.h"

static rtl_433_t        *rtl = 0;

// TODO: SIGINFO is not in POSIX...
#ifndef SIGINFO
#define SIGINFO 29
#endif

#ifdef _WIN32
BOOL WINAPI sighandler(int signum) {
    if (CTRL_C_EVENT == signum) {
        fprintf(stderr, "Signal caught, exiting!\n");
        if (rtl) stop_signal(rtl);
        else fprintf(stderr, "Error: No rtl_433 instance.\n");
        return TRUE;
    }
    return FALSE;
}
#else
static void sighandler(int signum) {
    if (signum == SIGPIPE) {
        signal(SIGPIPE, SIG_IGN);
    }
    else if (signum == SIGINFO/* TODO: maybe SIGUSR1 */) {
        if(rtl) rtl->cfg->stats_now++;
        return;
    }
    else if (signum == SIGALRM) {
        fprintf(stderr, "Async read stalled, exiting!\n");
    }
    else {
        fprintf(stderr, "Signal caught, exiting!\n");
    }
    if (rtl) stop_signal(rtl);
}
#endif

int main(int argc, char **argv) {
    int r = 1;
    struct sigaction *sigact_ptr = NULL;
#ifndef _WIN32
    struct sigaction sigact;
    sigact_ptr = &sigact;
#endif

    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

#ifndef _WIN32
    sigact.sa_handler = sighandler;
    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags = 0;
    sigaction(SIGINT, &sigact, NULL);
    sigaction(SIGTERM, &sigact, NULL);
    sigaction(SIGQUIT, &sigact, NULL);
    sigaction(SIGPIPE, &sigact, NULL);
    sigaction(SIGINFO, &sigact, NULL);
#else
    SetConsoleCtrlHandler((PHANDLER_ROUTINE)sighandler, TRUE);
#endif

    rtl_433_init(&rtl);
    if (rtl >= 0) {
        // Load settings (command line and/or config file)
        r = configure_librtl433(rtl->cfg, argc, argv, 1);

        // Start rtl_433
        if (r == CFG_SUCCESS_GO_ON) {
            r = start(rtl, sigact_ptr);
        }

        // clean up
        rtl_433_destroy(rtl);
        rtl = NULL;
    }
    return r;
}
