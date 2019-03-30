
/**
 *
 * toqutree (pa3)
 * significant modification of a quadtree .
 * toqutree.cpp
 * This file will be used for grading.
 *
 */

#include "toqutree.h"
#include "math.h"

toqutree::Node::Node(pair<int,int> ctr, int dim, HSLAPixel a)
	:center(ctr),dimension(dim),avg(a),NW(NULL),NE(NULL),SE(NULL),SW(NULL)
	{}

toqutree::~toqutree(){
	clear(root);
}

toqutree::toqutree(const toqutree & other) {
	root = copy(other.root);
}


toqutree & toqutree::operator=(const toqutree & rhs){
	if (this != &rhs) {
		clear(root);
		root = copy(rhs.root);
	}
	return *this;
}

toqutree::toqutree(PNG & imIn, int k){ 

/* This constructor grabs the 2^k x 2^k sub-image centered */
/* in imIn and uses it to build a quadtree. It may assume  */
/* that imIn is large enough to contain an image of that size. */

/* your code here */
	if (imIn.width() == pow(2,k) && imIn.height() == pow(2,k)){
		buildTree(imIn, k);
	} else {
	pair<int,int> ctrPoint;
	ctrPoint.first = imIn.width()/2;
	ctrPoint.second = imIn.height()/2;
	PNG subIm(ctrPoint.first, ctrPoint.second);
		// grab 2^k * 2 ^k subimage centered
		for (unsigned int i = 0; i < pow(2, k); i++){
			for (unsigned int j = 0; j < pow(2, k); j++){
				HSLAPixel * pixelSub = subIm.getPixel(i, j);
				* pixelSub = *imIn.getPixel(ctrPoint.first - pow(2,k-1) + i, ctrPoint.second - pow(2,k-1) + j);
			}
		}
		buildTree(subIm, k);
	}

}

int toqutree::size() {
	return countSize(root);
/* your code here */
}


