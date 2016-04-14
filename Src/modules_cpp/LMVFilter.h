#pragma once

//typedef unsigned char uchar;

/* Local mean and mean square filter */
class LMVFilterImpl;

class LMVFilter
{
public:
    LMVFilter( uchar *pSrc, int h, int w );
    ~LMVFilter();

    void filter( uchar *pDst, int r, int level);

private:
    LMVFilterImpl *impl_;
};
