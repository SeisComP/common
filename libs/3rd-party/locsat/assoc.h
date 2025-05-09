/******************************************************************************
 * Copyright 1990 Science Applications International Corporation.
 *
 * There was no particular license attached to the origin source code. The
 * sources have been modified by gempa GmbH to prefix structures with LOCSAT_.
 ******************************************************************************/


#ifndef LOCSAT_ASSOC_H
#define LOCSAT_ASSOC_H


typedef struct {
	long  arid;
	char  sta[16];
	char  phase[9];
	float belief;
	float delta;
	float seaz;
	float esaz;
	float timeres;
	char  timedef;
	float azres;
	char  azdef;
	float slores;
	char  slodef;
	float emares;
	float wgt;
	char  vmodel[16];
} LOCSAT_Assoc;


#define Na_Assoc_Init \
{ \
	-1,	/*	arid 	*/ \
	"-",	/*	sta 	*/ \
	"-",	/*	phase 	*/ \
	-1.0,	/*	belief 	*/ \
	-1.0,	/*	delta 	*/ \
	-999.0,	/*	seaz 	*/ \
	-999.0,	/*	esaz 	*/ \
	-999.0,	/*	timeres 	*/ \
	'-',	/*	timedef 	*/ \
	-999.0,	/*	azres 	*/ \
	'-',	/*	azdef 	*/ \
	-999.0,	/*	slores 	*/ \
	'-',	/*	slodef 	*/ \
	-999.0,	/*	emares 	*/ \
	-1.0,	/*	wgt 	*/ \
	"-" 	/*	vmodel 	*/ \
}


#endif
