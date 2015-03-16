#include <opencv\cv.h>
#include <string>
#include <bitset>
#include <iostream>
#include "SteganoRaw.h"

using namespace std;


//int imgStega(IplImage *img, char *msg)
//{
//	int width = img->width;
//	int height = img->height;
//	uchar *data = (uchar*)img->imageData;
//
//
//	int j, k = 0;
//	int len = strlen(msg);
//	char *new_str = (char*)malloc((len + 4)*sizeof(char));
//
//	new_str[0] = '$';
//
//	for (j = 1; j <= len; j++)
//		new_str[j] = msg[j - 1];
//
//	new_str[len + 1] = '$';
//	new_str[len + 2] = '$';
//	new_str[len + 3] = '$';
//
//	if (img->nChannels != 3)
//		return -1;
//
//	for (int i = 0; i < height; i++){
//		for (j = 0, k = 0; j < width || k < (len + 4); j += 3, k++)
//			data[j * 3 + i] = new_str[k];
//	}
//
//	return 0;
//}



int imgStega(IplImage *img, char *msg) {
	int width = img->width;
	int height = img->height;
	int len = strlen(msg);

	uchar* data = (uchar*)img->imageData;

	//create new string with delimiters
	string new_str = "";
	new_str.append("$");
	new_str.append(msg);
	new_str.append("$");

	//convert new string to array of chars in bits
	bitset<8>* new_str_bits = (bitset<8>*) malloc(new_str.size()*sizeof(bitset<8>));
	for (std::size_t i = 0; i < new_str.size(); ++i)
	{
		new_str_bits[i] = bitset<8>(new_str.c_str()[i]);
	}

	//apply steganography
	int neededAmountOfPixels = (new_str.size() * 8) / 3;
	int index = 0;
	//i iterates over pixels
	for (int i = 0; i < neededAmountOfPixels; i++) {
		//j is color component(BGR)
		for (int j = 0; j < 3; j++) {
			//get color value
			unsigned char color = data[i * 3 + j];
			bitset<8> bs(color);
			
			//change bs[7] to our text (MSB)
			bs[7] = new_str_bits[index/8].at(index%8);
			cout << bs[7];
			data[i * 3 + j] = char(bs.to_ulong());
			index++;
			
		}
		//TODO: make sure the last $ fills whole pixel
		//if not, we will get a character on the decoding side that has ascii number > 36 (36 == '$')

	}
	printf("\n%s\n", "steganografy passed");
	getchar();
	return 0;
}

unsigned char ToByte(bool b[8])
{
	unsigned char c = 0;
	for (int i = 0; i < 8; ++i)
		if (b[i])
			c |= 1 << i;
	return c;
}

char* imgDestega(IplImage *img) {
	int width = img->width;
	int height = img->height;
	int length = 0;
	bool firstDel = false;
	bool secondDel = false;
	uchar* data = (uchar*)img->imageData;

	string result = "";
	char* res = (char*)malloc(sizeof(char) * 0);
	bool tmp[8] = {0,0,0,0,0,0,0,0};
	int newIndex = 0;
	for (int i = 0; i < width*height; i++) {
		for (int j = 0; j < 3; j++) {
			unsigned char color = data[i * 3 + j];
			//convert color value to bits
			bitset<8> bs(color);
			tmp[newIndex%8] = bs.at(7);
			
			newIndex++;

			//convert last 8 bits to char
			if (newIndex%8 == 0){
				
				//printf("number: %u\n", ToByte(tmp));
				//cout << ToByte(tmp) << endl;
				//getchar();
				if (firstDel && secondDel)
					return "";
				if (!firstDel && ToByte(tmp)==36)
					firstDel = 1;
				else if (!secondDel && ToByte(tmp)==36)
					secondDel = 1;
				else if (secondDel)
					return "";
				else{
					length++;
					res = (char*)realloc(res, sizeof(char)*length);
					res[length - 1] = (char)ToByte(tmp);
					cout << ToByte(tmp);
					getchar();
				}
				
			}

			//check if char == $
		}
	}

	return res;
}



//char *imgDestega(IplImage *img)
//{
//	int width = img->width;
//	uchar *data = (uchar*)img->imageData;
//
//	int j, k = 0;
//	char find;
//	char *buffer = NULL;
//
//	for (j = 0; j < width; j += 3)
//	{
//		find = data[j * 3];
//
//		if (j == 0)
//		{
//			if (find != '$')
//				exit(EXIT_FAILURE);
//			else
//				continue;
//		}
//		else
//		{
//			if (find == '$' && data[j*3+1] == '$' && data[j*3+2] == '$')
//				break;
//			else
//			{
//				buffer = (char*)realloc(buffer, (k + 1)*sizeof(char));
//				buffer[k] = find;
//				k++;
//			}
//		}
//	}
//
//	buffer = (char*)realloc(buffer, (k + 1)*sizeof(char));
//	buffer[k] = '\0';
//
//	return buffer;
//}