/**
 * TIandSIprocmd
 * (TIandSI Professional CMD version)
 *
 * 雷霄骅 Lei Xiaohua,张晖 Zhang Hui
 * leixiaohua1020@126.com
 * zhanghuicuc@gmail.com
 * 中国传媒大学/数字电视技术
 * Communication University of China / Digital TV Technology
 *
 * http://blog.csdn.net/leixiaohua1020
 * 
 * 本工程可以计算一个压缩视频的时间信息TI（Temporal perceptual Information，
 * 也可以称时间复杂度）和空间信息SI（Spatial perceptual Information，也可以
 * 称空间复杂度）。计算方法出自标准：ITU-R BT.1788
 *
 * Professional: 支持压缩码流（而不是像素数据比如YUV，RGB）作为输入
 *
 * This software can calculate a video bitstream's TI(Temporal perceptual Information) 
 * and SI(Spatial perceptual Information) according to ITU-R BT.1788.
 *
 * Professional: Support bitstreams (not raw data such as YUV, RGB, etc.) as Input.
 *
 */
#include "TIandSIprocmd.h"

#ifdef WIN32
#include "XGetopt.h"
#else
#include <unistd.h>
#endif

#ifdef _DEBUG
#include <crtdbg.h>
#endif

#define REFRESH_EVENT  (SDL_USEREVENT + 1)
#define PADDING 19	//default 1
#define FRAMENUM 250

#pragma comment(lib, "SDL.lib")
#pragma comment(lib, "SDLmain.lib")

typedef struct SDLParam{
	SDL_Surface *screen; 
	SDL_Overlay *bmp; 
	SDL_Rect rect;
	bool graphically_ti;
	bool graphically_si;
	bool isinterval;
	char *show_YBuffer;
	char *show_UVBuffer;
	//Show Info
	int show_w;
	int show_h;
	//Pixel Info
	int pixel_w;
	int pixel_h;
}SDLParam;

int mark_exit=0;


int show_thread(void *opaque){
	SDLParam *sdlparam=(SDLParam *)opaque;

	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {  
		printf( "Could not initialize SDL - %s\n", SDL_GetError()); 
		return 0;
	} 

	sdlparam->screen = SDL_SetVideoMode(sdlparam->show_w, sdlparam->show_h, 0, 0);
	if(!sdlparam->screen) {  
		printf("SDL: could not set video mode - exiting\n");  
		return 0;
	}
	sdlparam->bmp = SDL_CreateYUVOverlay(sdlparam->pixel_w, sdlparam->pixel_h,SDL_YV12_OVERLAY, sdlparam->screen); 

	if(sdlparam->graphically_si){
		SDL_WM_SetCaption("Spatial perceptual Information",NULL);
	}else if(sdlparam->graphically_ti){
		SDL_WM_SetCaption("Temporal perceptual Information",NULL);
	}

	sdlparam->rect.x = 0;    
	sdlparam->rect.y = 0;    
	sdlparam->rect.w = sdlparam->show_w;    
	sdlparam->rect.h = sdlparam->show_h;    

	SDL_Event event;
	while(mark_exit==0) {
		SDL_WaitEvent(&event);
		switch(event.type){
		case REFRESH_EVENT:{
			SDL_LockYUVOverlay(sdlparam->bmp);
			sdlparam->bmp->pixels[0]=(Uint8 *)sdlparam->show_YBuffer;
			sdlparam->bmp->pixels[2]=(Uint8 *)sdlparam->show_UVBuffer;
			sdlparam->bmp->pixels[1]=(Uint8 *)sdlparam->show_UVBuffer+sdlparam->pixel_w*sdlparam->pixel_h/4;     
			sdlparam->bmp->pitches[0]=sdlparam->pixel_w;
			sdlparam->bmp->pitches[2]=sdlparam->pixel_w/2;   
			sdlparam->bmp->pitches[1]=sdlparam->pixel_w/2;
			SDL_UnlockYUVOverlay(sdlparam->bmp); 
			SDL_DisplayYUVOverlay(sdlparam->bmp, &sdlparam->rect); 
			break;
						   }
		case SDL_QUIT:{
			mark_exit=1;
			break;
					  }

		}
	}
	return 0;
}

