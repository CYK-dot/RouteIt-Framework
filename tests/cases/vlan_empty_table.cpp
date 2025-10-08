/**
 * @file vlan_empty_table.cpp
 * @author CYK-Dot
 * @brief testcases for table functions when Vlan-Table empty
 * @version 0.1
 * @date 2025-10-07
 *
 * @copyright Copyright (c) 2025 CYK-Dot, MIT License.
 */

/* Header import ------------------------------------------------------------------*/
#include <gtest/gtest.h>
#include "rti_vlan.h"

/* Config macros ------------------------------------------------------------------*/
#ifdef RTI_TEST_VLAN_TABLE_EMPTY

/* Mock variables and functions  --------------------------------------------------*/
static void* mock_create(void) { return nullptr; }
static void mock_delete(void*) {}
static void* mock_create_producer(void) { return nullptr; }
static void mock_delete_producer(void*) {}
static void* mock_create_consumer(void) { return nullptr; }
static void mock_delete_consumer(void*) {}
static RTI_VLAN_IFX mock_vlan_ifx = {
    mock_create,
    mock_delete, 
    mock_create_producer,
    mock_delete_producer,
    mock_create_consumer,
    mock_delete_consumer
};
static RTI_VLAN_DESC mock_vlan_desc = {&mock_vlan_ifx, "VLAN1", 1};

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
static RTI_VLAN_DESC mock2_vlan_desc = {&mock2_vlan_ifx, "VLAN2", 2};
static RTI_VLAN_DESC mock3_vlan_desc = {&mock2_vlan_ifx, "VLAN3", 3};

/* Test suites --------------------------------------------------------------------*/

/**
 * @brief testcases when static Vlan-Table empty
 * 
 */
