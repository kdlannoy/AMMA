#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <WinSock2.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <zmq.h>
#include <string.h>
#include <opencv/../../include/opencv2/opencv.hpp>
//#include <opencv/../opencv.hpp
#include <thread>
#include "encoder.h"
#include "yuv.h"
#include "param.h"



using namespace cv;
using namespace x265;
using namespace std;
#define IMG_SIZE 921600
#define BUFLEN 2048


void server(){

	//init socket	
	int rc = 0;
	uchar* img = (uchar*)malloc(IMG_SIZE);
	char buf[BUFLEN];
	void *context = zmq_ctx_new();

	void *socket = zmq_socket(context, ZMQ_PAIR);
	

	cout << "Initialize the socket" << endl;
	rc = zmq_bind(socket, "tcp://*:9000");
	if (rc == -1){
		cout << "Initialization failed." << endl;
		return;
	}

	while (1){
		//cout << "Test" << endl;

		//receive chunks of data
		for (int i = 0; i < IMG_SIZE / BUFLEN + 1; i++){
			rc = zmq_recv(socket, buf, BUFLEN, 0);

			if (rc == -1){
				cout << "Error receiving image." << endl;
				break;
			}

			memcpy(img + i*BUFLEN, buf, BUFLEN);
		}


		Mat imageToShow = Mat(480, 640, 16, img);
		imageToShow.data = img;
		imshow("Afbeelding", imageToShow);
		if (waitKey(1)>0)
			break;



		////convert data to a frame
		//IplImage* frame= cvCreateImage(cvSize(640,480), IPL_DEPTH_8U, 1);
		////memcpy(frame->imageData, img, IMG_SIZE);

		//
		////show frame
		//cvNamedWindow("Received", CV_WINDOW_AUTOSIZE);
		//cvShowImage("Received", frame);
		//waitKey(0);
		//if(waitKey(0) >= 0) break;




	}

	zmq_close(socket);
	zmq_ctx_destroy(context);
}