//帮助
void usage(){
	printf("\nHelp==================================\n");
	printf("TISIpro version 1.0:\n");
	printf("This software calculate value of TI and SI, and save result as *.csv file\n\n");
	printf("Lei Xiaohua,Zhang Hui \n");
	printf("leixiaohua1020@126.com\n");
	printf("Communication University of China / Digital TV Technology \n");
	printf("http://blog.csdn.net/leixiaohua1020\n\n");
	printf("----------------------------------------\n");
	printf("Options:\n");
	printf("-i\tInput file URL\n");
	printf("-o\tOutput *.csv file URL [Default is {Input Name}.csv]\n");
	printf("-l\tLimit calculate frame number\n");
	printf("-g\tShow result Graphically [Option is 'ti' or 'si']\n");
	printf("-n\tInterval to show result to the screen [Default is 5]\n");
	printf("-x\tvideo width if input is rawvideo\n");
	printf("-y\tvideo height if input is rawvideo\n");
	printf("-h\tShow this text.\n");
	printf("----------------------------------------\n");
	printf("Examples:\n");
	printf("TISIprogCmd -i test.avi\n");
	printf("TISIprogCmd -i test.avi -o test_cal.csv\n");
	printf("TISIprogCmd -i test.avi -g ti\n");
	printf("=========================================\n");
}

int comp(const void * elem1, const void * elem2) 
{
	float f = *((float*)elem1);
	float s = *((float*)elem2);
	if (f > s) return  1;
	if (f < s) return -1;
	return 0;
}

