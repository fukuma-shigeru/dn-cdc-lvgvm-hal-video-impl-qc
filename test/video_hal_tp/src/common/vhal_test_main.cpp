#include "vhal_test_common.h"
#include <dlt/dlt.h>

#define VHAL_TP_DLT_ID				"HVDC"

int main(int argc, char **argv) {
::testing::InitGoogleTest(&argc, argv);
::testing::AddGlobalTestEnvironment(new VhalApiTestEnvironment());

	return RUN_ALL_TESTS();
}

void VhalApiTestEnvironment::SetUp(void)
{
	DLT_REGISTER_APP(VHAL_TP_DLT_ID, "VideoHAL test");
}

void VhalApiTestEnvironment::TearDown(void)
{
	IlmFinalize();

	DLT_UNREGISTER_APP();
	usleep(200 * 500);
}
