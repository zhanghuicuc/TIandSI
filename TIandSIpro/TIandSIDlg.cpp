/**
 * TIandSIpro
 * (TIandSI Professional)
 *
 * 雷霄骅 Lei Xiaohua 张晖 Zhang Hui
 * leixiaohua1020@126.com
 * zhanghuicuc@gmail.com
 * 中国传媒大学/数字电视技术
 * Communication University of China / Digital TV Technology
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

#include "stdafx.h"
#include "TIandSI.h"
#include "TIandSIDlg.h"
#include "afxdialogex.h"
#include <atlconv.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
public:
//	afx_msg void OnDropFiles(HDROP hDropInfo);
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
//	ON_WM_DROPFILES()
END_MESSAGE_MAP()


// CTIandSIDlg 对话框




CTIandSIDlg::CTIandSIDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CTIandSIDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_padding = 0;
}

void CTIandSIDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_URLLIST, m_urllist);
	DDX_Control(pDX, IDC_INPUTURL, m_inputurl);
	DDX_Control(pDX, IDC_PROGRESS_ALL, m_progressall);
	DDX_Control(pDX, IDC_PROGRESS_ALL_TEXT, m_progressalltext);
	DDX_Control(pDX, IDC_PROGRESS_CUR, m_progresscur);
	DDX_Control(pDX, IDC_PROGRESS_CUR_TEXT, m_progresscurtext);
	DDX_Control(pDX, IDC_OUTPUT, m_output);
	DDX_Control(pDX, IDC_DRAW, m_draw);
	DDX_Control(pDX, IDC_CURVAL, m_curval);
	DDX_Text(pDX, IDC_PADDING, m_padding);
}

BEGIN_MESSAGE_MAP(CTIandSIDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CTIandSIDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_URLLIST_ADD, &CTIandSIDlg::OnBnClickedUrllistAdd)
	ON_BN_CLICKED(IDC_URLLIST_DELETE, &CTIandSIDlg::OnBnClickedUrllistDelete)
	ON_BN_CLICKED(IDC_URLLIST_DELETEALL, &CTIandSIDlg::OnBnClickedUrllistDeleteall)
	ON_BN_CLICKED(IDABOUT, &CTIandSIDlg::OnBnClickedAbout)
//	ON_BN_CLICKED(IDC_CHART_EDITOR, &CTIandSIDlg::OnBnClickedChartEditor)
//ON_BN_CLICKED(IDC_RADIO_DRAW, &CTIandSIDlg::OnClickedRadioDraw)
ON_BN_CLICKED(IDRESULT, &CTIandSIDlg::OnBnClickedResult)
ON_WM_DROPFILES()
ON_BN_CLICKED(IDPAUSE, &CTIandSIDlg::OnBnClickedPause)
ON_BN_CLICKED(IDSTOP, &CTIandSIDlg::OnBnClickedStop)
ON_COMMAND(IDWEBSITE, &CTIandSIDlg::OnWebsite)
ON_COMMAND(ID_URLLIST_QUICKADD, &CTIandSIDlg::OnUrllistQuickadd)
END_MESSAGE_MAP()


// CTIandSIDlg 消息处理程序

BOOL CTIandSIDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标
	//---------------
	
	m_output.SetCheck(TRUE);
	m_padding=1;
	UpdateData(FALSE);
	//初始化
	m_progressalltext.SetWindowText(L"0%");
	m_progresscurtext.SetWindowText(L"0%");
	m_progressall.SetRange(0,100);
	m_progresscur.SetRange(0,100);

	//SDL==========================
	sdlparam.graphically==true;
	//SDL_putenv()放在前面
	char variable[256];   
	CWnd* pWnd = GetDlgItem(IDC_SCREEN);  //获取图片控件的窗口指针   
	sprintf(variable,"SDL_WINDOWID=0x%1x",pWnd->GetSafeHwnd()); // 格式化字符串      
	SDL_putenv(variable); 

	if(SDL_Init(SDL_INIT_VIDEO)) {
		AfxMessageBox(L"Could not initialize SDL"); 
		return 0;
	} 


	CRect screenrect;
	GetDlgItem(IDC_SCREEN)->GetWindowRect(screenrect);
	sdlparam.screen = SDL_SetVideoMode(screenrect.Width(), screenrect.Height(), 0, 0);
	if(!sdlparam.screen) {  
		AfxMessageBox(L"SDL: could not set video mode");  
		return 0;
	}
	sdlparam.rect.x=0;
	sdlparam.rect.y=0;
	sdlparam.rect.w=screenrect.Width();
	sdlparam.rect.h=screenrect.Height();

	//-------------
	resultdlg=new ResultDlg;
	resultdlg->Create(IDD_RESULT_DIALOG);

	m_draw.InsertString(0,L"SI");
	m_draw.InsertString(1,L"TI");
	m_draw.InsertString(2,L"不显示");
	m_draw.SetCurSel(0);

	SetState(SYSTEM_PREPARE);
	//=============================

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CTIandSIDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CTIandSIDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CTIandSIDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//解码视频并且计算
int CTIandSIDlg::TIandSI(CString url,LPVOID lparam)
{
	CTIandSIDlg *dlg=(CTIandSIDlg *)lparam;;
	//----------------
	AVFormatContext	*pFormatCtx;
	int				i, video_stream,audio_stream;
	AVCodecContext	*pCodecCtx,*pCodecCtx_au;
	AVCodec			*pCodec,*pCodec_au;
	char in_url[500]={0};
	char out_url[500]={0};
	USES_CONVERSION;
	sprintf(in_url,"%s",W2A(url));
	char *suffix=(char *)strrchr(in_url, '.');//
	*suffix='\0';
	strcpy(out_url,in_url);
	*suffix='.';
	sprintf(out_url,"%s_tisi.csv",out_url);


	av_register_all();
	pFormatCtx = avformat_alloc_context();
	if(avformat_open_input(&pFormatCtx,in_url,NULL,NULL)!=0){
		AfxMessageBox(L"Couldn't open file.\n");
		return FALSE;
	}
	if(av_find_stream_info(pFormatCtx)<0)
	{
		AfxMessageBox(L"Couldn't find stream information.\n");
		return FALSE;
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
		AfxMessageBox(L"Didn't find a video stream.\n");
		return FALSE;
	}
	if(video_stream!=-1){

		pCodecCtx=pFormatCtx->streams[video_stream]->codec;
		pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
		if(pCodec==NULL)
		{
			AfxMessageBox(L"Codec not found.\n");
			return FALSE;
		}
		if(avcodec_open2(pCodecCtx, pCodec,NULL)<0)
		{
			AfxMessageBox(L"Could not open codec.\n");
			return FALSE;
		}

		//vtype=VE_TIL_SIL;
		std::vector<float> silist;
		std::vector<float> tilist;

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
		//打开输出文件
		FILE *fp=NULL;
		if(dlg->m_output.GetCheck()==TRUE){
			fp=fopen(out_url,"wb+");
			fprintf(fp,"TI,SI\n");
		}

		//Draw
		dlg->sdlparam.bmp = SDL_CreateYUVOverlay(pCodecCtx->width-2*m_padding, pCodecCtx->height-2*m_padding,SDL_YV12_OVERLAY, dlg->sdlparam.screen); 
		//Bar
		dlg->m_progresscur.SetPos(0);
		dlg->m_progresscurtext.SetWindowText(L" 0%");
		//记个数
		int framecnt=0;
		//微秒
		int ptime=0;
		while(av_read_frame(pFormatCtx, packet)>=0)
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
						float ti=0,si=0;
						int retval=TIandSICal(dlg,(char *)pFrameYUV->data[0],(char *)prev_ydata,pCodecCtx->width,pCodecCtx->height,sdlparam,ti,si);
						if(retval==-1){
							return -1;
						}
						CString curvalstr;
						curvalstr.Format(L"SI:%.3f TI:%.3f",si,ti);
						dlg->m_curval.SetWindowText(curvalstr);

						tilist.push_back(ti);
						silist.push_back(si);
						TRACE("%f,%f\n",ti,si);
						if(dlg->m_output.GetCheck()==TRUE)
							fprintf(fp,"%f,%f\n",ti,si);
						framecnt++;
						//Bar
						ptime=((float)pFrame->pkt_pts)*((float)pFormatCtx->streams[video_stream]->time_base.num)/((float)pFormatCtx->streams[video_stream]->time_base.den);
						int progress=(ptime*100)/(pFormatCtx->duration/1000000);
						if(progress<0||progress>100){
							progress=0;
						}
						dlg->m_progresscur.SetPos(progress);
						CString progressstr;
						progressstr.Format(L"%2d%%",progress);
						dlg->m_progresscurtext.SetWindowText(progressstr);
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


		//Free
		SDL_FreeYUVOverlay(dlg->sdlparam.bmp);
		//Bar
		dlg->m_progresscur.SetPos(100);
		dlg->m_progresscurtext.SetWindowText(L"100%");

		//文件名
		CString in_url1,filename;
		in_url1.Format(L"%s",A2W(in_url));
		int nPos=in_url1.ReverseFind('\\');   
		filename=in_url1.Mid(nPos+1);

		CString resulttistr(filename);
		CString resultsistr(filename);
		resulttistr.AppendFormat(L"\r\n=======\r\n");
		resultsistr.AppendFormat(L"\r\n=======\r\n");

		//计算平均值
		int num=0;
		float sum=0;
		num=silist.size();
		for (int i=0;i<num;i++){
			sum +=silist[i];
			resultsistr.AppendFormat(L"%.3f\r\n",silist[i]);
		}
		float siavg=sum/num;
		num=0;
		sum=0;
		num=tilist.size();

		for (int i=0;i<num;i++){
			sum +=tilist[i];
			resulttistr.AppendFormat(L"%.3f\r\n",tilist[i]);
		}
		float tiavg=sum/num;

		if(dlg->m_output.GetCheck()==TRUE){
			fprintf(fp,"TI_AVG,SI_AVG\n");
			fprintf(fp,"%f,%f\n",tiavg,siavg);
		}
		
		resultsistr.AppendFormat(L"SI_AVG\r\n%.3f\r\n=======\r\n",siavg);
		resulttistr.AppendFormat(L"TI_AVG\r\n%.3f\r\n=======\r\n",tiavg);

		dlg->resultdlg->AppendTIStr(resulttistr);
		dlg->resultdlg->AppendSIStr(resultsistr);

		fcloseall();

		av_free(out_buffer);
		av_free(pFrameYUV);
		avcodec_close(pCodecCtx);
	}
	avformat_close_input(&pFormatCtx);

	return 0;
}


//计算TI、SI
int CTIandSIDlg::TIandSICal(LPVOID lparam,char* ydata,char* prev_ydata,int width,int height,SDLParam sdlparam,float &ti,float &si)
{
	CTIandSIDlg *dlg=(CTIandSIDlg *)lparam;;

	//Check State-----
	while(dlg->sysstate==SYSTEM_PAUSE){
		Sleep(1000);
	}
	if(dlg->sysstate==SYSTEM_PREPARE){
		return -1;
	}
	/****************初始化设置**********************/
	int nYSize=height*width;
	float nFrameSize = nYSize*1.5;
	int realframe_size=(width-2*m_padding)*(height-2*m_padding);

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

	pFrame_0=pFrame+width*(m_padding-1);
	pFrame_1=pFrame+width*m_padding;
	pFrame_2=pFrame+width*(m_padding+1);
	pNextFrame_0=pNextFrame+width*(m_padding-1);
	
	//UV，绘图用
	unsigned char* NewUVBuffer= new unsigned char[realframe_size/2];
	memset(NewUVBuffer, 0x80, realframe_size/2);

	/********************计算SI***********************/
	memcpy(pFrame,ydata,nYSize);

	for(j = m_padding; j < height-m_padding; j++)
	{
		for(i = m_padding; i < width-m_padding; i+=4)
		{
			if(i+4>width-m_padding)
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

			__m128 gx=_mm_add_ps(_mm_sub_ps(_mm_add_ps(_mm_add_ps(_mm_sub_ps(_mm_sub_ps(_mm_sub_ps(pFrame_20,pFrame_22),pFrame_12),pFrame_12),pFrame_10),pFrame_10),pFrame_02),pFrame_00);

			__m128 gy=_mm_sub_ps(_mm_sub_ps(_mm_add_ps(_mm_add_ps(_mm_add_ps(_mm_sub_ps(pFrame_00,pFrame_20),_mm_sub_ps(pFrame_02,pFrame_22)),pFrame_01),pFrame_01),pFrame_21),pFrame_21);

			__m128 sobel_result = _mm_sqrt_ps(_mm_add_ps(_mm_mul_ps(gx, gx), _mm_mul_ps(gy,gy)));

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

				index+=4;
			}
			else
			{
				for(k=0;k<width-m_padding-i;k++)
				{
					frame_sobel[index+k]=sobel_result.m128_f32[k];
					pSobelScreen[index+k]=(unsigned char)sobel_result.m128_f32[k];
				}
				index+=width-m_padding-i;
			}		
		}
		pFrame_0 += width;
		pFrame_1 += width;
		pFrame_2 += width;
		pad_threshold=0;
	}

	for(i=0;i<index;i+=4)
	{
		__m128 sobel_result_0 = _mm_set_ps (frame_sobel[i],frame_sobel[i+1], frame_sobel[i+2], frame_sobel[i+3]);
		sobel_avg_sum = _mm_add_ps(sobel_avg_sum,sobel_result_0);
	}		
	avg_frame_sobel=(sobel_avg_sum.m128_f32[0]+
		sobel_avg_sum.m128_f32[1]+
		sobel_avg_sum.m128_f32[2]+
		sobel_avg_sum.m128_f32[3])/index;
	avg_sobel = _mm_set_ps (avg_frame_sobel,avg_frame_sobel, avg_frame_sobel, avg_frame_sobel);

	for(i=0;i<index;i+=4)
	{
		__m128 sobel_result_1 = _mm_set_ps (frame_sobel[i],frame_sobel[i+1], frame_sobel[i+2], frame_sobel[i+3]);
		__m128 sobel_square=_mm_mul_ps(_mm_sub_ps(sobel_result_1,avg_sobel),_mm_sub_ps(sobel_result_1,avg_sobel));
		sobel_square_sum = _mm_add_ps(sobel_square_sum,sobel_square);
	}

	si=sqrt((sobel_square_sum.m128_f32[0]+sobel_square_sum.m128_f32[1]+sobel_square_sum.m128_f32[2]+sobel_square_sum.m128_f32[3])/index);
	avg_frame_sobel=0;
	sobel_avg_sum=_mm_set1_ps(+0.0f);
	sobel_square_sum=_mm_set1_ps(+0.0f);

	index=0;

		//画图
		if(dlg->m_draw.GetCurSel()==0){

			SDL_LockYUVOverlay(dlg->sdlparam.bmp);
			dlg->sdlparam.bmp->pixels[0]=pSobelScreen;
			dlg->sdlparam.bmp->pixels[2]=NewUVBuffer;
			dlg->sdlparam.bmp->pixels[1]=NewUVBuffer+realframe_size/4;     
			dlg->sdlparam.bmp->pitches[0]=width-2*m_padding;
			dlg->sdlparam.bmp->pitches[2]=(width-2*m_padding)/2;   
			dlg->sdlparam.bmp->pitches[1]=(width-2*m_padding)/2;
			SDL_UnlockYUVOverlay(dlg->sdlparam.bmp);  

			CRect screenrect;
			dlg->GetDlgItem(IDC_SCREEN)->GetWindowRect(screenrect);
			sdlparam.rect.x = 0;    
			sdlparam.rect.y = 0;    
			sdlparam.rect.w = screenrect.Width();    
			sdlparam.rect.h = screenrect.Height();    
			SDL_DisplayYUVOverlay(dlg->sdlparam.bmp, &dlg->sdlparam.rect); 
		}

		memcpy(pFrame,prev_ydata,nYSize);
		memcpy(pNextFrame,ydata,nYSize);
		
		pFrame_0=pFrame+width*(m_padding-1);
		pNextFrame_0=pNextFrame+width*(m_padding-1);

		for(j = m_padding; j < height-m_padding; j++)
		{
			for(i = m_padding; i < width-m_padding; i+=4)
			{
				if(i+4>width-m_padding)
					pad_threshold=1;
				// load 16 components. (0~6 will be used)
				__m128i current_0 = _mm_unpacklo_epi8(_mm_loadu_si128((__m128i*)(pFrame_0+i-1)), _mm_setzero_si128());
				__m128i next_0 = _mm_unpacklo_epi8(_mm_loadu_si128((__m128i*)(pNextFrame_0+i-1)), _mm_setzero_si128());

				// pFrame_00 = { pFrame_0[i-1], pFrame_0[i], pFrame_0[i+1], pFrame_0[i+2] }
				__m128 pFrame_00 = _mm_cvtepi32_ps(_mm_unpacklo_epi16(current_0, _mm_setzero_si128()));
				__m128 pNextFrame_00 = _mm_cvtepi32_ps(_mm_unpacklo_epi16(next_0, _mm_setzero_si128()));

				__m128 diff_result=_mm_sub_ps(pNextFrame_00,pFrame_00);

				if(!pad_threshold)
				{
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
					for(k=0;k<width-m_padding-i;k++)
					{
						frame_diff[index+k]=diff_result.m128_f32[k];
						pDiffScreen[index+k]=(unsigned char)abs(diff_result.m128_f32[k]);
					}
					index+=width-m_padding-i;
				}		
			}
			pFrame_0 += width;
			pNextFrame_0 += width;
			pad_threshold=0;
		}

		for(i=0;i<index;i+=4)
		{
			__m128 diff_result_0 = _mm_set_ps (frame_diff[i],frame_diff[i+1], frame_diff[i+2], frame_diff[i+3]);
			diff_avg_sum = _mm_add_ps(diff_avg_sum,diff_result_0);
		}		
		avg_frame_diff=(diff_avg_sum.m128_f32[0]+
			diff_avg_sum.m128_f32[1]+
			diff_avg_sum.m128_f32[2]+
			diff_avg_sum.m128_f32[3])/index;
		avg_diff = _mm_set_ps (avg_frame_diff,avg_frame_diff, avg_frame_diff, avg_frame_diff);

		for(i=0;i<index;i+=4)
		{
			__m128 diff_result_1 = _mm_set_ps (frame_diff[i],frame_diff[i+1], frame_diff[i+2], frame_diff[i+3]);
			__m128 diff_square=_mm_mul_ps(_mm_sub_ps(diff_result_1,avg_diff),_mm_sub_ps(diff_result_1,avg_diff));
			diff_square_sum = _mm_add_ps(diff_square_sum,diff_square);
		}
		ti=sqrt((diff_square_sum.m128_f32[0]+diff_square_sum.m128_f32[1]+diff_square_sum.m128_f32[2]+diff_square_sum.m128_f32[3])/index);
		avg_frame_diff=0;
		diff_avg_sum=_mm_set1_ps(+0.0f);
		diff_square_sum=_mm_set1_ps(+0.0f);

		index=0;

		//画图
		if(dlg->m_draw.GetCurSel()==1){

			SDL_LockYUVOverlay(dlg->sdlparam.bmp);
			dlg->sdlparam.bmp->pixels[0]=pDiffScreen;
			dlg->sdlparam.bmp->pixels[2]=NewUVBuffer;
			dlg->sdlparam.bmp->pixels[1]=NewUVBuffer+realframe_size/4;     
			dlg->sdlparam.bmp->pitches[0]=width-2*m_padding;
			dlg->sdlparam.bmp->pitches[2]=(width-2*m_padding)/2;   
			dlg->sdlparam.bmp->pitches[1]=(width-2*m_padding)/2;
			SDL_UnlockYUVOverlay(dlg->sdlparam.bmp);  

			CRect screenrect;
			dlg->GetDlgItem(IDC_SCREEN)->GetWindowRect(screenrect);
			sdlparam.rect.x = 0;    
			sdlparam.rect.y = 0;    
			sdlparam.rect.w = screenrect.Width();    
			sdlparam.rect.h = screenrect.Height();    
			SDL_DisplayYUVOverlay(dlg->sdlparam.bmp, &dlg->sdlparam.rect); 
		}

		delete pFrame;
		delete pNextFrame;
		delete NewUVBuffer;
		delete pSobelScreen;
		delete pDiffScreen;
		delete frame_diff;
		delete frame_sobel;

		return 0;
}