//计算TI、SI
int tisi(char* ydata,char* prev_ydata,int width,int height,SDLParam sdlparam,float &TI,float &SI)
{
	int nYSize=height*width;
	float nFrameSize = nYSize*1.5;
	int realframe_size=(width-2*PADDING)*(height-2*PADDING);

	unsigned char *pFrame=(unsigned char*)malloc(nYSize);
	unsigned char *pNextFrame=(unsigned char*)malloc(nYSize);
	unsigned char *pFrame_0;
	unsigned char *pFrame_1;
	unsigned char *pFrame_2;
	unsigned char *pNextFrame_0;
	unsigned char *pSobelScreen=(unsigned char*)malloc(realframe_size);
	unsigned char *pDiffScreen=(unsigned char*)malloc(realframe_size);
	memset(pSobelScreen,0,realframe_size);
	memset(pDiffScreen,0,realframe_size);

	float *frame_sobel=(float*)malloc(realframe_size*sizeof(float));
	memset(frame_sobel,0.0f,realframe_size*sizeof(float));
	float avg_frame_sobel=0;
	int index=0;

	float *frame_diff=(float*)malloc(realframe_size*sizeof(float));
	memset(frame_diff,0.0f,realframe_size*sizeof(float));
	float avg_frame_diff=0;

	__m128 sobel_avg_sum=_mm_set1_ps(+0.0f);
	__m128 sobel_square_sum=_mm_set1_ps(+0.0f);
	__m128 diff_avg_sum=_mm_set1_ps(+0.0f);
	__m128 diff_square_sum=_mm_set1_ps(+0.0f);
	__m128 avg_sobel=_mm_set1_ps(+0.0f);
	__m128 avg_diff=_mm_set1_ps(+0.0f);

	int i,j,k;
	int pad_threshold=0;
	
	memcpy(pFrame,prev_ydata,nYSize);
	memcpy(pNextFrame,ydata,nYSize);

	pFrame_0=pFrame+width*(PADDING-1);
	pFrame_1=pFrame+width*PADDING;
	pFrame_2=pFrame+width*(PADDING+1);
	pNextFrame_0=pNextFrame+width*(PADDING-1);

	//Check
	if(mark_exit==1){
		return -1;
	}	

	for(j = PADDING; j < height-PADDING; j++)
	{
		for(i = PADDING; i < width-PADDING; i+=4)
		{
			if(i+4>width-PADDING)
				pad_threshold=1;
			// load 16 components. (0~6 will be used)
			__m128i current_0 = _mm_unpacklo_epi8(_mm_loadu_si128((__m128i*)(pFrame_0+i-1)), _mm_setzero_si128());
			__m128i current_1 = _mm_unpacklo_epi8(_mm_loadu_si128((__m128i*)(pFrame_1+i-1)), _mm_setzero_si128());
			__m128i current_2 = _mm_unpacklo_epi8(_mm_loadu_si128((__m128i*)(pFrame_2+i-1)), _mm_setzero_si128());
			__m128i next_0 = _mm_unpacklo_epi8(_mm_loadu_si128((__m128i*)(pNextFrame_0+i-1)), _mm_setzero_si128());

			// pFrame_00 = { pFrame_0[i-1], pFrame_0[i], pFrame_0[i+1], pFrame_0[i+2] }
			__m128 pFrame_00 = _mm_cvtepi32_ps(_mm_unpacklo_epi16(current_0, _mm_setzero_si128()));
			// pFrame_01 = { pFrame_0[i], pFrame_0[i+1], pFrame_0[i+2], pFrame_0[i+3] }
			__m128 pFrame_01 = _mm_cvtepi32_ps(_mm_unpacklo_epi16(_mm_srli_si128(current_0, 2), _mm_setzero_si128()));
			// pFrame_02 = { pFrame_0[i+1], pFrame_0[i+2], pFrame_0[i+3], pFrame_0[i+4] }
			__m128 pFrame_02 = _mm_cvtepi32_ps(_mm_unpacklo_epi16(_mm_srli_si128(current_0, 4), _mm_setzero_si128()));
			// pFrame_10 = { pFrame_1[i-1], pFrame_1[i], pFrame_1[i+1], pFrame_1[i+2] }
			__m128 pFrame_10 = _mm_cvtepi32_ps(_mm_unpacklo_epi16(current_1, _mm_setzero_si128()));
			// pFrame_12 = { pFrame_1[i+1], pFrame_1[i+2], pFrame_1[i+3], pFrame_1[i+4] }
			__m128 pFrame_12 = _mm_cvtepi32_ps(_mm_unpacklo_epi16(_mm_srli_si128(current_1, 4), _mm_setzero_si128()));
			// pFrame_20 = { pFrame_2[i-1], pFrame_2[i], pFrame_2[i+1], pFrame_2[i+2] }
			__m128 pFrame_20 = _mm_cvtepi32_ps(_mm_unpacklo_epi16(current_2, _mm_setzero_si128()));
			// pFrame_21 = { pFrame_2[i], pFrame_2[i+1], pFrame_2[i+2], pFrame_2[i+3] }
			__m128 pFrame_21 = _mm_cvtepi32_ps(_mm_unpacklo_epi16(_mm_srli_si128(current_2, 2), _mm_setzero_si128()));
			// pFrame_22 = { pFrame_2[i+1], pFrame_2[i+2], pFrame_2[i+3], pFrame_2[i+4] }
			__m128 pFrame_22 = _mm_cvtepi32_ps(_mm_unpacklo_epi16(_mm_srli_si128(current_2, 4), _mm_setzero_si128()));
			
			__m128 pNextFrame_00 = _mm_cvtepi32_ps(_mm_unpacklo_epi16(next_0, _mm_setzero_si128()));
			
			__m128 gx=_mm_add_ps(_mm_sub_ps(_mm_add_ps(_mm_add_ps(_mm_sub_ps(_mm_sub_ps(_mm_sub_ps(pFrame_20,pFrame_22),pFrame_12),pFrame_12),pFrame_10),pFrame_10),pFrame_02),pFrame_00);

			__m128 gy=_mm_sub_ps(_mm_sub_ps(_mm_add_ps(_mm_add_ps(_mm_add_ps(_mm_sub_ps(pFrame_00,pFrame_20),_mm_sub_ps(pFrame_02,pFrame_22)),pFrame_01),pFrame_01),pFrame_21),pFrame_21);

			__m128 sobel_result = _mm_sqrt_ps(_mm_add_ps(_mm_mul_ps(gx, gx), _mm_mul_ps(gy,gy)));

			__m128 diff_result=_mm_sub_ps(pNextFrame_00,pFrame_00);
			
			if(!pad_threshold)
			{
				frame_sobel[index]=sobel_result.m128_f32[0];
				frame_sobel[index+1]=sobel_result.m128_f32[1];
				frame_sobel[index+2]=sobel_result.m128_f32[2];
				frame_sobel[index+3]=sobel_result.m128_f32[3];
				pSobelScreen[index]=(unsigned char)sobel_result.m128_f32[0];
				pSobelScreen[index+1]=(unsigned char)sobel_result.m128_f32[1];
				pSobelScreen[index+2]=(unsigned char)sobel_result.m128_f32[2];
				pSobelScreen[index+3]=(unsigned char)sobel_result.m128_f32[3];
				
				frame_diff[index]=diff_result.m128_f32[0];
				frame_diff[index+1]=diff_result.m128_f32[1];
				frame_diff[index+2]=diff_result.m128_f32[2];
				frame_diff[index+3]=diff_result.m128_f32[3];
				pDiffScreen[index]=(unsigned char)abs(diff_result.m128_f32[0]);
				pDiffScreen[index+1]=(unsigned char)abs(diff_result.m128_f32[1]);
				pDiffScreen[index+2]=(unsigned char)abs(diff_result.m128_f32[2]);
				pDiffScreen[index+3]=(unsigned char)abs(diff_result.m128_f32[3]);
				index+=4;
			}
			else
			{
				for(k=0;k<width-PADDING-i;k++)
				{
					frame_sobel[index+k]=sobel_result.m128_f32[k];
					pSobelScreen[index+k]=(unsigned char)sobel_result.m128_f32[k];
					frame_diff[index+k]=diff_result.m128_f32[k];
					pDiffScreen[index+k]=(unsigned char)abs(diff_result.m128_f32[k]);
				}
				index+=width-PADDING-i;
			}		
		}
		pFrame_0 += width;
		pFrame_1 += width;
		pFrame_2 += width;
		pNextFrame_0 += width;
		pad_threshold=0;
	}

	//画图
	if(sdlparam.graphically_si==true&&sdlparam.isinterval==false){
		memcpy(sdlparam.show_YBuffer,pSobelScreen,realframe_size);
		SDL_Event event;
		event.type = REFRESH_EVENT;
		SDL_PushEvent(&event);
	}

	//画图
	if(sdlparam.graphically_ti==true&&sdlparam.isinterval==false){
		memcpy(sdlparam.show_YBuffer,pDiffScreen,realframe_size);
		SDL_Event event;
		event.type = REFRESH_EVENT;
		SDL_PushEvent(&event);
	}

	for(i=0;i<index;i+=4)
	{
		__m128 sobel_result_0 = _mm_set_ps (frame_sobel[i],frame_sobel[i+1], frame_sobel[i+2], frame_sobel[i+3]);
		sobel_avg_sum = _mm_add_ps(sobel_avg_sum,sobel_result_0);

		__m128 diff_result_0 = _mm_set_ps (frame_diff[i],frame_diff[i+1], frame_diff[i+2], frame_diff[i+3]);
		diff_avg_sum = _mm_add_ps(diff_avg_sum,diff_result_0);
	}		
	avg_frame_sobel=(sobel_avg_sum.m128_f32[0]+
		sobel_avg_sum.m128_f32[1]+
		sobel_avg_sum.m128_f32[2]+
		sobel_avg_sum.m128_f32[3])/index;
	avg_sobel = _mm_set_ps (avg_frame_sobel,avg_frame_sobel, avg_frame_sobel, avg_frame_sobel);

	avg_frame_diff=(diff_avg_sum.m128_f32[0]+
		diff_avg_sum.m128_f32[1]+
		diff_avg_sum.m128_f32[2]+
		diff_avg_sum.m128_f32[3])/index;
	avg_diff = _mm_set_ps (avg_frame_diff,avg_frame_diff, avg_frame_diff, avg_frame_diff);

	for(i=0;i<index;i+=4)
	{
		__m128 sobel_result_1 = _mm_set_ps (frame_sobel[i],frame_sobel[i+1], frame_sobel[i+2], frame_sobel[i+3]);
		__m128 sobel_square=_mm_mul_ps(_mm_sub_ps(sobel_result_1,avg_sobel),_mm_sub_ps(sobel_result_1,avg_sobel));
		sobel_square_sum = _mm_add_ps(sobel_square_sum,sobel_square);

		__m128 diff_result_1 = _mm_set_ps (frame_diff[i],frame_diff[i+1], frame_diff[i+2], frame_diff[i+3]);
		__m128 diff_square=_mm_mul_ps(_mm_sub_ps(diff_result_1,avg_diff),_mm_sub_ps(diff_result_1,avg_diff));
		diff_square_sum = _mm_add_ps(diff_square_sum,diff_square);
	}
	
	SI=sqrt((sobel_square_sum.m128_f32[0]+sobel_square_sum.m128_f32[1]+sobel_square_sum.m128_f32[2]+sobel_square_sum.m128_f32[3])/index);
	avg_frame_sobel=0;
	sobel_avg_sum=_mm_set1_ps(+0.0f);
	sobel_square_sum=_mm_set1_ps(+0.0f);
	
	TI=sqrt((diff_square_sum.m128_f32[0]+diff_square_sum.m128_f32[1]+diff_square_sum.m128_f32[2]+diff_square_sum.m128_f32[3])/index);
	avg_frame_diff=0;
	diff_avg_sum=_mm_set1_ps(+0.0f);
	diff_square_sum=_mm_set1_ps(+0.0f);

	index=0;

	//--------
	delete pFrame;
	delete pNextFrame;
	delete pSobelScreen;
	delete pDiffScreen;
	delete frame_diff;
	delete frame_sobel;

	return 0;
}

