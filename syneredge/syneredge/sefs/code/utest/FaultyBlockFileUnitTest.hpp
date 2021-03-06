
#include <cppunit/extensions/HelperMacros.h>
#include "FaultyBlockFile.hpp"

class FaultyBlockFileUnitTest : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(FaultyBlockFileUnitTest) ;
	CPPUNIT_TEST(testConstructor) ;
	CPPUNIT_TEST(testCreateFile) ;
	CPPUNIT_TEST(testOpenFile) ;
	CPPUNIT_TEST(testReadWriteBlocks) ;
	CPPUNIT_TEST(testZeroAndFreeBlocks) ;
	CPPUNIT_TEST(testFreeBlockListMethods) ;
	CPPUNIT_TEST_SUITE_END() ;
public:
	void setUp() ;
	void tearDown() ;
	void testConstructor() ;
	void testCreateFile() ;
	void testOpenFile() ;
	void testReadWriteBlocks() ;
	void testZeroAndFreeBlocks() ;
	void testFreeBlockListMethods() ;

private:
	SynerEdge::FaultyBlockFile *bf ;
} ;
