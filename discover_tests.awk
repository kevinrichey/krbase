BEGIN { FS = "[ \t()]+" }
/TEST_CASE/ { print "void TestCase_" $2 "(TestCounter*);" > "testcases.h"; print "TestCase_" $2 "," > "testcases.inc" }

