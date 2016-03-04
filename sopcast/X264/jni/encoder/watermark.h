#ifndef WATER_MARK_H_
#define WATER_MARK_H_
static int myclamp(int a){
    if (a > 255) {
        return 255;
    }
    if (a < 0) {
        return 0;
    }
    return a;
}

int GenerateLogo(unsigned char* imgbuf, const int wd, const int ht,
                 unsigned char* logobuf, const int logwd, const int loght,
                 const int dx, const int dy) {
    if (logwd + dx > wd || loght + dy > ht || loght <= 0 || logwd <= 0 || wd <= 0 || ht <= 0 || dx < 0 || dy < 0 || imgbuf == NULL || logobuf == NULL) {
        return -1;
    }
    int startx = wd - dx - logwd;
    int starty = dy;
    int endx = wd - dx;
    int endy = dy+loght;
    for (int j = starty; j < endy; ++j) {
        for (int i = startx; i < endx; ++i) {
//            imgbuf[(j*wd+i)*4+0] = 0;
//            imgbuf[(j*wd+i)*4+1] = 0;
//            imgbuf[(j*wd+i)*4+2] = 255;

            imgbuf[(j*wd+i)*4+2] = logobuf[((j-starty)*logwd+(i-startx))*4+1];
            imgbuf[(j*wd+i)*4+1] = logobuf[((j-starty)*logwd+(i-startx))*4+2];
            imgbuf[(j*wd+i)*4+0] = logobuf[((j-starty)*logwd+(i-startx))*4+3];


//            imgbuf[(j*wd+i)*4+2] = myclamp((((imgbuf[(j*wd+i)*4+2] - logobuf[((j-starty)*logwd+(i-startx))*4+1])*logobuf[((j-starty)*logwd+(i-startx))*4+3]) >> 8) + logobuf[((j-starty)*logwd+(i-startx))*4+1]);
//            imgbuf[(j*wd+i)*4+1] = myclamp((((imgbuf[(j*wd+i)*4+1] - logobuf[((j-starty)*logwd+(i-startx))*4+2])*logobuf[((j-starty)*logwd+(i-startx))*4+3]) >> 8) + logobuf[((j-starty)*logwd+(i-startx))*4+2]);
//            imgbuf[(j*wd+i)*4+0] = myclamp((((imgbuf[(j*wd+i)*4+0] - logobuf[((j-starty)*logwd+(i-startx))*4+3])*logobuf[((j-starty)*logwd+(i-startx))*4+3]) >> 8) + logobuf[((j-starty)*logwd+(i-startx))*4+3]);
        }
    }

    return 1;
}

#endif