class VlanEmptyTest : public ::testing::Test {
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

/**
 * @brief testcases when dynamic Vlan-Table empty
 * 
 */
class VlanDynamicEmptyTest : public ::testing::Test {
protected:
    void SetUp() override {
    }
    void TearDown() override {
    }
    static void SetUpTestSuite() {
        oldTable = RTIDFX_VlanGetTableAddr();
        oldTableSize = RTI_VLAN_VLANTABLE_SIZE(RTIDFX_VlanGetTableRecordMaxCount());

        newTableSize = RTI_VLAN_VLANTABLE_SIZE(2);
        newTable = (RTI_VLAN_RECORD *)malloc(newTableSize);
        
        EXPECT_NE(newTable, nullptr) << "malloc failed";
        RTI_ERR result = RTI_VlanDynamicSetup(newTable, 0);
        EXPECT_EQ(result, RTI_OK) << "dynamic setup failed";
    }
    static void TearDownTestSuite() {
        RTI_ERR err = RTIDFX_VlanTableForceSet(oldTable, oldTableSize);
        EXPECT_EQ(err, RTI_OK) << "force set table failed";
        free(newTable);
    }
    static RTI_VLAN_RECORD *oldTable;
    static RTI_VLAN_RECORD *newTable;
    static size_t newTableSize;
    static size_t oldTableSize;
};
RTI_VLAN_RECORD *VlanDynamicEmptyTest::oldTable;
RTI_VLAN_RECORD *VlanDynamicEmptyTest::newTable;
size_t VlanDynamicEmptyTest::newTableSize;
size_t VlanDynamicEmptyTest::oldTableSize;

/**
 * @brief testcases when static Vlan-Table empty and dynamic Vlan-Table not empty
 */
class VlanStaticEmptyDynamicNoEmptyTest : public ::testing::Test {
protected:
    void SetUp() override {
    }
    void TearDown() override {
    }
    static void SetUpTestSuite() {
        oldTable = RTIDFX_VlanGetTableAddr();
        oldTableSize = RTI_VLAN_VLANTABLE_SIZE(RTIDFX_VlanGetTableRecordMaxCount());

        newTableSize = RTI_VLAN_VLANTABLE_SIZE(2);
        newTable = (RTI_VLAN_RECORD *)malloc(newTableSize);
        
        EXPECT_NE(newTable, nullptr) << "malloc failed";
        RTI_ERR result = RTI_VlanDynamicSetup(newTable, newTableSize);
        EXPECT_EQ(result, RTI_OK) << "dynamic setup failed";
    }
    static void TearDownTestSuite() {
        RTI_ERR err = RTIDFX_VlanTableForceSet(oldTable, oldTableSize);
        EXPECT_EQ(err, RTI_OK) << "force set table failed";
        free(newTable);
    }
    static RTI_VLAN_RECORD *oldTable;
    static RTI_VLAN_RECORD *newTable;
    static size_t newTableSize;
    static size_t oldTableSize;
};
RTI_VLAN_RECORD *VlanStaticEmptyDynamicNoEmptyTest::oldTable;
RTI_VLAN_RECORD *VlanStaticEmptyDynamicNoEmptyTest::newTable;
size_t VlanStaticEmptyDynamicNoEmptyTest::newTableSize;
size_t VlanStaticEmptyDynamicNoEmptyTest::oldTableSize;

/* Test cases ---------------------------------------------------------------------*/

/**
 * @brief empty table cannot select
 * 
 */
TEST_F(VlanEmptyTest, SelectFromEmptyStaticTable) {
    RTI_VLAN_DESC desc;
    RTI_ERR result = RTIPriv_VlanSelect(1, &desc);
    EXPECT_EQ(result, RTI_ERR_OBJECT_EMPTY);
}
TEST_F(VlanDynamicEmptyTest, SelectFromEmptyDynamicTable) {
    RTI_VLAN_DESC desc;
    RTI_ERR result = RTIPriv_VlanSelect(1, &desc);
    EXPECT_EQ(result, RTI_ERR_OBJECT_EMPTY);
}

/**
 * @brief dynamic functions cannot be called when not setup
 * 
 */
TEST_F(VlanEmptyTest, DynamicFunctionsWhenNotSetup) {
    size_t count;
    RTI_VLAN_DESC desc = {&mock_vlan_ifx, "test", 1};

    EXPECT_NE(RTI_VlanDynamicRegister(&desc), RTI_OK);
    EXPECT_NE(RTI_VlanDynamicIsRegister(&desc), RTI_OK);
    EXPECT_NE(RTI_VlanDynamicGetFreeCount(&count), RTI_OK);
    EXPECT_NE(RTI_VlanDynamicGetAllCount(&count), RTI_OK);
}
TEST_F(VlanDynamicEmptyTest, DynamicFunctionsWhenSetupEmptyTable) {
    size_t count;
    RTI_VLAN_DESC desc = {&mock_vlan_ifx, "test", 1};

    EXPECT_NE(RTI_VlanDynamicRegister(&desc), RTI_OK);
    EXPECT_NE(RTI_VlanDynamicIsRegister(&desc), RTI_OK);
}

TEST_F(VlanStaticEmptyDynamicNoEmptyTest, DynamicRegisterCheck) {
    // check free size
    size_t count, countAll;
    RTI_ERR result = RTI_VlanDynamicGetFreeCount(&count);
    EXPECT_EQ(result, RTI_OK);
    EXPECT_EQ(count, 2);
    result = RTI_VlanDynamicGetAllCount(&countAll);
    EXPECT_EQ(result, RTI_OK);
    EXPECT_EQ(countAll, 2);
    // register new record
    result = RTI_VlanDynamicRegister(&mock_vlan_desc);
    EXPECT_EQ(result, RTI_OK) << "dynamic register failed";
    result = RTI_VlanDynamicRegister(&mock2_vlan_desc);
    EXPECT_EQ(result, RTI_OK) << "dynamic register failed";
    // select to check new records
    RTI_VLAN_DESC desc;
    result = RTIPriv_VlanSelect(1, &desc);
    EXPECT_EQ(result, RTI_OK);
    EXPECT_EQ(desc.ifx, &mock_vlan_ifx);
    EXPECT_STREQ(desc.name, "VLAN1");
    EXPECT_EQ(desc.id, 1);
    result = RTIPriv_VlanSelect(2, &desc);
    EXPECT_EQ(result, RTI_OK);
    EXPECT_EQ(desc.ifx, &mock2_vlan_ifx);
    EXPECT_STREQ(desc.name, "VLAN2");
    EXPECT_EQ(desc.id, 2);
    // resume
    result = RTIDFX_VlanTableUnregister(1);
    EXPECT_EQ(result, RTI_OK) << "unregister dynamic record failed";
    result = RTIPriv_VlanSelect(1, &desc);
    EXPECT_NE(result, RTI_OK) << "unregister failed, record did not removed";
    result = RTIDFX_VlanTableUnregister(2);
    EXPECT_EQ(result, RTI_OK) << "unregister dynamic record failed";
    result = RTIPriv_VlanSelect(2, &desc);
    EXPECT_NE(result, RTI_OK) << "unregister failed, record did not removed";
}

TEST_F(VlanStaticEmptyDynamicNoEmptyTest, DynamicRegisterStopWhenFull) {
    // register new record to full
    RTI_ERR result = RTI_VlanDynamicRegister(&mock_vlan_desc);
    EXPECT_EQ(result, RTI_OK) << "dynamic register failed";
    result = RTI_VlanDynamicRegister(&mock2_vlan_desc);
    EXPECT_EQ(result, RTI_OK) << "dynamic register failed";
    // register again
    result = RTI_VlanDynamicRegister(&mock3_vlan_desc);
    EXPECT_EQ(result, RTI_OK) << "dynamic register failed";
    // check free size
    size_t count, countAll;
    result = RTI_VlanDynamicGetFreeCount(&count);
    EXPECT_EQ(result, RTI_OK);
    EXPECT_EQ(count, 0);
    result = RTI_VlanDynamicGetAllCount(&countAll);
    EXPECT_EQ(result, RTI_OK);
    EXPECT_EQ(countAll, 2);
}

#endif