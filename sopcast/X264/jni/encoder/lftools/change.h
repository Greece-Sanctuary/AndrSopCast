#ifndef CHANGE_H_
#define CHANGE_H_


void NV21ToNV12(byte* nv21,byte* nv12, int width, int height){
	if(nv21 == NULL || nv12 == NULL)return;
	int framesize = width*height;
	int i = 0,j = 0;
	memcpy(nv12, nv21, framesize);
	for(i = 0; i < framesize; i++){
		nv12[i] = nv21[i];
	}
	for (j = 0; j < framesize/2; j+=2)
	{
		nv12[framesize + j-1] = nv21[j+framesize];
	}
	for (j = 0; j < framesize/2; j+=2)
	{
		nv12[framesize + j] = nv21[j+framesize-1];
	}
}

#endif
