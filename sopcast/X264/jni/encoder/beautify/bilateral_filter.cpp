#include "bilateral_filter.h"
#include <stdio.h>
#include <math.h>
#include <vector>
#include <memory.h>
#include <algorithm>
#include <iostream>
using namespace std;  // std c++ libs implemented in std
#define QX_DEF_PADDING					10
#define QX_DEF_THRESHOLD_ZERO			1e-6
#define QX_DEF_PI_DOUBLE				3.14159265359
#define QX_DEF_FLOAT_MAX				1.175494351e+38F
#define QX_DEF_DOUBLE_MAX				1.7E+308
#define QX_DEF_FLOAT_RELATIVE_ACCURACY	2.2204e-016
#define QX_DEF_INI_MAX					2147483647
#define QX_DEF_SHORT_MAX				65535
#define QX_DEF_CHAR_MAX					255
#define	QX_DEF_SEED						42
#define QX_DEF_THRESHOLD_ZERO			1e-6
#define QX_DEF_THRESHOLD_ZERO_DOUBLE	1e-16
#define QX_DEF_ENTER					10
#define QX_DEF_BLANK					32
#define QX_DEF_STRING_LENGTH			300


inline double *** qx_allocd_3(int n,int r,int c,int padding=QX_DEF_PADDING)
{
	double *a,**p,***pp;
	int rc=r*c;
	int i,j;
	a=(double*) malloc(sizeof(double)*(n*rc+padding));
	if(a==NULL) {printf("qx_allocd_3() fail, Memory is too huge, fail.\n"); getchar(); exit(0); }
	p=(double**) malloc(sizeof(double*)*n*r);
	pp=(double***) malloc(sizeof(double**)*n);
	for(i=0;i<n;i++)
		for(j=0;j<r;j++)
			p[i*r+j]=&a[i*rc+j*c];
	for(i=0;i<n;i++)
		pp[i]=&p[i*r];
	return(pp);
}
inline void qx_freed_3(double ***p)
{
	if(p!=NULL)
	{
		free(p[0][0]);
		free(p[0]);
		free(p);
		p=NULL;
	}
}
inline unsigned char** qx_allocu(int r,int c,int padding=QX_DEF_PADDING)
{
	unsigned char *a,**p;
	a=(unsigned char*) malloc(sizeof(unsigned char)*(r*c+padding));
	if(a==NULL) {printf("qx_allocu() fail, Memory is too huge, fail.\n"); getchar(); exit(0); }
	p=(unsigned char**) malloc(sizeof(unsigned char*)*r);
	for(int i=0;i<r;i++) p[i]= &a[i*c];
	return(p);
}
inline void qx_freeu(unsigned char **p)
{
	if(p!=NULL)
	{
		free(p[0]);
		free(p);
		p=NULL;
	}
}
inline unsigned char *** qx_allocu_3(int n,int r,int c,int padding=QX_DEF_PADDING)
{
	unsigned char *a,**p,***pp;
	int rc=r*c;
	int i,j;
	a=(unsigned char*) malloc(sizeof(unsigned char )*(n*rc+padding));
	if(a==NULL) {printf("qx_allocu_3() fail, Memory is too huge, fail.\n"); getchar(); exit(0); }
	p=(unsigned char**) malloc(sizeof(unsigned char*)*n*r);
	pp=(unsigned char***) malloc(sizeof(unsigned char**)*n);
	for(i=0;i<n;i++)
		for(j=0;j<r;j++)
			p[i*r+j]=&a[i*rc+j*c];
	for(i=0;i<n;i++)
		pp[i]=&p[i*r];
	return(pp);
}
inline void qx_freeu_3(unsigned char ***p)
{
	if(p!=NULL)
	{
		free(p[0][0]);
		free(p[0]);
		free(p);
		p=NULL;
	}
}
inline void qx_freeu_1(unsigned char*p)
{
	if(p!=NULL)
	{
		delete [] p;
		p=NULL;
	}
}
inline float** qx_allocf(int r,int c,int padding=QX_DEF_PADDING)
{
	float *a,**p;
	a=(float*) malloc(sizeof(float)*(r*c+padding));
	if(a==NULL) {printf("qx_allocf() fail, Memory is too huge, fail.\n"); getchar(); exit(0); }
	p=(float**) malloc(sizeof(float*)*r);
	for(int i=0;i<r;i++) p[i]= &a[i*c];
	return(p);
}
inline void qx_freef(float **p)
{
	if(p!=NULL)
	{
		free(p[0]);
		free(p);
		p=NULL;
	}
}
inline float *** qx_allocf_3(int n,int r,int c,int padding=QX_DEF_PADDING)
{
	float *a,**p,***pp;
	int rc=r*c;
	int i,j;
	a=(float*) malloc(sizeof(float)*(n*rc+padding));
	if(a==NULL) {printf("qx_allocf_3() fail, Memory is too huge, fail.\n"); getchar(); exit(0); }
	p=(float**) malloc(sizeof(float*)*n*r);
	pp=(float***) malloc(sizeof(float**)*n);
	for(i=0;i<n;i++)
		for(j=0;j<r;j++)
			p[i*r+j]=&a[i*rc+j*c];
	for(i=0;i<n;i++)
		pp[i]=&p[i*r];
	return(pp);
}
inline void qx_freef_3(float ***p)
{
	if(p!=NULL)
	{
		free(p[0][0]);
		free(p[0]);
		free(p);
		p=NULL;
	}
}
inline int** qx_alloci(int r,int c,int padding=QX_DEF_PADDING)
{
	int *a,**p;
	a=(int*) malloc(sizeof(int)*(r*c+padding));
	if(a==NULL) {printf("qx_alloci() fail, Memory is too huge, fail.\n"); getchar(); exit(0); }
	p=(int**) malloc(sizeof(int*)*r);
	for(int i=0;i<r;i++) p[i]= &a[i*c];
	return(p);
}
inline void qx_freei(int **p)
{
	if(p!=NULL)
	{
		free(p[0]);
		free(p);
		p=NULL;
	}
}
inline void qx_freei_1(int*p)
{
	if(p!=NULL)
	{
		delete [] p;
		p=NULL;
	}
}
inline double** qx_allocd(int r,int c,int padding=QX_DEF_PADDING)
{
	double *a,**p;
	a=(double*) malloc(sizeof(double)*(r*c+padding));
	if(a==NULL) {printf("qx_allocd() fail, Memory is too huge, fail.\n"); getchar(); exit(0); }
	p=(double**) malloc(sizeof(double*)*r);
	for(int i=0;i<r;i++) p[i]= &a[i*c];
	return(p);
}
inline void qx_freed(double **p)
{
	if(p!=NULL)
	{
		free(p[0]);
		free(p);
		p=NULL;
	}
}
inline unsigned char**** qx_allocu_4(int t,int n,int r,int c,int padding=QX_DEF_PADDING)
{
	unsigned char *a,**p,***pp,****ppp;
	int nrc=n*r*c,nr=n*r,rc=r*c;
	int i,j,k;
	a=(unsigned char*) malloc(sizeof(unsigned char)*(t*nrc+padding));
	if(a==NULL) {printf("qx_allocu_4() fail, Memory is too huge, fail.\n"); getchar(); exit(0); }
	p=(unsigned char**) malloc(sizeof(unsigned char*)*t*nr);
	pp=(unsigned char***) malloc(sizeof(unsigned char**)*t*n);
	ppp=(unsigned char****) malloc(sizeof(unsigned char***)*t);
	for(k=0;k<t;k++)
		for(i=0;i<n;i++)
			for(j=0;j<r;j++)
				p[k*nr+i*r+j]=&a[k*nrc+i*rc+j*c];
	for(k=0;k<t;k++)
		for(i=0;i<n;i++)
			pp[k*n+i]=&p[k*nr+i*r];
	for(k=0;k<t;k++)
		ppp[k]=&pp[k*n];
	return(ppp);
}
inline void qx_freeu_4(unsigned char ****p)
{
	if(p!=NULL)
	{
		free(p[0][0][0]);
		free(p[0][0]);
		free(p[0]);
		free(p);
		p=NULL;
	}
}
inline double**** qx_allocd_4(int t,int n,int r,int c,int padding=QX_DEF_PADDING)
{
	double *a,**p,***pp,****ppp;
	int nrc=n*r*c,nr=n*r,rc=r*c;
	int i,j,k;
	a=(double*) malloc(sizeof(double)*(t*nrc+padding));
	if(a==NULL) {printf("qx_allocd_4() fail, Memory is too huge, fail.\n"); getchar(); exit(0); }
	p=(double**) malloc(sizeof(double*)*t*nr);
	pp=(double***) malloc(sizeof(double**)*t*n);
	ppp=(double****) malloc(sizeof(double***)*t);
	for(k=0;k<t;k++)
		for(i=0;i<n;i++)
			for(j=0;j<r;j++)
				p[k*nr+i*r+j]=&a[k*nrc+i*rc+j*c];
	for(k=0;k<t;k++)
		for(i=0;i<n;i++)
			pp[k*n+i]=&p[k*nr+i*r];
	for(k=0;k<t;k++)
		ppp[k]=&pp[k*n];
	return(ppp);
}
inline void qx_freed_4(double ****p)
{

	if(p!=NULL)
	{
		free(p[0][0][0]);
		free(p[0][0]);
		free(p[0]);
		free(p);
		p=NULL;
	}
}


