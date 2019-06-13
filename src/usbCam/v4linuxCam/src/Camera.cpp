/*
 * Camera.cpp
 *
 *  Created on: Jun 12, 2019
 *      Author: xtarke
 */

#include <cstdio>
#include <cerrno>
#include <ostream>

#include <errno.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>

#include <libv4l2.h>
#include <jpeglib.h>

#include <linux/videodev2.h>

#include "Camera.h"

//https://gist.github.com/jayrambhia/5866483

Camera::Camera() {

//	// check if its device
//	if (!S_ISCHR(st.st_mode)) {
//		fprintf(stderr, "%s is no device\n", "/dev/video0");
//		exit(EXIT_FAILURE);
//	}

	// open device
	fd = v4l2_open("/dev/video0", O_RDWR | O_NONBLOCK, 0);

	// check if opening was successfull
	if (-1 == fd) {
		fprintf(stderr, "Cannot open '%s': %d, %s\n", "/dev/video0", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}

	int ret = print_caps(fd);

	ret = init_mmap(fd);

	printf("init: %d\n", ret);

	ret = capture_image(fd);

}

Camera::~Camera() {
	v4l2_close(fd);

	fd = -1;
}

/**
Do ioctl and retry if error was EINTR ("A signal was caught during the ioctl() operation."). Parameters are the same as on ioctl.

\param fd file descriptor
\param request request
\param argp argument
\returns result from ioctl
*/
int Camera::xioctl(int fd, int request, void* argp)
{
	int r;

	do r = v4l2_ioctl(fd, request, argp);
	while (-1 == r && EINTR == errno);

	return r;
}

int Camera::print_caps(int fd){
	struct v4l2_capability caps = {};
	struct v4l2_cropcap cropcap = {0};
	struct v4l2_fmtdesc fmtdesc = {0};
	struct v4l2_format fmt = {0};

	if (xioctl(fd, VIDIOC_QUERYCAP, &caps) == -1)
		return -1;

	printf( "Driver Caps:\n"
			"  Driver: \"%s\"\n"
			"  Card: \"%s\"\n"
			"  Bus: \"%s\"\n"
			"  Version: %d.%d\n"
			"  Capabilities: %08x\n",
			caps.driver,
			caps.card,
			caps.bus_info,
			(caps.version>>16)&&0xff,
			(caps.version>>24)&&0xff,
			caps.capabilities);

	cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (-1 == xioctl (fd, VIDIOC_CROPCAP, &cropcap))
	{
		perror("Querying Cropping Capabilities");
		return 1;
	}

	printf( "Camera Cropping:\n"
			"  Bounds: %dx%d+%d+%d\n"
			"  Default: %dx%d+%d+%d\n"
			"  Aspect: %d/%d\n",
			cropcap.bounds.width, cropcap.bounds.height, cropcap.bounds.left, cropcap.bounds.top,
			cropcap.defrect.width, cropcap.defrect.height, cropcap.defrect.left, cropcap.defrect.top,
			cropcap.pixelaspect.numerator, cropcap.pixelaspect.denominator);

	int support_grbg10 = 0;

	fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	char fourcc[5] = {0};
	char c, e;
	printf("  FMT : CE Desc\n--------------------\n");

	while (xioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc) == 0)
	{
		strncpy(fourcc, (char *)&fmtdesc.pixelformat, 4);
		if (fmtdesc.pixelformat == V4L2_PIX_FMT_SGRBG10)
			support_grbg10 = 1;
		c = fmtdesc.flags & 1? 'C' : ' ';
		e = fmtdesc.flags & 2? 'E' : ' ';
		printf("  %s: %c%c %s\n", fourcc, c, e, fmtdesc.description);
		fmtdesc.index++;
	}


	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width = 640;
	fmt.fmt.pix.height = 480;
	//fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_BGR24;
	//fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_GREY;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
	fmt.fmt.pix.field = V4L2_FIELD_NONE;

	if (xioctl(fd, VIDIOC_S_FMT, &fmt) == -1)
	{
		perror("Setting Pixel Format");
		return 1;
	}

	strncpy(fourcc, (char *)&fmt.fmt.pix.pixelformat, 4);
	printf( "Selected Camera Mode:\n"
			"  Width: %d\n"
			"  Height: %d\n"
			"  PixFmt: %s\n"
			"  Field: %d\n",
			fmt.fmt.pix.width,
			fmt.fmt.pix.height,
			fourcc,
			fmt.fmt.pix.field);

	return 0;
}


int Camera::init_mmap(int fd)
{
    struct v4l2_requestbuffers req = {0};
    req.count = 1;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req))
    {
        perror("Requesting Buffer");
        return 1;
    }

    struct v4l2_buffer buf = {0};
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = 0;
    if(-1 == xioctl(fd, VIDIOC_QUERYBUF, &buf))
    {
        perror("Querying Buffer");
        return 1;
    }

    buffer = (uint8_t *)mmap (NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
    printf("Length: %d\nAddress: %p\n", buf.length, buffer);
    printf("Image Length: %d\n", buf.bytesused);

    return 0;
}

int Camera::capture_image(int fd)
{
	fd_set fds;
	struct timeval tv = {0};
	struct v4l2_buffer buf = {0};

    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = 0;

    if(xioctl(fd, VIDIOC_QBUF, &buf) == -1) {
        perror("capture_image");
        return -1;
    }

    if(xioctl(fd, VIDIOC_STREAMON, &buf.type) == -1) {
        perror("capture_image: Start Capture");
        return -1;
    }

    FD_ZERO(&fds);
    FD_SET(fd, &fds);

    tv.tv_sec = 2;
    int r = select(fd+1, &fds, NULL, NULL, &tv);

    if(r == -1) {
        perror("Waiting for Frame");
        return -1;
    }

    if(xioctl(fd, VIDIOC_DQBUF, &buf) == -1)
    {
        perror("Retrieving Frame");
        return 1;
    }

    printf ("Saving image:\n");

    //http://jwhsmith.net/2014/12/capturing-a-webcam-stream-using-v4l2/

    /* MJPEG is nothing but an animated extension of the JPEG format, a sequence of JPEG images.
	Since we captured a single frame here, there is no real MJPEG involved: all we have is a
	JPEG image‘s data. This means that if you want to transform your buffer into a file… all you
	have to do is write it: */

    int jpgfile;
    if((jpgfile = open("myimage.jpeg", O_WRONLY | O_CREAT, 0660)) < 0){
        perror("open");
        exit(1);
    }

    write(jpgfile, buffer, buf.length);
    close(jpgfile);

    return 0;
}

/**
	Write image to jpeg file.

	\param img image to write
*/
void Camera::jpegWrite(unsigned char* img, char* jpegFilename)
{

}