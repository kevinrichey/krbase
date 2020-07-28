# Trim space, /, and * from left of s
# Don't trim tabs, they're used for Markdown formatting.
function trim_comment(s) {
	gsub(/^[ \/\*]+/, "", s);
	return s;
}

BEGIN { doc = 0; }

/\/\/@library/ { 
	$1 = ""
	print "----------------------------------------\n"
	print "#" $0 "\n"
}

/\/\/@module/ { 
	$1 = ""
	print "----------------------------------------\n"
	print "##" $0 "\n"
}

# Begin doc comment with /*@
/\/\*@/ { doc = 1; }

/@func/ {
	$1 = "";
	if ($0) print "###" $0 "\n";
	print "\t" prev "\n";
	next;
}

/@doc/ {
	$1 = "";
	if ($0) print "###" $0 "\n";
	next;
}

/@returns/ {
	$1 = "";
	print "\nReturns\n: " $0 "\n";
	next;
}

# End doc comment with */
/\*\//  { doc = 0; print ""; next; }

{
	if (doc) {
		sub(/^[ ]+/, ""); 
		print $0; 
	}
	else prev = $0;
}