BilateralFilter::BilateralFilter() {
	texture = NULL;
	image = NULL;
	image_filtered = NULL;
	temp = NULL;
	temp_2 = NULL;
	temp_factor = NULL;
	sigma_spatial = 0;
	sigma_range = 0;
}

BilateralFilter::~BilateralFilter() {
	DestroyBilateralFilter();
}

void BilateralFilter::InitBilateralFilter(float spatial, float range, int wd, int ht) {
	sigma_spatial = spatial;
	sigma_range = range;
	w = wd;
	h = ht;
	if (!texture) {
		texture=qx_allocu_3(h,w,3);
	}
	if (!image) {
		image=qx_allocd_3(h,w,3);
	}
	if (!image_filtered) {
		image_filtered=qx_allocd_3(h,w,3);
	}
	if (!temp) {
		temp=qx_allocd_3(h,w,3);
	}
	if (!temp_2) {
		temp_2=qx_allocd_3(2,w,3);
	}
	if (!temp_factor) {
		temp_factor=qx_allocd(h*2+2,w);
	}

	nr_iteration = 1;
	inv_sigma_range=1.0/(sigma_range*QX_DEF_CHAR_MAX);
	for(int i=0;i<=QX_DEF_CHAR_MAX;i++) range_table[i]=exp(-i*inv_sigma_range);
	alpha_w = exp(-sqrt(2.0)/(sigma_spatial*w));
	alpha_h = exp(-sqrt(2.0)/(sigma_spatial*h));
}

