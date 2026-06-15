/*******************************************************************************
    機能名称    ：  HAL certification program用公開API
    ファイル名称：  halcp_public.h
*******************************************************************************/
#ifndef HALCP_PUBLIC_H
#define HALCP_PUBLIC_H

#ifdef __cplusplus
/*--------------------------------------------------------------------------*/
/* includes for c++                                                         */
/*--------------------------------------------------------------------------*/
#include <string>

namespace halcp
{
extern "C" {
#endif /* __cplusplus */

/*--------------------------------------------------------------------------*/
/* includes                                                                 */
/*--------------------------------------------------------------------------*/
#include <stdint.h>

/*--------------------------------------------------------------------------*/
/* define                                                                   */
/*--------------------------------------------------------------------------*/
/* return value */
#define HALCP_RET_OK        (0)    /* Success exit, result OK */
#define HALCP_RET_NG        (1)    /* result NG */
#define HALCP_RET_ERR_ARG   (-1)   /* Incorrect Argument */
#define HALCP_RET_ERR_INPUT (-2)   /* Incorrect Input */

/*--------------------------------------------------------------------------*/
/* typedef                                                                  */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/* struct                                                                   */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/* prototype                                                                */
/*--------------------------------------------------------------------------*/

/***************************************************************************
 Name: HalcpIndication
 Brief: print indication message to do test flow
 Argument: [in] const char* msg: indcation message
 Return: HALCP_RET_OK: Success
         HALCP_RET_ERR_ARG: error that occurs in msg == NULL
 Detail: - Print a message to stdout.
         - New line isn't auto append for the message.
****************************************************************************/
int32_t HalcpIndication(const char *msg);

/***************************************************************************
 Name: HalcpWaitInputAnyKey
 Brief: Wait for test to progress until any keystroke occurs
 Argument: [in] const char* prompt: Message to print while waiting
             - if prompt == NULL, print default message.
 Return: HALCP_RET_OK: Success
 Detail: - Wait any key input to stdin.
         - Enter key is accepted too and finish waiting.
         - Print a prompt message before the wait.
         - New line isn't auto append for the message.
****************************************************************************/
int32_t HalcpWaitInputAnyKey(const char *prompt);

/***************************************************************************
 Name: HalcpWaitInputResult
 Brief: Wait for test results to be entered
 Argument: [in] const char* prompt: Message to print while waiting
             - if prompt == NULL, print default message.
           [in] const char* ok_chars: Character set to judge OK
             - if ok_chars == NULL, set to "yY".
             - if prompt == NULL, ok_chars set to "yY".
 Return: HALCP_RET_OK: result is OK
         HALCP_RET_NG: result is NG
         HALCP_RET_ERR_INPUT: Error occurred during input value analysis
 Detail: - Wait input of test result to stdin.
         - If any of the characters listed in ok_chars are entered,
           result is consided OK and "HALCP_RET_OK" is returned.
         - At least one character must be listed as a string in ok_chars.
         - Enter key only input is not accepted and continue waiting.
         - Print a prompt message before the wait.
         - New line isn't auto append for the message.
****************************************************************************/
int32_t HalcpWaitInputResult(const char *prompt, const char *ok_chars);


#ifdef __cplusplus
}

/* Overloaded functions that can handle std::string type. */
int32_t HalcpIndication(const std::string &msg);
int32_t HalcpWaitInputAnyKey(const std::string &prompt);
int32_t HalcpWaitInputResult(const std::string &prompt, const std::string &ok_chars);
int32_t HalcpWaitInputResult(const char *prompt, const std::string &ok_chars);
int32_t HalcpWaitInputResult(const std::string &prompt, const char *ok_chars);
}
#endif /* __cplusplus */

#endif /* HALCP_PUBLIC_H */
