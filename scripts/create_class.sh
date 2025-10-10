#!/bin/bash

#
# process arguments
#

usage() { echo "Usage: $0 [-x] [-d DIR] -c NAME" 1>&2; exit 1; }

extensible=0
dir='.'
classname='?'

while getopts ":xd:c:" o; do
    case "${o}" in
        c) classname=${OPTARG} ;;
        x) extensible=1 ;;
        d) dir=${OPTARG} ;;
        *) usage ;;
    esac
done
shift $((OPTIND-1))

if [ "$classname" == "?" ]; then
    usage
fi

UCLASS=$(echo $classname | tr 'a-z' 'A-Z')
lclass=$(echo $classname | tr 'A-Z' 'a-z')

#
# templates
#

header=$(cat <<EOF
#ifndef ${UCLASS}_H_
#define ${UCLASS}_H_

typedef struct ${classname} ${classname};

${classname}* ${lclass}_create();
void ${lclass}_destroy(${classname}* ${lclass});

#endif
EOF
)

final_source=$(cat <<EOF
#include "${lclass}.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct ${classname} {
    // TODO: add fields here
} ${classname};

static void ${lclass}_initialize(${classname}* ${lclass})
{
    memset(${lclass}, 0, sizeof(*${lclass}));
    // TODO: initialize fields here
}

static void ${lclass}_finalize(${classname}* ${lclass})
{
    // TODO: finalize fields here
}

${classname}* ${lclass}_create()
{
    ${classname}* ${lclass} = malloc(sizeof(${classname}));
    if (${lclass} == NULL) {
        fprintf(stderr, "Memory exausted.\n");
        exit(EXIT_FAILURE);
    }
    ${lclass}_initialize(${lclass});
    return ${lclass};
}

void ${lclass}_destroy(${classname}* ${lclass})
{
    ${lclass}_finalize(${lclass});
    free(${lclass});
}

EOF
)

extensible_priv_header=$(cat <<EOF
#ifndef ${UCLASS}_PRIV_H_
#define ${UCLASS}_PRIV_H_

typedef struct ${classname}VTable {
    // TODO: add methods here
} ${classname}VTable;

typedef struct ${classname} {
    ${classname}VTable vt;
    // TODO: add fields here
} ${classname};

void ${lclass}_initialize(${classname}* ${lclass});
void ${lclass}_finalize(${classname}* ${lclass});

#endif
EOF
)

extensible_source=$(cat <<EOF
#include "${lclass}.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "${lclass}_priv.h"

void ${lclass}_initialize(${classname}* ${lclass})
{
    memset(${lclass}, 0, sizeof(*${lclass}));
    // TODO: initialize fields here
}

void ${lclass}_finalize(${classname}* ${lclass})
{
    // TODO: finalize fields here
}

${classname}* ${lclass}_create()
{
    ${classname}* ${lclass} = malloc(sizeof(${classname}));
    if (${lclass} == NULL) {
        fprintf(stderr, "Memory exausted.\n");
        exit(EXIT_FAILURE);
    }
    ${lclass}_initialize(${lclass});
    return ${lclass};
}

void ${lclass}_destroy(${classname}* ${lclass})
{
    ${lclass}_finalize(${lclass});
    free(${lclass});
}

EOF
)

#
# generate files
#

echo "${header}" > "${dir}/${lclass}.h"
if [ "$extensible" -eq "0" ]; then
    echo "${final_source}" > "${dir}/${lclass}.c"
else
    echo "${extensible_priv_header}" > "${dir}/${lclass}_priv.h"
    echo "${extensible_source}" > "${dir}/${lclass}.c"
fi