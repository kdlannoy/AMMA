#include <opencv\cv.h>
#include "SteganoRaw.h"


int imgStega(IplImage *img, char *msg)
{
	int width = img->width;
	int height = img->height;
	uchar *data = (uchar*)img->imageData;


	int j, k = 0;
	int len = strlen(msg);
	char *new_str = (char*)malloc((len + 4)*sizeof(char));

	new_str[0] = '$';

	for (j = 1; j <= len; j++)
		new_str[j] = msg[j - 1];

	new_str[len + 1] = '$';
	new_str[len + 2] = '$';
	new_str[len + 3] = '$';

	if (img->nChannels != 3)
		return -1;

	for (int i = 0; i < height; i++){
		for (j = 0, k = 0; j < width || k < (len + 4); j += 3, k++)
			data[j * 3 + i] = new_str[k];
	}

	return 0;
}

char *imgDestega(IplImage *img)
{
	int width = img->width;
	uchar *data = (uchar*)img->imageData;

	int j, k = 0;
	char find;
	char *buffer = NULL;

	for (j = 0; j < width; j += 3)
	{
		find = data[j * 3];

		if (j == 0)
		{
			if (find != '$')
				exit(EXIT_FAILURE);
			else
				continue;
		}
		else
		{
			if (find == '$' && data[j*3+1] == '$' && data[j*3+2] == '$')
				break;
			else
			{
				buffer = (char*)realloc(buffer, (k + 1)*sizeof(char));
				buffer[k] = find;
				k++;
			}
		}
	}

	buffer = (char*)realloc(buffer, (k + 1)*sizeof(char));
	buffer[k] = '\0';

	return buffer;
}