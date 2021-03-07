#include <opencv2/opencv.hpp>
#include <iostream>


using namespace std;
using namespace cv;

Mat image,document;
vector<Point> cornerPoints , reorderedCornerPoints;
float width = 420, height = 596;

Mat getedges(Mat image)
{
	Mat imageGray, imageBlur, imageCanny,imageDilate;
	cvtColor(image,imageGray, COLOR_BGR2GRAY);
	GaussianBlur(imageGray, imageBlur, Size(3, 3), 3, 0);
	Canny(imageBlur, imageCanny, 25, 75);
	dilate(imageCanny, imageDilate, getStructuringElement(MORPH_RECT, Size(3,3)));
	
	return imageDilate;
}

vector<Point> contourBiggest(Mat imageProcessed)
{
	vector<vector<Point>> contourStorage;
	vector<Vec4i> hierarchy;

	findContours(imageProcessed, contourStorage,hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

	vector<vector<Point>> edges(contourStorage.size());
	vector<Point> biggestContour;
	
	int maximumArea = 0;
	for (int i = 0; i < contourStorage.size(); i++)
	{
		int area = contourArea(contourStorage[i]);
		
		if (area > 1000)
		{
			float perimeter = arcLength(contourStorage[i], true);
			approxPolyDP(contourStorage[i], edges[i], 0.02 * perimeter, true);

			if (area > maximumArea && edges[i].size()==4)
			{
				
				maximumArea = area;
				biggestContour = { edges[i][0], edges[i][1] , edges[i][2] , edges[i][3] };
				//drawContours(image, edges, i, Scalar(0, 255, 0), 2);
			}
			
		}
	}
	return biggestContour;
}


void drawpoints(vector<Point> point)
{
	for (int i = 0; i < point.size(); i++)
	{
		circle(image, point[i], 10, Scalar(0, 255, 0),FILLED);
		putText(image, to_string(i), point[i], FONT_HERSHEY_PLAIN, 4, Scalar(255,0,0), 4);
	}
}

vector<Point> reorder(vector<Point> CornerPoint)
{
		vector<Point> newPoints;
		vector<int> sumPoints, subPoints;

		for (int i = 0; i < 4; i++)
		{
			sumPoints.push_back(CornerPoint[i].x + CornerPoint[i].y);
			subPoints.push_back(CornerPoint[i].x - CornerPoint[i].y);
		}

		newPoints.push_back(CornerPoint[min_element(sumPoints.begin(), sumPoints.end()) - sumPoints.begin()]); //0
		newPoints.push_back(CornerPoint[max_element(subPoints.begin(), subPoints.end()) - subPoints.begin()]); //1
		newPoints.push_back(CornerPoint[min_element(subPoints.begin(), subPoints.end()) - subPoints.begin()]); //2
		newPoints.push_back(CornerPoint[max_element(sumPoints.begin(), sumPoints.end()) - sumPoints.begin()]); //3

		return newPoints;
}

Mat getDocument(vector<Point> point)
{
	Point2f sourceImage[4] = { point[0],point[1],point[2],point[3] };
	Point2f destinationImage[4] = { {0.0f,0.0f},{width,0.0f},{0.0f,height},{width,height} };

	Mat matrix = getPerspectiveTransform(sourceImage, destinationImage);
	warpPerspective(image, document, matrix, Point(width, height));

	return document;
}


int main()
{
	image = imread("6.jpeg");
	
	//Processing the image
	Mat imageProcessed = getedges(image);

	//get contours : get biggest
	cornerPoints = contourBiggest(imageProcessed);
	
	reorderedCornerPoints = reorder(cornerPoints);
	
	//warp image
	document = getDocument(reorderedCornerPoints);

	//Crop 
	int cropValue = 10;
	Rect roi(cropValue,cropValue, width -(2 * cropValue), height -(2 * cropValue));
	document = document(roi);

	 String mywin = "Image";
	
	
	/* drawpoints(reorderedCornerPoints);*/
	 imshow("Image", image);
	 imshow("Document", document);
	waitKey(0);
	return 0;
}