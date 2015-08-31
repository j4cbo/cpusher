/*
    SimpleClientGLView.m
	Syphon (SDK)
	
    Copyright 2010 bangnoise (Tom Butterworth) & vade (Anton Marini).
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#import "SimpleClientGLView.h"
#import "SimpleClientAppDelegate.h"

@implementation SimpleClientGLView

@synthesize syClient;

-(void) awakeFromNib
{	
	const GLint on = 1;
	[[self openGLContext] setValues:&on forParameter:NSOpenGLCPSwapInterval];
}


#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include "registry.h"
#include "pattern.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <math.h>
#include <sys/time.h>
#include <time.h>

int pusher_send_fd;

#define BRIGHTNESS 127

static void fill_pusher_address(struct sockaddr_in * dest, int i) {
    memset(dest, 0, sizeof *dest);
    dest->sin_family = AF_INET;
    dest->sin_port = htons(registry.pushers[i].last_broadcast.my_port);
    dest->sin_addr.s_addr = (registry.pushers[i].last_broadcast.ip[3] << 24)
    | (registry.pushers[i].last_broadcast.ip[2] << 16)
    | (registry.pushers[i].last_broadcast.ip[1] << 8)
    | registry.pushers[i].last_broadcast.ip[0];
}

uint32_t pixelBuffer[CLIENT_WINDOW_SIZE*CLIENT_WINDOW_SIZE];

rgb_t lookup_pixel(double x, double y) {
    if (x == 999 && y == 999) {
        rgb_t black = { 0,0,0 };
        return black;
    }

    int xscale = ((x / 2) + .5) * CLIENT_WINDOW_SIZE;
    int yscale = ((y / 2) + .5) * CLIENT_WINDOW_SIZE;
    if (xscale < 0) xscale = 0;
    if (xscale >= CLIENT_WINDOW_SIZE) xscale = CLIENT_WINDOW_SIZE;
    if (yscale < 0) yscale = 0;
    if (yscale >= CLIENT_WINDOW_SIZE) yscale = CLIENT_WINDOW_SIZE;
    int idx = yscale * CLIENT_WINDOW_SIZE + xscale;
    uint32_t argb = pixelBuffer[idx];
    //NSLog(@"lookupPixel: %f %f -> %d %d -> 0x%08x\n", x, y, xscale, yscale, argb);

    rgb_t ret;
    ret.r = argb & 0xff;
    ret.g = (argb >> 8) & 0xff;
    ret.b = (argb >> 16) & 0xff;
    return ret;
}

static int push_pusher(int pusher) {
    const struct pusher_broadcast *pb = &registry.pushers[pusher].last_broadcast;
    char buffer[1536];
    int strip = 0;
    size_t pixel = 0;
    int pixels_calculated = 0;
    int computed_spp = ((sizeof buffer) - 4) / (pb->pixels_per_strip * 3 + 1);
    int max_strips_per_packet = pb->max_strips_per_packet;
    struct sockaddr_in dest;
    size_t this_packet_size;
    fill_pusher_address(&dest, pusher);

    if (max_strips_per_packet > computed_spp) {
        max_strips_per_packet = computed_spp;
    }

    buffer[0] = buffer[1] = buffer[2] = buffer[3] = 0;

    const struct pusher_config *cfg = pusher_config_for(registry.pushers[pusher].id);
    if (!cfg) {
        return 0;
    }

    while (strip < pb->strips_attached) {
        /* Build a packet */
        int strip_in_packet = 0;
        while (strip_in_packet < max_strips_per_packet && strip < pb->strips_attached) {
            int i;
            int buffer_offset = 4 + (pb->pixels_per_strip * 3 + 1) * strip_in_packet;
            buffer[buffer_offset] = strip;
            for (i = 0; i < pb->pixels_per_strip; i++) {
                if (pixel >= cfg->valid_pixels) {
                    buffer[buffer_offset + 1 + 3*i] = 0;
                    buffer[buffer_offset + 2 + 3*i] = 0;
                    buffer[buffer_offset + 3 + 3*i] = 0;
                } else {
                    rgb_t rgb = lookup_pixel(cfg->pixel_locations[pixel].x,
                                             cfg->pixel_locations[pixel].y);
                    buffer[buffer_offset + 1 + 3*i] = rgb.r;
                    buffer[buffer_offset + 2 + 3*i] = rgb.g;
                    buffer[buffer_offset + 3 + 3*i] = rgb.b;
                    pixel++;
                    pixels_calculated++;
                }
            }

            strip++;
            strip_in_packet++;
        }

        this_packet_size = 4 + strip_in_packet * (pb->pixels_per_strip * 3 + 1);
        if (sendto(pusher_send_fd, buffer, this_packet_size, 0, (const struct sockaddr *)&dest, sizeof dest) < 0) {
            perror("sendto");
        }
    }

    return pixels_calculated;
}