toqutree::Node * toqutree::buildTree(PNG & im, int k) {

	stats s = stats(im);
	int width = im.width();
	int height = im.height();
	pair<int,int> ul(0,0);
	pair<int,int> lr(width - 1, height - 1);
	HSLAPixel avgPixel = s.getAvg(ul,lr);
	pair<int,int> optSplitPos;
	// Base cases
	if (k == 0){
		Node* node = new Node(ul, k, avgPixel);
		return node;
	}

	PNG nwChild(width/2, height/2);
	PNG seChild(width/2, height/2);
	PNG swChild(width/2, height/2);
	PNG neChild(width/2, height/2);
	
	if (k == 1){

		HSLAPixel * pixelNewNW = nwChild.getPixel(0, 0);
		pixelNewNW = im.getPixel(0,0);

		HSLAPixel * pixelNewSe = seChild.getPixel(0, 0);
		pixelNewSe = im.getPixel(1,1);

		HSLAPixel * pixelNewSw = swChild.getPixel(0, 0);
		pixelNewSw = im.getPixel(0,1);
		
		HSLAPixel * pixelNewNe = neChild.getPixel(0, 0);
		pixelNewNe = im.getPixel(1,0);

	}

	

	else {

		// find centre
		unsigned int ctrUl_x = width/4; //4
		unsigned int ctrUl_y = height/4; // 4
		unsigned int ctrLr_x = width - ctrUl_x - 1; //11
		unsigned int ctrLr_y = height - ctrUl_y - 1; //11

		pair<int,int> ctrul(ctrUl_x, ctrUl_y);
		pair<int,int> ctrlr(ctrLr_x, ctrLr_y);

		double minEnt = 1000000.0;
		double avgEntropy;
		
		optSplitPos.first = 0;
		optSplitPos.second = 0;
		pair<int,int> ulSubRect(0,0);
		pair<int,int> lrSubRect((width/2)-1,(height/2)-1);
		long rectArea = s.rectArea(ulSubRect, lrSubRect);
		// we use this var when we made child nodes: 1-TOPLEFT 2-TOPRIGHT 3-BOTTOMLEFT 4-BOTTOMRIGHT
		int whichQ = 0;

		//  QTOPLEFT
		for (unsigned int x = ctrUl_x; x < (width/2); x++){
			for (unsigned int y = ctrUl_y; y < (height/2); y ++){
				pair<int, int> curSplitPos;
				curSplitPos.first = x;
				curSplitPos.second = y;

				// get Entropy
				avgEntropy = getEntropyTopLeftQ(curSplitPos,rectArea, width, height, s);

				// check if avgEntropy < minEntropy
				if (isMinEntropy(minEnt, avgEntropy)){
					minEnt = avgEntropy;
					optSplitPos.first = x;
					optSplitPos.second = y;
					whichQ = 1;
				}
			}
		}

		// TOPRIGHTQ
		for (unsigned int x = (width/2); x < (width/4)*3; x++){
			for (unsigned int y = ctrUl_y; y < (height/4)*3; y ++){
				pair<int, int> curSplitPos;
				curSplitPos.first = x;
				curSplitPos.second = y;

				// get Entropy
				if (x == (width/2)) {
					// see image 1. TopRight_x==width/2
					avgEntropy = getEntropyTopRightQNoSplit(curSplitPos,rectArea, width, height, s);
				} else {
					// rest of the cases
					avgEntropy = getEntropyTopRightQ(curSplitPos,rectArea, width, height, s);
				}

				// check if avgEntropy < minEntropy
				if (isMinEntropy(minEnt, avgEntropy)){
					minEnt = avgEntropy;
					optSplitPos.first = x;
					optSplitPos.second = y;
					whichQ = 1;
				}
			}
		}

		// BOTTOMRIGHTQ() - WIP
		for (unsigned int x = (width/2); x < ctrLr_x; x++){
			for (unsigned int y = (height/2); y < ctrLr_y; y ++){
				pair<int, int> curSplitPos;
				curSplitPos.first = x;
				curSplitPos.second = y;

				// get Entropy
				if (x == (width/2)) {
					// all 4 perfect squares. see 4. bottomRightQ_nosplit
					if (y == (height/2)){
						avgEntropy = getEntropyBottomRightQfourPerfectSquares(curSplitPos, rectArea, width, height, s);
					} else {
					// see 4. bottomRightQ_x==width/2 else case. SE SW horizonal Split
						avgEntropy = getEntropyBottomRighNENWNoSplits(curSplitPos, rectArea, width, height, s);
					}
				} else if (x > (width/2)) {
					// see 4. bottomRightQ_ y = height/2. NW & SW perf squares, SE NE vertical split
					if (y == (height/2)){
						avgEntropy = getEntropyBottomRighNWSWNoSplits(curSplitPos, rectArea, width, height, s);
					}
					else {
						// see 4. bottomRight_x > width/2 and y > height/2. only NW no split. SE split in 4, rest in 2.
						avgEntropy = getEntropyBottomRighOnlyNWnoSplit(curSplitPos, rectArea, width, height, s);
					}
				}
					
				 else {

					// TODO TODO,,,,,,
					avgEntropy = getEntropyTopRightQ(curSplitPos,rectArea, width, height, s);
				}

				// check if avgEntropy < minEntropy
				if (isMinEntropy(minEnt, avgEntropy)){
					minEnt = avgEntropy;
					optSplitPos.first = x;
					optSplitPos.second = y;
					whichQ = 1;
				}
			}
		}




		// after 4 for loops - make child image, check whichQ to decide how we split it
	}

	// for splitting
	// TODO: have to get optSplitPos first 
	// Make 4 PNG im for each child
	Node* node = new Node(optSplitPos, k, avgPixel);

	// Call Recursive func here
	node->NW = buildTree(nwChild, k--);
	node->NE = buildTree(neChild, k--);
	node->SE = buildTree(seChild, k--);
	node->SW = buildTree(swChild, k--);

	return node;
}

