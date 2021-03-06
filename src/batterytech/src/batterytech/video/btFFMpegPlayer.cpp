/*
 * BatteryTech
 * Copyright (c) 2010 Battery Powered Games LLC.
 *
 * This code is a component of BatteryTech and is subject to the 'BatteryTech
 * End User License Agreement'.  Among other important provisions, this
 * license prohibits the distribution of source code to anyone other than
 * authorized parties.  If you have any questions or would like an additional
 * copy of the license, please contact: support@batterypoweredgames.com
 */

//============================================================================
// Name        : btFFMpegPlayer.cpp
// Description : Experimental FFMpeg support - not production ready
//============================================================================

#include "../batterytech_globals.h"

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

#ifdef WITH_FFMPEG
#if _WIN32 || (TARGET_OS_MAC && !TARGET_OS_IPHONE)

#include "btFFMpegPlayer.h"
#include "../util/strx.h"
#include "../platform/platformgeneral.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <iostream>
#include "../Context.h"
#include "../render/GraphicsConfiguration.h"
#include "../Logger.h"
#include "../render/ShaderProgram.h"
#include "../render/Texture.h"
#include "../render/QuadRenderer.h"
#include "../render/RenderContext.h"

#define FFMPEG_SCALE_WIDTH 512
#define FFMPEG_SCALE_HEIGHT 512

