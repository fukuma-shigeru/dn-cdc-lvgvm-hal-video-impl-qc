/*******************************************************************************
    機能名称    ：  Hal certification program用API
    ファイル名称：  halcp_main.cpp
*******************************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/un.h>
#include <errno.h>

#include <iostream>

#include "halcp_public.h"

namespace halcp
{

enum ResultType {
  RESULT_TYPE_OK,
  RESULT_TYPE_NG,
};

#if 0
static int32_t RegisterResult(ResultType type)
{
  if (RESULT_TYPE_OK == type) {
    //EXPECT_EQ(1,1);
    printf("***** OK *****\n");
  } else {
    //EXPECT_EQ(1,0);
    printf("----- NG -----\n");
  }
  return HALCP_RET_OK;
}
#endif /* 0 */

/***************************************************************************
 Name: HalcpIndication
 Brief: print indication message to do test flow
 Argument: [in] const char* msg: indcation message
 Return: HALCP_RET_OK: Success
         HALCP_RET_ERR_ARG: error that occurs in msg == NULL
 Detail: - Print a message to stdout.
         - New line isn't auto append for the message.
****************************************************************************/
extern "C" int32_t HalcpIndication(const char *msg)
{
  if (NULL == msg) {
    return HALCP_RET_ERR_ARG;
  } else {
    printf("%s", msg);
    return HALCP_RET_OK;
  }
}

/***************************************************************************
 Name: HalcpIndication
 Brief: print indication message to do test flow
 Argument: [in] std::string &msg: indcation message
 Return: HALCP_RET_OK: Success
 Detail: - Print a message to cout.
         - New line isn't auto append for the message.
****************************************************************************/
int32_t HalcpIndication(const std::string &msg)
{
  std::cout << msg;
  return HALCP_RET_OK;
}


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
static const char* g_default_wait_anykey_prompt =
  "Please input any key for continue test:";

extern "C" int32_t HalcpWaitInputAnyKey(const char *prompt)
{
  /* selection of prompt message */
  if (prompt == NULL) {
    printf("%s", g_default_wait_anykey_prompt);
  } else {
    printf("%s", prompt);
  }
  fflush(stdout);

  /* read any input */
  char tmp[2];
  int ret = scanf("%1[^\n]%*[^\n]", tmp);

  if (ret < 0) {
    printf("input EOF or raise error in scanf.(err=%d)\n", errno);
  } else {
    /* read skip enter */
    scanf("%*c");
  }

  return HALCP_RET_OK;
}

/***************************************************************************
 Name: HalcpWaitInputAnyKey
 Brief: Wait for test to progress until any keystroke occurs
 Argument: [in] const std::string &prompt: Message to print while waiting
 Return: HALCP_RET_OK: Success
 Detail: - Wait any key input to cin.
         - Enter key is accepted too and finish waiting.
         - Print a prompt message before the wait.
         - New line isn't auto append for the message.
****************************************************************************/
int32_t HalcpWaitInputAnyKey(const std::string &prompt)
{
  std::cout << prompt;
  std::string input_key;
  std::getline(std::cin, input_key);
  return HALCP_RET_OK;
}


/***************************************************************************
 Name: HalcpWaitInputResult
 Brief: Wait for test results to be entered
 Argument: [in] const char* prompt: Message to print while waiting
             - if prompt == NULL, print default message.
           [in] const char* ok_chars: Character set to judge OK
             - if ok_chars == NULL, set to "yY".
             - if prompt == NULL, ok_chars set to "yY".
 Return: HALCP_RET_OK: Success
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
static const char* g_default_wait_result_prompt =
  "Please input test result('y''Y' is OK, others is NG):";
static const char* g_default_ok_chars = "yY";

extern "C" int32_t HalcpWaitInputResult(const char *prompt,
                                        const char *ok_chars)
{
  const char *l_prompt = nullptr;
  const char *l_ok_chars = nullptr;
  /* selection of prompt message */
  if (nullptr == prompt) {
    l_prompt = g_default_wait_result_prompt;
  } else {
    l_prompt = prompt;
  }
  /* selection of prompt message */
  if (nullptr == prompt || nullptr == ok_chars) {
    l_ok_chars = g_default_ok_chars;
  } else {
    l_ok_chars = ok_chars;
  }

  /* read any input */
  char tmp[2];

  int ret;
  do {
    printf("%s", l_prompt);
    fflush(stdout);
    ret = scanf("%1[^\n]%*[^\n]", tmp);
    if (ret < 0) {
      printf("input EOF or raise error in scanf.(err=%d)\n", errno);
      return HALCP_RET_ERR_INPUT;
    }
    /* read skip enter */
    scanf("%*c");
  } while (ret == 0);

  const char *c = l_ok_chars;
  for (c = l_ok_chars; *c != '\0'; c++) {
    if (tmp[0] == *c) {
      break;
    }
  }
  if (*c != '\0') {
    return HALCP_RET_OK;
  } else {
    return HALCP_RET_NG;
  }
}

/***************************************************************************
 Name: HalcpWaitInputResult
 Brief: Wait for test results to be entered
 Argument: [in] const std::string &prompt: Message to print while waiting
           [in] const std::string &ok_chars: Character set to judge OK
 Return: HALCP_RET_OK: Success
         HALCP_RET_NG: result is NG
         HALCP_RET_ERR_INPUT: Error occurred during input value analysis
 Detail: - Wait input of test result to cin
         - If any of the characters listed in ok_chars are entered,
           result is consided OK and "HALCP_RET_OK" is returned.
         - At least one character must be listed as a string in ok_chars.
         - Enter key only input is not accepted and continue waiting.
         - Print a prompt message before the wait.
         - New line isn't auto append for the message.
****************************************************************************/
int32_t HalcpWaitInputResult(const std::string &prompt,
                             const std::string &ok_chars)
{
  /* read any input */
  std::string input_key;
  do {
    std::cout << prompt;
    std::getline(std::cin, input_key);
    if (0 == input_key.size()) {
      /* Enter key only */
      continue;
    }
    if (ok_chars.find_first_of(input_key.front()) == std::string::npos) {
      return HALCP_RET_NG;
    } else {
      return HALCP_RET_OK;
    }
  } while (true);
}

int32_t HalcpWaitInputResult(const char *prompt,
                             const std::string &ok_chars)
{
  if (nullptr == prompt) {
    return HalcpWaitInputResult(nullptr, nullptr);
  } else {
    return HalcpWaitInputResult(std::string(prompt), ok_chars);
  }
}

int32_t HalcpWaitInputResult(const std::string &prompt,
                             const char *ok_chars)
{
  if (nullptr == ok_chars) {
    return HalcpWaitInputResult(prompt, std::string(g_default_ok_chars));
  } else {
    return HalcpWaitInputResult(prompt, std::string(ok_chars));
  }
}

} /* namespace halcp */
