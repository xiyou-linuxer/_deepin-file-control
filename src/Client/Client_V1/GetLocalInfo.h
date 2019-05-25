#pragma once
#include <iostream>                                                                                                                                                      
#include <stdio.h>
#include <string.h>
#include<unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <errno.h>
#include<fcntl.h>

#define LEN 1024 

int GetMac(char* macAddr) ;
void printError(const char* fileName, const int line) ;
