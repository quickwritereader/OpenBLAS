/***************************************************************************
Copyright (c) 2013-2019, The OpenBLAS Project
All rights reserved.
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:
1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in
the documentation and/or other materials provided with the
distribution.
3. Neither the name of the OpenBLAS project nor the names of
its contributors may be used to endorse or promote products
derived from this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE OPENBLAS PROJECT OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/
#include "common.h"
#include <math.h>
#include <altivec.h>


#if defined(DOUBLE)
    #define ABS fabs
#else
    #define ABS fabsf
#endif
#if defined(POWER8)
#define vector_load(ptr,immediate_index) vec_vsx_ld( immediate_index * 16 ,ptr )
#else
#define vector_load(ptr,immediate_index) ptr[immediate_index]
#endif
typedef __vector float __vf;
typedef __vector unsigned int __vui;
typedef __vector unsigned char __vuchar;
typedef __vector bool int __vbi;
/**
 * Find  maximum index 
 * Warning: requirements n>0  and n % 64 == 0
 * @param n     
 * @param x     pointer to the vector
 * @param maxf  (out) maximum absolute value .( only for output )
 * @return  index 
 */
static BLASLONG siamax_kernel_64(BLASLONG n, FLOAT *x, FLOAT *maxf) {
    BLASLONG index;
    BLASLONG i=0;
    register __vui static_index0 = {0,1,2,3};
    register __vui temp0 = {4,4,4, 4}; //temporary vector register
    register __vui temp1=  temp0<<1;  //{8,8,8,8}
    register __vui static_index1=static_index0 +temp0;//{4,5,6,7};
    register __vui static_index2=static_index0 +temp1;//{8,9,10,11};
    register __vui static_index3=static_index1 +temp1; //{12,13,14,15};
    temp0=vec_xor(temp0,temp0);
    temp1=temp1 <<1 ; //{16,16,16,16}
    register __vui quadruple_indices=temp0;//{0,0,0,0}
    register __vf quadruple_values={0,0,0,0};
    register __vf * v_ptrx=(__vf *)x;
    for(; i<n; i+=64){
       //absolute temporary vectors
       register __vf v0=vec_abs(vector_load(v_ptrx, 0));
       register __vf v1=vec_abs(vector_load(v_ptrx, 1));
       register __vf v2=vec_abs(vector_load(v_ptrx, 2));
       register __vf v3=vec_abs(vector_load(v_ptrx, 3));
       register __vf v4=vec_abs(vector_load(v_ptrx, 4));
       register __vf v5=vec_abs(vector_load(v_ptrx, 5));
       register __vf v6=vec_abs(vector_load(v_ptrx, 6));       
       register __vf v7=vec_abs(vector_load(v_ptrx, 7));
       //cmp quadruple pairs
       register __vbi r1=vec_cmpgt(v1,v0);
       register __vbi r2=vec_cmpgt(v3,v2);
       register __vbi r3=vec_cmpgt(v5,v4);
       register __vbi r4=vec_cmpgt(v7,v6);
      
       //select
       register __vui ind0_first= vec_sel(static_index0,static_index1,r1);
       register __vf vf0= vec_sel(v0,v1,r1);

       register __vui ind1= vec_sel(static_index2,static_index3,r2);
       register __vf vf1= vec_sel(v2,v3,r2);

       register __vui ind2= vec_sel(static_index0,static_index1,r3);
       v0=vec_sel(v4,v5,r3);

       register __vui ind3= vec_sel(static_index2,static_index3,r4);
       v1=vec_sel(v6,v7,r4);

       // cmp selected
        r1=vec_cmpgt(vf1,vf0);
       r2=vec_cmpgt(v1,v0);

       v_ptrx+=8;
       //select from above 
       ind0_first= vec_sel(ind0_first,ind1,r1);
       vf0= vec_sel(vf0,vf1,r1) ;

       ind2= vec_sel(ind2,ind3,r2);
       vf1= vec_sel(v0,v1,r2);

       //second indices actually should be within [16,31] so ind2+16
       ind2 +=temp1;
       
       //final cmp and select index and value for the first 32 values
       r1=vec_cmpgt(vf1,vf0);
       ind0_first = vec_sel(ind0_first,ind2,r1);
       vf0= vec_sel(vf0,vf1,r1);
 
       ind0_first+=temp0; //get absolute index

       temp0+=temp1;
       temp0+=temp1; //temp0+32
       //second part of 32
       // absolute temporary vectors
       v0=vec_abs(vector_load(v_ptrx, 0));
       v1=vec_abs(vector_load(v_ptrx, 1));
       v2=vec_abs(vector_load(v_ptrx, 2));
       v3=vec_abs(vector_load(v_ptrx, 3));
       v4=vec_abs(vector_load(v_ptrx, 4));
       v5=vec_abs(vector_load(v_ptrx, 5));
       v6=vec_abs(vector_load(v_ptrx, 6));       
       v7=vec_abs(vector_load(v_ptrx, 7));
       //cmp quadruple pairs
       r1=vec_cmpgt(v1,v0);
       r2=vec_cmpgt(v3,v2);
       r3=vec_cmpgt(v5,v4);
       r4=vec_cmpgt(v7,v6);
       //select
       register __vui ind0_second= vec_sel(static_index0,static_index1,r1);
       register __vf vv0= vec_sel(v0,v1,r1);

       ind1= vec_sel(static_index2,static_index3,r2);
       register __vf vv1= vec_sel(v2,v3,r2);

       ind2= vec_sel(static_index0,static_index1,r3);
       v0=vec_sel(v4,v5,r3);

       ind3= vec_sel(static_index2,static_index3,r4);
       v1=vec_sel(v6,v7,r4);

       // cmp selected
       r1=vec_cmpgt(vv1,vv0);
       r2=vec_cmpgt(v1,v0);

       v_ptrx+=8;
       //select from above 
       ind0_second= vec_sel(ind0_second,ind1,r1);
       vv0= vec_sel(vv0,vv1,r1) ;

       ind2= vec_sel(ind2,ind3,r2);
       vv1= vec_sel(v0,v1,r2) ;  

       //second indices actually should be within [16,31] so ind2+16
       ind2 +=temp1;
       
       //final cmp and select index and value for the second 32 values
       r1=vec_cmpgt(vv1,vv0);
       ind0_second = vec_sel(ind0_second,ind2,r1);
       vv0= vec_sel(vv0,vv1,r1);

       ind0_second+=temp0; //get absolute index
    
       //find final quadruple from 64 elements
       r2=vec_cmpgt(vv0,vf0);
       ind2 = vec_sel( ind0_first,ind0_second,r2);
       vv0= vec_sel(vf0,vv0,r2);       

       //compare with old quadruple and update 
       r3=vec_cmpgt(vv0,quadruple_values);
       quadruple_indices = vec_sel( quadruple_indices,ind2,r3);
       quadruple_values= vec_sel(quadruple_values,vv0,r3);      

       temp0+=temp1;
       temp0+=temp1; //temp0+32
 
    }

    //now we have to chose from 4 values and 4 different indices
    // we will compare pairwise if pairs are exactly the same we will choose minimum between index
    // otherwise we will assign index of the maximum value
    float a1,a2,a3,a4;
    unsigned int i1,i2,i3,i4;
    a1= quadruple_values[0];
    a2= quadruple_values[1];
    a3= quadruple_values[2];
    a4= quadruple_values[3];
    i1= quadruple_indices[0];
    i2= quadruple_indices[1];
    i3= quadruple_indices[2];
    i4= quadruple_indices[3];
    if(a1==a2){
      index=i1>i2?i2:i1;
    }else if(a2>a1){
      index=i2;
      a1=a2;
    }else{
       index= i1;
    }

    if(a4==a3){
      i1=i3>i4?i4:i3;
    }else if(a4>a3){
      i1=i4;
      a3=a4;
    }else{
       i1= i3;
    }

    if(a1==a3){
       index=i1>index?index:i1;
       *maxf=a1; 
    }else if(a3>a1){
       index=i1;
       *maxf=a3;
    }else{ 
        *maxf=a1;
    }
    return index;

}