/* your code here */

// Note that you will want to practice careful memory use
// In this function. We pass the dynamically allocated image
// via pointer so that it may be released after it is used .
// similarly, at each level of the tree you will want to 
// declare a dynamically allocated stats object, and free it
// once you've used it to choose a split point, and calculate
// an average.

PNG toqutree::render(){

// My algorithm for this problem included a helper function
// that was analogous to Find in a BST, but it navigated the 
// quadtree, instead.

/* your code here */

}

/* oops, i left the implementation of this one in the file! */
void toqutree::prune(double tol){
	//TODO NOTE: we need to pass in avgColor of ROOT because prune will be recursively called and we
	// will need to remember the avgColour of the root!!!!!!s
	// prune(avgColor, root, tol); 

}

/* called by destructor and assignment operator*/
void toqutree::clear(Node * & curr){
/* your code here */
}

/* done */
/* called by assignment operator and copy constructor */
toqutree::Node * toqutree::copy(const Node * other) {

/* your code here */
}


//** HELPERS****

int toqutree::countSize(Node * root){
	if (root == NULL ){
		return 0;
	} else if (root->NW == NULL){ // will always have 4 child and first child is NW.
		return 1;
	} else {
		return (countSize(root->NW) + countSize(root->NE) + countSize(root->SE) + countSize(root->SW));
	}
}

PNG toqutree::makeImage(int dim, PNG & im, pair<int,int> splitPoint){
	unsigned int width = pow(2,dim);
	unsigned int height = width;
	PNG newIm(width, height);
	for (unsigned int i = 0; i < width; i++){
		for (unsigned int j = 0; j < height; j++){
			HSLAPixel * pixelNew = newIm.getPixel(i, j);
			* pixelNew = *im.getPixel(splitPoint.first + i, splitPoint.second + j);
		}
	}
	return newIm;

}

// TODO: return pointer or &???
PNG toqutree::stitchImgVertical(int dim, PNG & im, pair<int,int> splitPoint){
	unsigned int width = pow(2,dim);
	unsigned int height = width;
	PNG newIm(width, height);
	int x = splitPoint.first; // 7
	int y = splitPoint.second; // 7

	// first vertical
	for (unsigned int i = 0; i < width - x; i++){
		for (unsigned int j = 0; j < height; j++){
			HSLAPixel * pixelNew = newIm.getPixel(i, j);
			* pixelNew = *im.getPixel(width + x + i, y + j);
		}
	}

	// second vertical
	for (unsigned int i = width - x ; i < x; i++){
		for (unsigned int j = 0 ; j < height; j++){
			HSLAPixel * pixelNew = newIm.getPixel(i, j);
			* pixelNew = *im.getPixel(i, y + j);
		}
	}

	return newIm;

}

PNG toqutree::stitchImgHor(int dim, PNG & im, pair<int,int> splitPoint){
	unsigned int width = pow(2,dim);
	unsigned int height = width;
	PNG newIm(width, height);
	int x = splitPoint.first; // 7
	int y = splitPoint.second; // 7

	// first Horizontal (bottom)
	for (unsigned int i = 0; i < width; i++){
		for (unsigned int j = 0; j < height - y; j++){
			HSLAPixel * pixelNew = newIm.getPixel(i, j);
			* pixelNew = *im.getPixel(x + i, y + height + j);
		}
	}

	// second Horizontal (top)
	for (unsigned int i = 0; i < width; i++){
		for (unsigned int j = height - y; j < y; j++){
			HSLAPixel * pixelNew = newIm.getPixel(i, j);
			pixelNew = im.getPixel(x + i, j);
		}
	}

	return newIm;

}

