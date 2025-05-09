/******************************************************************************
 * Copyright 1990 Science Applications International Corporation.
 *
 * There was no particular license attached to the origin source code. The
 * sources have been modified by gempa GmbH to prefix structures with LOCSAT_.
 ******************************************************************************/


#ifndef LOCSAT_ARRIVAL_H
#define LOCSAT_ARRIVAL_H


typedef struct {
	char   sta[16];
	double time;
	long   arid;
	char   iphase[9];
	char   stype[2];
	float  deltim;
	float  azimuth;
	float  delaz;
	float  slow;
	float  delslo;
} LOCSAT_Arrival;


#define Na_Arrival_Init \
{ \
	"-",	/*	sta 	*/ \
	-9999999999.999,	/*	time 	*/ \
	-1,	/*	arid 	*/ \
	"-",	/*	iphase 	*/ \
	"-",	/*	stype 	*/ \
	-1.0,	/*	deltim 	*/ \
	-1.0,	/*	azimuth 	*/ \
	-1.0,	/*	delaz 	*/ \
	-1.0,	/*	slow 	*/ \
	-1.0	/*	delslo 	*/ \
}


#endif
