#!/bin/bash
#
# ZFS/ZPOOL configuration test script.

basedir="$(dirname $0)"

SCRIPT_COMMON=common.sh
if [ -f "${basedir}/${SCRIPT_COMMON}" ]; then
. "${basedir}/${SCRIPT_COMMON}"
else
echo "Missing helper script ${SCRIPT_COMMON}" && exit 1
fi

PROG=zpios-sanity.sh
HEADER=

usage() {
cat << EOF
USAGE:
$0 [hv]

DESCRIPTION:
        ZPIOS sanity tests

OPTIONS:
        -h      Show this message
        -v      Verbose
        -x      Destructive hd/sd/md/dm/ram tests
	-f      Don't prompt due to -x

EOF
}

while getopts 'hvxf' OPTION; do
	case $OPTION in
	h)
		usage
		exit 1
		;;
	v)
		VERBOSE=1
		;;
	x)
		DANGEROUS=1
		;;
	f)
		FORCE=1
		;;
	?)
		usage
		exit
		;;
	esac
done

if [ $(id -u) != 0 ]; then
	die "Must run as root"
fi

zpios_test() {
	CONFIG=$1
	TEST=$2
	LOG=`mktemp`

	${ZPIOS_SH} -f -c ${CONFIG} -t ${TEST} &>${LOG}
	if [ $? -ne 0 ]; then
		if [ ${VERBOSE} ]; then
			printf "FAIL:     %-13s\n" ${CONFIG}
			cat ${LOG}
		else
			if [ ! ${HEADER} ]; then
				head -2 ${LOG}
				HEADER=1
			fi

			printf "FAIL:     %-13s" ${CONFIG}
			tail -1 ${LOG}
		fi
	else
		if [ ${VERBOSE} ]; then
			cat ${LOG}
		else
			if [ ! ${HEADER} ]; then
				head -2 ${LOG}
				HEADER=1
			fi

			tail -1 ${LOG}
		fi
	fi

	rm -f ${LOG}
}

if [ ${DANGEROUS} ] && [ ! ${FORCE} ]; then
	cat << EOF
The -x option was passed which will result in UNRECOVERABLE DATA LOSS
on on the following block devices:

  /dev/sd[abcd]
  /dev/hda
  /dev/ram0
  /dev/md0
  /dev/dm-0

To continue please confirm by entering YES:
EOF
	read CONFIRM
	if [ ${CONFIRM} != "YES" ] && [ ${CONFIRM} != "yes" ]; then
		exit 0;
	fi
fi

#
# These configurations are all safe and pose no risk to any data on
# the system which runs them.  They will confine all their IO to a
# file in /tmp or a loopback device configured to use a file in /tmp.
#
SAFE_CONFIGS=(						\
	file-raid0 file-raid10 file-raidz file-raidz2	\
	lo-raid0 lo-raid10 lo-raidz lo-raidz2		\
)

#
# These configurations are down right dangerous.  They will attempt
# to use various real block devices on your system which may contain
# data you car about.  You are STRONGLY advised not to run this unless
# you are certain there is no data on the system you care about.
#
DANGEROUS_CONFIGS=(					\
	hda-raid0					\
	sda-raid0					\
	ram0-raid0					\
	md0-raid10 md0-raid5				\
	dm0-raid0					\
)

for CONFIG in ${SAFE_CONFIGS[*]}; do
	zpios_test $CONFIG tiny
done

if [ ${DANGEROUS} ]; then
	for CONFIG in ${DANGEROUS_CONFIGS[*]}; do
		zpios_test $CONFIG tiny
	done
fi

exit 0