# $id$

BEGIN {
    printf "const char *ldscript_default = ";
}

{
    printf "\n\"";
    gsub("\"", "\\\"");
    printf "%s\"", $0;
}

END {
    print ";";
}
