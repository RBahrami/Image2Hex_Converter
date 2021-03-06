/*
MIT License

Copyright (c) 2021 Reza Bahrami

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/	
#include "pch.h"
#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <fstream>
#include <stdio.h>
#include <Windows.h>
#include <math.h>

#define MAX_IMAGE_SIZE	262144

using namespace std;
using namespace cv;

// Global Data Objects
Mat oImg, nImg, fImg;

void salt(Mat image, int n)
{
	for (int k = 0; k < n; k++)
	{
		int i = rand() % image.cols;
		int j = rand() % image.rows;

		if (image.channels() == 1) // gray-level image
		{
			image.at<uchar>(j, i) = 255;
		}
		else // color image
		{
			image.at<Vec3b>(j, i)[0] = 255;
			image.at<Vec3b>(j, i)[1] = 255;
			image.at<Vec3b>(j, i)[2] = 255;
		}
	}
}

void pepper(Mat image, int n)
{
	for (int k = 0; k < n; k++)
	{
		int i = rand() % image.cols;
		int j = rand() % image.rows;

		if (image.channels() == 1) // gray-level image
		{
			image.at<uchar>(j, i) = 0;
		}
		else // color image
		{
			image.at<Vec3b>(j, i)[0] = 0;
			image.at<Vec3b>(j, i)[1] = 0;
			image.at<Vec3b>(j, i)[2] = 0;
		}
	}
}

void salt_and_pepper(Mat &image, int percent)
{
	uint pixelCount = image.rows * image.cols;
	uint noise = (double)pixelCount * percent / 200;
	salt(image, noise);
	pepper(image, noise);
}

void saveHex(const Mat &image, const string fileName)
{
	ofstream file;
	file.open(fileName + ".hex");
	file << "M = " << image.rows << " N = " << image.cols << endl;
	for (int i = 0; i < image.rows; i++)
	{
		const uchar *data = image.ptr<uchar>(i);
		for (int j = 0; j < image.cols; j++)
		{
			file << setfill('0') << setw(2) << hex << +data[j] << " ";
		}
		file << endl;
	}
	file.close();
}

void loadHex(const string fileName, Mat &image)
{
	ifstream file;
	file.open(fileName + ".hex");
	int M, N;
	char ch = NULL;
	while (ch != '=')
	{
		file >> ch;
	}
	file >> M;
	ch = NULL;
	
	while (ch != '=')
	{
		file >> ch;
	}
	file >> N;

	image.create(M, N, CV_8UC1);
	string line;
	getline(file, line); // Goto next line
	int i = 0;
	while (getline(file, line)) // Read Hex Data and store them in image
	{
		istringstream row(line);
		int x;
		uchar *data = image.ptr<uchar>(i++); // Output row
		while (row >> hex >> x)
			*data++ = x;
	}


	file.close();
}

int loadImage(const string fileName, Mat &image)
{
	image = imread(fileName, 1);
	if (!image.data)
	{
		cout << "No image data" << endl;
		return -1;
	}
	cvtColor(image, image, CV_BGR2GRAY);
	uint imageSize = oImg.cols * oImg.rows;
	cout << "Image Size = " << oImg.size << "[" << imageSize << "]" << endl;
	if (imageSize > MAX_IMAGE_SIZE)
	{
		double scale = (double)MAX_IMAGE_SIZE / imageSize;
		scale = sqrt(scale);
		resize(oImg, oImg, Size(), scale, scale);
		cout << "Image Size is out of range. Resized Image to : Image Size = " << oImg.size << "[" << oImg.cols * oImg.rows << "]" << endl;
	}
	imshow("Image", image);
	waitKey(1);
	BringWindowToTop(GetConsoleWindow());
	return 0;
}

double PSNR(const Mat& I1, const Mat& I2)
{
	Mat s1;
	absdiff(I1, I2, s1);       // |I1 - I2|
	s1.convertTo(s1, CV_32F);  // cannot make a square on 8 bits
	s1 = s1.mul(s1);           // |I1 - I2|^2

	Scalar s = sum(s1);         // sum elements per channel

	double sse = s.val[0] + s.val[1] + s.val[2]; // sum channels

	if (sse <= 1e-10) // for small values return zero
		return 0;
	else
	{
		double  mse = sse / (double)(I1.channels() * I1.total());
		double psnr = 10.0*log10((255 * 255) / mse);
		return psnr;
	}
}

void clear_screen(char fill = ' ')
{
	COORD tl = { 0,0 };
	CONSOLE_SCREEN_BUFFER_INFO s;
	HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
	GetConsoleScreenBufferInfo(console, &s);
	DWORD written, cells = s.dwSize.X * s.dwSize.Y;
	FillConsoleOutputCharacter(console, fill, cells, tl, &written);
	FillConsoleOutputAttribute(console, s.wAttributes, cells, tl, &written);
	SetConsoleCursorPosition(console, tl);
}

int decode_cmd(string &cmd)
{
	
	stringstream cmdStream(cmd);
	cmdStream >> cmd;
	
	// Load command
	if (cmd == "ld")
	{
		cmdStream >> cmd;
		if (cmd == "img")
		{
			cmdStream >> cmd;
			loadImage(cmd, oImg);
			
		}
		else if (cmd == "nimg")
		{
			loadHex("../../inputImage", nImg);
			imshow("Noisy Image", nImg);
			waitKey(1);
			BringWindowToTop(GetConsoleWindow());
		}
		else if (cmd == "fimg")
		{
			loadHex("../../outputImage", fImg);
			imwrite("../../outputImage.jpg", fImg);
			imshow("Filtered Image", fImg);
			waitKey(1);
			BringWindowToTop(GetConsoleWindow());
		}
		else
		{
			cout << "Error at '" << cmd << "' : unrecognized or incomplete command line. type 'help' and press Enter for more information." << endl;
		}
	}

	// Create Noisy Image
	else if (cmd == "addnoise")
	{
		int noisePercent;
		cmdStream >> noisePercent;
		if (noisePercent >= 0 && noisePercent <= 100)
		{
			oImg.copyTo(nImg);
			salt_and_pepper(nImg, noisePercent);
			imshow("Noisy Image", nImg);
			saveHex(nImg, "../../inputImage");
			imwrite("../../inputImag.jpg", nImg);
			waitKey(1);
			BringWindowToTop(GetConsoleWindow());
		}
		else
		{
			cout << "Input number is out of range! [0-100]";
		}
	}

	// Compare Original and noisy image
	else if (cmd == "cmp")
	{
		if (!oImg.data)
		{
			cout << "Please load original image first." << endl;
			return 0;
		}
		if (nImg.data)
		{
			cout << "Noisy Image PSNR = " << PSNR(oImg, nImg) << " dB" << endl;
		}
		if (fImg.data)
		{
			cout << "Adaptive Median Filtered Image PSNR = " << PSNR(oImg, fImg) << " dB" << endl;
		}
	}

	// Switch to HighGUI
	else if (cmd == "wait")
	{
		cout << "HighGUI is Active. Press a key in any image window to return." << endl;
		waitKey(0);
	}
	
	else if (cmd == "clear" || cmd == "clc")
	{
		clear_screen();
		cout << "Image <> HEX Converter v1.2" << endl << "Created By Reza Bahrami." << endl << endl;

	}

	// Show Help
	else if (cmd == "help")
	{
		cout << endl << " ld [arg]" << endl
			<< "\timg [Image location]\t-Load Original image" << endl
			<< "\tnimg \t\t\t-Load Noisy image from hex file" << endl
			<< "\tfimg \t\t\t-Load Filtered image from hex file" << endl;
		cout << endl << " addnoise [Noise Intensity] \t-Add Salt and Pepper noise to Original image and Save it (inputImage)" << endl;
		cout << endl << " cmp \t\t\t\t-Calculate PSNR for Noisy Image and Filtered Image" << endl;
		cout << endl << " wait \t\t\t\t-Switch to HighGUI" << endl << endl;
		cout << endl << " exit \t\t\t\t-Exit" << endl << endl;
	}
	
	// Exit
	else if (cmd == "exit" || cmd == "end")
	{
		return -1;
	}

	// Command is not recognized
	else
	{
		cout  << "'" << cmd << "' is not recognized as an command, type 'help' and press Enter for more information." << endl;
	}
	return 0;
}

int main()
{
	BringWindowToTop(GetConsoleWindow());
	cout << "Image <> HEX Converter v1.2" << endl << "Created By Reza Bahrami." << endl << endl;

	for (;;)
	{
		string cmd;
		
		// Get Command from user
		cout << "Enter command (type help for more information) : ";
		getline(cin, cmd);
		// Process input command
		if (decode_cmd(cmd) == -1) break;
	}

	return 0;
}
