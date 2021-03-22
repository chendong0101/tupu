#include <gtest/gtest.h>
#include <gflags/gflags.h>

#include "config.h"
#include "service/property_id_service.h"

class TestEnvironment : public testing::Environment
{
public:
    virtual void SetUp()
    {
        PropertyIdService::getInstance()->init();
    }
};

int main(int argc, char** argv)
{
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    ::testing::AddGlobalTestEnvironment(new TestEnvironment);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}