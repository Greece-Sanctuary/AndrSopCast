#ifndef BILATERAL_FILTER_H_
#define BILATERAL_FILTER_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


class BilateralFilter
{
	typedef BilateralFilter CLASSNAME;
public:
	BilateralFilter();
	~BilateralFilter();
	void InitBilateralFilter(float spatial, float range, int wd, int ht);
	void DestroyBilateralFilter();
	void Process();
	void GetProcessImage(unsigned char* img);
	void SetProcessImage(unsigned char* img, float spatial, float range);
protected:
	/*1st-order recursive bilateral filter*/
	void RecursiveBilateralFilter(double***out,double***in,unsigned char***texture,
		double sigma_spatial,double sigma_range,int h,int w,double***temp,double***temp_2w,
		double**factor,double**temp_factor,double**temp_factor_2w);
private:
	float sigma_spatial;
	float sigma_range;
	int w, h;
	unsigned char***texture;
	double***image;
	double***image_filtered;
	double***temp;
	double***temp_2;
	int nr_iteration;
	double**temp_factor;
	double range_table[255+1];
	double inv_sigma_range;
	double alpha_w, alpha_h;
};

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* BILATERAL_FILTER_H_ */
