/** 
 * TIandSIcmd
 * (TIandSI CMD version)
 *
 * 雷霄骅 Lei Xiaohua 张晖 Zhang Hui
 * leixiaohua1020@126.com
 * zhanghuicuc@gmail.com
 * 中国传媒大学/数字电视技术
 * Communication University of China / Digital TV Technology
 *
 * http://blog.csdn.net/leixiaohua1020
 *
 * 本工程可以计算一个YUV序列的时间信息TI（Temporal perceptual Information，
 * 也可以称时间复杂度）和空间信息SI（Spatial perceptual Information，也可以
 * 称空间复杂度）。计算方法出自标准：ITU-R BT.1788
 *
 * This software can calculate a YUV sequence's TI(Temporal perceptual Information) 
 * and SI(Spatial perceptual Information) according to ITU-R BT.1788.
 *
 */
#include "TIandSIcmd.h"

#ifdef WIN32
#include "XGetopt.h"
#else
#include <unistd.h>
#endif

#ifdef _DEBUG
#include <crtdbg.h>
#endif

#define PADDING 20	//default 1

int comp (const void * elem1, const void * elem2) 
{
	float f = *((float*)elem1);
	float s = *((float*)elem2);
	if (f > s) return  1;
	if (f < s) return -1;
	return 0;
}