PNG toqutree::stitchImgVandH(int dim, PNG & im, pair<int,int> splitPoint){
	unsigned int width = pow(2,dim);
	unsigned int height = width;
	PNG newIm(width, height);
	int x = splitPoint.first; // 7
	int y = splitPoint.second; // 7

	// #4 bottom right corner
	for (unsigned int i = 0; i < width - x; i++){
		for (unsigned int j = 0; j < height - y; j++){
			HSLAPixel * pixelNew = newIm.getPixel(i, j);
			* pixelNew = *im.getPixel( x + width + i, y + height + j);
		}
	}

	// #3 bottom left corner
	for (unsigned int i = width - x; i < x; i++){
		for (unsigned int j = 0; j < height - y; j++){
			HSLAPixel * pixelNew = newIm.getPixel(i, j);
			* pixelNew = *im.getPixel(i, y + height + j);
		}
	}

	// #2 Top right corner
	for (unsigned int i = 0; i < width - x; i++){
		for (unsigned int j = height - y; j < y; j++){
			HSLAPixel * pixelNew = newIm.getPixel(i, j);
			* pixelNew = *im.getPixel( x + width + i, j);
		}
	}

	// #1 Top left corner
	for (unsigned int i = width - x; i < x; i++){
		for (unsigned int j = height - y; j < y; j++){
			HSLAPixel * pixelNew = newIm.getPixel(i, j);
			* pixelNew = *im.getPixel(i, j);
		}
	}

	return newIm;
}

bool toqutree::isMinEntropy(double min, double avg){
	if (avg < min){
		return true;
	}
	return false;
}

double toqutree::getEntropyTopLeftQ(pair<int, int> curSplitPos, long rectArea, int width, int height, stats s) {
	int x = curSplitPos.first;
	int y = curSplitPos.second;
	pair<int,int> lrSe((width/2) + x - 1,(height/2) + y - 1);
	double entropySE = s.entropy(curSplitPos, lrSe);

	pair<int,int> ulNwTopLeft(0,0);
	pair<int,int> lrNwTopLeft(x-1,y-1);
	pair<int,int> ulNwTopright(((width/2) + x),0);
    pair<int,int> lrNwTopright(width-1,y-1);
	pair<int,int> ulNwBottomLeft(0, (y+(height/2)));
	pair<int,int> lrNwBottomLeft((x-1) , (height -1));
	pair<int,int> ulNwBottomRight((width/2)+x, y+(height/2));
	pair<int,int> lrNwBottomRight( (width -1), (height -1));

	double entropyNW = ((s.entropy(ulNwTopLeft, lrNwTopLeft) * s.rectArea(ulNwTopLeft, lrNwTopLeft))
						+ (s.entropy(ulNwTopright, lrNwTopright) * s.rectArea(ulNwTopright, lrNwTopright))
						+ (s.entropy(ulNwBottomLeft, lrNwBottomLeft) * s.rectArea(ulNwBottomLeft, lrNwBottomLeft))
						+ (s.entropy(ulNwBottomRight, lrNwBottomRight) * s.rectArea(ulNwBottomRight, lrNwBottomRight)))
						/rectArea;

	pair<int,int> ulSeRight((width/2) + x,y);
	pair<int,int> lrSeRight(width -1,(height/2) + y - 1);
	pair<int,int> ulSeLeft(0,y);
	pair<int,int> lrSeLeft(x-1,(height/2) + y - 1);
	double entropySW = ((s.entropy(ulSeRight,lrSeRight) * s.rectArea(ulSeRight,lrSeRight))
						+ (s.entropy(ulSeLeft, lrSeLeft) * s.rectArea(ulSeLeft, lrSeLeft)))
						/rectArea ;


	pair<int,int> ulNeTop(x,0);
	pair<int,int> lrNeTop((width/2) + x - 1,y-1);
	pair<int,int> ulNeBottom(x,y+(height/2));
	pair<int,int> lrNeBottom((width/2) + x - 1,height -1);
	double entropyNE = getEntropyFromTwo(ulNeTop,lrNeTop,ulNeBottom,lrNeBottom, s, rectArea);

	return ((entropySE + entropyNW + entropySW + entropyNE) / 4);
}

