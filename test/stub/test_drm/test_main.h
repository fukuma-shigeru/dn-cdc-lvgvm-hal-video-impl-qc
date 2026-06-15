
#ifndef TEST_COMMON_H
#define TEST_COMMON_H

#include "gtest/gtest.h"

/* ŠÂ‹«İ’è */
class VhalTestEnvironment : public ::testing::Environment
{
public:
	virtual void SetUp();
	virtual void TearDown();
};

#endif /* TEST_COMMON_H */
