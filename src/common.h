/*
# Description:
#   common.h
#
# Copyright (C) 2006- by EPS(Embedded and Parallel Systems Lab) @ NTUT CSIE
#
# Date: $Date: 2007/12/20 12:51:11 $
# Version: $Revision: 1.4 $
#
# History:
#
# $Log: common.h,v $
# Revision 1.4  2007/12/20 12:51:11  ffbli
# ok
#
#
*/

#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "define.h"
#include "socket.h"

// read d2mce config
int read_config();


// get the file's a line string
int fgetline(FILE *fp, char *line, int lim);
//split the string with symbol
int split(char symbol, char *line, char **lst, int lst_len);

//string
char *str_malloc(char *src_str, int maxlen, int add_len);
unsigned int hash_str(char *str);

#endif // COMMON_H_

