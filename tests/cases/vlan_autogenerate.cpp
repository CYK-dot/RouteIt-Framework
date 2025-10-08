/**
 * @file vlan_autogenerate.cpp
 * @author CYK-Dot
 * @brief Brief description
 * @version 0.1
 * @date 2025-10-08
 *
 * @copyright Copyright (c) 2025 CYK-Dot, MIT License.
 */

/* Header import ------------------------------------------------------------------*/
#include <gtest/gtest.h>
#include "rti_vlan.h"
#include "test_vlanid.h"

/* Config macros ------------------------------------------------------------------*/
#ifdef RTI_TEST_VLAN_TABLE_GENERATE

/* Mock variables and functions  --------------------------------------------------*/
static void* mock1_create(void) { return nullptr; }
static void mock1_delete(void*) {}
static void* mock1_create_producer(void) { return nullptr; }
static void mock1_delete_producer(void*) {}
static void* mock1_create_consumer(void) { return nullptr; }
static void mock1_delete_consumer(void*) {}
static RTI_VLAN_IFX mock1_vlan_ifx = {
    mock1_create,
    mock1_delete, 
    mock1_create_producer,
    mock1_delete_producer,
    mock1_create_consumer,
    mock1_delete_consumer
};
RTI_VLAN_REGISTER_STATIC(&mock1_vlan_ifx, AUTO_VLAN1);

static void* mock2_create(void) { return nullptr; }
static void mock2_delete(void*) {}
static void* mock2_create_producer(void) { return nullptr; }
static void mock2_delete_producer(void*) {}
static void* mock2_create_consumer(void) { return nullptr; }
static void mock2_delete_consumer(void*) {}
static RTI_VLAN_IFX mock2_vlan_ifx = {
    mock2_create,
    mock2_delete, 
    mock2_create_producer,
    mock2_delete_producer,
    mock2_create_consumer,
    mock2_delete_consumer
};
RTI_VLAN_REGISTER_STATIC(&mock2_vlan_ifx, AUTO_VLAN2);

/* Test suites --------------------------------------------------------------------*/

/**
 * @brief testcases when only operating static Vlan-Table
 * 
 */
class VlanGenerateStaticTest : public ::testing::Test {
protected:
    void SetUp() override {
    }
    void TearDown() override {
    }
    static void SetUpTestSuite() {
    }
    static void TearDownTestSuite() {
    }
};

/* Test cases ---------------------------------------------------------------------*/

/**
 * @brief select from static table
 * 
 */
TEST_F(VlanGenerateStaticTest, SelectFromStaticTable) {
    RTI_VLAN_DESC desc;
    RTI_ERR result = RTIPriv_VlanSelect(RTI_VLANID_AUTO_VLAN1, &desc);
    EXPECT_EQ(result, RTI_OK);
    EXPECT_EQ(desc.ifx, &mock1_vlan_ifx);
    EXPECT_STREQ(desc.name, "AUTO_VLAN1");
    EXPECT_EQ(desc.id, RTI_VLANID_AUTO_VLAN1);

    result = RTIPriv_VlanSelect(RTI_VLANID_AUTO_VLAN2, &desc);
    EXPECT_EQ(result, RTI_OK);
    EXPECT_EQ(desc.ifx, &mock2_vlan_ifx);
    EXPECT_STREQ(desc.name, "AUTO_VLAN2");
    EXPECT_EQ(desc.id, RTI_VLANID_AUTO_VLAN2);
}


#endif

