/******************************************************************************
 * Copyright 1990 Science Applications International Corporation.
 *
 * There was no particular license attached to the origin source code. The
 * sources have been modified by gempa GmbH to prefix structures with LOCSAT_.
 ******************************************************************************/


#ifndef LOCSAT_ORIGIN_H
#define LOCSAT_ORIGIN_H


typedef struct {
	float  lat;
	float  lon;
	float  depth;
	double time;
	long   nass;
	long   ndef;
	long   ndp;
	char   auth[16];
} LOCSAT_Origin;


#define Na_Origin_Init \
{ \
	-999.0, /*      lat     */ \
	-999.0, /*      lon     */ \
	-999.0, /*      depth   */ \
	-9999999999.999,        /*      time    */ \
	-1,     /*      nass    */ \
	-1,     /*      ndef    */ \
	-1,     /*      ndp     */ \
	"-"     /*      auth    */ \
}


#endif
