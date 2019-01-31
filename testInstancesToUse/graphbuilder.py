#!/usr/bin/python

import sys

if len(sys.argv) < 2:
    if len(sys.argv) < 1:
        sys.argv[0] = 'graphbuilder.py'
    print('Usage: ./{:s} <file>'.format(sys.argv[0]))
    raise SystemExit

with open(sys.argv[1]) as file:

    # Write filename to output
    print("# These are the results from " + sys.argv[1])
    print("# This is only the parameter vo")

    # Remove first two lines
    file.readline()
    file.readline()

    # Print header line
    print('# X Y')

    for line in file.readlines():

        clms = line.split(' ')
        solved = clms[2]
        if solved != 'S':
            # This instance was not solved
            continue

        time = clms[4]
        vo = clms[8]
        kk = clms[9]

        if not vo.isdigit() or not kk.isdigit():
            print('ERROR in line: ' + line)
            continue

        print('{:d}        {:s}'.format(int(vo), time))
