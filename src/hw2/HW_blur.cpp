void
filter(ChannelPtr<uchar>p1, int len, int steps, int kernelSize, ChannelPtr<uchar> p2);

void
HW_blur(ImagePtr I1, int width, int height, ImagePtr I2)
{
	ImagePtr temp;																//use temporary image
	IP_copyImageHeader(I1, temp);
	IP_copyImageHeader(I1, I2);
	int w = I1->width();
	int h = I1->height();
	int total = w * h;

	if (width % 2 == 0) width += 1;												//make sure width is odd
	if (width > w)  width  = 2 * ((int) w / 2) - 1;								//make sure slider width is less than input width 
	
	if (height % 2 == 0) height += 1;											//make sure height is odd
	if (height > h) height = 2 * ((int) h / 2) - 1;								//make sure slider height is less than input height
	
	int type;
	ChannelPtr<uchar> p1, p2, p3;
	for (int ch = 0; IP_getChannel(I1, ch, p1, type); ch++) {
		IP_getChannel(temp, ch, p2, type);
		for (int v = 0; v < h; v++, p1+=w, p2+=w) filter(p1, w, 1, width, p2);	//filter horizontally one step at a time
		p2 -= (total - 1);														//go one step back
		IP_getChannel(I2, ch, p3, type);
		for (int v = 0; v < w; v++, p2++, p3++)  filter(p2, h, w, height, p3);	//filter vertically every w steps
	}
}

void
filter(ChannelPtr<uchar>p1, int len, int steps, int kernelSize, ChannelPtr<uchar> p2)
{
	//if filter size is 1
	if (kernelSize == 1) {
		for (int v = 0; v < len; v++, p1 += steps, p2 += steps) *p2 = *p1;		//input = output image
		return;																	//no need to continue
	}

	int  sizeBuff = len + kernelSize - 1;										//calculate buffer size
	uchar* circBuff;															//define circular buffer
	circBuff = new uchar[sizeBuff];
	if (!circBuff) return;														//stop if circular buffer is null

	int pad = kernelSize / 2;													//calcutate extra space needed for edge pixels
	int v = 0;
	for (; v < pad; v++) circBuff[v] = *p1;										//fill up buffer from 0 to extra

	len += pad;
	for (; v < len; v++, p1 += steps) circBuff[v] = *p1;						//fill up buffer from extra to new width/height for every step					
	pad += len;																	//increase padding
	p1 -= steps;																//go one step back
	for (int v = len; v < pad; v++) circBuff[v] = *p1;							//fill up buffer's new extra space

	int sum = 0;																//sum of buffer's intensities
	for (int v = 0; v < kernelSize; v++) sum += circBuff[v];					//calculate sum from 0 to size
	*p2 = sum / kernelSize;														//assign average to output image
	for (int i = kernelSize; i < sizeBuff; i++, p2 += steps) {
		sum += (circBuff[i] - circBuff[i - kernelSize]);						//calculate sum from size to buffer size
		*p2 = sum / kernelSize;													//assign average to output image
	}

	delete circBuff;
}