int main(int argc, char* argv[])
{
#ifdef _DEBUG
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	//_CrtSetBreakAlloc(1166);
#endif

	char in_url[500]={0};
	char out_url[500]={0};
	int limit_num=0;
	int width=0;
	int height=0;
	bool limit_is=false;
	bool graphically_ti=false;
	bool graphically_si=false;
	bool isinterval=true;
	int intervalcnt=5;
	//接收参数------------------
	extern char *optarg;
	int opt;
	//--------------------------
	if(argc==1){
		usage();
		return 0;
	}

	while ((opt =getopt(argc, argv,"i:o:g:l:n:x:y:h")) != -1)
	{
		switch (opt)
		{
		case 'h':{
			usage();
			return 0;
				 }
		case 'i':{
			strcpy(in_url,optarg);
			break;
				 }
		case 'o':{
			strcpy(out_url,optarg);
			break;
				 }
		case 'l':{
			limit_num=atoi(optarg);
			limit_is=true;
			break;
				 }
		case 'n':{
			intervalcnt=atoi(optarg);
			break;
				 }
		case 'g':{
			if(strcmp(optarg,"ti")==0){
				graphically_ti=true;
			}else if(strcmp(optarg,"si")==0){
				graphically_si=true;
			}
			break;
				 }
		case 'x':{
			width=atoi(optarg);
			break;
				 }
		case 'y':{
			height=atoi(optarg);
			break;
				 }
		default:
			printf("Unknown: %c\n", opt);
			usage();
			return 0;
		}
	}

	if(strcmp(in_url,"")==0){
		printf("Input Video's URL is not set. Exit.\n");
		return 0;
	}

	if(strcmp(out_url,"")==0){
		printf("Output .csv file is not set. Default is {Input Name}.csv\n");
		char *suffix=strchr(in_url, '.');
		*suffix='\0';
		strcpy(out_url,in_url);
		*suffix='.';
		sprintf(out_url,"%s.csv",out_url);
	}
	
	AVFormatContext	*pFormatCtx;
	int				i, video_stream,audio_stream;
	AVCodecContext	*pCodecCtx,*pCodecCtx_au;
	AVCodec			*pCodec,*pCodec_au;

	av_register_all();
	pFormatCtx = avformat_alloc_context();
	if(avformat_open_input(&pFormatCtx,in_url,NULL,NULL)!=0){
		printf("Couldn't open file.\n");
		return FALSE;
	}
	if(strcmp(pFormatCtx->iformat->name,"rawvideo")!=0)
	{
		if(av_find_stream_info(pFormatCtx)<0)
		{
			printf("Couldn't find stream information.\n");
			return FALSE;
		}
	}
	
	video_stream=-1;
	audio_stream=-1;
	for(i=0; i<pFormatCtx->nb_streams; i++) {
		if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO){
			video_stream=i;
		}if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_AUDIO){
			audio_stream=i;
		}
	}
	if(video_stream==-1)
	{
		printf("Didn't find a video stream.\n");
		return FALSE;
	}
	if(video_stream!=-1){

		pCodecCtx=pFormatCtx->streams[video_stream]->codec;
		pCodecCtx->width=width;
		pCodecCtx->height=height;
		pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
		if(pCodec==NULL)
		{
			printf("Codec not found.\n");
			return FALSE;
		}
		if(avcodec_open2(pCodecCtx, pCodec,NULL)<0)
		{
			printf("Could not open codec.\n");
			return FALSE;
		}

		//------------SDL----------------
		SDLParam sdlparam={NULL,NULL,{0,0,0,0},graphically_ti,graphically_si,isinterval,NULL,NULL,0,0,0,0};
		if(graphically_ti==true||graphically_si==true){
			sdlparam.graphically_si=graphically_si;
			sdlparam.graphically_ti=graphically_ti;
			sdlparam.show_w=pCodecCtx->width-2*PADDING;
			sdlparam.show_h=pCodecCtx->height-2*PADDING;
			sdlparam.pixel_w=pCodecCtx->width-2*PADDING;
			sdlparam.pixel_h=pCodecCtx->height-2*PADDING;
			//FIX
			sdlparam.show_YBuffer=(char *)malloc(sdlparam.pixel_w*sdlparam.pixel_h);
			sdlparam.show_UVBuffer=(char *)malloc(sdlparam.pixel_w*sdlparam.pixel_h/2);
			memset(sdlparam.show_UVBuffer,0x80,sdlparam.pixel_w*sdlparam.pixel_h/2);

			SDL_Thread *video_tid = SDL_CreateThread(show_thread,&sdlparam);
		}

		//---------------
		float* silist=(float*)malloc(FRAMENUM*sizeof(float));
		float* tilist=(float*)malloc((FRAMENUM-1)*sizeof(float));
		float* old_silist;
		float* old_tilist;

		AVFrame	*pFrame,*pFrameYUV;
		pFrame=avcodec_alloc_frame();
		pFrameYUV=avcodec_alloc_frame();
		uint8_t *out_buffer;
		out_buffer=(uint8_t *)av_malloc(avpicture_get_size(PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height));
		avpicture_fill((AVPicture *)pFrameYUV, out_buffer, PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);

		int ret, got_picture;
		static struct SwsContext *img_convert_ctx;
		int y_size = pCodecCtx->width * pCodecCtx->height;

		AVPacket *packet=(AVPacket *)av_malloc(sizeof(AVPacket));
		av_new_packet(packet, y_size);
		
		//计算TI的时候使用
		int prev_has=0;
		uint8_t *prev_ydata=(uint8_t *)av_malloc(pCodecCtx->width*pCodecCtx->height);

		img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, PIX_FMT_YUV420P, SWS_BILINEAR, NULL, NULL, NULL); 
		
		//打开文件
		FILE *fp=fopen(out_url,"wb+");
		fprintf(fp,"TI,SI\n");
		
		//记个数
		int framecnt=0;
		int realloc_time=1;
		while(av_read_frame(pFormatCtx, packet)>=0&&(framecnt<limit_num||!limit_is))
		{
			if(packet->stream_index==video_stream)
			{
				ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
				if(ret < 0)
				{
					printf("Decode Error.\n");
					return -1;
				}
				if(got_picture)
				{
					sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);
					//有前面的帧，才能计算TI
					if(prev_has==1){
						if(framecnt%intervalcnt==0){
							sdlparam.isinterval=false;
						}else{
							sdlparam.isinterval=true;
						}
						float ti=0,si=0;
						int retval=tisi((char *)pFrameYUV->data[0],(char *)prev_ydata,pCodecCtx->width,pCodecCtx->height,sdlparam,ti,si);
						if(retval==-1)
							break;

						if(framecnt>=FRAMENUM*realloc_time)
						{
							realloc_time++;
							old_tilist=tilist;
							old_silist=silist;
							if( (tilist = (float*)realloc( tilist, (FRAMENUM*realloc_time-1)*sizeof(float)))==NULL)								
							{
								free( old_tilist );  // free original block
								return -1;
							}
							if( (silist = (float*)realloc( silist, (FRAMENUM*realloc_time)*sizeof(float)))==NULL)								
							{
								free( old_silist );  // free original block
								return -1;
							}
						}
						tilist[framecnt]=ti;
						silist[framecnt]=si;
						printf("%f,%f\n",ti,si);
						fprintf(fp,"%f,%f\n",ti,si);
						framecnt++;
					}else{
						prev_has=1;
					}
					//拷贝亮度数据
					memcpy(prev_ydata,pFrameYUV->data[0],pCodecCtx->width*pCodecCtx->height);
				}
			}
			av_free_packet(packet);
		}
		sws_freeContext(img_convert_ctx);
		
		//计算平均值和最大值
		float sum=0;
		for (int i=0;i<framecnt;i++)
			sum +=silist[i];
		float avg_si=sum/framecnt;
		qsort(silist,FRAMENUM*realloc_time,sizeof(float),comp);
		float max_si=silist[FRAMENUM*realloc_time-1];

		sum=0;
		for (int i=0;i<framecnt-1;i++)
			sum +=tilist[i];
		float avg_ti=sum/(framecnt-1);
		qsort(tilist,(FRAMENUM*realloc_time-1),sizeof(float),comp);
		float max_ti=tilist[FRAMENUM*realloc_time-2];

		fprintf(fp,"TI_AVG,SI_AVG\n");
		fprintf(fp,"%f,%f\n",avg_ti,avg_si);
		fprintf(fp,"TI_MAX,SI_MAX\n");
		fprintf(fp,"%f,%f\n",max_ti,max_si);
		fclose(fp);

		av_free(out_buffer);
		av_free(pFrameYUV);
		avcodec_close(pCodecCtx);

		if(graphically_ti==true||graphically_si==true){
			free(sdlparam.show_YBuffer);
			free(sdlparam.show_UVBuffer);
			SDL_Event event;
			event.type=SDL_QUIT;
			SDL_PushEvent(&event);
		}
		delete silist;
		delete tilist;
	}
	avformat_close_input(&pFormatCtx);
	

#ifdef _DEBUG
	_CrtDumpMemoryLeaks();//调试运行到该步，输出检测信息
#endif

	return 0;
}