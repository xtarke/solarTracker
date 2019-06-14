/*
 * Camera.h
 *
 *  Created on: Jun 12, 2019
 *      Author: xtarke
 */

#ifndef CAMERA_H_
#define CAMERA_H_

#include <vector>

class Camera {

private:
	int fd = -1;
	uint8_t *buffer;

	int xioctl(int fd, int request, void* argp);
	int print_caps(int fd);
	int init_mmap(int fd);
	int capture_image(int fd);
	void jpegWrite(unsigned char* img, char* jpegFilename);


	void compressYUYVtoJPEG(uint8_t *input, const int width, const int height, std::vector<uint8_t>& output);

	struct buffer {
		void *start;
	    size_t length;
	};

	struct buffer *buffers = NULL;


public:
	Camera();
	virtual ~Camera();
};

#endif /* CAMERA_H_ */
