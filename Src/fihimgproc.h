#ifndef __FIH_IMGPROC_H__
#define __FIH_IMGPROC_H__

#include <string>
#include <list>

typedef enum
{
    IMGPROC_ShowGrayChannel,
	IMGPROC_Dehazing,
    IMGPROC_MyDehazing,
    IMGPROC_ComptAveragePic, //compute average picture
    IMGPROC_LMSFilter,
	IMGPROC_CLAHE,
	IMGPROC_SHDR_CPP,
	IMGPROC_PermTest,
	IMGPROC_Var
}FIH_IMGPROC_CMD;

/* FIH Image Process Parameters */
typedef struct
{
    /* Face Beauty Parameters */
    int                          fb_level;

    /* Local Histogram Equalization */
}FIH_IMGPROC_Params;

struct ImgProcMFCPrivate;

class ImgProcMFC
{
public:
	                              ImgProcMFC();
    virtual                       ~ImgProcMFC();

private:
    ImgProcMFCPrivate             *myPrivate;
    int                           dispW; //display width
    int                           dispH; //display height

public:
    int                           OpenPicture(std::string &filename);
    void                          SavePicture(std::string &dstname);
    void*                         PrepareDispData(void);
    void                          SetDispSize(int width, int height);
    void                          ImageProcess(FIH_IMGPROC_CMD cmd, FIH_IMGPROC_Params *pParams);
	void                          MultiImageProcess(std::list<std::string> &, FIH_IMGPROC_CMD);
};

#endif