double toqutree::getEntropyTopRightQNoSplit(pair<int, int> curSplitPos, long rectArea, int width, int height, stats s) {
	int x = curSplitPos.first;
	int y = curSplitPos.second;
	pair<int,int> lrSe(width -1, y + (height/2) - 1);
	double entropySE = s.entropy(curSplitPos, lrSe);

	pair<int,int> ulSw(0, y);
	pair<int,int> lrSw(x-1, y + (height/2) - 1);
	double entropySW = s.entropy(ulSw, lrSw);

	pair<int,int> ulNeTop(x, 0);
	pair<int,int> lrNeTop(width-1, y - 1);
	pair<int,int> ulNeBottom(x, y + (height/2));
	pair<int,int> lrNeBottom(width -1, height -1);
	double entropyNE = getEntropyFromTwo(ulNeTop,lrNeTop,ulNeBottom,lrNeBottom, s, rectArea);
	
	pair<int,int> ulNwTop(0, 0);
	pair<int,int> lrNwTop(x-1, y - 1);
	pair<int,int> ulNwBottom(0, y + (height/2));
	pair<int,int> lrNwBottom(x -1, height -1);
	double entropyNW = getEntropyFromTwo(ulNwTop,lrNwTop,ulNwBottom,lrNwBottom, s, rectArea);
	
	return (entropySE + entropyNW + entropySW + entropyNE) / 4;
}

double toqutree::getEntropyTopRightQ(pair<int, int> curSplitPos, long rectArea, int twokWidth, int twokHeight, stats s) {
	int x = curSplitPos.first;
	int y = curSplitPos.second;
	
	pair<int,int> ulSeRight(x, y);
	pair<int,int> lrSeRight((twokWidth -1), (y + (twokHeight/2) - 1));
	pair<int,int> ulSeLeft(0, y);
	pair<int,int> lrSeLeft((x- (twokWidth/2) - 1), (y + (twokHeight/2) - 1));

	double entropySE = getEntropyFromTwo(ulSeRight,lrSeRight,ulSeLeft,lrSeLeft, s, rectArea);

	pair<int,int> ulSw(x- (twokWidth/2), y);
	pair<int,int> lrSw(x-1, y + (twokHeight/2) - 1);
	double entropySW = s.entropy(ulSw,lrSw);

	pair<int,int> ulNwTop(x- (twokWidth/2), 0);   // (x,y) = (10,6)
	pair<int,int> lrNwTop(x-1, y-1);
	pair<int,int> ulNwBottom(x- (twokWidth/2), y+(twokHeight/2));
	pair<int,int> lrNwBottom(x-1, twokHeight- 1);
	double entropyNw = getEntropyFromTwo(ulNwTop,lrNwTop, ulNwBottom, lrNwBottom, s, rectArea);

	pair<int,int> ulNeTopRight(x, 0);   
	pair<int,int> lrNeTopRight(twokWidth-1, y-1);
	pair<int,int> ulNeBottomRight(x, y+(twokHeight/2));
	pair<int,int> lrNeBottomRight(twokWidth-1, twokHeight- 1);
	pair<int,int> ulNeTopLeft(0, 0);   
	pair<int,int> lrNeTopLeft(x-(twokWidth/2)-1, y-1);
	pair<int,int> ulNeBottomLeft(0, y+(twokHeight/2));
	pair<int,int> lrNeBottomLeft(x-(twokWidth/2)-1, twokHeight-1);
	double entropyNe = getEntropyFromFour(ulNeTopRight, lrNeTopRight, ulNeBottomRight, lrNeBottomRight, ulNeTopLeft, lrNeTopLeft, ulNeBottomLeft, lrNeBottomLeft, s, rectArea);

}

