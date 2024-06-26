
#ifndef COMMAND_EXECUTOR_H
#define COMMAND_EXECUTOR_H
#include <signal.h>
#include "prompt.h"
#include "signal_handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include "command_parser.h"
#include "job_manager.h"

void execute_command(char *command, char *args[]);

void printf_r(char ** args);
char* concatenate_arguments(char *args[]);

void restore_default_signals();
#endif 
