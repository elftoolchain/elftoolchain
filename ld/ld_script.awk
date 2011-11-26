# $Id$

BEGIN {
    printf "const char *ldscript_default = ";
}

{
    printf "\"";
    gsub("\"", "\\\"");
    printf "%s\\n\"\n", $0;
}

END {
    print ";";
}