#define FPS 60
/*
int main() {
    int pusher;

    struct timeval last_frame;
    struct timeval frame_interval = { 0, 1000000 / FPS };
    gettimeofday(&last_frame, NULL);

    uint64_t sleep_total_time = 0;
    int sleep_count = 0;
    int frame_counter = 0;

    registry_init();
    registry_lock();

    while (1) {        if (++frame_counter == FPS) {
            frame_counter = 0;
            if (sleep_count) {
                printf("%d pixels; avg sleep: %d\n", pixels_pushed, (int)(sleep_total_time / sleep_count));
            }

            for (pusher = 0; pusher < registry.num_pushers; pusher++) {
                if (!pusher_config_for(registry.pushers[pusher].id)) {
                    printf("\n/!\\ UNKNOWN PUSHER: %06x\n\n", registry.pushers[pusher].id);
                }
            }

            sleep_count = 0;
            sleep_total_time = 0;
        }

        struct timeval now, next_frame, diff;
        gettimeofday(&now, NULL);
        timeradd(&last_frame, &frame_interval, &next_frame);
        if (timercmp(&now, &next_frame, <)) {
            timersub(&next_frame, &now, &diff);
            uint64_t usec = (uint64_t)diff.tv_sec * 1000000 + (uint64_t)diff.tv_usec;
            usleep(usec);
            sleep_total_time += usec;
            sleep_count++;
        } else {
            timersub(&now, &next_frame, &diff);
            uint64_t usec = (uint64_t)diff.tv_sec * 1000000 + (uint64_t)diff.tv_usec;
            printf("dropped frame - %lld us\n", (long long)usec);
            memcpy(&next_frame, &now, sizeof now);
        }
        
        memcpy(&last_frame, &next_frame, sizeof next_frame);
        
        registry_lock();
    }
}
*/
static void processBuffer(uint32_t *buffer) {
    uint32_t center = buffer[CLIENT_WINDOW_SIZE * (CLIENT_WINDOW_SIZE/2) + (CLIENT_WINDOW_SIZE/2)];

    int pixels_pushed = 0;
    registry_lock();
    for (int pusher = 0; pusher < registry.num_pushers; pusher++) {
        pixels_pushed += push_pusher(pusher);
    }

    registry_unlock();
}

- (void)drawRect:(NSRect)dirtyRect
{
	[[self openGLContext] makeCurrentContext];
	
	CGLContextObj cgl_ctx = [[self openGLContext] CGLContextObj];
	
	NSRect frame = self.frame;
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);
		
	// Setup OpenGL states
	glViewport(0, 0, frame.size.width, frame.size.height);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0.0, frame.size.width, 0.0, frame.size.height, -1, 1);
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	
	// Get a new frame from the client
	SyphonImage *image = [syClient newFrameImageForContext:cgl_ctx];
	if(image)
	{
		glEnable(GL_TEXTURE_RECTANGLE_ARB);
		glBindTexture(GL_TEXTURE_RECTANGLE_ARB, [image textureName]);

        // let's snag the texture out
        glGetTexImage(GL_TEXTURE_RECTANGLE_ARB, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);

		// do a nearest linear interp.
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

		glColor4f(1.0, 1.0, 1.0, 1.0);
		
		// why do we need it ?
		glDisable(GL_BLEND);
		
		NSSize imageSize = [image textureSize];
		NSSize scaled;
		float wr = imageSize.width / frame.size.width;
		float hr = imageSize.height / frame.size.height;
		float ratio;
		ratio = (hr < wr ? wr : hr);
		scaled = NSMakeSize((imageSize.width / ratio), (imageSize.height / ratio));
		
		GLfloat tex_coords[] = 
		{
			0.0,	0.0,
			imageSize.width,	0.0,
			imageSize.width,	imageSize.height,
			0.0,	imageSize.height
		};
		
		
		float halfw = scaled.width * 0.5;
		float halfh = scaled.height * 0.5;
		
		GLfloat verts[] = 
		{
			-halfw, -halfh,
			halfw, -halfh,
			halfw, halfh,
			-halfw, halfh
		};
		
		glTranslated(frame.size.width * 0.5, frame.size.height * 0.5, 0.0);
		
		glEnableClientState( GL_TEXTURE_COORD_ARRAY );
		glTexCoordPointer(2, GL_FLOAT, 0, tex_coords );
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(2, GL_FLOAT, 0, verts );
		glDrawArrays( GL_TRIANGLE_FAN, 0, 4 );



		glDisableClientState( GL_TEXTURE_COORD_ARRAY );
		glDisableClientState(GL_VERTEX_ARRAY);

        glReadPixels(0, 0, CLIENT_WINDOW_SIZE, CLIENT_WINDOW_SIZE, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, pixelBuffer);
        processBuffer(pixelBuffer);

        GLenum err = GL_NO_ERROR;
        while((err = glGetError()) != GL_NO_ERROR)
        {
            NSLog(@"err: %d\n", err);//Process/log the error.
        }

        glBindTexture(GL_TEXTURE_RECTANGLE_ARB, 0);


		// We are responsible for releasing the frame
		[image release];
	}
	
	// Restore OpenGL states
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	
	[[self openGLContext] flushBuffer];
}


@end