int TIandSI(char* strPath,int width,int height,int pixfmt)
{
	FILE* fyuv=fopen(strPath,"rb");

	char* out_filename=strPath;
	strcat(out_filename,"_TISI.csv");
	FILE* fout;
	fout=fopen(out_filename,"wb+");

	if(fyuv==NULL || fout==NULL)
	{
		printf("can't open input file or output file\n");
		return 1;
	}

	int nFrameNum;
	float nFrameSize;
	int nYSize=height*width;

	unsigned char *pFrame=(unsigned char*)malloc(nYSize);
	unsigned char *pNextFrame=(unsigned char*)malloc(nYSize);
	unsigned char *pFrame_0;
	unsigned char *pFrame_1;
	unsigned char *pFrame_2;
	unsigned char *pNextFrame_0;

	unsigned long file_length;

	switch (pixfmt) {
	case 400: nFrameSize=nYSize; break;
	case 422: nFrameSize=nYSize*2; break;
	case 444: nFrameSize = nYSize*3; break;
	default :
	case 420: nFrameSize = nYSize*1.5; break;
	}

	fseek(fyuv,0,SEEK_END);
	file_length=ftell(fyuv);
	fseek(fyuv,0,SEEK_SET);
	nFrameNum=file_length/nFrameSize;
	int realframe_size=(width-2*PADDING)*(height-2*PADDING);
	
	float *frame_sobel=(float*)malloc(realframe_size*sizeof(float));
	memset(frame_sobel,0.0f,realframe_size*sizeof(float));
	float avg_frame_sobel=0;
	float *SI=(float*)malloc(nFrameNum*sizeof(float));
	float avg_SI=0;float max_SI;
	int index=0;

	float *frame_diff=(float*)malloc(realframe_size*sizeof(float));
	float avg_frame_diff=0;
	float *TI=(float*)malloc((nFrameNum-1)*sizeof(float));
	float avg_TI=0;float max_TI;

	__m128 sobel_avg_sum=_mm_set1_ps(+0.0f);
	__m128 sobel_square_sum=_mm_set1_ps(+0.0f);
	__m128 diff_avg_sum=_mm_set1_ps(+0.0f);
	__m128 diff_square_sum=_mm_set1_ps(+0.0f);
	__m128 avg_sobel=_mm_set1_ps(+0.0f);
	__m128 avg_diff=_mm_set1_ps(+0.0f);
	
	/*_asm
	{
		movss sobel_avg_sum,f
		shufps sobel_avg_sum,sobel_avg_sum,0
		movss sobel_square_sum,f
		shufps sobel_square_sum,sobel_square_sum,0
		movss diff_avg_sum,f
		shufps diff_avg_sum,diff_avg_sum,0
		movss diff_square_sum,f
		shufps diff_square_sum,diff_square_sum,0
		movss avg_sobel,f
		shufps avg_sobel,avg_sobel,0
		movss avg_diff,f
		shufps avg_diff,avg_diff,0
	}*/
	
	int i,j,k,m;
	int pad_threshold=0;
	
	fprintf(fout,"----------------SITI-------------------\n");
	for(k = 0;k < nFrameNum; k++)
	{
		fseek(fyuv,k*nFrameSize,SEEK_SET);
		fread(pFrame,1,nYSize,fyuv);
		pFrame_0=pFrame+width*(PADDING-1);
		pFrame_1=pFrame+width*PADDING;
		pFrame_2=pFrame+width*(PADDING+1);

		if(k<nFrameNum-1)
		{
			fseek(fyuv,(k+1)*nFrameSize,SEEK_SET);
			fread(pNextFrame,1,nYSize,fyuv);
			pNextFrame_0=pNextFrame+width*(PADDING-1);
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

				/*__m128 gx = _mm_add_ps( _mm_mul_ps(pFrame_00,const_p_one),
					_mm_add_ps( _mm_mul_ps(pFrame_02,const_n_one),
					_mm_add_ps( _mm_mul_ps(pFrame_10,const_p_two),
					_mm_add_ps( _mm_mul_ps(pFrame_12,const_n_two),
					_mm_add_ps( _mm_mul_ps(pFrame_20,const_p_one),
					_mm_mul_ps(pFrame_22,const_n_one))))));*/

				__m128 gx=_mm_add_ps(_mm_sub_ps(_mm_add_ps(_mm_add_ps(_mm_sub_ps(_mm_sub_ps(_mm_sub_ps(pFrame_20,pFrame_22),pFrame_12),pFrame_12),pFrame_10),pFrame_10),pFrame_02),pFrame_00);

				/*__m128 gy = _mm_add_ps( _mm_mul_ps(pFrame_00,const_p_one), 
					_mm_add_ps( _mm_mul_ps(pFrame_01,const_p_two), 
					_mm_add_ps( _mm_mul_ps(pFrame_02,const_p_one),
					_mm_add_ps( _mm_mul_ps(pFrame_20,const_n_one), 
					_mm_add_ps( _mm_mul_ps(pFrame_21,const_n_two), 
					_mm_mul_ps(pFrame_22,const_n_one))))));*/

				__m128 gy=_mm_sub_ps(_mm_sub_ps(_mm_add_ps(_mm_add_ps(_mm_add_ps(_mm_sub_ps(pFrame_00,pFrame_20),_mm_sub_ps(pFrame_02,pFrame_22)),pFrame_01),pFrame_01),pFrame_21),pFrame_21);

				__m128 sobel_result = _mm_sqrt_ps(_mm_add_ps(_mm_mul_ps(gx, gx), _mm_mul_ps(gy,gy)));
				
				if(!pad_threshold)
				{
					frame_sobel[index]=sobel_result.m128_f32[0];
					frame_sobel[index+1]=sobel_result.m128_f32[1];
					frame_sobel[index+2]=sobel_result.m128_f32[2];
					frame_sobel[index+3]=sobel_result.m128_f32[3];
				}
				else
				{
					for(m=0;m<width-PADDING-i;m++)
						frame_sobel[index+m]=sobel_result.m128_f32[m];
				}
				

				if(k<nFrameNum-1)
				{
					__m128i next_0=_mm_unpacklo_epi8(_mm_loadu_si128((__m128i*)(pNextFrame_0+i-1)), _mm_setzero_si128());
					__m128 pNextFrame_00 = _mm_cvtepi32_ps(_mm_unpacklo_epi16(next_0, _mm_setzero_si128()));
					__m128 diff_result=_mm_sub_ps(pNextFrame_00,pFrame_00);
					
					if(!pad_threshold)
					{
						frame_diff[index]=diff_result.m128_f32[0];
						frame_diff[index+1]=diff_result.m128_f32[1];
						frame_diff[index+2]=diff_result.m128_f32[2];
						frame_diff[index+3]=diff_result.m128_f32[3];
					}
					else
					{
						for(m=0;m<width-PADDING-i;m++)
							frame_diff[index+m]=diff_result.m128_f32[m];
					}
				}	

				if(!pad_threshold)
					index+=4;
				else
					index+=width-PADDING-i;
			}
			pFrame_0 += width;
			pFrame_1 += width;
			pFrame_2 += width;

			if(k<nFrameNum-1)
				pNextFrame_0 += width;

			pad_threshold=0;
		}

		for(i=0;i<index;i+=4)
		{
			__m128 sobel_result_0 = _mm_set_ps (frame_sobel[i],frame_sobel[i+1], frame_sobel[i+2], frame_sobel[i+3]);
			sobel_avg_sum = _mm_add_ps(sobel_avg_sum,sobel_result_0);
			
			if(k<nFrameNum-1)
			{
				__m128 diff_result_0 = _mm_set_ps (frame_diff[i],frame_diff[i+1], frame_diff[i+2], frame_diff[i+3]);
				diff_avg_sum = _mm_add_ps(diff_avg_sum,diff_result_0);
			}
		}		
		avg_frame_sobel=(sobel_avg_sum.m128_f32[0]+
						sobel_avg_sum.m128_f32[1]+
						sobel_avg_sum.m128_f32[2]+
						sobel_avg_sum.m128_f32[3])/index;
		avg_sobel = _mm_set_ps (avg_frame_sobel,avg_frame_sobel, avg_frame_sobel, avg_frame_sobel);

		if(k<nFrameNum-1)
		{
			avg_frame_diff=(diff_avg_sum.m128_f32[0]+
				diff_avg_sum.m128_f32[1]+
				diff_avg_sum.m128_f32[2]+
				diff_avg_sum.m128_f32[3])/index;
			avg_diff = _mm_set_ps (avg_frame_diff,avg_frame_diff, avg_frame_diff, avg_frame_diff);
		}

		for(i=0;i<index;i+=4)
		{
			__m128 sobel_result_1 = _mm_set_ps (frame_sobel[i],frame_sobel[i+1], frame_sobel[i+2], frame_sobel[i+3]);
			__m128 sobel_square=_mm_mul_ps(_mm_sub_ps(sobel_result_1,avg_sobel),_mm_sub_ps(sobel_result_1,avg_sobel));
			sobel_square_sum = _mm_add_ps(sobel_square_sum,sobel_square);

			if(k<nFrameNum-1)
			{
				__m128 diff_result_1 = _mm_set_ps (frame_diff[i],frame_diff[i+1], frame_diff[i+2], frame_diff[i+3]);
				__m128 diff_square=_mm_mul_ps(_mm_sub_ps(diff_result_1,avg_diff),_mm_sub_ps(diff_result_1,avg_diff));
				diff_square_sum = _mm_add_ps(diff_square_sum,diff_square);
			}
		}

		SI[k]=sqrt((sobel_square_sum.m128_f32[0]+sobel_square_sum.m128_f32[1]+sobel_square_sum.m128_f32[2]+sobel_square_sum.m128_f32[3])/index);
		fprintf(fout,"SI(%d):%.3f,",k,SI[k]);
		printf("SI(%d):%.3f,",k,SI[k]);
		avg_frame_sobel=0;
		sobel_avg_sum=_mm_set1_ps(+0.0f);
		sobel_square_sum=_mm_set1_ps(+0.0f);
		
		if(k<nFrameNum-1)
		{
			TI[k]=sqrt((diff_square_sum.m128_f32[0]+diff_square_sum.m128_f32[1]+diff_square_sum.m128_f32[2]+diff_square_sum.m128_f32[3])/index);
			fprintf(fout,"TI(%d):%.3f\n",k,TI[k]);
			printf("TI(%d):%.3f\n",k,TI[k]);
			avg_frame_diff=0;
			diff_avg_sum=_mm_set1_ps(+0.0f);
			diff_square_sum=_mm_set1_ps(+0.0f);
		}
		index=0;
	}
	
	for(k=0;k<nFrameNum;k++)
		avg_SI+=SI[k];
	avg_SI=avg_SI/nFrameNum;
	qsort(SI,nFrameNum,sizeof(float),comp);
	max_SI=SI[nFrameNum-1];

	for(k=0;k<nFrameNum-1;k++)
		avg_TI+=TI[k];
	avg_TI=avg_TI/(nFrameNum-1);
	qsort(TI,nFrameNum-1,sizeof(float),comp);
	max_TI=TI[nFrameNum-2];

	/*for(k = 0;k < nFrameNum-1; k++)
	{		
		fseek(fyuv,k*nFrameSize,SEEK_SET);
		fread(pFrame,1,nYSize,fyuv);
		fseek(fyuv,(k+1)*nFrameSize,SEEK_SET);
		fread(pNextFrame,1,nYSize,fyuv);
		pFrame_0=pFrame+width*19;
		pNextFrame_0=pNextFrame+width*19;

		for(j = 1; j < height-39; j++)
		{
			for(i = 20; i < width-23; i+=4)
			{
				__m128i current_0=_mm_unpacklo_epi8(_mm_loadu_si128((__m128i*)(pFrame_0+i-1)), _mm_setzero_si128());
				__m128i next_0=_mm_unpacklo_epi8(_mm_loadu_si128((__m128i*)(pNextFrame_0+i-1)), _mm_setzero_si128());
				__m128 pFrame_00 = _mm_cvtepi32_ps(_mm_unpacklo_epi16(current_0, _mm_setzero_si128()));
				__m128 pNextFrame_00 = _mm_cvtepi32_ps(_mm_unpacklo_epi16(next_0, _mm_setzero_si128()));
				__m128 result=_mm_sub_ps(pNextFrame_00,pFrame_00);
				
				frame_diff[index++]=result.m128_f32[0];
				frame_diff[index++]=result.m128_f32[1];
				frame_diff[index++]=result.m128_f32[2];
				frame_diff[index++]=result.m128_f32[3];
			}
			pFrame_0 += width;
			pNextFrame_0 += width;
		}

		for(i=0;i<index;i+=4)
		{
			__m128 result_0 = _mm_set_ps (frame_diff[i],frame_diff[i+1], frame_diff[i+2], frame_diff[i+3]);
			avg_sum = _mm_add_ps(avg_sum,result_0);
		}		
		avg_frame_diff=(avg_sum.m128_f32[0]+
			avg_sum.m128_f32[1]+
			avg_sum.m128_f32[2]+
			avg_sum.m128_f32[3])/index;
		__m128 avg_diff = _mm_set_ps (avg_frame_diff,avg_frame_diff, avg_frame_diff, avg_frame_diff);

		for(i=0;i<index;i+=4)
		{
			__m128 result_1 = _mm_set_ps (frame_diff[i],frame_diff[i+1], frame_diff[i+2], frame_diff[i+3]);
			__m128 square=_mm_mul_ps(_mm_sub_ps(result_1,avg_diff),_mm_sub_ps(result_1,avg_diff));
			square_sum = _mm_add_ps(square_sum,square);
		}

		TI[k]=sqrt((square_sum.m128_f32[0]+square_sum.m128_f32[1]+square_sum.m128_f32[2]+square_sum.m128_f32[3])/index);
		avg_frame_diff=0;
		index=0;
		avg_sum=_mm_set1_ps(+0.0f);
		square_sum=_mm_set1_ps(+0.0f);
	}*/

	fprintf(fout,"\n----------------Summary-------------------\n");
	fprintf(fout,"avg_SI:%.3f,avg_TI:%.3f\n",avg_SI,avg_TI);
	fprintf(fout,"max_SI:%.3f,max_TI:%.3f\n",max_SI,max_TI);

	//--------
	fclose(fyuv);
	fclose(fout);
	delete pFrame;
	delete pNextFrame;
	delete SI;
	delete TI;
	delete frame_diff;
	delete frame_sobel;
	
	return 0;
}