BLASLONG CNAME(BLASLONG n, FLOAT *x, BLASLONG inc_x) {
    BLASLONG i = 0;
    BLASLONG j = 0;
    FLOAT maxf = 0.0;
    BLASLONG max = 0;

    if (n <= 0 || inc_x <= 0) return (max);

    if (inc_x == 1) {

        BLASLONG n1 = n & -64;
        if (n1 > 0) {

            max = siamax_kernel_64(n1, x, &maxf);

            i = n1;
        }

        while (i < n) {
            if (ABS(x[i]) > maxf) {
                max = i;
                maxf = ABS(x[i]);
            }
            i++;
        }
        return (max + 1);

    } else {

        BLASLONG n1 = n & -4;
        while (j < n1) {

            if (ABS(x[i]) > maxf) {
                max = j;
                maxf = ABS(x[i]);
            }
            if (ABS(x[i + inc_x]) > maxf) {
                max = j + 1;
                maxf = ABS(x[i + inc_x]);
            }
            if (ABS(x[i + 2 * inc_x]) > maxf) {
                max = j + 2;
                maxf = ABS(x[i + 2 * inc_x]);
            }
            if (ABS(x[i + 3 * inc_x]) > maxf) {
                max = j + 3;
                maxf = ABS(x[i + 3 * inc_x]);
            }

            i += inc_x * 4;

            j += 4;

        }


        while (j < n) {
            if (ABS(x[i]) > maxf) {
                max = j;
                maxf = ABS(x[i]);
            }
            i += inc_x;
            j++;
        }
        return (max + 1);
    }
}