double toqutree::getEntropyFromFour(pair<int,int> ulTopRight, pair<int,int> lrTopRight, pair<int,int> ulBottomRight,
pair<int,int> lrBottomRight, pair<int,int> ulTopLeft, pair<int,int> lrTopLeft, pair<int,int> ulBottomLeft,pair<int,int> lrBottomLeft, stats s, long rectArea) {
	double entropy = ((s.entropy(ulTopRight, lrTopLeft) * s.rectArea(ulTopLeft, lrTopLeft))
						+ (s.entropy(ulTopRight, ulTopRight) * s.rectArea(ulTopRight, ulTopRight))
						+ (s.entropy(ulBottomLeft, lrBottomLeft) * s.rectArea(ulBottomLeft, lrBottomLeft))
						+ (s.entropy(ulBottomRight, lrBottomRight) * s.rectArea(ulBottomRight, lrBottomRight)))
						/rectArea;
	return entropy;
}

double toqutree::getEntropyFromTwo(pair<int,int> ulTop, pair<int,int> lrTop, pair<int,int> ulBottom,
pair<int,int> lrBottom, stats s, long rectArea) {
	double entropy = (((s.entropy(ulTop,lrTop) * (s.rectArea(ulBottom,lrBottom)))
					  + (s.entropy(ulBottom,lrBottom) * (s.rectArea(ulBottom,lrBottom))))
					  / rectArea);

	return entropy;
}

double toqutree::getEntropyBottomRightQfourPerfectSquares(pair<int, int> curSplitPos, long rectArea, int twokWidth, int twokheight, stats s) {
	int x = curSplitPos.first;
	int y = curSplitPos.second;
	pair<int,int> ulSe = curSplitPos;
	pair<int,int> lrSe(twokWidth-1, twokheight-1);
	double entropySE = s.entropy(ulSe, lrSe);

	pair<int,int> ulSw(0, y);
	pair<int,int> lrSw(x-1, twokheight-1);
	double entropySW = s.entropy(ulSw, lrSw);

	pair<int,int> ulNE(x, 0);
	pair<int,int> lrNE(twokWidth-1, y-1);
	double entropyNE = s.entropy(ulNE, lrNE);

	
	pair<int,int> ulNW(0, 0);
	pair<int,int> lrNW(x-1, y - 1);
	double entropyNW = s.entropy(ulNW, lrNW);
	
	return (entropySE + entropyNW + entropySW + entropyNE) / 4;
}

double toqutree::getEntropyBottomRighNENWNoSplits(pair<int, int> curSplitPos, long rectArea, int twokWidth, int twokheight, stats s) {
	int x = curSplitPos.first;
	int y = curSplitPos.second;
	pair<int,int> ulNE(x, y - (twokheight/2));
	pair<int,int> lrNE(twokWidth-1, y -1);
	double entropyNE = s.entropy(ulNE, lrNE);

	pair<int,int> ulNW(0, y - (twokheight/2));
	pair<int,int> lrNW(x-1, y - 1);
	double entropyNW = s.entropy(ulNW, lrNW);

	pair<int,int> ulSWTop(0, 0);
	pair<int,int> lrSWTop(x-1, y - (twokheight/2));
	pair<int,int> ulSWBottom(0, y);
	pair<int,int> lrSWBottom(x-1, twokheight -1);
	double entropySW = getEntropyFromTwo(ulSWTop,lrSWTop,ulSWBottom,lrSWBottom, s, rectArea);
	
	pair<int,int> ulSETop(x, 0);
	pair<int,int> lrSETop(twokWidth-1, y - (twokheight/2) - 1);
	pair<int,int> ulSEBottom(x, y);
	pair<int,int> lrSEBottom(twokWidth-1, twokheight -1);
	double entropySE = getEntropyFromTwo(ulSETop,lrSETop,ulSEBottom,lrSEBottom, s, rectArea);
	
	return (entropySE + entropyNW + entropySW + entropyNE) / 4;
}

