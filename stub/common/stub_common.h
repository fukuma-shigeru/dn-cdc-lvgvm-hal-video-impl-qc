
#ifndef STUB_CLIENT_H
#define STUB_CLIENT_H


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


typedef void ( *StubClientNotifyListener )( const char* p_path );

/*
	戻り値：負の値:エラー、負の値以外:成功
*/
int StubClientRegisterListerner(const char* p_path, StubClientNotifyListener p_listener);
int StubClientUnRegisterListerner(const char* p_path);

int StubClientRead_i32(const char* p_path, int* value);
int StubClientRead_bin(const char* p_path, int* value, int size);
int StubClientWrite_bin(const char* p_path, int* value, int size);
int StubClientRead_str(const char* p_path, char* p_str);
int StubClientWrite_str(const char* p_path, char* p_str);

#ifdef __cplusplus
}
#endif /* __cplusplus */



#endif /* STUB_CLIENT_H */

