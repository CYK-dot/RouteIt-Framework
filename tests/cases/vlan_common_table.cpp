/**
 * @file vlan_common_table.cpp
 * @author CYK-Dot
 * @brief testcases for table functions when Vlan-Table not empty
 * @version 0.1
 * @date 2025-10-07
 *
 * @copyright Copyright (c) 2025 CYK-Dot, MIT License.
 */

/* Header import ------------------------------------------------------------------*/
#include <gtest/gtest.h>
#include "rti_vlan.h"

/* Config macros ------------------------------------------------------------------*/
#ifdef RTI_TEST_VLAN_TABLE_COMMON

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
RTI_VLAN_REGISTER_STATIC_WITH_ID(&mock1_vlan_ifx, VLAN1, 1);

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
RTI_VLAN_REGISTER_STATIC_WITH_ID(&mock2_vlan_ifx, VLAN2, 2);

static void* mock3_create(void) { return nullptr; }
static void mock3_delete(void*) {}
static void* mock3_create_producer(void) { return nullptr; }
static void mock3_delete_producer(void*) {}
static void* mock3_create_consumer(void) { return nullptr; }
static void mock3_delete_consumer(void*) {}
static RTI_VLAN_IFX mock3_vlan_ifx = {
    mock3_create,
    mock3_delete, 
    mock3_create_producer,
    mock3_delete_producer,
    mock3_create_consumer,
    mock3_delete_consumer
};
static RTI_VLAN_DESC mock3_vlan_desc = {&mock3_vlan_ifx, (char *)"VLAN3", 3};
static RTI_VLAN_IFX mock4_vlan_ifx = {
    mock3_create,
    mock3_delete, 
    mock3_create_producer,
    mock3_delete_producer,
    mock3_create_consumer,
    mock3_delete_consumer
};
static RTI_VLAN_DESC mock4_vlan_desc = {&mock4_vlan_ifx, (char *)"VLAN4", 4};

/* Test suites --------------------------------------------------------------------*/

/**
 * @brief testcases when only operating static Vlan-Table
 * 
 */
