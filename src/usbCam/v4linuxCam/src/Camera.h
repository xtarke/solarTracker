/*
 * Camera.h
 *
 *  Created on: Jun 12, 2019
 *      Author: xtarke
 */

#ifndef CAMERA_H_
#define CAMERA_H_

class Camera {

private:
	int fd = -1;
	uint8_t *buffer;

	int xioctl(int fd, int request, void* argp);
	int print_caps(int fd);
	int init_mmap(int fd);
	int capture_image(int fd);
	void jpegWrite(unsigned char* img, char* jpegFilename);

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
