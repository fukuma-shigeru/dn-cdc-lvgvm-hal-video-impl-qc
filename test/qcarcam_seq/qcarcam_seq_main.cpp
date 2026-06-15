/* ここにQCarCamテストコードを記載ください。 */

#include "qcarcam_seq.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <unistd.h>

#define BUF 1024
#define InputCamera 0
#define InputHDMI 1
#define InputError 3  

#define InputGBM 7
#define InputSHM 8
#define Input_0_0 9

#define Type    20
#define Size    21
#define Name    22
#define Buffer  23
#define Epar    999

#define qcar_file          "qcarcam_seq.ini"
#define qualcomm_file      "qualcomm_seq.ini"

int TypeCheck(char *buf_p, const char *m1, const char *m2);
int Sizecheck_width(char *buf_p);
int Sizecheck_height(char *buf_p, uint32_t width );

int BufTypeCheck(char *buf_p, const char *m3, const char *m4);
void BufType(char *buf_p);

void re_space(char *q_buf);
void resource();

int main(int argc, char* argv[])
{
	FILE *file 	= NULL;
	uint32_t 	width;
	uint32_t 	height;
	char		buftype[100];
	char		name[100];
	int			buf_count = 0;
	int 		inputType;
	int			inputbufType;
	int			inputDevice;
	int 		sizeNum;
	int 		selectCmd = 0;
	bool 		errF = false;
	bool		type_f = false;
	bool		size_f = false;
	bool		name_f = false;
	bool		buftype_f = false;
	bool		all_f	=false;
	
	char 		*fname = NULL;
	char 		q_buf[1024][BUF];
	char 		Inputbuffer[1024];

	//ファイル名を表示
	//実行する 一つ目は実行モジュール名 2つ目以降がある場合
	if(argc>=2)
	{
		fname = argv[1];
		PRINT_LOGI("%s",fname);
		PRINT_LOGD("This file name");
	}
	
	//引数がある場合のみファイルを開く  
	if(fname!=NULL ) 
	{
		file = fopen(fname, "r");
	}
	else
	{
		char wkbuf[1024];
		readlink("/proc/self/exe", wkbuf, sizeof(wkbuf));
		char* ptr = strrchr(wkbuf, '/'); //strrchr 後ろから見てくれます
		if (ptr != nullptr)
		{
			ptr[1] = '\0'; // /の次にnullもじでうちきる
		}
		const char* filename[] = {qcar_file, qualcomm_file};
		for (size_t i=0; i<(sizeof(filename)/sizeof(filename[0])); i++)
		{
			char* p_tmppath = nullptr;
			asprintf(&p_tmppath, "%s%s", wkbuf, filename[i]);
			file = fopen(p_tmppath, "r");
			free(p_tmppath);
			if (file != nullptr)
			{
			break;
			}
		}
	}
	
	//ファイルの中身が入っている場合
	if (file!=NULL)
	{
		int	 i =0; 
		char 		*q_buf_val[1024];  //<-ファイル終端まで
		PRINT_LOGI("Read Contents  ");
		//取り込んだファイルを終端まで1行ずつ取り出す
		while (fgets(q_buf[i],sizeof(q_buf[i]),file)!= NULL )
		{
			
			char	*p;
			re_space(q_buf[i]);
			PRINT_LOGI("%s",q_buf[i]);
			
			//改行コードを削除
			q_buf_val[i] = q_buf[i];
			p = strchr(q_buf[i],'\n');
			if (p) *p = '\0';
			p = strchr(q_buf[i],'\r');
			if (p) *p = '\0';
			
			i++;
			buf_count = i;
		}
		
		//読み込んだファイルのライン数
		PRINT_LOGI("This Loading  = %dLines",i);
		if(i<=3) //<- 2/27 <=3に変更
		{
			PRINT_LOGE("Loading Lines error");
			errF = true;
		}
		const char *m1 = "Camera";
		const char *m2 = "HDMI";
		const char *m3 = "GBM";
		const char *m4 = "SHM";
		fclose(file);
		
		for(int i =0; i <= buf_count -1; i++)
		{
			PRINT_LOGI("/*---------------%dLines---------------*/",i+1);
			char *str_p = q_buf_val[i];
			PRINT_LOGI("%s",str_p);
			char *buf_p;
			char str[1024];
			char *str_t = "type=";
			char *str_s = "size=";
			char *str_n = "name=";
			char *str_b = "buftype=";
			int pracase =0;
			
			if(strchr(str_p,'='))
			{
				buf_p = strchr(str_p,'='); //イコールの位置
				size_t len = buf_p - str_p +1; //イコールまでの文字数
				strncpy(str,str_p,len); //文字列の先頭からlenまでをコピー
				PRINT_LOGI("length = %ld",len);
				memmove(buf_p, buf_p + 1, strlen(buf_p)); //buf_p(=より後ろ)の先頭文字(=)削除
			}
			
			if(strncasecmp(str_t,str,4)==0)
			{
				pracase = Type;
			}
			
			else if(strncasecmp(str_s,str,4)==0)
			{
				if(strchr(buf_p,'x')||strchr(buf_p,'X'))
				{
					pracase = Size;
				}
			}
			
			else if(strncasecmp(str_n,str,4)==0)
			{
				pracase = Name;
			}
			
			else if(strncasecmp(str_b,str,7)==0)
			{
				pracase = Buffer;
			}
			else
			{
				pracase = Epar;
			}

			switch(pracase)
			{
				case Type:
						inputType = TypeCheck( buf_p,  m1,  m2);
						if(InputError == inputType)
						{
							PRINT_LOGE("ERROR_TYPE");
						}
						else
						{
							type_f = true;
							
						}
						break;
				case Size:
						width  = Sizecheck_width(buf_p);
						height = Sizecheck_height(buf_p,width);
						if(Epar == width || Epar == height)
						{
							PRINT_LOGE("ERROR_SIZE");
						}
						else
						{
							size_f = true;
						}	
						break;
				case Name:
						if(buf_p!=NULL)
						{
							strcpy(name,buf_p);
							name_f = true;
						}
						else
						{
							PRINT_LOGE("ERROR_NAME");
						}
						break;
				case Buffer:
						inputbufType = BufTypeCheck( buf_p,  m3,  m4);
						if(InputError != inputbufType)
						{
							BufType(buf_p);
							strcpy(buftype,buf_p);
							buftype_f = true;
						}
						else
						{
							PRINT_LOGE("ERROR_BUF");
						}
						
						break;
				case Epar:
						PRINT_LOGE("ERROR_PRAMETER");
						break;
				default:
						break;
			}
			
			PRINT_LOGI("/*-------------------------------------*/");
			if(type_f == true && size_f == true && name_f == true && buftype_f == true )
			{
				all_f = true;
				PRINT_LOGI("ALL OK");
				PRINT_LOGI("inputType=%d width=%d height=%d name=%s buftype=%s",inputType,width,height,name,buftype);
				break;
			}
		}
		
		if(all_f == false)
		{
			errF = true;
			PRINT_LOGI("ERROR");
		}
		//PRINT_LOGI("Check Pram");
		//return 0;
		
	}
	
/*---------------------------ここから下手入力----------------------------------------------*/

	//ファイルを取り込まない場合または、取り込む時にエラーが出た場合 手入力
	if (file == NULL || errF == true)
	{

		//読み込んだ際データに不備等がある場合はファイル読み込みから入力に切り替わったことを知らせる
		if(errF == true)
		{
			PRINT_LOGI("Change Input Mode");
		}
		errF = false;
		PRINT_LOGI( "\x1b[36mPlease select input-type.  (0:Camera / 1:HDMI)\x1b[m");
		inputType = InputError; 
		fgets(Inputbuffer, sizeof(Inputbuffer), stdin);
		sscanf(Inputbuffer,"%d%*c",&inputType);

		if (inputType == InputCamera || inputType == InputHDMI )
		{
			PRINT_LOGI("SUCCESS");
			PRINT_LOGI( "\x1b[36mPlease select input-size.  (1:1920x1080 / 2:1280x720 / 3:640x480)\x1b[m");
			sizeNum = 0; 
			fgets(Inputbuffer, sizeof(Inputbuffer), stdin);
			sscanf(Inputbuffer,"%d%*c",&sizeNum);
			switch(sizeNum)
			{
			case 1:
				width  = 1920;
				height = 1080;
				PRINT_LOGI("your select 1:1920x1080");
				break;
	   		case 2:
				width  = 1280;
				height = 720;
				PRINT_LOGI("your select 2:1280x720");
				break;
	   		case 3:
	   			width  = 640;
				height = 480;
				PRINT_LOGI("your select 3:640x480");
				break;
	   		default :
				errF = true;
				break;
			}
			
			#if (VHAL_TIER1_TARGET == 0)
				snprintf(name, sizeof(name), "%s" ,(inputType == InputCamera ? D_BEV_QCARCAM_CHAR_CAM : D_BEV_QCARCAM_CHAR_HDMI));
				snprintf(buftype, sizeof(buftype), "shm");
			#else
				snprintf(name, sizeof(name), "INPUT_0_0");
				snprintf(buftype, sizeof(buftype), "gbm");
			#endif
			
		}
		else
		{
    		PRINT_LOGE("ValueError");
			return 1;
		}
    }

/*-------ここまで手打ち------*/
	
	if(errF == true)
	{
		PRINT_LOGE("ValueError");
		return 1;
	}

	/*関数の呼び出し*/
	QCarCamRet_e	ret  = QCARCAM_RET_OK;
	ret = BEV_Capture_Initializaion();

	//エラーの場合のデバッグ文
	if( ret != QCARCAM_RET_OK )
	{
		PRINT_LOGE("QCarCamInitializaion() ret=%d", ret );
		return 1;
	}

	//Type width height 値の確認
	PRINT_LOGI("CheckPoint inputType 	= %d",inputType);
	if(inputType == 0)
	{
		PRINT_LOGI("\x1b[36mCheckPoint inputType :Camera \x1b[m");
	}
	else
	{
		PRINT_LOGI("\x1b[36mCheckPoint inputType :HDMI \x1b[m");
	}
	PRINT_LOGI("\x1b[36mCheckPoint width 		= %d\x1b[m",width);
	PRINT_LOGI("\x1b[36mCheckPoint height 		= %d\x1b[m",height);
	PRINT_LOGI("\x1b[33m=== name=[%s] btyp=[%s] ===\x1b[m" ,name ,buftype);
//	getchar();
	ret = BEV_Capture_Setting(inputType,width,height ,name ,buftype );

    //エラーの場合のデバッグ文　
	if( ret != QCARCAM_RET_OK )
	{
		resource();
		PRINT_LOGE("QCarCamSetting() ret=%d", ret );
		return 1;
	}

	//Please select command で 2(end)が選ばれるまで繰り返す
	while(selectCmd!= 2)
	{
		ret = BEV_Capture_Start();
		PRINT_LOGD("Start");
    	//エラーの場合のデバッグ文
		if( ret != QCARCAM_RET_OK )
		{
			resource();
			PRINT_LOGE("QCarCamStart() ret=%d", ret );
			return 1;
		}
		if(ret == QCARCAM_RET_OK)
		{
			//Enterキーが押されるまで待機
			PRINT_LOGE("\x1b[36mWhen you press 'enter',it will stop\x1b[m");
			while(getchar() != '\n') ;
	 		ret = BEV_Capture_Stop();
			//エラーの場合のデバッグ文
			if( ret != QCARCAM_RET_OK )
		 	{
		 		resource();
		 		PRINT_LOGE("QCarCamStop() ret=%d", ret );
		 		return 1;
		 	}

			selectCmd = 0; //1.2以外が入れられたとき scanfで正しく変換できなたったときの対処
			while(0 == selectCmd)
			{
				PRINT_LOGI( "\x1b[36mPlease select command. (1:restart / 2:end )\x1b[m "); // 1 2 もdehineでまとめる
				fgets(Inputbuffer, sizeof(Inputbuffer), stdin);
				sscanf(Inputbuffer,"%d%*c",&selectCmd);
		 	 	if(selectCmd == 2 )
		 	 	{
			 		PRINT_LOGD("End");
			 		ret = BEV_Capture_End();
		 	 	}
		 	 	else if(selectCmd != 1)
		 	 	{
			 		PRINT_LOGE("ValueError");
		 	 		selectCmd = 0;
		 	 	}
			}
	  	}
	}
	if( ret != QCARCAM_RET_OK )
	{
		PRINT_LOGE("Failed in %d",ret);
		return 1;
	}
	return 0;
}

