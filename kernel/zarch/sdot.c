/***************************************************************************
Copyright (c) 2013-2018,The OpenBLAS Project
All rights reserved.
Redistribution and use in source and binary forms,with or without
modification,are permitted provided that the following conditions are
met:
1. Redistributions of source code must retain the above copyright
notice,this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
notice,this list of conditions and the following disclaimer in
the documentation and/or other materials provided with the
distribution.
3. Neither the name of the OpenBLAS project nor the names of
its contributors may be used to endorse or promote products
derived from this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES,INCLUDING,BUT NOT LIMITED TO,THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE OPENBLAS PROJECT OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT,INDIRECT,INCIDENTAL,SPECIAL,EXEMPLARY,OR CONSEQUENTIAL
DAMAGES (INCLUDING,BUT NOT LIMITED TO,PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE,DATA,OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY,WHETHER IN CONTRACT,STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
USE OF THIS SOFTWARE,EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/

#include "common.h"

static FLOAT sdot_kernel_32(BLASLONG n, FLOAT *x, FLOAT *y)
{
    FLOAT dot;

    __asm__ volatile (   
        "vzero %%v0                      \n\t"
        "srlg  %%r0,%1,5                 \n\t"
        "xgr   %%r1,%%r1                 \n\t"
        "0:                              \n\t"
        "pfd 1,1024(%%r1,%2)             \n\t"
        "pfd 1,1024(%%r1,%3)             \n\t"

        "vl  %%v16,0(%%r1,%2)            \n\t"
        "vl  %%v17,16(%%r1,%2)           \n\t"
        "vl  %%v18,32(%%r1,%2)           \n\t"
        "vl  %%v19,48(%%r1,%2)           \n\t"
        "vl  %%v20,64(%%r1,%2)           \n\t"
        "vl  %%v21,80(%%r1,%2)           \n\t"
        "vl  %%v22,96(%%r1,%2)           \n\t"
        "vl  %%v23,112(%%r1,%2)          \n\t"

        "vl  %%v24,0(%%r1,%3)            \n\t"
        "vfmasb   %%v0,%%v16,%%v24,%%v0  \n\t"
        "vl  %%v25,16(%%r1,%3)           \n\t"
        "vfmasb   %%v0,%%v17,%%v25,%%v0  \n\t"
        "vl  %%v26,32(%%r1,%3)           \n\t"
        "vfmasb   %%v0,%%v18,%%v26,%%v0  \n\t"
        "vl  %%v27,48(%%r1,%3)           \n\t"
        "vfmasb   %%v0,%%v19,%%v27,%%v0  \n\t"
        "vl  %%v28,64(%%r1,%3)           \n\t"
        "vfmasb   %%v0,%%v20,%%v28,%%v0  \n\t"
        "vl  %%v29,80(%%r1,%3)           \n\t"
        "vfmasb   %%v0,%%v21,%%v29,%%v0  \n\t"
        "vl  %%v30,96(%%r1,%3)           \n\t"
        "vfmasb   %%v0,%%v22,%%v30,%%v0  \n\t"
        "vl  %%v31,112(%%r1,%3)          \n\t"
        "vfmasb   %%v0,%%v23,%%v31,%%v0  \n\t"
        
        "agfi   %%r1,128                 \n\t"
        "brctg  %%r0,0b                  \n\t"
        "vrepf  %%v1,%%v0,1              \n\t"
        "vrepf  %%v2,%%v0,2              \n\t"
        "vrepf  %%v3,%%v0,3              \n\t"
        "aebr   %%f0,%%f1                \n\t"
        "aebr   %%f0,%%f2                \n\t"
        "aebr   %%f0,%%f3                \n\t"
        "ler    %0,%%f0                      "
        :"=f"(dot)
        :"r"(n),"ZR"((const FLOAT (*)[n])x),"ZR"((const FLOAT (*)[n])y)
        :"memory","cc","r0","r1","v0","v1","v2","v3","v16","v17","v18","v19","v20","v21","v22","v23","v24","v25","v26","v27","v28","v29","v30","v31"
    );

    return dot;
}

FLOAT CNAME(BLASLONG n,FLOAT *x,BLASLONG inc_x,FLOAT *y,BLASLONG inc_y)
{
	BLASLONG i=0;
	BLASLONG ix=0,iy=0;

	FLOAT  dot = 0.0 ;

	if ( n <= 0 )  return(dot);

	if ( (inc_x == 1) && (inc_y == 1) )
	{

		BLASLONG n1 = n & -32;

		if ( n1 )
			dot = sdot_kernel_32(n1,x,y);

		i = n1;
		while(i < n)
		{

			dot += y[i] * x[i] ;
			i++ ;

		}
		return(dot);


	}

	BLASLONG n1 = n & -2;

	while(i < n1)
	{

		dot += y[iy] * x[ix] + y[iy+inc_y] * x[ix+inc_x];
		ix  += inc_x*2 ;
		iy  += inc_y*2 ;
		i+=2 ;

	}

	while(i < n)
	{

		dot += y[iy] * x[ix] ;
		ix  += inc_x ;
		iy  += inc_y ;
		i++ ;

	}
	return(dot);

}