//usage
void tisi_usage(){
	printf("---------------------帮助------------------------\n");
	printf("TISI\n\n本程序可以：\n");
	printf("计算输入YUV文件的TI、SI值，并输出至csv文件\n\n");
	printf("输入选项如下\n");
	printf("-i\t输入YUV文件的路径（必填）\n");
	printf("-x\t输入YUV文件的宽度（必填）\n");
	printf("-y\t输入YUV文件的高度（必填）\n");
	printf("-f\t输入YUV文件的像素格式（必填，420或422或444或400）\n");
	printf("-h\t打开本帮助\n");
	printf("-------------------------------------------------\n");
}

int _tmain(int argc, char* argv[])
{
#ifdef _DEBUG
_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	int ret;
	char filename[500];
	char width_s[10];
	char height_s[10];
	char pixfmt_s[10]; 
	const char* width_const,*height_const,*pixfmt_const;
	int width,height,pixfmt;

	printf("-----------------------------------------------------\n");
	printf("TISI\n");
	printf("2015 雷霄骅，张晖 中国传媒大学/数字电视技术\n");
	printf("-----------------------------------------------------\n");

	extern char *optarg;
	int opt;
	if(argc==1){
		printf("请设定参数！\n输入 -h 以获得详细帮助信息\n");
		tisi_usage();
		return 1;
	}
	//分析命令行参数，注意用法。
	//选项都是一个字母，后面有冒号的代表该选项还有相关参数
	//一直循环直到获取所有的opt
	while ((opt =getopt(argc, argv,"i:x:y:f:h")) != -1)
	{
		switch (opt)
		{
		case 'h':
			tisi_usage();
			return 1;
		case 'i':{
			strcpy(filename,optarg);
			break;}
		case 'x':{
			strcpy(width_s,optarg);
			break;}
		case 'y':{
			strcpy(height_s,optarg);
			break;}
		case 'f':{
			strcpy(pixfmt_s,optarg);
			break;}
		default:
			printf("未知命令: %c\n", opt);
			tisi_usage();
			return 1;
		}
	}
	//-------------------------
	width_const=width_s;
	height_const=height_s;
	pixfmt_const=pixfmt_s;
	width=atoi(width_const);
	height=atoi(height_const);
	pixfmt=atoi(pixfmt_const);

	ret=TIandSI(filename,width,height,pixfmt);
	if(ret)
		return 1;

#ifdef _DEBUG
_CrtDumpMemoryLeaks();//调试运行到该步，输出检测信息
#endif
	
	return 0;
}