void client(){
	/* x265_param_alloc:
	*  Allocates an x265_param instance. The returned param structure is not
	*  special in any way, but using this method together with x265_param_free()
	*  and x265_param_parse() to set values by name allows the application to treat
	*  x265_param as an opaque data struct for version safety */
	x265_param *param = x265_param_alloc();

	/*      returns 0 on success, negative on failure (e.g. invalid preset/tune name). */
	x265_param_default_preset(param, "ultrafast", "zerolatency");

	/* x265_param_parse:
	*  set one parameter by name.
	*  returns 0 on success, or returns one of the following errors.
	*  note: BAD_VALUE occurs only if it can't even parse the value,
	*  numerical range is not checked until x265_encoder_open().
	*  value=NULL means "true" for boolean options, but is a BAD_VALUE for non-booleans. */
	#define X265_PARAM_BAD_NAME  (-1)
	#define X265_PARAM_BAD_VALUE (-2)
	x265_param_parse(param, "fps", "30");
	x265_param_parse(param, "input-res", "352x288"); //wxh
	x265_param_parse(param, "bframes", "0");
	x265_param_parse(param, "rc-lookahead", "20");
	x265_param_parse(param, "repeat-headers", "1");

	/* x265_picture_alloc:
	*  Allocates an x265_picture instance. The returned picture structure is not
	*  special in any way, but using this method together with x265_picture_free()
	*  and x265_picture_init() allows some version safety. New picture fields will
	*  always be added to the end of x265_picture */
	x265_picture *pic_in = x265_picture_alloc();
	x265_picture *pic_out = x265_picture_alloc();


	/***
	* Initialize an x265_picture structure to default values. It sets the pixel
	* depth and color space to the encoder's internal values and sets the slice
	* type to auto - so the lookahead will determine slice type.
	*/
	x265_picture_init(param, pic_in);



	/* x265_encoder_encode:
	*      encode one picture.
	*      *pi_nal is the number of NAL units outputted in pp_nal.
	*      returns negative on error, zero if no NAL units returned.
	*      the payloads of all output NALs are guaranteed to be sequential in memory. */
	x265_nal *pp_nal;
	uint32_t pi_nal;
	//x265_encoder_encode(encoder, &pp_nal, &pi_nal, pic_in, pic_out);

	InputFileInfo info;
	info.filename = "bus_cif.yuv";
	info.depth = 8;
	info.csp = param->internalCsp;
	info.width = param->sourceWidth;
	info.height = param->sourceHeight;
	info.fpsNum = param->fpsNum;
	info.fpsDenom = param->fpsDenom;
	info.sarWidth = param->vui.sarWidth;
	info.sarHeight = param->vui.sarHeight;
	info.skipFrames = 0;
	info.frameCount = 0;
	getParamAspectRatio(param, info.sarWidth, info.sarHeight);

	Input*  input = new YUVInput(info);

	param->totalFrames = 150;
	if (param->logLevel >= X265_LOG_INFO)
	{
		char buf[128];
		int p = sprintf(buf, "%dx%d fps %d/%d %sp%d", param->sourceWidth, param->sourceHeight,
			param->fpsNum, param->fpsDenom, x265_source_csp_names[param->internalCsp], info.depth);

		int width, height;
		getParamAspectRatio(param, width, height);
		if (width && height)
			p += sprintf(buf + p, " sar %d:%d", width, height);

		fprintf(stderr, "%s  [info]: %s\n", input->getName(), buf);
	}

	input->startReader();

	std::fstream bitstreamFile;
	bitstreamFile.open("out.hevc", std::fstream::binary | std::fstream::out);
	if (!bitstreamFile)
	{
		x265_log(NULL, X265_LOG_ERROR, "failed to open bitstream file <%s> for writing\n", "out.hevc");
		return;
	}
	
	/* x265_encoder_open:
	*      create a new encoder handler, all parameters from x265_param are copied */
	x265_encoder *encoder = x265_encoder_open(param);









	//initialize the socket
	int rc = 0;
	void *context = zmq_ctx_new();
	void *socket = zmq_socket(context, ZMQ_PAIR);

	//CameraReader camera = new CameraReader();

	cout << "Enter ip of server" << endl;
	string ipaddress;
	cin >> ipaddress;
	cout << "Server ip: " << "tcp://" + ipaddress + ":9000" << endl;
	cout << "Initialize socket" << endl;
	rc = zmq_connect(socket, ("tcp://" + ipaddress + ":9000").c_str()); /*127.0.0.1*/
	cout << "RC: " + rc << endl;


	//initialize camera
	/*CvCapture* capture = cvCreateCameraCapture(0);
	if(!capture){
	cout<<"No camera found."<<endl;
	return 1;
	}*/

	VideoCapture stream1(0);

	while (1){
		////query frame
		//static IplImage *frame = NULL;
		//frame = cvQueryFrame(capture);
		////cvShowImage("Sending", frame);

		Mat cameraFrame;
		stream1.read(cameraFrame);

		/*		imshow("Testingwindow", cameraFrame);
				if (waitKey(1) >= 0)
				break;
				*/


		//cout << "Cols: " << (int)cameraFrame.cols <<  "\nRows: " << (int)cameraFrame.rows << "\nType: " << (int)cameraFrame.type() << endl;

		//         //   //sending frame
		uchar* img = cameraFrame.data;
		x265_encoder_encode(encoder, &pp_nal, &pi_nal, pic_in, pic_out);
		int img_size = cameraFrame.cols*cameraFrame.rows * 3;







		for (int i = 0; i < img_size / BUFLEN + 1; i++){
			rc = zmq_send(socket, (img + i*BUFLEN), BUFLEN, 0);
			if (rc == -1){
				cout << "Error while sending" << endl;
				return;
			}
		}

	}

	zmq_close(socket);
	zmq_ctx_destroy(context);


	/* x265_picture_free:
	*  Use x265_picture_free() to release storage for an x265_picture instance
	*  allocated by x265_picture_alloc() */
	x265_picture_free(pic_in);
	x265_picture_free(pic_out);

	/* x265_param_free:
	*  Use x265_param_free() to release storage for an x265_param instance
	*  allocated by x265_param_alloc() */
	x265_param_free(param);

	/* x265_encoder_close:
	*      close an encoder handler */
	x265_encoder_close(encoder);
	x265_cleanup();


}

int main(int argc, char** argv){
	thread t1(server);
	thread t2(client);

	t1.join();
	t2.join();

	return 0;
}