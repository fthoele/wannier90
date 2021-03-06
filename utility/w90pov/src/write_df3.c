//------------------------------------------------------------//
// Copyright (C) 2012 Daniel Aberg                            //
//                                                            //
// This file is distributed under the terms of the GNU        //
// General Public License. See the file `LICENSE' in          //
// the root directory of the present distribution, or         //
// http://www.gnu.org/copyleft/gpl.txt .                      //
//                                                            //
//------------------------------------------------------------//
// Code to enable use of the POV-Ray ray-tracing software for //
// the rendering of Wannier function isosurfaces from .xsf    //
// files generated by Wannier90.                              //
//------------------------------------------------------------// 
#include <stdio.h>
#include <stdlib.h>

#define FIX_SHORT(x) (*(unsigned short *)&(x) = SWAP_2(*(unsigned short *)&(x)))
#define SWAP_2(x) ( (((x) & 0xff) << 8) | ((unsigned short)(x) >> 8) )

enum { big_endian=10 , little_endian=20 , unknown_endian=99 };

int get_native_byteorder(void) {
  int native_byteorder, native_intsize ;
  
  union {
    char cvec[sizeof(int)/sizeof(char)];
    int ival;
  } x;
  
  if(sizeof(char) != 1) {
    fprintf(stderr,"@%s:%d: The byteorder detection and swapping routines "
	    "require sizeof(char) = 1. This code is compiled so that "
	    " sizeof(char) = %d. Aborting!\n",
	    __FILE__,__LINE__,(int) sizeof(char));
    exit(1);
  }
  
  native_intsize = sizeof(int);
  x.ival = 99;
  if(x.cvec[0] == 99)
    native_byteorder = little_endian;
  else if(x.cvec[sizeof(int)/sizeof(char)-1] == 99)
    native_byteorder = big_endian;
  else {
    fprintf(stderr,"@%s:%d: Could not understand native integer format. Aborting!\n",
	    __FILE__,__LINE__);
    exit(1);
  }
  
  return native_byteorder;
}

void write_df3_(int nx[1], int *ny, int *nz, int *num, double *wanfun, double *maxv, double *minv) {
  FILE *pekp, *pekm;
  int nnx, nny, nnz, i;				
  unsigned short valp, valm;
  double maxp, minp, c, tmp;
  char filenamep[30];
  char filenamem[30];
  int endian;
  
  maxp=-1e6;
  minp=1e6;
  
  c=65535.0;
  
  endian=get_native_byteorder();

  if(sizeof(short)!=2) {
    printf("short is not a 16-bit integer.\nWill quit now...\n\n");
    exit(99);
  }

  sprintf(filenamep,"wan_%3.3dp.df3",*num);
  sprintf(filenamem,"wan_%3.3dm.df3",*num);
  printf("will write %s\n",filenamep);


  if((pekp=fopen(filenamep,"wb"))==NULL) { 
    printf("Can't open %s\n",filenamep);
    exit(99);}
  if((pekm=fopen(filenamem,"wb"))==NULL) { 
    printf("Can't open %s\n",filenamem);
    exit(99);}
 
  nnx=*nx;
  nny=*ny;
  nnz=*nz;

  printf("%d %d %d\n",nnx,nny,nnz);
  
  fputc((nnx/256)%256,pekp);
  fputc(nnx%256,pekp);
  fputc((nny/256)%256,pekp);
  fputc(nny%256,pekp);
  fputc((nnz/256)%256,pekp);
  fputc(nnz%256,pekp);
  fputc((nnx/256)%256,pekm);
  fputc(nnx%256,pekm);
  fputc((nny/256)%256,pekm);
  fputc(nny%256,pekm);
  fputc((nnz/256)%256,pekm);
  fputc(nnz%256,pekm);

  maxp=0.0;
  for(i=0;i<nnx*nny*nnz;i++) {
    if( wanfun[i] > maxp) {
      maxp=wanfun[i];
    }
    if( wanfun[i] < minp) {
      minp=wanfun[i];
    }
  }
  printf("MAX=%f\n",maxp);
  printf("MIN=%f\n",minp);
  //printf("ZERO= %f \n",-minp/(maxp-minp));
  
  for(i=0;i<nnx*nny*nnz;i++) {

    //wanfun[i]=wanfun[i]*c/maxp;
    //wanfun[i]=wanfun[i]*c/minp;
    // wanfun[i]=(wanfun[i]-minp)*c/(maxp-minp);
    
    
    if(wanfun[i]<=0.0) { 
      valp=0;
      tmp=wanfun[i]*c/minp;
      valm=(unsigned short)tmp;
	} else {
      tmp=wanfun[i]*c/maxp;
      valp=(unsigned short)tmp;
      valm=0;
    }
  
  //printf("%d %d %f\n",i,val,wanfun[i]);
    if(endian==little_endian) {
      FIX_SHORT(valp);
      FIX_SHORT(valm);
    }
    fwrite(&valp, sizeof(unsigned short), 1, pekp);
    fwrite(&valm, sizeof(unsigned short), 1, pekm);
  }
  fclose(pekp);
  fclose(pekm);

  *minv=minp;
  *maxv=maxp;


}
