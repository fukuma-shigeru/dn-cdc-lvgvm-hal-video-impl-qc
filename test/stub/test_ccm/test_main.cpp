
#include "test_main.h"
#include <dlt/dlt.h>

#define VHAL_TP_DLT_ID				"HVDC"

int main(int argc, char **argv)
{

	::testing::InitGoogleTest(&argc, argv);
	::testing::AddGlobalTestEnvironment(new VhalTestEnvironment());

	return RUN_ALL_TESTS();
}

void VhalTestEnvironment::SetUp(void)
{
	DLT_REGISTER_APP(VHAL_TP_DLT_ID, "ccm_stub test");

}

void VhalTestEnvironment::TearDown(void)
{
	DLT_UNREGISTER_APP();
	usleep(200 * 500);
}

