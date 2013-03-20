
#include "ofxRGBDepthCalibration.h"
using namespace cv;
using namespace ofxCv;

void ofxRGBDepthCalibration::refineDepthCalibration(ofxCv::Calibration& depthCalibrationBase,
													ofxCv::Calibration& depthCalibrationRefined,
													ofxDepthImageProvider* depthImageProvider)
{
	
//	depthCalibrationBase.load(initialDepthCalibrationPath);
	
	int width  = depthImageProvider->getRawIRImage().getWidth();
	int height = depthImageProvider->getRawIRImage().getHeight();
	vector<Point3f> depthObjectCollection;
	vector<Point2f> depthImageCollection;
	//Now with the current image try to create a projection for the depthCamera
	for(int y = 0; y < height; y++){
		for(int x = 0; x < width; x++){
			ofVec3f objectPoint = depthImageProvider->getWorldCoordinateAt(x, y);
			//			cout << "object point is " << objectPoint << endl;
			if(objectPoint.z != 0){
				depthObjectCollection.push_back(Point3f(objectPoint.x,objectPoint.y,objectPoint.z));
				depthImageCollection.push_back(Point2f(x,y));
			}
		}
	}
	cout << "depth object points for refinment " << depthObjectCollection.size() << endl;
	
	vector< vector<Point3f> > depthObjectPoints;
	vector< vector<Point2f> > depthImagePoints;
	depthObjectPoints.push_back(depthObjectCollection);
	depthImagePoints.push_back(depthImageCollection);
	
	vector<Mat> depthRotations, depthTranslations; //DUMMY extrinsics
	Mat depthCameraMatrix = depthCalibrationBase.getDistortedIntrinsics().getCameraMatrix();
	Mat depthDistCoeffs = depthCalibrationBase.getDistCoeffs();
	cout << "added depth object points " << depthObjectPoints.size() << " and depth image points " << depthImagePoints.size() << endl;
	cout << "base matrix is " << depthCameraMatrix << " base distortion is " << depthDistCoeffs << endl;
	calibrateCamera(depthObjectPoints, depthImagePoints, cv::Size(width,height), depthCameraMatrix, depthDistCoeffs, depthRotations, depthTranslations, CV_CALIB_USE_INTRINSIC_GUESS);
	
	cout << "Final camera matrix is " << depthCameraMatrix << endl;
	cout << "Final distortion coefficients " << depthDistCoeffs << endl;
	cout << "Depth rotation " << depthRotations[0] << endl;
	cout << "Depth translation " << depthTranslations[0] << endl;
	
	//Intrinsics depthIntrinsics;
	//depthIntrinsics.setup(depthCameraMatrix, cv::Size(640,480));
	
	Intrinsics newDepth;
	newDepth.setup(depthCameraMatrix,cv::Size(width,height));
	depthCalibrationRefined.setIntrinsics(newDepth, depthDistCoeffs);
	
//	depthCalibrationRefined.save(matrixDirectory + "depthCalib.yml");
//	depthCameraMatrix = depthCalibrationRefined.getDistortedIntrinsics().getCameraMatrix();
//	fov = ofVec2f(depthCameraMatrix.at<double>(0,0), depthCameraMatrix.at<double>(1,1));
//	pp = ofVec2f(depthCameraMatrix.at<double>(0,2),depthCameraMatrix.at<double>(1,2));
//	cout << "fov " << fov << endl;
//	cout << "principal point " << pp << endl;
//	btnCalibrateDepthCamera->setLabel("Depth Camera Self-Calibrated!");
//	depthCameraSelfCalibrated = true;
}

