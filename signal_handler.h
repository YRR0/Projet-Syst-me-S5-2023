#ifndef SIGNAL_HANDLER_H
#define SIGNAL_HANDLER_H

#include "command_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "prompt.h"
#include "command_executor.h"
#include "job_manager.h"

void ignore_signals();
void restore_default_signals();
int pipeLimitedTwo(char*, char*);
int killProject(char ** c);


#endif 