using namespace std;


	btFFMpegPlayer::btFFMpegPlayer(Context *context, const char *assetName) {
		this->context = context;
		char myFilename[255];
		strcpy(myFilename, "assets\\");
		strcat(myFilename, assetName);
		_platform_convert_path(myFilename, myFilename);
		this->assetName = strDuplicate(myFilename);
		initialized = FALSE;
		position = 0;
		vidFramePosition = 0;
		state = STATE_NONE;
		initializePlayer();
		initializeGL();
        audioReady = false;
        audioBytesLeft = 0;
        frameBytesUsed = 0;
	}

	btFFMpegPlayer::~btFFMpegPlayer() {
		delete [] assetName;
		assetName = NULL;
		context = NULL;
	    // Free the RGB image
	    free(videoBuffer);
	    av_free(videoFrameRGB);

	    // Free the YUV frame
	    av_free(videoFrame);

	    // Close the codec
	    avcodec_close(videoCodecContext);

	    // Close the video file
	    avformat_close_input(&pFormatCtx);
	    glDeleteTextures(1, &textureId);

	}

	void btFFMpegPlayer::play(BOOL32 loop) {
		// TODO support looping
		state = STATE_PLAYING;
	}

	void btFFMpegPlayer::stop() {
		state = STATE_NONE;
	}

	void btFFMpegPlayer::initializePlayer() {
	    av_register_all();
	    pFormatCtx = avformat_alloc_context();
	    if(avformat_open_input(&pFormatCtx, assetName, NULL, NULL)!=0) {
	    	cout << "Unable to open file " << assetName << endl;
	    	return;
	    }
	     // Retrieve stream information
	     if(avformat_find_stream_info(pFormatCtx, NULL)<0) {
	    	 cout << "Couldn't find stream information for " << assetName << endl;
	    	 return;
	     }
	     // Dump information about file onto standard error
	     av_dump_format(pFormatCtx, 0, assetName, false);
	     // Find the first video stream
	     videoStream = -1;
	     for(unsigned int i=0; i<pFormatCtx->nb_streams; i++)
	         if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO) {
	             videoStream = i;
	             break;
	         }
	     if (videoStream == -1) {
	    	 cout << "Didn't find a video stream for " << assetName << endl;
	    	 return;
	     } else {
		     // Get a pointer to the codec context for the video stream
		     videoCodecContext = pFormatCtx->streams[videoStream]->codec;
		     // Find the decoder for the video stream
		     videoCodec = avcodec_find_decoder(videoCodecContext->codec_id);
		     if(videoCodec==NULL) {
		    	 cout << "Video codec not found for " << assetName << endl;
		    	 return;
		     }
		     // Open video codec
		     if(avcodec_open2(videoCodecContext, videoCodec, NULL)<0) {
		    	 cout << "Could not open codec for " << assetName << endl;
		    	 return;
		     }
		     // Hack to correct wrong frame rates that seem to be generated by some codecs
		     if(videoCodecContext->time_base.num>1000 && videoCodecContext->time_base.den==1) {
		 		videoCodecContext->time_base.den=1000;
		     }
		     // Allocate video frame
		     videoFrame = avcodec_alloc_frame();
		     // Allocate an AVFrame structure
		     videoFrameRGB=avcodec_alloc_frame();
		     if(videoFrameRGB==NULL) {
		    	 cout << "Unable to allocate frame for " << assetName << endl;
		    	 return;
		     }
		     // Determine required buffer size and allocate buffer
		     //numBytes=avpicture_get_size(PIX_FMT_RGB24, videoCodecContext->width, videoCodecContext->height);
		     int numBytes=avpicture_get_size(PIX_FMT_RGB24, FFMPEG_SCALE_WIDTH, FFMPEG_SCALE_HEIGHT);
		     videoBuffer=(uint8_t*)malloc(numBytes);
             
		     // Assign appropriate parts of buffer to image planes in videoFrameRGB
		     avpicture_fill((AVPicture *)videoFrameRGB, videoBuffer, PIX_FMT_RGB24, FFMPEG_SCALE_WIDTH, FFMPEG_SCALE_HEIGHT);
	     }
	     // find the first audio stream
	     audioStream = -1;
	     for (unsigned int i=0; i<pFormatCtx->nb_streams; i++)
	         if (pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_AUDIO) {
	        	 audioStream  =i;
	             break;
	         }
	     if (audioStream == -1) {
	    	 cout << "Didn't find an audio stream for " << assetName << endl;
	     } else {
	    	 audioCodecContext = pFormatCtx->streams[audioStream]->codec;
	    	 audioCodec = avcodec_find_decoder(audioCodecContext->codec_id);
	    	 if (!audioCodec) {
				 cout << "Audio codec not found for " << assetName << endl;
				 return;
			 }
		     // Open video codec
		     if(avcodec_open2(audioCodecContext, audioCodec, NULL)<0) {
		    	 cout << "Could not open codec for " << assetName << endl;
		    	 return;
		     }
             
             int bufferSize = 200;
             audioRingBuffer = new RingBuffer<AVFrame>(bufferSize);
            
             for( int i = 0; i < bufferSize; ++i )
             {
                 audioRingBuffer->setAt(i,avcodec_alloc_frame());
             }
             audioBufferSize = 40000;
             audioBuffer = (uint8_t*)malloc(audioBufferSize);
	     }

	     initialized = TRUE;
	     state = STATE_LOADED;
	}

	void btFFMpegPlayer::initializeGL() {
		GLuint textureIds[1];
		glGenTextures(1, textureIds);
		textureId = textureIds[0];
	}

	void btFFMpegPlayer::renderFrame() {
		if (!initialized) {
			cout << "Player not initialized" << endl;
			return;
		}
		if (state == STATE_PLAYING) {
			//while (vidFramePosition < position) {
				//decodeFrame();
			//}
		}
	    glBindBuffer(GL_ARRAY_BUFFER, 0);
	    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	    glFrontFace(GL_CW);
		glDisable(GL_DEPTH_TEST);
		context->renderContext->projMatrix.identity();
		context->renderContext->mvMatrix.identity();
		context->renderContext->projMatrix.ortho(0, context->gConfig->width, context->gConfig->height, 0, -1, 1);
		context->renderContext->colorFilter = Vector4f(1, 1, 1, 1);
	    Texture::lastTextureId = textureId;
		// render frame to GL
		glBindTexture(GL_TEXTURE_2D, textureId);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, FFMPEG_SCALE_WIDTH, FFMPEG_SCALE_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, videoFrameRGB->data[0]);
		glBindTexture(GL_TEXTURE_2D, textureId);
		int left = 0;
		int top = 0;
		int right = context->gConfig->viewportWidth;
		int bottom = context->gConfig->viewportHeight;
		context->quadRenderer->render(NULL, top, right, bottom, left);
	}

	void btFFMpegPlayer::decodeFrame() {
		if (av_read_frame(pFormatCtx, &packet)>=0) {
			if (packet.stream_index == audioStream) {
				int consumed = 0;
				while (consumed < packet.size) {
	                AVFrame* pAudioFrame = audioRingBuffer->getCurrent();
	                //cout << "writing to ring buffer " << audioRingBuffer->fillPoint << endl;
					int result = avcodec_decode_audio4(audioCodecContext, pAudioFrame, &frameFinished, &packet);
					if (result < 0) {
						break;
					} else {
						consumed += result;
						// cout << "Consumed " << consumed << " of " << packet.size << endl;
					}
					if (frameFinished) {
	                    // cout << "decoded!\n";
	                    audioReady = true;
	                    audioBufferSize = av_samples_get_buffer_size(NULL, audioCodecContext->channels,
	                                                                 pAudioFrame->nb_samples,
	                                                                 audioCodecContext->sample_fmt, 1);
						//memcpy(audioBuffer,pAudioFrame->data[0],audioBufferSize);
					}
				}
			} else if (packet.stream_index == videoStream) {
				avcodec_decode_video2(videoCodecContext, videoFrame, &frameFinished, &packet);
				if (frameFinished) {
					vidFramePosition = (float)packet.pts / videoCodecContext->time_base.den;
					static struct SwsContext *img_convert_ctx;
					// Convert the image into YUV format that SDL uses
					if(img_convert_ctx == NULL) {
						int w = videoCodecContext->width;
						int h = videoCodecContext->height;
						img_convert_ctx = sws_getContext(w, h,
										videoCodecContext->pix_fmt,
										FFMPEG_SCALE_WIDTH, FFMPEG_SCALE_HEIGHT, PIX_FMT_RGB24, SWS_BICUBIC,
										NULL, NULL, NULL);
						if(img_convert_ctx == NULL) {
							fprintf(stderr, "Cannot initialize the conversion context!\n");
							return;
						}
					}
					sws_scale(img_convert_ctx, videoFrame->data, videoFrame->linesize, 0,
							  videoCodecContext->height, videoFrameRGB->data, videoFrameRGB->linesize);
				}
			}
			av_free_packet(&packet);
		} else {
			state = STATE_ENDED;
		}
	}

	// bufferLen is in bytes
	// pSoundBuffer takes shorts
	void btFFMpegPlayer::addAudioBuffer(void *pSoundBuffer, long bufferLen) {
        if(state == STATE_PLAYING) {
     		long bufferBytesWanted = bufferLen;
            long bufferBytesCopied = 0;
            while( bufferBytesWanted > 0 ) {
                if( audioBytesLeft == 0 ) {
                   //  cout << "fill\n";
                    audioFrame = audioRingBuffer->read();
                    //cout << "reading from ring buffer " << audioRingBuffer->oldest << endl;
                    while (audioFrame == NULL) {
                    //cout << "NULL - decoding from addAudioBuffer " << endl;
                   		decodeFrame();
                    	audioFrame = audioRingBuffer->read();
                        //cout << "reading from ring buffer " << audioRingBuffer->oldest << endl;
                    }
                    audioBytesLeft = av_samples_get_buffer_size(NULL, audioCodecContext->channels, audioFrame->nb_samples, audioCodecContext->sample_fmt, 1);
                    // audioBytesLeft = audioFrame->nb_samples * 4;
                    // cout << "audioBytesLeft = " << audioBytesLeft << endl;
                    frameBytesUsed = 0;
                }
                long bytesToCopy = FFMIN(bufferBytesWanted,audioBytesLeft);
                memcpy((unsigned char*)pSoundBuffer+bufferBytesCopied, audioFrame->data[0]+frameBytesUsed, bytesToCopy);
                audioBytesLeft -= bytesToCopy;
                frameBytesUsed += bytesToCopy;
                bufferBytesCopied += bytesToCopy;
                bufferBytesWanted -= bytesToCopy;
            }
           // cout << "copyied,left,size" << bufferBytesCopied << " " << audioBytesLeft << " " << audioBufferSize << "\n";
        }
	}

	void btFFMpegPlayer::update(F32 delta) {
		if (state == STATE_PLAYING) {
			position += delta;
		}
	}

	void btFFMpegPlayer::pause() {
		state = STATE_PAUSED;
	}

	F32 btFFMpegPlayer::getPosition() {
		if (state == STATE_ENDED ) return -1;
		return position;
	}

	BOOL32 btFFMpegPlayer::isPlaying() {
		return (state == STATE_PLAYING);
	}

	BOOL32 btFFMpegPlayer::isPaused() {
		return (state == STATE_PAUSED);
	}

	BOOL32 btFFMpegPlayer::isEnded() {
		return (state == STATE_ENDED);
	}

#endif
#endif
