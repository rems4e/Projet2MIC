//
//  horloge.cpp
//  Projet2MIC
//
//  Created by Rémi Saurel on 06/02/12.
//  Copyright (c) 2012 Rémi Saurel. All rights reserved.
//

#include "horloge.h"
#include <sys/time.h>
#include <time.h>
#include <stdio.h>

horloge_t horloge() {
	static struct timeval refheure = {.tv_sec = -1, .tv_usec = 0};
	if(refheure.tv_sec == -1)
		gettimeofday(&refheure, 0);
	struct timeval temp;
	gettimeofday(&temp, 0);
	
	return (horloge_t)(temp.tv_sec - refheure.tv_sec + (temp.tv_usec - refheure.tv_usec) / 1.0E6);
}

void attendre(horloge_t sec) {
	struct timespec a = {.tv_sec = (long)sec, .tv_nsec = (long)(((double)sec - (long)sec) * 1E9)};
	nanosleep(&a, 0);
}
