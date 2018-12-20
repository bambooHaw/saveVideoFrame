#include "global.h"

int main(int argc, char* argv[])
{
	int ret = 0;

	COLOR_PRINT_BLACK_GREEN("Start...");
	initGlobalInfo(&gInfo, argc, argv);

	ret = _requestResourceCam(&gInfo);
	if(ret) ERR_PRINT("Error:  _requestResourceCam");
	else
	{
		ret = _getImageFrame(&gInfo);
		if(ret) ERR_PRINT("Error:  _getImageFrameRaw");
		else
		{
			ret = _saveImgFrame(IMG_SAVE_PATH, gInfo.frameBuf, gInfo.frameSize);
			if(ret) ERR_PRINT("Error:  _saveImgFrame");
		}
	}

	_releaseResourceCam(&gInfo);
	COLOR_PRINT_BLACK_GREEN("Done!");
	
	if(ret) COLOR_PRINT_BLACK_RED("Save image FAILED.");
	else COLOR_PRINT_UNDERLINE_RED_YELLOW("Save image SUCCESSED!(%s)", IMG_SAVE_PATH);
	
	return ret;
}
