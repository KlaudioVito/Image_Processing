#include "MainWindow.h"
#include "Correlate.h"
//#include "hw2/HW_correlate.cpp"

extern MainWindow *g_mainWindowP;
enum { WSIZE_T, HSIZE_T, STEPX, STEPY, SAMPLER, STEPX_T, STEPY_T, SAMPLER_T, SQRT_SUM_T };
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Correlation::Correlation:
//
// Constructor.
//
Correlate::Correlate(QWidget *parent) : ImageFilter(parent)
{
	m_template = NULL;
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Correlation::controlPanel:
//
// Create group box for control panel.
//
QGroupBox*
Correlate::controlPanel()
{
	// init group box
	m_ctrlGrp = new QGroupBox("Correlation");

	// layout for assembling filter widget
	QVBoxLayout *vbox = new QVBoxLayout;

	// create file pushbutton
	m_button = new QPushButton("File");
	m_button2 = new QPushButton("match");
	m_button2->setDisabled(true);
	m_label = new QLabel("X: none, Y: none, Corr: none");
	m_label_template = new QLabel;

	// assemble dialog
	vbox->addWidget(m_button);
	vbox->addWidget(m_button2);
	vbox->addWidget(m_label_template);
	vbox->addStretch();
	vbox->addWidget(m_label);
	m_ctrlGrp->setLayout(vbox);

	// init signal/slot connections
	connect(m_button, SIGNAL(clicked()), this, SLOT(load()));
	connect(m_button2, SIGNAL(clicked()), this, SLOT(match()));

	return(m_ctrlGrp);
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Correlation::applyFilter:
//
// Run filter on the image, transforming I1 to I2.
// Overrides ImageFilter::applyFilter().
// Return 1 for success, 0 for failure.
//
bool
Correlate::applyFilter(ImagePtr I1, bool gpuFlag, ImagePtr I2)
{
	// error checking
	if (I1.isNull())		return 0;
	if (m_template.isNull())	return 0;
	m_width = I1->width();
	m_height = I1->height();
	// Correlation image
	if (!(gpuFlag && m_shaderFlag)) correlate(I1, m_template, I2);
	else    g_mainWindowP->glw()->applyFilterGPU(m_nPasses);

	if (gpuFlag) m_button2->setDisabled(false);
	else m_button2->setDisabled(true);
	return 1;
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Correlation::Correlation:
//
// Correlation image I1 with convolution filter in kernel.
// Output is in I2.
//
void
Correlate::correlate(ImagePtr I1, ImagePtr kernel, ImagePtr I2)
{
	/*
	char buf[100];
	float *corr;
	corr = HW_correlate(I1, kernel, I2);
	sprintf(buf, "X: %.2f, Y: %.2f, Corr: %.5f", corr[0], corr[1], corr[2]);
	m_label->setText(QString(buf));
	delete[] corr;
	*/
}


void Correlate::match() {

	// Read the output image from GPU
	ImagePtr out;
	out = g_mainWindowP->imageDst();

	// get width and height of template image
	int w = m_template->width();
	int h = m_template->height();

	int type, coord, x, y, count = 0, max = 0;
	ChannelPtr<uchar> p1, p2, p3, endd;
	int total = out->width() * out->height();
	IP_getChannel(out, 0, p1, type);

	// get max correlation value and find x and y coordinate
	for (endd = p1 + total; p1<endd;) {
		if (*p1 > max) {
			max = *p1;
			coord = count;
		}
		count++;
		p1++;
	}

	x = (int)ceil(coord%m_width - w / 2);
	y = (int)ceil(coord / m_width - h / 2);
	char buf[100];
	sprintf(buf, "X: %d %t Y: %d %t Corr: %.2f", x, y, (float) max/255.0);
	m_label->setText(QString(buf));

	// Match template image to input image
	ImagePtr src, dst;
	src = g_mainWindowP->imageSrc();
	IP_copyImageHeader(src, dst);
	for (int ch = 0; IP_getChannel(src, ch, p1, type); ch++) {
		IP_getChannel(dst, ch, p2, type);
		IP_getChannel(m_template, ch, p3, type);
		for (int i = 0; i < m_width; i++) {
			for (int j = 0; j < m_height; j++) {
				if (i < y) {    // if we are above template image
					*p2++ = *p1 / 2;
					p1++;
				}
				else if (i >= y && i <= (y - 1 + h)) {    // if we are on the rows where template image resides
					if (j < x) *p2 = *p1 / 2; // if we are before the template image columns
					else if (j >= x && j < (x + w)) {    // if we are on the columns where template resides
						*p2 = *p1 / 2 + *p3 / 2;
						p3++;
					}
					else *p2 = *p1 / 2;   // if we are after the template image columns

					p1++;
					p2++;
				}
				else {    // if we are below the template images
					*p2++ = *p1 / 2;
					p1++;
				}
			}
		}
	}

	// pass new texture to GPU
	QImage q;
	IP_IPtoQImage(dst, q);
	g_mainWindowP->glw()->setCorrOutTexture(q);
	g_mainWindowP->glw()->updateGL();
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Correlation::load:
//
// Slot to load filter kernel from file.
//
int
Correlate::load()
{
	QFileDialog dialog(this);

	// open the last known working directory
	if (!m_currentDir.isEmpty())
		dialog.setDirectory(m_currentDir);

	// display existing files and directories
	dialog.setFileMode(QFileDialog::ExistingFile);

	// invoke native file browser to select file
	m_file = dialog.getOpenFileName(this,
		"Open File", m_currentDir,
		"Images (*.AF *.png *.jpg);;All files (*)");

	// verify that file selection was made
	if (m_file.isNull()) return 0;

	// save current directory
	QFileInfo f(m_file);
	m_currentDir = f.absolutePath();

	// read kernel
	m_template = IP_readImage(qPrintable(m_file));

	IP_castImage(m_template, BW_IMAGE, m_template);
	// init vars
	int w = m_template->width();
	int h = m_template->height();

	// update button with filename (without path)
	m_button->setText(f.fileName());
	m_button->update();

	QImage q;
	IP_IPtoQImage(m_template, q);
	QPixmap p;
	p = QPixmap::fromImage(q);
	m_label_template->setPixmap(p);

	// Pass template image to GPU
	g_mainWindowP->glw()->setTemplateTexture(q);

	// apply filter to source image and display result
	g_mainWindowP->preview();

	return 1;
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Correlation::initShader:
//
// init shader program and parameters.
//
void
Correlate::initShader()
{
	m_nPasses = 1;

	// initialize GL function resolution for current context
	initializeGLFunctions();

	UniformMap uniforms;

	// init uniform hash table based on uniform variable names and location IDs
	uniforms["u_StepX"] = STEPX;
	uniforms["u_StepY"] = STEPY;
	uniforms["u_Sampler"] = SAMPLER;

	uniforms["u_SizeW_T"] = WSIZE_T;
	uniforms["u_SizeH_T"] = HSIZE_T;
	uniforms["u_StepX_T"] = STEPX_T;
	uniforms["u_StepY_T"] = STEPY_T;
	uniforms["u_Sampler_T"] = SAMPLER_T;

	uniforms["u_Sqrt_Sum_T"] = SQRT_SUM_T;

	QString v_name = ":/vshader_passthrough";
	QString f_name = ":/hw2/fshader_correlate";

#ifdef __APPLE__
	v_name += "_Mac";
	f_name += "_Mac";
#endif

	// compile shader, bind attribute vars, link shader, and initialize uniform var table
	g_mainWindowP->glw()->initShader(m_program[PASS1],
		v_name + ".glsl",
		f_name + ".glsl",
		uniforms,
		m_uniform[PASS1]);

	m_shaderFlag = true;
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Correlation::gpuProgram:
//
// Active gpu program
//
void
Correlate::gpuProgram(int pass)
{
	int w = m_template->width();
	int h = m_template->height();
	int type;
	float sum = 0.0;

	ChannelPtr<uchar> p1;
	IP_getChannel(m_template, 0, p1, type);
	for (int i = 0; i < w*h; i++) 
		sum += (p1[i] / 255.0)*(p1[i] / 255.0);

	glUseProgram(m_program[pass].programId());
	glUniform1i(m_uniform[pass][WSIZE_T], w);
	glUniform1i(m_uniform[pass][HSIZE_T], h);
	glUniform1f(m_uniform[pass][STEPX], (GLfloat) 1.0f / m_width);
	glUniform1f(m_uniform[pass][STEPY], (GLfloat) 1.0f / m_height);
	glUniform1i(m_uniform[pass][SAMPLER], 0);

	glUniform1f(m_uniform[pass][STEPX_T], (GLfloat) 1.0f / w);
	glUniform1f(m_uniform[pass][STEPY_T], (GLfloat) 1.0f / h);
	glUniform1i(m_uniform[pass][SAMPLER_T], 2);

	glUniform1f(m_uniform[pass][SQRT_SUM_T], pow(sum, 0.5));
}
