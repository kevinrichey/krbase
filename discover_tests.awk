# Split lines by spaces, tabs, and parens
BEGIN { FS = "[ \t()]+" }

/TEST_CASE/ { 
	# Write function prototypes to testcases.h.
	print "void TestCase_" $2 "(TestCounter*);" > "testcases.h";
	# Write function pointer identifiers to testcases.inc.
	print "{ TestCase_" $2 ", \"" $2 "\"}," > "testcases.inc" 
}

