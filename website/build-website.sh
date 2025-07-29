#!/bin/sh

(
    echo 'OATH Toolkit NEWS'
    echo '================='
    echo ''
    cat ../NEWS | sed -n '/(20/,$ p' | sed 's/^* Noteworthy changes in release/.Version/g'
) > NEWS.txt

(
    echo 'Contribute'
    echo '=========='
    echo ''
    cat ../CONTRIBUTING.md | sed -e '1,2d'
) > contrib.txt

(
    echo 'Manual for pam_oath'
    echo '==================='
    echo ''
    cat ../pam_oath/README | sed -e '1,5d'
) > pam_oath.txt

(
    LAST_VERSION=$(grep '^\* Noteworthy.*(20' ../NEWS |head -1|sed -n -e 's/.* \(.*\) (.*/\1/p')
    LAST_DATE=$(grep '^\* Noteworthy.*(20' ../NEWS |head -1|sed -n -e 's/.*(\(.*\)).*/\1/p')
    echo 'OATH Toolkit'
    echo '============'
    echo ''
    echo ".$LAST_DATE: OATH Toolkit $LAST_VERSION Released"
    cat intro-1.txt
    sed -n 2,18p ../README
    cat intro-2.txt
) > index.txt

mkdir html

cp *.css html/

for txt in index.txt NEWS.txt download.txt docs.txt pam_oath.txt contrib.txt CVE-*.txt; do
    html=`echo $txt | sed 's/\.txt$/.html/'`
    asciidoc --backend=xhtml11 --conf-file=config.cfg -a index-only -o tmp $txt
    tr -d '\015' < tmp > tmp2
    xsltproc --html --output html/$html oath-toolkit.xsl tmp2
done

for man in ../oathtool/oathtool.1 ../pskctool/pskctool.1; do
    html=`basename $man`.html
    groff -man -T html $man \
	| grep -v -e '<meta http-equiv' -e 'text-align: center' \
	| sed 's/ align="center"//' > tmp
    xsltproc --html --output html/$html oath-toolkit.xsl tmp
    manhtml=man-`basename $man|sed 's/.1//'`.html
    groff -man -T html $man > html/$manhtml
done

for lib in oath pskc; do
    cp -r ../lib$lib/gtk-doc/html html/lib$lib
    cp ../lib$lib/gtk-doc/lib$lib.pdf ../lib$lib/gtk-doc/html/*.devhelp2 html/lib$lib
    mkdir -p html/lib$lib-api/
    cp ../lib$lib/gtk-doc/html/*.png ../lib$lib/gtk-doc/html/*.css html/lib$lib-api/
    for src in `find ../lib$lib/gtk-doc/html/ -name "*.html" -print`; do
	dst=`basename $src`
	xsltproc --html --output tmp oath-toolkit.xsl $src
	sed \
	    -e 's#»<a href="#»<a href="../#g' \
	    -e 's#"asciidoc.css"#"../asciidoc.css"#' \
	    -e 's#"style.css"#"../style.css"#' \
	    -e 's#"layout.css"#"../layout.css"#' < tmp > html/lib$lib-api/$dst
    done
done

rm -f tmp tmp2
