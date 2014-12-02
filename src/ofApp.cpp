#include "ofApp.h"


//--------------------------------------------------------------
void ofApp::setup() {
	ofSetLogLevel(OF_LOG_VERBOSE);
	
	// enable depth->video image calibration
	//kinect.setRegistration(true);
    
	kinect.init();
	kinect.open();
    
    //load image, set the path
    coco.loadImage("coco8.jpg");
    
	
#ifdef USE_TWO_KINECTS
	//kinect2.init();
	//kinect2.open();
#endif
	
	colorImg.allocate(kinect.width, kinect.height);
	grayImage.allocate(kinect.width, kinect.height);
	grayThreshNear.allocate(kinect.width, kinect.height);
	grayThreshFar.allocate(kinect.width, kinect.height);
	
	nearThreshold = 230;
	farThreshold = 70;
	bThreshWithOpenCV = true;
    rotate=false;
    float a=0.1;
    float yKinect=0;
    //size of pixels
    pointSize=3;
    acc=1;
    zoom=1.0;
	
	ofSetFrameRate(60);
	
	// zero the tilt on startup
	angle = 0;
	//kinect.setCameraTiltAngle(angle);
	
	
}

//--------------------------------------------------------------
void ofApp::update() {
    
    //background color
	
	ofBackground(95, 77, 77);
    
	
	kinect.update();
	
	// there is a new frame and we are connected
	if(kinect.isFrameNew())
	{
		kinect.markDataStale();

		// load grayscale depth image from the kinect source
		grayImage.setFromPixels(kinect.getDepthPixels(), kinect.width, kinect.height);
		
		// we do two thresholds - one for the far plane and one for the near plane
		// we then do a cvAnd to get the pixels which are a union of the two thresholds
		if(bThreshWithOpenCV) {
			grayThreshNear = grayImage;
			grayThreshFar = grayImage;
			grayThreshNear.threshold(nearThreshold, true);
			grayThreshFar.threshold(farThreshold);
			cvAnd(grayThreshNear.getCvImage(), grayThreshFar.getCvImage(), grayImage.getCvImage(), NULL);
		} else {
			
			// or we do it ourselves - show people how they can work with the pixels
			unsigned char * pix = grayImage.getPixels();
            
			
			int numPixels = grayImage.getWidth() * grayImage.getHeight();
			for(int i = 0; i < numPixels; i++) {
				if(pix[i] < nearThreshold && pix[i] > farThreshold) {
					pix[i] = 255;
				} else {
					pix[i] = 0;
				}
			}
		}
		
		// update the cv images
		grayImage.flagImageChanged();
		
		// find contours which are between the size of 20 pixels and 1/3 the w*h pixels.
		// also, find holes is set to true so we will get interior contours as well....
		contourFinder.findContours(grayImage, 10, (kinect.width*kinect.height)/2, 20, false);
	}
	
#ifdef USE_TWO_KINECTS
	//kinect2.update();
#endif
}

//--------------------------------------------------------------
void ofApp::draw() {
	
	ofSetColor(255, 255, 255);
    easyCam.begin();
    drawPointCloud();
    easyCam.end();
    
}

void ofApp::drawPointCloud()
{
    
	//int w = 678;
	//int h = 640;
    
    //int wImage=1276;
    //int hImage=1050;
	int wImage = coco.width;
	int hImage = coco.height;
	/*{//magical hack to allow this to kinda-sorta-work:
		//TODO: figure out why image is larger than depth texture??
		wImage = CDepthBasics::cDepthWidth; //anything larger will cause a crash at the moment.
		hImage = CDepthBasics::cDepthHeight; //anything larger will cause a crash at the moment.
	}//end magical hack
    */

	ofMesh mesh;
	mesh.setMode(OF_PRIMITIVE_POINTS);
    unsigned char * pixels = coco.getPixels();
	if (pixels == nullptr)
	{
		printf("missing pixels... did you forget to include the image file?\n");
		return;
	}
    for (int i = 0; i < wImage; i+=acc){
        for (int j = 0; j < hImage; j+=acc){
            unsigned char red = pixels[(j * wImage + i)*3];
            unsigned char green = pixels[(j * wImage + i)*3+1];
            unsigned char blue = pixels[(j * wImage + i)*3+2];
            float valR = int(red);
            float valG = int(green);
            float valB = int(blue);
            ofColor c1(valR, valG, valB);
            mesh.addColor(c1);
            
            if(rotate){
                ofRotateY(a);
                //a+=0.00000000005;
                if(a>=100){
                    a=0;
                }
                a+=0.0000005;
            }
            
            mesh.addVertex(kinect.getWorldCoordinateAt(i,j));
            
        }
    }
    
	glPointSize(pointSize);
	ofPushMatrix();
	// the projected points are 'upside down' and 'backwards'
	ofScale(zoom, -zoom, -zoom);
	ofTranslate(0, 0, -1000); // center the points a bit
	glEnable(GL_DEPTH_TEST);
    
	mesh.drawVertices();
	glDisable(GL_DEPTH_TEST);
	ofPopMatrix();
}

//--------------------------------------------------------------
void ofApp::exit() {
	//kinect.setCameraTiltAngle(0); // zero the tilt on exit
	kinect.close();
	
#ifdef USE_TWO_KINECTS
	kinect2.close();
#endif
}

//--------------------------------------------------------------
void ofApp::keyPressed (int key) {
	switch (key) {
		case ' ':
			bThreshWithOpenCV = !bThreshWithOpenCV;
			break;
        case 'r':
            a=0;
			rotate=!rotate;;
			break;
            
			
		case'p':
			bDrawPointCloud = !bDrawPointCloud;
			break;
            
            
		case'g':
			pointSize++;
			break;
            
        case'f':
			pointSize--;
            if(pointSize==0)pointSize=1;
			break;
            
        case'd':
			acc++;
			break;
            
        case's':
			acc--;
            if(acc==0)acc=1;
			break;
            
        case'z':
			zoom=zoom+0.1;
            break;
            
        case'x':
			zoom=zoom-0.1;
            break;
            
            
            
            
            
		case '>':
		case '.':
			farThreshold ++;
			if (farThreshold > 255) farThreshold = 255;
			break;
			
		case '<':
		case ',':
			farThreshold --;
			if (farThreshold < 0) farThreshold = 0;
			break;
			
		case '+':
		case '=':
			nearThreshold ++;
			if (nearThreshold > 255) nearThreshold = 255;
			break;
			
		case '-':
			nearThreshold --;
			if (nearThreshold < 0) nearThreshold = 0;
			break;
			
		case 'w':
			//kinect.enableDepthNearValueWhite(!kinect.isDepthNearValueWhite());
			break;
			
		case 'o':
			//kinect.setCameraTiltAngle(angle); // go back to prev tilt
			//kinect.open();
			break;
			
		case 'c':
			//kinect.setCameraTiltAngle(0); // zero the tilt
			//kinect.close();
			break;
			
		case OF_KEY_UP:
			//angle++;
			//if(angle>30) angle=30;
			//kinect.setCameraTiltAngle(angle);
			break;
			
		case OF_KEY_DOWN:
			//angle--;
			//if(angle<-30) angle=-30;
			//kinect.setCameraTiltAngle(angle);
			break;
	}
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button)
{}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button)
{}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button)
{}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h)
{}