int TypeCheck(char *buf_p, const char *m1, const char *m2)
{
	int 		inputType;
	PRINT_LOGI("%s",m1);
	PRINT_LOGI("%s",m2);
	//読み込んだ文字列が期待通りであるかの確認
	if(strcasecmp(buf_p,m1) ==0)
	{
		inputType = InputCamera;
		PRINT_LOGI("Select: Camera");
		return inputType;
	}
	else if(strcasecmp(buf_p,m2) ==0)
	{
		inputType = InputHDMI;
		PRINT_LOGI("Select: HDMI");
		return inputType;
	}
	else
	{
		inputType = InputError;
		PRINT_LOGE("InputType error Line");
		return inputType;
	}
}

int Sizecheck_width(char *buf_p)
{
	uint32_t 		width;
	char			*width_x;
	char			width_ch[5];
	
	if(strchr(buf_p,'x'))
	{
		width_x = strchr(buf_p,'x');
	}
	else if(strchr(buf_p,'X'))
	{
		width_x = strchr(buf_p,'X');
	}
	
	size_t len = width_x - buf_p +1; 
	strncpy(width_ch,buf_p,len);
	
	width = atoi(width_ch);
	PRINT_LOGI("width = %d",width);
	if(width == 1920 || width == 1280 || width == 640)
	{
		return width;
	}
	else
	{
		width = Epar;
		return width;
	}
}