class VlanStaticTest : public ::testing::Test {
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
 * @brief testcases when setup dynamic Vlan-Table
 * 
 */
class VlanDynamicSetupTest : public ::testing::Test {
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
 * @brief testcases when only operating dynamic Vlan-Table
 * 
 */
class VlanDynamicTest : public ::testing::Test {
protected:
    void SetUp() override {
    }
    void TearDown() override {
    }
    static void SetUpTestSuite() {
        oldTable = RTIDFX_VlanGetTableAddr();
        oldTableSize = RTI_VLAN_VLANTABLE_SIZE(RTIDFX_VlanGetTableRecordMaxCount());

        newTableSize = RTI_VLAN_VLANTABLE_SIZE(3);
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
    static size_t oldTableSize;
    static RTI_VLAN_RECORD *newTable;
    static size_t newTableSize;
};
RTI_VLAN_RECORD *VlanDynamicTest::oldTable;
RTI_VLAN_RECORD *VlanDynamicTest::newTable;
size_t VlanDynamicTest::newTableSize;
size_t VlanDynamicTest::oldTableSize;

/* Test cases ---------------------------------------------------------------------*/

/**
 * @brief select from static table
 * 
 */
TEST_F(VlanStaticTest, SelectFromStaticTable) {
    RTI_VLAN_DESC desc;
    RTI_ERR result = RTIPriv_VlanSelect(1, &desc);
    EXPECT_EQ(result, RTI_OK);
    EXPECT_EQ(desc.ifx, &mock1_vlan_ifx);
    EXPECT_STREQ(desc.name, "VLAN1");
    EXPECT_EQ(desc.id, 1);

    result = RTIPriv_VlanSelect(2, &desc);
    EXPECT_EQ(result, RTI_OK);
    EXPECT_EQ(desc.ifx, &mock2_vlan_ifx);
    EXPECT_STREQ(desc.name, "VLAN2");
    EXPECT_EQ(desc.id, 2);
}

/**
 * @brief dynamic functions cannot be called when not setup
 * 
 */
TEST_F(VlanStaticTest, DynamicFunctionsWhenNotSetup) {
    size_t count;

    EXPECT_NE(RTI_VlanDynamicRegister(&mock3_vlan_desc), RTI_OK);
    EXPECT_NE(RTI_VlanDynamicIsRegister(&mock3_vlan_desc), RTI_OK);
    EXPECT_NE(RTI_VlanDynamicGetFreeCount(&count), RTI_OK);
    EXPECT_NE(RTI_VlanDynamicGetAllCount(&count), RTI_OK);
}

/**
 * @brief dynamic setup should fail if table too short 
 * 
 */
TEST_F(VlanDynamicSetupTest, DynamicSetupFail) {
    void *dynamicTable = malloc(RTI_VLAN_VLANTABLE_SIZE(1));
    EXPECT_NE(dynamicTable, nullptr);
    RTI_ERR result = RTI_VlanDynamicSetup(dynamicTable, 0);
    EXPECT_NE(result, RTI_OK) << "dynamic setup using zero size,unexpected passed";
    result = RTI_VlanDynamicSetup(dynamicTable, RTI_VLAN_VLANTABLE_SIZE(1));
    EXPECT_NE(result, RTI_OK) << "dynamic setup using invalid size,unexpected passed";
    free(dynamicTable);
}

/**
 * @brief dynamic setup should copy static table
 * 
 */
TEST_F(VlanDynamicSetupTest, DynamicSetupCheck) {
    // setup
    void *dynamicTable = malloc(RTI_VLAN_VLANTABLE_SIZE(2));
    void *staticOldTable = RTIDFX_VlanGetTableAddr();
    size_t staticOldTableSize = RTI_VLAN_VLANTABLE_SIZE(0);
    EXPECT_NE(dynamicTable, nullptr);
    RTI_ERR result = RTI_VlanDynamicSetup(dynamicTable, RTI_VLAN_VLANTABLE_SIZE(2));
    EXPECT_EQ(result, RTI_OK) << "dynamic setup using valid size,unexpected failed";
    // select from dynamic table
    RTI_VLAN_DESC desc;
    result = RTIPriv_VlanSelect(1, &desc);
    EXPECT_EQ(result, RTI_OK);
    EXPECT_EQ(desc.ifx, &mock1_vlan_ifx);
    EXPECT_STREQ(desc.name, "VLAN1");
    EXPECT_EQ(desc.id, 1);
    result = RTIPriv_VlanSelect(2, &desc);
    EXPECT_EQ(result, RTI_OK);
    EXPECT_EQ(desc.ifx, &mock2_vlan_ifx);
    EXPECT_STREQ(desc.name, "VLAN2");
    EXPECT_EQ(desc.id, 2);
    // resume
    result = RTI_VlanDynamicSetup(staticOldTable, RTI_VLAN_VLANTABLE_SIZE(2));
    EXPECT_EQ(result, RTI_OK) << "resume to static table failed";
    free(dynamicTable);
}

/**
 * @brief dynamic register should not cover static records
 * 
 */
TEST_F(VlanDynamicTest, DynamicRegisterCheck) {
    // check free size
    size_t count, countAll;
    RTI_ERR result = RTI_VlanDynamicGetFreeCount(&count);
    EXPECT_EQ(result, RTI_OK);
    EXPECT_EQ(count, 1);
    result = RTI_VlanDynamicGetAllCount(&countAll);
    EXPECT_EQ(result, RTI_OK);
    EXPECT_EQ(countAll, 3);
    // register new record
    RTI_VLAN_DESC desc;
    result = RTI_VlanDynamicRegister(&mock3_vlan_desc);
    EXPECT_EQ(result, RTI_OK) << "dynamic register failed";
    // select to check new records
    result = RTIPriv_VlanSelect(3, &desc);
    EXPECT_EQ(result, RTI_OK);
    EXPECT_EQ(desc.ifx, &mock3_vlan_ifx);
    EXPECT_STREQ(desc.name, "VLAN3");
    EXPECT_EQ(desc.id, 3);
    // select to check old records
    RTI_VLAN_DESC descSelect;
    result = RTIPriv_VlanSelect(1, &descSelect);
    EXPECT_EQ(result, RTI_OK);
    EXPECT_EQ(descSelect.ifx, &mock1_vlan_ifx);
    EXPECT_STREQ(descSelect.name, "VLAN1");
    EXPECT_EQ(descSelect.id, 1);
    result = RTIPriv_VlanSelect(2, &descSelect);
    EXPECT_EQ(result, RTI_OK);
    EXPECT_EQ(descSelect.ifx, &mock2_vlan_ifx);
    EXPECT_STREQ(descSelect.name, "VLAN2");
    EXPECT_EQ(descSelect.id, 2);
    // resume
    result = RTIDFX_VlanTableUnregister(3);
    EXPECT_EQ(result, RTI_OK) << "unregister dynamic record failed";
    result = RTIPriv_VlanSelect(3, &desc);
    EXPECT_NE(result, RTI_OK) << "unregister failed, record did not removed";
}

/**
 * @brief dynamic register should stop when table is full
 * 
 */
TEST_F(VlanDynamicTest, DynamicRegisterStopWhenFull) {
    // register new record
    RTI_ERR result = RTI_VlanDynamicRegister(&mock3_vlan_desc);
    EXPECT_EQ(result, RTI_OK) << "dynamic register failed";
    // register again
    result = RTI_VlanDynamicRegister(&mock4_vlan_desc);
    EXPECT_NE(result, RTI_OK) << "dynamic register should stop when table is full";
    // check free size
    size_t count, countAll;
    result = RTI_VlanDynamicGetFreeCount(&count);
    EXPECT_EQ(result, RTI_OK);
    EXPECT_EQ(count, 0);
    result = RTI_VlanDynamicGetAllCount(&countAll);
    EXPECT_EQ(result, RTI_OK);
    EXPECT_EQ(countAll, 3);
}

#endif