UINT Thread_Process(LPVOID lpParam){
	CTIandSIDlg *dlg=(CTIandSIDlg *)lpParam;

	dlg->m_progressall.SetPos(0);
	dlg->m_progressalltext.SetWindowText(L"0%");
	//载入

	dlg->SetState(SYSTEM_PROCESS);

	int i,j;
	for(i=0;i<dlg->urllist.size();i++){
		dlg->m_urllist.SetCurSel(i);

		int progress=i/dlg->urllist.size();
		dlg->m_progressall.SetPos(progress);
		CString progresstext;
		progresstext.Format(L"%d%%",progress);
		dlg->m_progressalltext.SetWindowText(progresstext);
		//------------------------------------
		int retval=dlg->TIandSI(dlg->urllist[i],dlg);
		if(retval==-1){
			return -1;
		}
		
	}
	dlg->m_progressall.SetPos(100);
	dlg->m_progressalltext.SetWindowText(L"100%");

	dlg->SetState(SYSTEM_PREPARE);

	AfxMessageBox(L"处理完毕！");
	return 0;
}

void CTIandSIDlg::OnBnClickedOk()
{
	//更新参数
	UpdateData(TRUE);
	// TODO: 在此添加控件通知处理程序代码
	
	if(urllist.size()<1){
		AfxMessageBox(L"请在列表中添加视频序列！");
		return;
	}
	SystemClear();

	pThreadProcess=AfxBeginThread(Thread_Process,this);//开启线程

}


