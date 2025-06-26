VERSION=@VERSION@

CLEANFILES=libjwt.spec

all:

rpmcheck:
	rpmlint -i dist/libjwt.spec

rpm-prep:
	cd ${abs_top_srcdir} && mkdir -p ./rpmbuild
	cd ${abs_top_srcdir}/rpmbuild && mkdir -p {SOURCES,SPECS,RPMS,SRPMS,BUILD}
	cd ${abs_top_srcdir} && git archive --format=tar --prefix=libjwt-${VERSION}/ master | gzip > ./rpmbuild/SOURCES/v${VERSION}.tar.gz

rpm: rpm-prep
	rpmbuild -ba  --define "_topdir ${abs_top_srcdir}/rpmbuild" libjwt.spec
