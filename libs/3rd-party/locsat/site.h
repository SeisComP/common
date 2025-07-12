/******************************************************************************
 * Copyright 1990 Science Applications International Corporation.
 *
 * There was no particular license attached to the origin source code. The
 * sources have been modified by gempa GmbH to prefix structures with LOCSAT_.
 ******************************************************************************/


#ifndef LOCSAT_SITE_H
#define LOCSAT_SITE_H


typedef struct {
	// Input
	char  sta[16];
	float lat;  // In degrees
	float lon;  // In degrees
	float elev; // In km

	// Temporary
	float distance;
	float azimuth;
	float backazimuth;

	double azimsd;
	double slowsd;
	double bestslow;
	double bestazim;
	double comprtime;
	double sheartime;
	double ordercompr;
	double ordersminusp;
	double orderdsd;
	double orderdsd2;
	double dis;
	int indexdsd;
	int indexdsd2;
	int indexcompr;
	int indexsminusp;
	int goodcompr;
	int goodsminusp;
	int goodazim;
	int goodslow;
	int iwave;
} LOCSAT_Site;


#endif