void CTIandSIDlg::OnBnClickedUrllistAdd()
{
	UpdateData(TRUE);
	//获取地址加入列表
	CString url;
	m_inputurl.GetWindowText(url);

	if(url.IsEmpty()==TRUE){
		AfxMessageBox(L"输入地址为空！");
		return;
	}

	urllist.push_back(url);

	RefreshUrllist();
}


void CTIandSIDlg::OnBnClickedUrllistDelete()
{
	//删除元素
	int urlindex=m_urllist.GetCurSel();
	if(urlindex!=-1){
	urllist.erase(urllist.begin()+urlindex);
	}else{
		AfxMessageBox(L"没有选中任何元素！");
	}

	RefreshUrllist();
}


void CTIandSIDlg::OnBnClickedUrllistDeleteall()
{
	if(urllist.size()==0){
		AfxMessageBox(L"列表已经为空！");
	}
	//清空元素
	urllist.clear();
	RefreshUrllist();
}

void CTIandSIDlg::RefreshUrllist(){
	//重新载入列表
	m_urllist.ResetContent();
	int i=0;
	for(i=0;i<urllist.size();i++){
		CString record;
		//文件名
		CString filename;
		int nPos=urllist[i].ReverseFind('\\');   
		filename=urllist[i].Mid(nPos+1);
		m_urllist.InsertString(i,filename);
	}
}

