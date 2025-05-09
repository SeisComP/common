/******************************************************************************
 * Copyright 1990 Science Applications International Corporation.
 *
 * There was no particular license attached to the origin source code. The
 * sources have been modified by gempa GmbH to prefix structures with LOCSAT_.
 ******************************************************************************/


#ifndef LOCSAT_ORIGERR_H
#define LOCSAT_ORIGERR_H


typedef struct {
	float sxx;
	float syy;
	float szz;
	float stt;
	float sxy;
	float sxz;
	float syz;
	float stx;
	float sty;
	float stz;
	float sdobs;
	float smajax;
	float sminax;
	float strike;
	float sdepth;
	float stime;
	float conf;
} LOCSAT_Origerr;


#define Na_Origerr_Init \
{ \
	-1.0,   /*      sxx     */ \
	-1.0,   /*      syy     */ \
	-1,     /*      szz     */ \
	-1.0,   /*      stt     */ \
	-1.0,   /*      sxy     */ \
	-1.0,   /*      sxz     */ \
	-1.0,   /*      syz     */ \
	-1.0,   /*      stx     */ \
	-1.0,   /*      sty     */ \
	-1.0,   /*      stz     */ \
	-1.0,   /*      sdobs   */ \
	-1.0,   /*      smajax  */ \
	-1.0,   /*      sminax  */ \
	-1.0,   /*      strike  */ \
	-1.0,   /*      sdepth  */ \
	-1.0,   /*      stime   */ \
	 0.0    /*      conf    */ \
}


#endif
