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

define final_header <<EOF
#ifndef ${UCLASS}_H_
#define ${UCLASS}_H_

typedef struct ${classname} ${classname};

${classname}* ${lclass}_create();
void ${lclass}_destroy(${classname}* ${lclass});

#endif
EOF

define final_source <<EOF
#include "${lclass}.h"

#include <string.h>
#include <stdlib.h>

typedef struct ${classname} {
    // TODO: add fields here
} ${classname};

void ${lclass}_initialize(${classname}* ${lclass})
{
    memset(${lclass}, 0, sizeof(* ${lclass}));
    // TODO: initialize fields here
}

${classname}* ${lclass}_create()
{
    ${classname}* ${lclass} = malloc(sizeof(${classname}));
    ${lclass}_initialize(&${lclass});
    return ${lclass};
}

void ${lclass}_finalize(${classname}* ${lclass})
{
    // TODO: finalize fields here
}

void ${lclass}_destroy(${classname}* ${lclass})
{
    ${lclass}_finalize(${lclass});
    free(${lclass});
}

#endif
EOF

echo $final_header
echo
ehco $final_source