#include <WinSock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <opencv\cv.h>
#include <opencv\highgui.h>
#include <zmq.h>
#include <WinSock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <opencv/../../include/opencv2/opencv.hpp>
//#include <opencv/../opencv.hpp
#include <thread>


using namespace cv;
using namespace std;
#define IMG_SIZE 921600
#define BUFLEN 2048
    

void server(){

	//init socket	
	int rc = 0;
	uchar* img = (uchar*) malloc(IMG_SIZE);
	char buf[BUFLEN];
	void *context = zmq_ctx_new();
	
	void *socket = zmq_socket(context, ZMQ_PAIR);

	cout << "Initialize the socket" << endl;
	rc = zmq_bind(socket, "tcp://*:9000");
	if(rc == -1){
		cout << "Initialization failed." << endl;
		return;
	}

	while(1){
		//cout << "Test" << endl;

		//receive chunks of data
		for(int i=0; i<IMG_SIZE/BUFLEN+1; i++){
			rc = zmq_recv(socket, buf, BUFLEN, 0);

	 		if (rc == -1){
				cout << "Error receiving image." << endl;
				break;
			}
			
			memcpy(img+i*BUFLEN, buf, BUFLEN);
		}
		

		Mat imageToShow = Mat(480,640,16,img);
		imageToShow.data = img;
		imshow("Afbeelding", imageToShow);
		if(waitKey(1)>0)
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
	 //initialize the socket
        int rc = 0;
        void *context = zmq_ctx_new();
        void *socket = zmq_socket(context, ZMQ_PAIR);
		
		//CameraReader camera = new CameraReader();

		cout << "Enter ip of server" << endl;
		string ipaddress;
		cin >> ipaddress;
		cout << "Server ip: " << "tcp://"+ipaddress+":9000" <<endl;
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

        while(1){
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
				int img_size = cameraFrame.cols*cameraFrame.rows*3;

				
				


				

                for(int i=0; i<img_size/BUFLEN+1; i++){
	                rc = zmq_send(socket, (img+i*BUFLEN), BUFLEN, 0);
	                if (rc == -1){
	                        cout << "Error while sending" << endl;
	                        return;
	                }
	            }
				
        }

        zmq_close(socket);
        zmq_ctx_destroy(context);


}
int main( int argc, char** argv )
{
   thread t1(server);
   thread t2(client);

   t1.join();
   t2.join();

  

    return 0;
}