void CTIandSIDlg::OnBnClickedAbout()
{
	CAboutDlg dlg;
	dlg.DoModal();
}



//void CTIandSIDlg::OnClickedRadioDraw()
//{
//	UpdateData(TRUE);
//}


void CTIandSIDlg::SystemClear()
{
	resultdlg->SystemClear();
}

void CTIandSIDlg::SetState(Systemstate state)
{
	sysstate=state;
	switch(state){
	case SYSTEM_PREPARE:{
		m_output.EnableWindow(TRUE);
		GetDlgItem(IDC_URLLIST_DELETE)->EnableWindow(TRUE);
		GetDlgItem(IDC_URLLIST_DELETEALL)->EnableWindow(TRUE);
		GetDlgItem(IDOK)->EnableWindow(TRUE);
		GetDlgItem(IDPAUSE)->EnableWindow(FALSE);
		GetDlgItem(IDSTOP)->EnableWindow(FALSE);
		GetDlgItem(IDC_URLLIST_ADD)->EnableWindow(TRUE);
		GetDlgItem(IDPAUSE)->SetWindowText(L"暂停");
		break;
				 }
	case SYSTEM_PAUSE:{
		GetDlgItem(IDPAUSE)->SetWindowText(L"继续");
		break;
				 }
	case SYSTEM_PROCESS:{
		m_output.EnableWindow(FALSE);
		GetDlgItem(IDC_URLLIST_DELETE)->EnableWindow(FALSE);
		GetDlgItem(IDC_URLLIST_DELETEALL)->EnableWindow(FALSE);
		GetDlgItem(IDOK)->EnableWindow(FALSE);
		GetDlgItem(IDPAUSE)->EnableWindow(TRUE);
		GetDlgItem(IDSTOP)->EnableWindow(TRUE);
		GetDlgItem(IDC_URLLIST_ADD)->EnableWindow(FALSE);
		GetDlgItem(IDPAUSE)->SetWindowText(L"暂停");
		state=SYSTEM_PROCESS;
		break;
				 }
	}
}