int Sizecheck_height(char *buf_p, uint32_t width)
{

	uint32_t 		height;
	char			*height_ch;
	
	if(strchr(buf_p,'x'))
	{
		height_ch = strchr(buf_p,'x');
	}
	else if(strchr(buf_p,'X'))
	{
		height_ch = strchr(buf_p,'X');
	}
	
	memmove(height_ch, height_ch + 1, strlen(height_ch));
	height = atoi(height_ch);
	PRINT_LOGI("height = %d",height);
	
	switch(width)
	{
		case 1920:
			if(height == 1080)
			{
				break;
			}
			else
			{
				height = Epar;
				break;
			}
		case 1280:
			if(height == 720)
			{
				break;
			}
			else
			{
				height = Epar;
				break;
			}
		case 640:
			if(height == 480)
			{
				break;
			}
			else
			{
				height = Epar;
				break;
			}
		default:
			height = Epar;
			break;
	}
	return height;
}

int BufTypeCheck(char *buf_p, const char *m3, const char *m4)
{
	
	int 		inputbufType;
	PRINT_LOGI("%s",m3);
	PRINT_LOGI("%s",m4);
	//読み込んだ文字列が期待通りであるかの確認
	if(strcasecmp(buf_p,m3) ==0)
	{
		inputbufType = InputGBM;
		PRINT_LOGI("Select: GBM");
		return inputbufType;
	}
	else if(strcasecmp(buf_p,m4) ==0)
	{
		inputbufType = InputSHM;
		PRINT_LOGI("Select: SHM");
		return inputbufType;
	}
	else
	{
		inputbufType = InputError;
		PRINT_LOGE("InputType error Line");
		return inputbufType;
	}
}
	
void BufType(char *buf_p)
{
	char	buftype[3];
	while(*buf_p)
	{
		*buf_p = tolower((unsigned char)*buf_p);
		buf_p++;
	}
}

void re_space(char *q_buf)
{
	char	*str_r = q_buf;
	char	*str_w = q_buf;
			
	//空白を詰める
	while(*str_r!= '\n')
	{
		if(*str_r!= ' ')
		{
			*str_w = *str_r;	
			str_w++;
		}
		str_r++;
	}
	*str_w = '\0';

}

void resource()
{
	BEV_Capture_End();
	PRINT_LOGD("finish");
}