void
HW_convolve(ImagePtr I1, ImagePtr Ikernel, ImagePtr I2) {
    IP_copyImageHeader(I1, I2);
    int w = I1->width();
    int h = I1->height();
    int total = w*h;
    int type, y, x, ch, i, j;
	ChannelPtr<uchar> p1, p2, end, cursor;
	end = p1 + total;

	// Kernel information
	int wKernel = Ikernel->width();
	int hKernel = Ikernel->height();

	// Output = Input if kernel is of size 1x1
	if (wKernel == 1 && hKernel == 1) {
		IP_copyImage(I1, I2);
		return;
	}
	ChannelPtr<float> pKernel;
	IP_getChannel(Ikernel, 0, pKernel, type);

	// Padding and Buffer information
	int wPadding = wKernel / 2;
	int hPadding = hKernel / 2;
    int sizeBuff = w + wPadding * 2;

    // buffer allocation
    uchar** buff = new uchar*[hKernel];
    for(i = 0; i < hKernel; i++) 
		buff[i] = new uchar[sizeBuff];

    for(ch = 0; IP_getChannel(I1, ch, p1, type); ch++) {
        IP_getChannel(I2, ch, p2, type);
        cursor = p1;										// always start at first pixel in the row
        // populate buffer
        for(i = 0; i < hPadding; i++) {
            for(j = 0; j < wPadding; j++)
				buff[i][j] = *cursor;						// left padding
            for(j = wPadding; j < w+ wPadding; j++)
				buff[i][j] = *cursor++;						// in between padding
            cursor--;										// move cursor one step back to accomodate for last pixel
            for(j = w+ wPadding; j < sizeBuff; j++)
				buff[i][j] = *cursor;						// right padding
        }
        
		cursor = p1;										// move cursor back to first pixel in the current row
        for(i = hPadding; i < hKernel; i++) {
            for(j = 0; j < wPadding; j++)
				buff[i][j] = *cursor;						// left padding
            for(j = wPadding; j < w+ wPadding; j++)
				buff[i][j] = *cursor++;						// in between padding
            cursor--;										// move cursor one step back to accomodate for last pixel 
			for(j = w+ wPadding; j < sizeBuff; j++)
				buff[i][j] = *cursor;						// right padding
			cursor++;										// move cursor to next buffer row
        }

        //perform convolution
        for(y = 0; y < h; y++) {
            for (x = 0; x < w; x++) {
                float conv = 0;
                for (i = 0; i < hKernel; i++) {
                    for (j = 0; j < wKernel; j++) 
						conv += buff[i][j+x] * (*pKernel++);// convolve
                }

                *p2++ = CLIP(conv, 0, MaxGray);				// assign convoluted 0-255 value to output
				pKernel -= (wKernel * hKernel);				// move kernelPtr back to first value in the first row
            }
            for (i = 0; i<hKernel -1; i++)
                for (j = 0; j<sizeBuff; j++)
					buff[i][j] = buff[i+1][j];				// move all row buffer one index up
            for(j = 0; j < wPadding; j++)
				buff[hKernel -1][j] = *cursor;				// left padding
            for(j = wPadding; j < w+ wPadding; j++)
				buff[hKernel -1][j] = *cursor++;			// in between padding
            cursor--;										// move cursor one step back to accomodate for last pixel	
			for(j = w+ wPadding; j < sizeBuff; j++)
				buff[hKernel -1][j] = *cursor;				// right padding
			cursor++;										//move cursor to next buffer row
			if (y >= (h - hPadding - 2)) cursor -= w;		//position curser for next iteration
        }
    }
	//take care of memory leakage
    for (i = 0; i < hKernel; i++) delete[] buff[i];
	delete[] buff;
}
