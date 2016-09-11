#include <opencv2/core.hpp>
#include <opencv2/core/utility.hpp>
#include "opencv2/imgcodecs.hpp"
#include <opencv2/highgui.hpp>
#include <iostream>
#include <sstream>

#define GRIDS 4

using namespace std;
using namespace cv;
void balance_white(cv::Mat mat);
int improveFrame(Mat I, Mat& Out);

static void help()
{
    cout
        << "------------------------------------------------------------------------------" << endl
        << "Usage:"                                                                         << endl
        << "./videoImprove <referenceVideo>  " << endl
        << "--------------------------------------------------------------------------"     << endl
        << endl;
}


int main( int argc, char* argv[])
{
    help();
    if (argc != 2)
    {
        cout << "Not enough parameters" << endl;
        return -1;
    }

    const string videoIn = argv[1];
    VideoCapture captVideo(videoIn);

	if (!captVideo.isOpened())
    {
        cout  << "Could not open reference " << videoIn << endl;
        return -1;
    }

	Size refS = Size((int) captVideo.get(CAP_PROP_FRAME_WIDTH),
						(int) captVideo.get(CAP_PROP_FRAME_HEIGHT));

    cout << "Reference frame resolution: Width=" << refS.width << "  Height=" << refS.height
         << " of nr#: " << captVideo.get(CAP_PROP_FRAME_COUNT) << endl;

    Mat I, Out;
	int delay = 1;
    char c;
    int frameNum = -1;          // Frame counter

    const char* WIN_UT = "Under Test";
    const char* WIN_RF = "Reference";

	for(;;) //Show the image captured in the window and repeat
    {
        captVideo >> I;

        if (I.empty())
        {
            cout << " < < <  Game over!  > > > ";
            break;
        }

        ++frameNum;
        cout << "Frame: " << frameNum << "# ";

        ///////////////////////////////// improve ////////////////////////////////////////////////////
        improveFrame(I ,Out);

        ////////////////////////////////// Show Image /////////////////////////////////////////////
        imshow(WIN_RF, I);
        imshow(WIN_UT, Out);

        c = (char)waitKey(delay);
        if (c == 27) break;
    }

    return 0;
}

int improveFrame(Mat I, Mat& Out) 
{
    Mat O[(GRIDS * GRIDS) -1], Ip, Op;

    if (I.empty())
    {
        cout << "The image could not be loaded." << endl;
        return -1;
    }
    Out.create(I.rows, I.cols, I.type());
    const int nChannels = I.channels();
    int nrows = I.rows;
    int ncols = I.cols;
    int subrow = nrows/GRIDS, subcols = ncols/GRIDS;
    cout << I.rows / GRIDS << endl;
    cout << I.cols / GRIDS << endl;

    cout << I.size() << endl;
    cout << I.rows << endl;
    cout << I.cols << endl;
    cout << I.type() << endl;
    cout << I.depth() << endl;

    // Reduce brigtness of light area 
    /*
    for (int y = 0; y < nrows; ++y) {
        uchar* ptr = I.ptr<uchar>(y);
        for (int x = 0; x < ncols; ++x) {
            bool rewrite = false;
            //if ((ptr[x * 3 + 0] > 100) && (ptr[x * 3 + 1] > 100) && (ptr[x * 3 + 2] > 100)) {
            if ((ptr[x * 3 + 0] + ptr[x * 3 + 1] + ptr[x * 3 + 2]) > 300) {
                rewrite = true;
            }
            for (int j = 0; j < 3; ++j) {
                if (rewrite) {
                    ptr[x * 3 + j] = (uchar)(ptr[x * 3 + j] / 10);
                }
            }
        }
    }
    */

	// divide image in 16 regions
	// get indices and size of the regions
	// extract the rectangle
	// iterate over the region and decide new value and put in the matrix
	// goto new rectangle and repeat
    int k;
    for (int i=0; i<nrows-1; i = i + subrow) {
        int sub_x_end = (i + subrow - 1) > (nrows-1) ? nrows - 1 : (i + subrow - 1);
        for (int j=0; j<ncols-1; j = j + subcols) {
            //I.copyTo(Ip);
            int sub_y_end = (j + subcols - 1) > (ncols-1) ? ncols - 1 : (j + subcols - 1);
            cout << "rec_x_start = " << i << " rec_x_end = " << sub_x_end << " rec_y_start  = " << j << " rec_y_end  = " << sub_y_end << endl;
            //O[k] = I(Rect(j,i, sub_y_end - j,sub_x_end - i));
            I(Rect(j, i, sub_y_end - j,sub_x_end - i)).copyTo(Op);
			//balance_white(o[k]);
			balance_white(Op);
            //namedWindow("Output", WINDOW_AUTOSIZE);
            //imshow( "Output", Op );
            Op.copyTo(Out(Rect(j, i, sub_y_end - j,sub_x_end - i)));
            k++;
            Op.release();
            //waitKey();
        }
    }


    /*balance_white(I);
	namedWindow("Input", WINDOW_AUTOSIZE);
	imshow( "Input", I );
    waitKey();

	namedWindow("Oput", WINDOW_AUTOSIZE);
	imshow( "Oput", Out );
    waitKey();

    destroyAllWindows();
*/
    return 0;
}

void balance_white(cv::Mat mat)
{
  double discard_ratio = 0.05;
  int hists[3][256];
  memset(hists, 0, 3*256*sizeof(int));

  for (int y = 0; y < mat.rows; ++y) {
    uchar* ptr = mat.ptr<uchar>(y);
    for (int x = 0; x < mat.cols; ++x) {
      for (int j = 0; j < 3; ++j) {
        hists[j][ptr[x * 3 + j]] += 1;
      }
    }
  }

  // cumulative hist
  int total = mat.cols*mat.rows;
  int vmin[3], vmax[3];
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 255; ++j) {
      hists[i][j + 1] += hists[i][j];
    }
    vmin[i] = 0;
    vmax[i] = 255;
    while (hists[i][vmin[i]] < discard_ratio * total)
      vmin[i] += 1;
    while (hists[i][vmax[i]] > (1 - discard_ratio) * total)
      vmax[i] -= 1;
    if (vmax[i] < 255 - 1)
      vmax[i] += 1;
  }


  for (int y = 0; y < mat.rows; ++y) {
    uchar* ptr = mat.ptr<uchar>(y);
    for (int x = 0; x < mat.cols; ++x) {
      for (int j = 0; j < 3; ++j) {
        int val = ptr[x * 3 + j];
        if (val < vmin[j])
          val = vmin[j];
        if (val > vmax[j])
          val = vmax[j];
        ptr[x * 3 + j] = static_cast<uchar>((val - vmin[j]) * 255.0 / (vmax[j] - vmin[j]));
      }
    }
  }
}