double toqutree::getEntropyBottomRighNWSWNoSplits(pair<int, int> curSplitPos, long rectArea, int twokWidth, int twokheight, stats s) {
	int x = curSplitPos.first;
	int y = curSplitPos.second;


	pair<int,int> ulNW(x - (twokWidth/2), y - (twokWidth/2)); // y will be 0
	pair<int,int> lrNW(x-1, y - 1);
	double entropyNW = s.entropy(ulNW, lrNW);

	pair<int,int> ulSW(x - (twokWidth/2), y);
	pair<int,int> lrSW(x-1, twokheight - 1);
	double entropySW = s.entropy(ulSW, lrSW);

	pair<int,int> ulNELeft(0, 0);
	pair<int,int> lrNELeft(x - (twokWidth/2) - 1 , y -1);
	pair<int,int> ulNERight(x, 0);
	pair<int,int> lrNERight(twokWidth-1, y -1);
	double entropyNE = getEntropyFromTwo(ulNELeft,lrNELeft,ulNERight,lrNERight, s, rectArea);

	
	pair<int,int> ulSELeft(0, y);
	pair<int,int> lrSELeft(x - (twokWidth/2) - 1, twokheight - 1);
	pair<int,int> ulSERight(x, y);
	pair<int,int> lrSERight(twokWidth-1, twokheight -1);
	double entropySE = getEntropyFromTwo(ulSELeft,lrSELeft,ulSERight,lrSERight, s, rectArea);
	
	return (entropySE + entropyNW + entropySW + entropyNE) / 4;
}

double toqutree::getEntropyBottomRighOnlyNWnoSplit(pair<int, int> curSplitPos, long rectArea, int twokWidth, int twokheight, stats s) {
	int x = curSplitPos.first;
	int y = curSplitPos.second;


	pair<int,int> ulNW(x - (twokWidth/2), y - (twokheight/2));
	pair<int,int> lrNW(x-1, y - 1);
	double entropyNW = s.entropy(ulNW, lrNW);

	pair<int,int> ulSWTop(x - (twokWidth/2), 0);
	pair<int,int> lrSWTop(x-1, y - (twokheight/2) - 1);
	pair<int,int> ulSWBottom(x - (twokWidth/2), y);
	pair<int,int> lrSWBottom(x - 1, twokheight - 1);
	double entropySW = getEntropyFromTwo(ulSWTop,lrSWTop,ulSWBottom,lrSWBottom, s, rectArea);

	pair<int,int> ulNELeft(0, y - (twokheight/2));
	pair<int,int> lrNELeft(x - (twokWidth/2) - 1 , y -1);
	pair<int,int> ulNERight(x, y - (twokheight/2));
	pair<int,int> lrNERight(twokWidth-1, y -1);
	double entropyNE = getEntropyFromTwo(ulNELeft,lrNELeft,ulNERight,lrNERight, s, rectArea);

	
	pair<int,int> ulSETopLeft(0, 0);   
	pair<int,int> lrSETopLeft(x-(twokWidth/2)-1, y - (twokheight/2)-1);
	pair<int,int> ulSETopRight(x, 0);   
	pair<int,int> lrSETopRight(twokWidth-1,  y - (twokheight/2)-1);
	pair<int,int> ulSEBottomLeft(0, y);
	pair<int,int> lrSEBottomLeft(x-(twokWidth/2)-1, twokheight-1);
	pair<int,int> ulSEBottomRight(x, y);
	pair<int,int> lrSEBottomRight(twokWidth-1, twokheight- 1);
	
	
	double entropySE = getEntropyFromFour(ulSETopLeft, lrSETopLeft, ulSETopRight, lrSETopRight, ulSEBottomLeft, lrSEBottomLeft, 
	ulSEBottomRight, lrSEBottomRight, s, rectArea);
	
	return (entropySE + entropyNW + entropySW + entropyNE) / 4;
}