void BilateralFilter::DestroyBilateralFilter() {
	if (texture) {
		qx_freeu_3(texture);
		texture = NULL;
	}
	if (image) {
		qx_freed_3(image);
		image = NULL;
	}
	if (image_filtered) {
		qx_freed_3(image_filtered);
		image_filtered = NULL;
	}
	if (temp) {
		qx_freed_3(temp);
		temp = NULL;
	}
	if (temp_2) {
		qx_freed_3(temp_2);
		temp_2 = NULL;
	}
	if (temp_factor) {
		qx_freed(temp_factor);
		temp_factor = NULL;
	}
}

void BilateralFilter::SetProcessImage(unsigned char* img, float spatial, float range) {
	for(int y=0;y<h;y++){
		for(int x=0;x<w;x++) {
			for(int c=0;c<3;c++)  {
				texture[y][x][c] = img[(y*w+x)*3+c];
				image[y][x][c] = img[(y*w+x)*3+c];
			}
		} 
	}
	if (abs(sigma_spatial - spatial) < 0.01) {
		return;
	}
	sigma_spatial = spatial;
	sigma_range = range;
	inv_sigma_range=1.0/(sigma_range*QX_DEF_CHAR_MAX);
	for(int i=0;i<=QX_DEF_CHAR_MAX;i++) range_table[i]=exp(-i*inv_sigma_range);
	alpha_w = exp(-sqrt(2.0)/(sigma_spatial*w));
	alpha_h = exp(-sqrt(2.0)/(sigma_spatial*h));
}

