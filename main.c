#include "global.h"

int main(int argc, char* argv[])
{
	int ret = 0;

	COLOR_PRINT_BLACK_RED("Start...");
	initGlobalInfo(&gInfo, argc, argv);

	ret = _requestResourceCam(&gInfo);
	if(ret) ERR_PRINT("Error:  _requestResourceCam");
	else
	{
		gettimeofday(&gInfo.tv[0], NULL);
		ret = _getImageFrame(&gInfo);
		if(ret) ERR_PRINT("Error:  _getImageFrameRaw");
		else
		{
			ret = _saveImgFrame(IMG_SAVE_PATH, gInfo.frameBuf, gInfo.frameSize);
			if(ret) ERR_PRINT("Error:  _saveImgFrame");
			gettimeofday(&gInfo.tv[1], NULL);
		}
	}

	_releaseResourceCam(&gInfo);
	COLOR_PRINT_BLACK_RED("Done!");
	//saving a img cost
    COLOR_PRINT_BLACK_GREEN("Saving a frame img cost: %.2f ms", \
                        (gInfo.tv[1].tv_sec  - gInfo.tv[0].tv_sec)*1000.0 + \
						(gInfo.tv[1].tv_usec - gInfo.tv[0].tv_usec)/1000.0);
	
	if(ret) COLOR_PRINT_BLACK_RED("Save image FAILED.");
	else COLOR_PRINT_UNDERLINE_RED_YELLOW("Save image SUCCESSED!(%s)", IMG_SAVE_PATH);
	
	return ret;
}