void CTIandSIDlg::OnBnClickedResult()
{
	resultdlg->ShowWindow(TRUE);
}




void CTIandSIDlg::OnDropFiles(HDROP hDropInfo)
{
	LPTSTR pFilePathName =(LPTSTR)malloc(500);
	::DragQueryFile(hDropInfo, 0, pFilePathName,500);  // 获取拖放文件的完整文件名，最关键！

	m_inputurl.SetWindowText(pFilePathName);

	::DragFinish(hDropInfo);   // 注意这个不能少，它用于释放Windows 为处理文件拖放而分配的内存
	free(pFilePathName);

	CDialogEx::OnDropFiles(hDropInfo);
}


void CTIandSIDlg::OnBnClickedPause()
{
	if(sysstate==SYSTEM_PROCESS){
		SetState(SYSTEM_PAUSE);
	}else{
		SetState(SYSTEM_PROCESS);
	}
}


void CTIandSIDlg::OnBnClickedStop()
{
	SetState(SYSTEM_PREPARE);
}


void CTIandSIDlg::OnWebsite()
{
	ShellExecuteA(NULL, "open","http://blog.csdn.net/leixiaohua1020",NULL,NULL,SW_SHOWNORMAL);
}


void CTIandSIDlg::OnUrllistQuickadd()
{
	CFileDialog dlg(TRUE,NULL,NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT|OFN_ALLOWMULTISELECT); 
	//解决文件名字符串不够长,更改缓冲区大小
	TCHAR szLargeBuf[8192]={0}; 
	dlg.m_ofn.lpstrFile =szLargeBuf;
	dlg.m_ofn.nMaxFile=8192;
	if(dlg.DoModal()==IDOK) 
	{ 
		POSITION pos = dlg.GetStartPosition(); 
		while(pos) 
		{ 
			CString szFileName = dlg.GetNextPathName(pos); 
			urllist.push_back(szFileName); 
		} 
		RefreshUrllist();
	} 
}

void CTIandSIDlg::OnCancel()
{
	if(IDOK==AfxMessageBox(L"确定退出？",MB_OKCANCEL|MB_ICONINFORMATION)){
		resultdlg->DestroyWindow();
		delete resultdlg;
		CDialogEx::OnCancel();
	}

}