void BilateralFilter::GetProcessImage(unsigned char* img) {
	for(int y=0;y<h;y++){
		for(int x=0;x<w;x++) {
			for(int c=0;c<3;c++)  {
				img[(y*w+x)*3+c] = image_filtered[y][x][c];
			}
		} 
	}
}

void BilateralFilter::Process() {
	RecursiveBilateralFilter(image_filtered,image,texture,sigma_spatial,sigma_range,h,w,temp,temp_2,temp_factor,&(temp_factor[h]),&(temp_factor[h+h]));
}

void BilateralFilter::RecursiveBilateralFilter(double***out,double***in,unsigned char***texture,
												double sigma_spatial,double sigma_range,int h,int w,
												double***temp,double***temp_2w,double**factor,
												double**temp_factor,double**temp_factor_2w)
{
	double***in_=in;
	unsigned char tpr,tpg,tpb,tcr,tcg,tcb;
	double ypr,ypg,ypb,ycr,ycg,ycb;
	double fp,fc;//factor
	double inv_alpha_=1-alpha_w;
	for(int y=0;y<h;y++)/*horizontal filtering*/
	{
		double*temp_x=temp[y][0];
		double*in_x=in_[y][0];
		unsigned char*texture_x=texture[y][0];
		*temp_x++=ypr=*in_x++; *temp_x++=ypg=*in_x++; *temp_x++=ypb=*in_x++;
		tpr=*texture_x++; tpg=*texture_x++; tpb=*texture_x++;

		double*temp_factor_x=temp_factor[y];//factor
		*temp_factor_x++=fp=1; 
		for(int x=1;x<w;x++) //from left to right
		{
			tcr=*texture_x++; tcg=*texture_x++; tcb=*texture_x++;
			unsigned char dr=abs(tcr-tpr);
			unsigned char dg=abs(tcg-tpg);
			unsigned char db=abs(tcb-tpb);
			int range_dist=(((dr<<1)+dg+db)>>2);
			double weight=range_table[range_dist];
			double alpha_=weight*alpha_w;
			*temp_x++=ycr=inv_alpha_*(*in_x++)+alpha_*ypr; *temp_x++=ycg=inv_alpha_*(*in_x++)+alpha_*ypg; *temp_x++=ycb=inv_alpha_*(*in_x++)+alpha_*ypb;//update temp buffer
			tpr=tcr; tpg=tcg; tpb=tcb;
			ypr=ycr; ypg=ycg; ypb=ycb;
			*temp_factor_x++=fc=inv_alpha_+alpha_*fp;//factor
			fp=fc;
		}
		int w1=w-1;
		*--temp_x; *temp_x=0.5*((*temp_x)+(*--in_x)); 
		*--temp_x; *temp_x=0.5*((*temp_x)+(*--in_x)); 
		*--temp_x; *temp_x=0.5*((*temp_x)+(*--in_x));
		tpr=*--texture_x; tpg=*--texture_x; tpb=*--texture_x;
		ypr=*in_x; ypg=*in_x; ypb=*in_x;
		
		*--temp_factor_x; *temp_factor_x=0.5*((*temp_factor_x)+1);//factor
		fp=1;

		////for(int x=w-2;x>=0;x--) //from right to left
		////{
		////	tcr=*--texture_x; tcg=*--texture_x; tcb=*--texture_x;
		////	unsigned char dr=abs(tcr-tpr);
		////	unsigned char dg=abs(tcg-tpg);
		////	unsigned char db=abs(tcb-tpb);
		////	int range_dist=(((dr<<1)+dg+db)>>2);
		////	double weight=range_table[range_dist];
		////	double alpha_=weight*alpha_w;

		////	ycr=inv_alpha_*(*--in_x)+alpha_*ypr; ycg=inv_alpha_*(*--in_x)+alpha_*ypg; ycb=inv_alpha_*(*--in_x)+alpha_*ypb;
		////	*--temp_x; *temp_x=0.5*((*temp_x)+ycr);
		////	*--temp_x; *temp_x=0.5*((*temp_x)+ycg);
		////	*--temp_x; *temp_x=0.5*((*temp_x)+ycb);
		////	tpr=tcr; tpg=tcg; tpb=tcb;
		////	ypr=ycr; ypg=ycg; ypb=ycb;

		////	fc=inv_alpha_+alpha_*fp;//factor
		////	*--temp_factor_x; *temp_factor_x=0.5*((*temp_factor_x)+fc);
		////	fp=fc;
		////}
	}

	inv_alpha_=1-alpha_h;
	in_=temp;/*vertical filtering*/
	double*ycy,*ypy,*xcy,*xpy;
	unsigned char*tcy,*tpy;
	memcpy(out[0][0],temp[0][0],sizeof(double)*w*3);
	
	double**in_factor=temp_factor;//factor
	double*ycf,*ypf,*xcf,*xpf;
	memcpy(factor[0],in_factor[0],sizeof(double)*w);
	for(int y=1;y<h;y++)
	{
		tpy=texture[y-1][0];
		tcy=texture[y][0];
		xcy=in_[y][0];
		ypy=out[y-1][0];
		ycy=out[y][0];
		
		xcf=&in_factor[y][0];//factor
		ypf=&factor[y-1][0];
		ycf=&factor[y][0];
		for(int x=0;x<w;x++)
		{
			unsigned char dr=abs((*tcy++)-(*tpy++));
			unsigned char dg=abs((*tcy++)-(*tpy++));
			unsigned char db=abs((*tcy++)-(*tpy++));
			int range_dist=(((dr<<1)+dg+db)>>2);
			double weight=range_table[range_dist];
			double alpha_=weight*alpha_h;
			for(int c=0;c<3;c++) *ycy++=inv_alpha_*(*xcy++)+alpha_*(*ypy++);
			*ycf++=inv_alpha_*(*xcf++)+alpha_*(*ypf++);
		}
	}
	int h1=h-1;
	ycf=&temp_factor_2w[0][0];//factor
	ypf=&temp_factor_2w[1][0];
	memcpy(ypf,in_factor[h1],sizeof(double)*w);
	for(int x=0;x<w;x++) factor[h1][x]=0.5*(factor[h1][x]+ypf[x]);
	
	ycy=temp_2w[0][0];
	ypy=temp_2w[1][0];
	memcpy(ypy,in_[h1][0],sizeof(double)*w*3);
	int k=0; for(int x=0;x<w;x++) for(int c=0;c<3;c++) out[h1][x][c]=0.5*(out[h1][x][c]+ypy[k++])/factor[h1][x];
	
	for(int y=h1-1;y>=0;y--)
	{
		tpy=texture[y+1][0];
		tcy=texture[y][0];
		xcy=in_[y][0];
		double*ycy_=ycy;
		double*ypy_=ypy;
		double*out_=out[y][0];
		
		xcf=&in_factor[y][0];//factor
		double*ycf_=ycf;
		double*ypf_=ypf;
		double*factor_=&factor[y][0];
		for(int x=0;x<w;x++)
		{
			unsigned char dr=abs((*tcy++)-(*tpy++));
			unsigned char dg=abs((*tcy++)-(*tpy++));
			unsigned char db=abs((*tcy++)-(*tpy++));
			int range_dist=(((dr<<1)+dg+db)>>2);
			double weight=range_table[range_dist];
			double alpha_=weight*alpha_h;
			
			double fcc=inv_alpha_*(*xcf++)+alpha_*(*ypf_++);//factor
			*ycf_++=fcc;
			*factor_=0.5*(*factor_+fcc); 
			for(int c=0;c<3;c++) {
				double ycc=inv_alpha_*(*xcy++)+alpha_*(*ypy_++);
				*ycy_++=ycc;
				*out_=0.5*(*out_+ycc)/(*factor_); 
				*out_++;
			}
			*factor_++;
		}
		memcpy(ypy,ycy,sizeof(double)*w*3);
		memcpy(ypf,ycf,sizeof(double)*w);//factor
		
		//for(int x=0;x<w;x++) for(int c=0;c<3;c++) out[y][x][c]/=factor[y][x];
	}
}
